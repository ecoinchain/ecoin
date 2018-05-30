// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
}

MinerSetup::~MinerSetup()
{
    delete ui;
}

