// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <amount.h>

#include <QWidget>
#include <QTimer>
#include <memory>
#include <boost/asio.hpp>

#include "speed.hpp"

class ISolver;
class PlatformStyle;

namespace Ui {
    class MinerSetup;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class MinerSetup : public QWidget
{
    Q_OBJECT

public:
    explicit MinerSetup(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~MinerSetup();

private Q_SLOTS:
	void on_startbutton_clicked();
	void on_stopbutton_clicked();

	void timer_interrupt();

private:
	void start_mining(std::string host, std::string port, std::string user, std::string password, std::vector<std::unique_ptr<ISolver>>);

private:
	Ui::MinerSetup* ui;

	boost::asio::io_service miner_io_service;
	std::thread miner_io_thread;
	Speed speed;

	QTimer ui_update_timer;
};
