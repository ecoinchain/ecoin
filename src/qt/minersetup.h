// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <memory>
#include <vector>

#include <QPointer>
#include <QWidget>
#include <QTimer>

#include <QLabel>
#include <QCheckBox>
#include <boost/asio.hpp>

#include "amount.h"

#include "speed.hpp"

class ISolver;
class PlatformStyle;
class WalletModel;

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
    explicit MinerSetup(const PlatformStyle *platformStyle, WalletModel*, QWidget *parent = 0);
    ~MinerSetup();

public Q_SLOTS:
	void error_report(QString error_message);

private Q_SLOTS:
	void on_target_change(QString newtarget);
	void on_startbutton_clicked();
	void on_stopbutton_clicked();

	void timer_interrupt();

	bool eventFilter(QObject * watched, QEvent * event) override;

private:
	void start_mining(std::string host, std::string port, std::string user, std::string password, std::vector<std::unique_ptr<ISolver>>);

private:
	Ui::MinerSetup* ui;
	WalletModel *model;

	boost::asio::io_service miner_io_service;
	std::thread miner_io_thread;
	Speed speed;

	QTimer ui_update_timer;

	QPointer<QLabel> message_widget;

	std::vector<QCheckBox*> checkboxies;
};
