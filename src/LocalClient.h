#pragma once
// Copyright (c) 2016 Genoil <jw@meneer.net>
// Copyright (c) 2016 Jack Grigg <jack@z.cash>
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libstratum/ZcashStratum.h"

#include <memory>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <thread>
#include <atomic>

#ifdef ENABLE_WALLET
#include <wallet/wallet.h>
#endif

#include <miner.h>

#include "json/json_spirit_value.h"

using namespace std;
using namespace json_spirit;

template <typename Miner, typename Solution>
class LocalClient
{
public:
#ifdef ENABLE_WALLET
	LocalClient(boost::asio::io_service& io_s, Miner * m, CWallet* w);
#else
	LocalClient(boost::asio::io_service& io_s, Miner * m);
#endif
	~LocalClient() { }

    bool isRunning() { return m_running; }
    bool submit(const Solution* solution, const std::string& jobid);

	template<typename Func>
	void set_report_error(Func f)
	{
		report_error.connect(f);
	}

	template<typename Func>
	void set_dismiss_error(Func f)
	{
		dismiss_error.connect(f);
	}

private:
    void startWorking();
	void workLoop(boost::asio::coroutine, boost::system::error_code ec);

    bool m_running = true;

    int m_worktimeout = 60;
    Miner * p_miner;

	struct workJob : ZcashJob {
				    // network and disk
		std::vector<CTransactionRef> vtx;
		std::vector<CAmount> vTxFees;
		std::vector<int64_t> vTxSigOpsCost;
		std::vector<unsigned char> vchCoinbaseCommitment;

#ifdef ENABLE_WALLET
		CReserveKey key;

		workJob(CWallet*c)
		: key(c)
		{}
#endif
	};

	int job_cur_index = 0;
	std::array<std::shared_ptr<workJob>, 256> workingjobs;

	std::unique_ptr<boost::asio::io_service::work> m_io_work;

    boost::asio::io_service& m_io_service;

    string m_target;

	typedef boost::signals2::signal<void (std::string, bool can_auto_dismiss)> report_error_t;

	report_error_t report_error;

	typedef boost::signals2::signal<void()> dismiss_error_t;

	dismiss_error_t dismiss_error;

	boost::asio::steady_timer m_worker_timer;
#ifdef ENABLE_WALLET
	CWallet *pwallet;
#endif

	uint256 nonce1;
	size_t nonce1Size;
	arith_uint256 nonce2Space;
	arith_uint256 nonce2Inc;
};

// ZcashStratumClient
typedef LocalClient<ZcashMiner, EquihashSolution> ZcashLocalMiner;
