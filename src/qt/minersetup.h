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
#include <QNetworkAccessManager>
#include <QNetworkReply>
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

Q_SIGNALS:
	void MinerStatusChanged(bool);

public Q_SLOTS:
	void error_report(QString error_string, bool can_auto_dismiss);
	void dismiss_error();
	void openUrl(QString);

private Q_SLOTS:
	void dismiss_error_invoked();
	void on_startbutton_clicked();
	void on_stopbutton_clicked();

	void on_refresh_clicked();

	void on_location_editTextChanged(QString);
	void on_username_editTextChanged(QString);

	void timer_interrupt();

	void second_timer_interrupt();

	void process_network_rpc_finished();

	void process_network_error(QNetworkReply::NetworkError);

	void set_pending_balance(QString);

private:
	bool eventFilter(QObject * watched, QEvent * event) override;
	void start_mining(std::vector<std::string> locations, std::string user, std::string password, std::vector<std::unique_ptr<ISolver>>);

private:
	Ui::MinerSetup* ui;
	WalletModel *model;

	std::shared_ptr<boost::asio::io_service> miner_io_service;
	std::thread miner_io_thread;
	Speed speed;

	QTimer ui_update_timer;

	QTimer balance_update_timer;

	QPointer<QLabel> message_widget;

	std::vector<QCheckBox*> checkboxies;

	QNetworkAccessManager m_networkmanager;
};
