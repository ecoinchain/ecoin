// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <thread>
#include <QCheckBox>
#include <qt/forms/ui_miner.h>

#include <qt/platformstyle.h>

#include "minersetup.h"

MinerSetup::MinerSetup(const PlatformStyle *platformStyle, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MinerSetup)
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
    delete ui;
}

