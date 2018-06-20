// Copyright (c) 2016 Genoil <jw@meneer.net>
// Copyright (c) 2016 Jack Grigg <jack@z.cash>
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "version.h"
#include "streams.h"
#include "ui_interface.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "validation.h"

#include "utilstrencodings.h"
#include "util.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include <utilmoneystr.h>

#include "LocalClient.h"

using namespace json_spirit;

#ifdef ENABLE_WALLET
template <typename Miner, typename Solution>
LocalClient<Miner, Solution>::LocalClient(boost::asio::io_service& io_s, Miner * m, CWallet* w)
    : m_io_service(io_s)
	, m_worker_timer(m_io_service)
	, pwallet(w)
	, p_miner(m)
{
    m_worktimeout = 0;
	startWorking();
}
#else
template <typename Miner, typename Solution>
LocalClient<Miner, Solution>::LocalClient(boost::asio::io_service& io_s, Miner * m)
    : m_io_service(io_s)
	, m_worker_timer(m_io_service)
	, p_miner(m)
{
    m_worktimeout = 0;
	startWorking();
}
#endif

template <typename Miner, typename Solution>
void LocalClient<Miner, Solution>::startWorking()
{
	p_miner->setServerNonce("00000000");

	this->workLoop(boost::asio::coroutine(), boost::system::error_code());

	boost::signals2::connection c = uiInterface.NotifyBlockTip.connect(
		[this](bool ibd, const CBlockIndex * pindex) mutable
		{
			boost::system::error_code ec;
			m_worker_timer.cancel(ec);
		}
	);

	m_io_work.reset(new boost::asio::io_service::work(m_io_service));
}

#include "miner.h"

template <typename Miner, typename Solution>
void LocalClient<Miner, Solution>::workLoop(boost::asio::coroutine coro, boost::system::error_code ec)
{
	BOOST_ASIO_CORO_REENTER(coro)
	{
		while(true)
		{
			{
				#ifdef ENABLE_WALLET
					// Each thread has its own key
					std::shared_ptr<workJob> workOrder = std::make_shared<workJob>(pwallet);
				#else
					std::shared_ptr<workJob> workOrder = std::make_shared<workJob>();
				#endif

				#ifdef ENABLE_WALLET
					boost::optional<CScript> scriptPubKey = GetMinerScriptPubKey(workOrder->key);
				#else
					boost::optional<CScript> scriptPubKey = GetMinerScriptPubKey();
				#endif

				std::unique_ptr<CBlockTemplate> pblocktemplate(BlockAssembler(Params()).CreateNewBlock(*scriptPubKey));

				if (!pblocktemplate.get())
				{
					if (gArgs.GetArg("-mineraddress", "").empty())
					{
						LogPrintf("Error in Miner: Keypool ran out, please call keypoolrefill before restarting the mining thread\n");
					}
					else
					{
						// Should never reach here, because -mineraddress validity is checked in init.cpp
						LogPrintf("Error in Miner: Invalid -mineraddress\n");
					}
					return;
				}
				CBlock *pblock = &pblocktemplate->block;
				pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);

				workOrder->clean = true;

				workOrder->time = std::to_string(pblock->GetBlockHeader().nTime);
				workOrder->header = pblock->GetBlockHeader();
				workOrder->vtx = pblock->vtx;

				workOrder->vTxFees = pblocktemplate->vTxFees;
				workOrder->vTxSigOpsCost = pblocktemplate->vTxSigOpsCost;
				workOrder->vchCoinbaseCommitment = pblocktemplate->vchCoinbaseCommitment;

				workOrder->header.nNonce = nonce1;
				workOrder->nonce1Size = nonce1Size;
				workOrder->nonce2Space = nonce2Space;
				workOrder->nonce2Inc = nonce2Inc;
				workOrder->serverTarget.SetCompact(pblock->nBits);

				workingjobs[job_cur_index] = workOrder;
				job_cur_index = job_cur_index % 256;
				p_miner->setJob(std::static_pointer_cast<ZcashJob>(workOrder));
			}

			m_worker_timer.expires_from_now(std::chrono::seconds(6));
			BOOST_ASIO_CORO_YIELD m_worker_timer.async_wait(std::bind(&LocalClient::workLoop, this, coro, std::placeholders::_1));
			if (ec == boost::asio::error::operation_aborted)
			{
				for (auto & p : workingjobs) p.reset();
			}
		}
	}
}

#ifdef ENABLE_WALLET
static bool ProcessBlockFound(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey)
#else
static bool ProcessBlockFound(CBlock* pblock)
#endif // ENABLE_WALLET
{
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0]->vout[0].nValue));

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("Miner: generated block is stale");
    }

#ifdef ENABLE_WALLET
    if (gArgs.GetArg("-mineraddress", "").empty()) {
        // Remove key from key pool
        reservekey.KeepKey();
    }

    // Track how many getdata requests this block gets
    {
        LOCK(wallet.cs_wallet);
        wallet.mapRequestCount[pblock->GetHash()] = 0;
    }
#endif

    // Process this block the same as if we had received it from another node
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
    if (!ProcessNewBlock(Params(), shared_pblock, true, NULL))
        return error("Miner: ProcessNewBlock, block not accepted");

    return true;
}


template <typename Miner, typename Solution>
bool LocalClient<Miner, Solution>::submit(const Solution* solution, const std::string& jobid)
{
	// build pblock from solution.
	int id = std::stoi(jobid);

	std::shared_ptr<workJob> job_of_sol = workingjobs[id];

	CBlockHeader* header = &(job_of_sol->header);

	header->nSolution = solution->solution;
	header->nNonce = solution->nonce;

	CBlock tmpblock(job_of_sol->header);

	tmpblock.vtx = job_of_sol->vtx;

#ifdef ENABLE_WALLET
	auto ProcessBlockFoundRet = ProcessBlockFound(&tmpblock, *pwallet, job_of_sol->key);
#else
	auto ProcessBlockFoundRet = ProcessBlockFound(&tmpblock);
#endif
	return ProcessBlockFoundRet;
}

// create StratumClient class
template class LocalClient<ZcashMiner, EquihashSolution>;
