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

#include "wallet/wallet.h"
#include "key_io.h"

#include "libstratum/ZcashStratum.h"
#include "libstratum/StratumClient.h"
#include "validation.h"

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

	ui->stopbutton->hide();

	// fill CPU{id} checkbox.
	int number_of_cpus = std::thread::hardware_concurrency();

	for (int i = 0; i < number_of_cpus; i++)
	{
		auto checkbox = new QCheckBox(ui->cpu_select_group);

		checkbox->setText(QString("CPU%1").arg(i));

		ui->cpu_select_group_layout->addWidget(checkbox, i/2 , i%2, 1, 1);
	}

	CWallet *pwallet = ::vpwallets.size() > 0 ? ::vpwallets[0] : nullptr;

    LOCK2(cs_main, pwallet->cs_wallet);

    // Generate a new key that is added to wallet
    CPubKey newKey;
    if (!pwallet->GetKeyFromPool(newKey)) {
        // TODO. return;
    }
    pwallet->LearnRelatedScripts(newKey, OUTPUT_TYPE_DEFAULT);
    CTxDestination dest = GetDestinationForKey(newKey, OUTPUT_TYPE_DEFAULT);

    pwallet->SetAddressBook(dest, "mining_receive_address", "pool_mining");

    ui->username->setCurrentText(QString::fromStdString(EncodeDestination(dest)));

	ui_update_timer.setInterval(std::chrono::milliseconds(500));

	connect(&ui_update_timer, SIGNAL(timeout()), this, SLOT(timer_interrupt()));
	ui_update_timer.start();
}

MinerSetup::~MinerSetup()
{
	if (miner_io_thread.joinable())
		miner_io_thread.join();
    delete ui;
}

void MinerSetup::start_mining(std::string host, std::string port,
	std::string user, std::string password, std::vector<std::unique_ptr<ISolver>> i_solvers)
{
	miner_io_service.reset();

	ZcashMiner miner(&speed, std::move(i_solvers));
	ZcashStratumClient sc{
		miner_io_service, &miner, host, port, user, password, 0, 0
	};

	miner.onSolutionFound([&](const EquihashSolution& solution, const std::string& jobid) {
		return sc.submit(&solution, jobid);
	});

	miner_io_service.run();

	miner.stop();
}

void MinerSetup::timer_interrupt()
{
	double allshares = speed.GetShareSpeed() * 60;

	ui->hashrate->setText(QString("%1/s").arg(speed.GetSolutionSpeed()));
	ui->acceptedshare->setText(QString("%1/min").arg(allshares));
}

void MinerSetup::on_startbutton_clicked()
{
	int num_threads = 8;

	std::string user = ui->username->currentText().toStdString();
	std::string location = "47.97.167.150:3333";
	location = "192.168.0.110:3333";
	// start miner.
	if (user.length() == 0)
	{
		std::cerr << "Invalid address. Use -u to specify your address." << std::endl;
		return;
	}

	size_t delim = location.find(':');
	std::string host = delim != std::string::npos ? location.substr(0, delim) : location;
	std::string port = delim != std::string::npos ? location.substr(delim + 1) : "2142";

	if (this->miner_io_thread.joinable())
		this->miner_io_thread.join();

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

	ui->startbutton->hide();
	ui->stopbutton->show();
}

void MinerSetup::on_stopbutton_clicked()
{
	miner_io_service.stop();

	if (this->miner_io_thread.joinable())
		this->miner_io_thread.join();

	ui->stopbutton->hide();
	ui->startbutton->show();
}
