// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "minersetup.h"

#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

#include <QCheckBox>
#include <qt/forms/ui_miner.h>

#include <qt/platformstyle.h>

#include "MinerFactory.h"

#include "libstratum/ZcashStratum.h"
#include "libstratum/StratumClient.h"

MinerSetup::MinerSetup(const PlatformStyle *platformStyle, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MinerSetup)
	, speed(15)
{
#ifdef Q_OS_MAC
	double iconscale = 1.0;
#else
	double iconscale = logicalDpiX() / 96.0;
#endif
	ui->setupUi(this);

	// fill CPU{id} checkbox.
	int number_of_cpus = std::thread::hardware_concurrency();

	for (int i = 0; i < number_of_cpus; i++)
	{
		auto checkbox = new QCheckBox(ui->cpu_select_group);

		checkbox->setText(QString("CPU%1").arg(i));

		ui->cpu_select_group_layout->addWidget(checkbox, i/2 , i%2, 1, 1);
	}

}

MinerSetup::~MinerSetup()
{
	if (miner_io_thread.joinable())
		miner_io_thread.join();
    delete ui;
}

void MinerSetup::start_mining(const std::string& host, const std::string& port,
	const std::string& user, const std::string& password, std::vector<std::unique_ptr<ISolver>> i_solvers)
{
	std::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service);

	ZcashMiner miner(&speed, std::move(i_solvers));
	ZcashStratumClient sc{
		io_service, &miner, host, port, user, password, 0, 0
	};

	miner.onSolutionFound([&](const EquihashSolution& solution, const std::string& jobid) {
		return sc.submit(&solution, jobid);
	});

//	signal(SIGINT, stratum_sigint_handler);

	int c = 0;
	while (sc.isRunning()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (++c % 1000 == 0)
		{
			double allshares = speed.GetShareSpeed() * 60;
			double accepted = speed.GetShareOKSpeed() * 60;
			std::cout << "Speed [" << INTERVAL_SECONDS << " sec]: " <<
				speed.GetHashSpeed() << " I/s, " <<
				speed.GetSolutionSpeed() << " Sols/s" <<
				//accepted << " AS/min, " <<
				//(allshares - accepted) << " RS/min"
				CL_N << std::endl;
		}
	}
}

void MinerSetup::on_startstopbutton_clicked()
{
	int num_threads = 0;

	std::string user;
	std::string location = "47.97.167.150:3333";
	// start miner.
	if (user.length() == 0)
	{
		std::cerr << "Invalid address. Use -u to specify your address." << std::endl;
		return;
	}

	size_t delim = location.find(':');
	std::string host = delim != std::string::npos ? location.substr(0, delim) : location;
	std::string port = delim != std::string::npos ? location.substr(delim + 1) : "2142";

	this->miner_io_thread = std::thread([this, num_threads, user, host, port](){

		std::vector<int> cuda_enabled;
		std::vector<int> cuda_blocks;
		std::vector<int> cuda_tpb;

		std::vector<int> opencl_enabled;
		std::vector<int> opencl_threads;

		auto _MinerFactory = new MinerFactory();

		start_mining(host, port, user, "x",
			_MinerFactory->GenerateSolvers(num_threads, cuda_enabled.size(), cuda_enabled.data(), cuda_blocks.data(),
			cuda_tpb.data(), opencl_enabled.size(), 0, opencl_enabled.data()));
	});
}
