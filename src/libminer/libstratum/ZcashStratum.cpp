// Copyright (c) 2016 Jack Grigg <jack@z.cash>
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "version.h"
#include "ZcashStratum.h"

#include "utilstrencodings.h"
#include "streams.h"

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <boost/thread/exceptions.hpp>
#include <boost/circular_buffer.hpp>
#include <sodium/crypto_generichash_blake2b.h>

#include "speed.hpp"

#ifdef WIN32
#include <Windows.h>
#endif

#include <boost/static_assert.hpp>
#include "util.h"
#include "crypto/equihash.h"

typedef uint32_t eh_index;


#define BOOST_LOG_CUSTOM(sev, pos) BOOST_LOG_TRIVIAL(sev) << "miner#" << pos << " | "

void static ZcashMinerThread(ZcashMiner* miner, int size, int pos, ISolver *solver, Speed * speed)
{
	LogPrintf( "miner#%d, Starting thread %d(%s)%s", pos, pos, solver->getname(), solver->getdevinfo());

	if (solver->GetType() == SolverType::CPU) {
#ifdef WIN32
	HANDLE hThread = minerThreads[i].native_handle();
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
#else
	// todo: linux set low priority
	pthread_setschedprio(pthread_self(), 19);
#endif
	}

    std::shared_ptr<std::mutex> m_zmt(new std::mutex);
    CBlockHeader header;
    arith_uint256 space;
    size_t offset;
    arith_uint256 inc;
    arith_uint256 target;
	std::string jobId;
	std::string nTime;
    std::atomic_bool workReady {false};
    std::atomic_bool cancelSolver {false};
	std::atomic_bool pauseMining {false};

    miner->NewJob.connect(NewJob_t::slot_type(
		[&m_zmt, &header, &space, &offset, &inc, &target, &workReady, &cancelSolver, pos, &pauseMining, &jobId, &nTime]
        (std::shared_ptr<ZcashJob> job) mutable {
            std::lock_guard<std::mutex> lock{*m_zmt};
            if (job) {
				LogPrintf( "miner#%d, Loading new job #%s", pos, job->jobId());
				jobId = job->jobId();
				nTime = job->time;
                header = job->header;
                space = job->nonce2Space;
                offset = job->nonce1Size * 4; // Hex length to bit length
                inc = job->nonce2Inc;
                target = job->serverTarget;
				pauseMining.store(false);
                workReady.store(true);
                /*if (job->clean) {
                    cancelSolver.store(true);
                }*/
            } else {
                workReady.store(false);
                cancelSolver.store(true);
				pauseMining.store(true);
            }
        }
    ).track_foreign(m_zmt)); // So the signal disconnects when the mining thread exits

    try {

		solver->start();

        while (true) {
            // Wait for work
            bool expected;
            do {
                expected = true;
				if (!miner->minerThreadActive[pos])
					throw boost::thread_interrupted();
                //boost::this_thread::interruption_point();
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } while (!workReady.compare_exchange_weak(expected, false));
            // TODO change atomically with workReady
            cancelSolver.store(false);

            // Calculate nonce limits
            arith_uint256 nonce;
            arith_uint256 nonceEnd;
			CBlockHeader actualHeader;
			std::string actualJobId;
			std::string actualTime;
			arith_uint256 actualTarget;
			size_t actualNonce1size;
            {
				std::lock_guard<std::mutex> lock{*m_zmt.get()};
				arith_uint256 baseNonce = UintToArith256(header.nNonce);
				arith_uint256 add(pos);
				nonce = baseNonce | (add << (8 * 19));
				nonceEnd = baseNonce | ((add + 1) << (8 * 19));
				//nonce = baseNonce + ((space/size)*pos << offset);
				//nonceEnd = baseNonce + ((space/size)*(pos+1) << offset);

				// save job id and time
				actualHeader = header;
				actualJobId = jobId;
				actualTime = nTime;
				actualNonce1size = offset / 4;
				actualTarget = target;
			}

			// I = the block header minus nonce and solution.
			CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
			{
				//std::lock_guard<std::mutex> lock{ *m_zmt.get() };
				CEquihashInput I{ actualHeader };
				ss << I;
			}

			char *tequihash_header = (char *)&ss[0];
			unsigned int tequihash_header_len = ss.size();

			// Start working
			while (true) {
				//LogPrintf( "miner#%d, Running Equihash solver with nNonce = %s", pos, nonce.ToString());

				auto bNonce = ArithToUint256(nonce);

				auto solutionFound =
					[&actualHeader, &bNonce, speed, &actualTarget, &miner, pos, actualJobId, &actualTime, &actualNonce1size]
						(const std::vector<uint32_t>& index_vector, size_t cbitlen, const unsigned char* compressed_sol)
				{
					actualHeader.nNonce = bNonce;
					if (compressed_sol)
					{
						actualHeader.nSolution = std::vector<unsigned char>(1344);
						for (size_t i = 0; i < cbitlen; ++i)
							actualHeader.nSolution[i] = compressed_sol[i];
					}
					else
						actualHeader.nSolution = GetMinimalFromIndices(index_vector, cbitlen);

					speed->AddSolution();

					//LogPrintf( "miner#%d, RChecking solution against target %s", pos, actualTarget.ToString());

					uint256 headerhash = actualHeader.GetHash();

					if (UintToArith256(headerhash) > actualTarget) {
						//LogPrintf( "miner#%d, Too large: %s, target %s", pos, headerhash.ToString(), actualTarget.ToString());
						return false;
					}

					// Found a solution
					LogPrintf( "miner#%d, Found solution with header hash: %s", pos, headerhash.ToString());
					EquihashSolution solution{ bNonce, actualHeader.nSolution, actualTime, actualNonce1size };

					CDataStream ss2(SER_NETWORK, PROTOCOL_VERSION);
					ss2 << actualHeader.nSolution;
					std::string strHex2 = HexStr(ss2.begin(), ss2.end());
					miner->submitSolution(solution, actualJobId);
// 					std::cerr << "submitSolution: nVersion = " << actualHeader.nVersion
// 						<< "\nhashPrevBlock = " << actualHeader.hashPrevBlock.ToString()
// 						<< "\nhashMerkleRoot = " << actualHeader.hashMerkleRoot.ToString()
// 						<< "\nnTime = " << actualHeader.nTime
// 						<< "\nnBits = " << actualHeader.nBits
// 						<< "\nnonce = " << actualHeader.nNonce.ToString()
// 						<< "\nnSolution = " << HexStr(actualHeader.nSolution) << std::endl;
					speed->AddShare();
					return false;
				};

				std::function<bool()> cancelFun = [&cancelSolver]() {
					return cancelSolver.load();
				};

				std::function<void(void)> hashDone = [speed]() {
					speed->AddHash();
				};

				solver->solve(tequihash_header,
					tequihash_header_len,
					(const char*)bNonce.begin(),
					bNonce.size(),
					cancelFun,
					solutionFound,
					hashDone);

                // Check for stop
				if (!miner->minerThreadActive[pos])
					throw boost::thread_interrupted();
                //boost::this_thread::interruption_point();

				// Update nonce
				nonce += inc;

                if (nonce == nonceEnd) {
                    break;
                }

                // Check for new work
                if (workReady.load()) {
					LogPrintf( "miner#%d, New work received, dropping current work", pos);
                    break;
                }

				if (pauseMining.load())
				{
					LogPrintf( "miner#%d, Mining paused", pos);
					break;
				}
            }
        }
    }
    catch (const boost::thread_interrupted&)
    {
        //throw;
    }
    catch (const std::runtime_error &e)
    {
		LogPrintf("miner#%d, error: %s", pos, e.what());
		exit(0);
    }

	try
	{
		solver->stop();
	}
	catch (const std::runtime_error &e)
	{
		LogPrintf( "miner#%d, error: %s", pos, e.what());
	}

	LogPrintf( "miner#%d, Thread #%d ended(%s)", pos, pos, solver->getname());
}

ZcashJob* ZcashJob::clone() const
{
    ZcashJob* ret = new ZcashJob();
    ret->job = job;
    ret->header = header;
    ret->time = time;
    ret->nonce1Size = nonce1Size;
    ret->nonce2Space = nonce2Space;
    ret->nonce2Inc = nonce2Inc;
    ret->serverTarget = serverTarget;
    ret->clean = clean;
    return ret;
}

void ZcashJob::setTarget(std::string target)
{
	if (target.size() > 0) {
        serverTarget = UintToArith256(uint256S(target));
    } else {
		LogPrintf( "miner | New job but no server target, assuming powLimit");
		serverTarget = UintToArith256(uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
    }
}


std::string ZcashJob::getSubmission(const EquihashSolution* solution)
{
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << solution->nonce;
    ss << solution->solution;
    std::string strHex = HexStr(ss.begin(), ss.end());

    std::stringstream stream;
    stream << "\"" << job;
    stream << "\",\"" << time;
    stream << "\",\"" << strHex.substr(nonce1Size, 64-nonce1Size);
    stream << "\",\"" << strHex.substr(64);
    stream << "\"";
    return stream.str();
}


ZcashMiner::ZcashMiner(Speed* speed, std::vector<std::unique_ptr<ISolver>> && i_solvers)
	: speed(speed)
	, solvers(std::move(i_solvers))
{
	m_isActive = false;
	nThreads = solvers.size();
}


ZcashMiner::~ZcashMiner()
{
    stop();
}


std::string ZcashMiner::userAgent()
{
	return "coinyeeminer/0.17.4";
}

void ZcashMiner::start()
{
    if (!minerThreads.empty()) {
        stop();
    }

	m_isActive = true;

	minerThreads.resize(nThreads);
	minerThreadActive.resize(nThreads);

	// start solvers
	// #1 start cpu threads
	// #2 start CUDA threads
	// #3 start OPENCL threads
	for (int i = 0; i < solvers.size(); ++i) {
		minerThreadActive[i] = true;
		minerThreads[i] = std::thread(boost::bind(&ZcashMinerThread, this, nThreads, i, solvers[i].get(), speed));
	}

	speed->Reset();
}


void ZcashMiner::stop()
{
	m_isActive = false;
	if (!minerThreads.empty())
	{
		for (int i = 0; i < nThreads; i++)
			minerThreadActive[i] = false;
		for (int i = 0; i < nThreads; i++)
		{
			if (minerThreads[i].joinable())
				minerThreads[i].join();
		}
	}
}


void ZcashMiner::setServerNonce(const std::string& n1str)
{
    //auto n1str = params[1].get_str();
	LogPrintf( "miner | Extranonce is %s", n1str);
    std::vector<unsigned char> nonceData(ParseHex(n1str));
    while (nonceData.size() < 32) {
        nonceData.push_back(0);
    }
    CDataStream ss(nonceData, SER_NETWORK, PROTOCOL_VERSION);
    ss >> nonce1;

	//BOOST_LOG_TRIVIAL(info) << "miner | Full nonce " << nonce1.ToString();

    nonce1Size = n1str.size();
    size_t nonce1Bits = nonce1Size * 4; // Hex length to bit length
    size_t nonce2Bits = 256 - nonce1Bits;

    nonce2Space = 1;
    nonce2Space <<= nonce2Bits;
    nonce2Space -= 1;

    nonce2Inc = 1;
    nonce2Inc <<= nonce1Bits;
}


std::shared_ptr<ZcashJob> ZcashMiner::parseJob(const Array& params)
{
    if (params.size() < 2) {
        throw std::logic_error("Invalid job params");
    }

    ZcashJob* ret = new ZcashJob();
    ret->job = params[0].get_str();

	if (params.size() < 6) {
		throw std::logic_error("Invalid job params");
	}

	std::stringstream ssHeader;
	ssHeader << params[1].get_str() // nVersion
				<< params[2].get_str() // hashPrevBlock
				<< params[3].get_str() // hashMerkleRoot
				<< params[4].get_str() // nTime
				<< params[5].get_str() // nBits
				// Empty nonce
				<< "0000000000000000000000000000000000000000000000000000000000000000"
				<< "00"; // Empty solution
	auto strHexHeader = ssHeader.str();
	std::vector<unsigned char> headerData(ParseHex(strHexHeader));
	CDataStream ss(headerData, SER_NETWORK, PROTOCOL_VERSION);
	try {
		ss >> ret->header;
	} catch (const std::ios_base::failure&) {
		throw std::logic_error("ZcashMiner::parseJob(): Invalid block header parameters");
	}

	ret->time = params[4].get_str();
	ret->clean = params[6].get_bool();


    ret->header.nNonce = nonce1;
    ret->nonce1Size = nonce1Size;
    ret->nonce2Space = nonce2Space;
    ret->nonce2Inc = nonce2Inc;

    return std::shared_ptr<ZcashJob>(ret);
}


void ZcashMiner::setJob(std::shared_ptr<ZcashJob> job)
{
    NewJob(job);
}


void ZcashMiner::onSolutionFound(
        const std::function<bool(const EquihashSolution&, const std::string&)> callback)
{
    solutionFoundCallback = callback;
}


void ZcashMiner::submitSolution(const EquihashSolution& solution, const std::string& jobid)
{
    solutionFoundCallback(solution, jobid);
	speed->AddShare();
}


void ZcashMiner::acceptedSolution(bool stale)
{
	speed->AddShareOK();
}


void ZcashMiner::rejectedSolution(bool stale)
{
}


void ZcashMiner::failedSolution()
{
}

std::mutex benchmark_work;
std::vector<uint256*> benchmark_nonces;
std::atomic_int benchmark_solutions;

std::string s_hexdump(std::vector<unsigned char> nSolution)
{
	std::string s;

	s += HexStr(nSolution.begin(), nSolution.end());

	return s;
}
