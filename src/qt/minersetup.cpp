// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "minersetup.h"

#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

#include <QVariant>
#include <QString>
#include <QCheckBox>
#include <QFontMetrics>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QJsonDocument>

#include "widgets/flowlayout.h"
#include "qt/widgets/overlaydialogembeder.h"

#include <qt/forms/ui_miner.h>
#include <qt/platformstyle.h>

#include "MinerFactory.h"

#include "wallet/wallet.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "recentrequeststablemodel.h"

#include "key_io.h"

#include "libstratum/ZcashStratum.h"
#include "libstratum/StratumClient.h"
#include "validation.h"
#include "bitcoinunits.h"
#include "guiconstants.h"

MinerSetup::MinerSetup(const PlatformStyle *platformStyle, WalletModel* model, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MinerSetup)
	, model(model)
	, speed(15)
{
#ifdef Q_OS_MAC
	double iconscale = 1.0;
#else
	double iconscale = logicalDpiX() / 96.0;
#endif
	ui->setupUi(this);

	QFontMetrics fm = ui->username->fontMetrics();
#ifdef Q_OS_MAC
	ui->username->setMinimumSize(QSize(fm.width("6KtbGLHHTqskwx6nS28mTXkERW15uVtLT1    "), 0) * 2);
#else
	ui->username->setMinimumSize(QSize(fm.width("6KtbGLHHTqskwx6nS28mTXkERW15uVtLT1    "), 0));
#endif

#ifdef Q_OS_MAC
	ui->startbutton->setStyleSheet(QStringLiteral("padding : 5px;"));
	ui->stopbutton->setStyleSheet(QStringLiteral("padding : 5px;"));
#else
	ui->startbutton->setStyleSheet(QStringLiteral("padding : %1px;").arg(iconscale * 5));
	ui->stopbutton->setStyleSheet(QStringLiteral("padding : %1px;").arg(iconscale * 5));
#endif

	ui->stopbutton->hide();

	// fill CPU{id} checkbox.
	int number_of_cpus = std::thread::hardware_concurrency();

	int number_of_colum = std::max(2, (int)sqrt(number_of_cpus));

	for (int i = 0; i < number_of_cpus; i++)
	{
		auto checkbox = new QCheckBox(ui->cpu_select_group);

		checkbox->setText(QString("CPU%1").arg(i));

		checkbox->setCheckState(Qt::Checked);

		ui->cpu_select_group_layout->addWidget(checkbox, i/number_of_colum , i%number_of_colum, 1, 1);

		checkboxies.push_back(checkbox);
	}

	ui_update_timer.setInterval(std::chrono::milliseconds(500));
	connect(&ui_update_timer, SIGNAL(timeout()), this, SLOT(timer_interrupt()));
	ui_update_timer.start();

	balance_update_timer.setInterval(std::chrono::minutes(30));
	connect(&balance_update_timer, SIGNAL(timeout()), this, SLOT(second_timer_interrupt()));
	balance_update_timer.start();

	connect(ui->balance_detail,SIGNAL(linkActivated(QString)),this, SLOT(openUrl(QString)));

	if(!model || !model->getOptionsModel() || !model->getAddressTableModel() || !model->getRecentRequestsTableModel())
		return;

	// looop through address book.
	AddressTableModel * addrmodel = model->getAddressTableModel();

	for (int i =0; i < addrmodel->rowCount(QModelIndex()); i++)
	{
		QVariant stored_label = addrmodel->data(addrmodel->index(i, AddressTableModel::ColumnIndex::Label, QModelIndex()), Qt::DisplayRole);

		if (stored_label.toString() == "mining_receive_address")
		{

			QVariant stored_address = addrmodel->data(addrmodel->index(i, AddressTableModel::ColumnIndex::Address, QModelIndex()), Qt::DisplayRole);

			ui->username->setCurrentText(stored_address.toString());

#ifdef Q_OS_MAC
	ui->username->setMinimumSize(QSize(fm.width(stored_address.toString() + "   "), 0) * 2);
#else
	ui->username->setMinimumSize(QSize(fm.width(stored_address.toString() + "   "), 0));
#endif
			return;
		}
	}

	/* Generate new receiving address */
	OutputType address_type = model->getDefaultAddressType();

	if (address_type != OUTPUT_TYPE_LEGACY) {
		address_type = OUTPUT_TYPE_P2SH_SEGWIT;
	}

	QString address = addrmodel->addRow(AddressTableModel::Receive, "mining_receive_address", "", address_type);
	ui->username->setCurrentText(address);

#ifdef Q_OS_MAC
	ui->username->setMinimumSize(QSize(fm.width(address + "   "), 0) * 2);
#else
	ui->username->setMinimumSize(QSize(fm.width(address + "   "), 0));
#endif

	SendCoinsRecipient info(address, "mining_receive_address", {}, tr("address to receive mining payment"));

    /* Store request for later reference */
	model->getRecentRequestsTableModel()->addNewRequest(info);
}

MinerSetup::~MinerSetup()
{
	miner_io_service->stop();
	delete ui;
	if (miner_io_thread.joinable())
		miner_io_thread.join();
}

void MinerSetup::start_mining(std::vector<std::string> locations, std::string user, std::string password, std::vector<std::unique_ptr<ISolver>> i_solvers)
{
	miner_io_service = std::make_shared<boost::asio::io_service>();

	ZcashMiner miner(&speed, std::move(i_solvers));
	ZcashStratumClient sc{
		*miner_io_service, &miner, locations, user, password, 0, 0
	};

	sc.set_report_error([this](std::string error, bool can_auto_dismiss)
	{
		QMetaObject::invokeMethod(this, "error_report", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(error.c_str(), error.length())), Q_ARG(bool, can_auto_dismiss));
	});

	sc.set_dismiss_error([this]{
		dismiss_error();
	});

	miner.onSolutionFound([&](const EquihashSolution& solution, const std::string& jobid) {
		return sc.submit(&solution, jobid);
	});

	QMetaObject::invokeMethod(this, "second_timer_interrupt", Qt::QueuedConnection);

	miner_io_service->run();

	miner.stop();
	speed.Reset();
}

void MinerSetup::timer_interrupt()
{
	ui->hashrate->setText(QString("%1H/s").arg(speed.GetSolutionSpeed()));
}

void MinerSetup::second_timer_interrupt()
{
	if (ui->location->currentText() == "erpool.org")
	{
		if (IsValidDestinationString(ui->username->currentText().toStdString()))
		{
			QUrl url = QString("%1/userapi/getBenefit/%2").arg("http://47.96.53.188/").arg(ui->username->currentText());

			QNetworkReply* api_replay = m_networkmanager.get(QNetworkRequest(url));

			connect(api_replay, SIGNAL(finished()), api_replay, SLOT(deleteLater()));
			connect(api_replay, SIGNAL(error(QNetworkReply::NetworkError)), api_replay, SLOT(deleteLater()));

			connect(api_replay, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(process_network_error(QNetworkReply::NetworkError)));
			connect(api_replay, SIGNAL(readChannelFinished()), this, SLOT(process_network_rpc_finished()));
		}
	}
	else
	{
		ui->balance->setText(tr("unsupported miner pool, please view on there website"));
	}
}

void MinerSetup::process_network_rpc_finished()
{
	QNetworkReply* api_replay = dynamic_cast<QNetworkReply*>(sender());

	if (api_replay)
	{
		auto replay_json = QJsonDocument::fromJson(api_replay->readAll());

		// {"errCode":null,"errMsg":null,"data":{"userName":"coinyee","address":null,"assignedValue":"","toAssignValue":2055.37600000,"pooFee":0,"minAssign":0.00050}}
		if (replay_json["data"].isObject())
		{
			auto pending_balance = replay_json["data"]["toAssignValue"].toString();

			set_pending_balance(pending_balance);
		}
		else
		{
			ui->balance->setText(tr("query failed"));
		}
	}
}

void MinerSetup::process_network_error(QNetworkReply::NetworkError)
{
	ui->balance->setText(tr("query failed"));
}

void MinerSetup::set_pending_balance(QString pending_balance)
{
	//int unit = walletModel->getOptionsModel()->getDisplayUnit();
	CAmount pending_balance_value = 0;
	BitcoinUnits::parse(BitcoinUnit::BTC, pending_balance, &pending_balance_value);
	pending_balance = BitcoinUnits::format(BitcoinUnit::BTC, pending_balance_value, false, BitcoinUnits::separatorStandard);

	ui->balance->setText(QString(R"htmlstring(<html><head/><body><p><span style=" font-size:14pt;">%1 </span><span style=" font-size:9pt;">%2</span></p></body></html>)htmlstring")
	.arg(pending_balance).arg(QAPP_COIN_UNIT)
	);
}

void MinerSetup::on_startbutton_clicked()
{
	int num_threads = std::accumulate(checkboxies.begin(), checkboxies.end(), 0, [](int sum, QCheckBox* box){ return box->checkState() == Qt::Unchecked ?  sum  : sum + 1;});

	std::vector<std::string> locations;

	std::string user = ui->username->currentText().toStdString();
	if (ui->location->currentText() == QStringLiteral("erpool.org"))
	{
		locations.push_back("r.erpool.vip:3333");
	}
	else
	{
		locations.push_back(ui->location->currentText().toStdString());
	}

	// start miner.
	if (!IsValidDestinationString(user))
	{
		error_report(tr("Invalid address."), false);
		return;
	}

#if defined(_WIN32)
	{
		std::wstring ComputerName;

		DWORD ComputerNamelen = 200;
		ComputerName.resize(ComputerNamelen);

		if (GetComputerNameW(&ComputerName[0], &ComputerNamelen))
		{
			ComputerName.resize(ComputerNamelen);

			user = user + "." + QString::fromStdWString(ComputerName).toStdString();
		}
	}
#endif

	if (this->miner_io_thread.joinable())
		this->miner_io_thread.join();

	this->miner_io_thread = std::thread([this, num_threads, user, locations](){

		std::vector<int> cuda_enabled;
		std::vector<int> cuda_blocks;
		std::vector<int> cuda_tpb;

		std::vector<int> opencl_enabled;
		std::vector<int> opencl_threads;

		auto _MinerFactory = new MinerFactory();

		start_mining(locations, user, "",
			_MinerFactory->GenerateSolvers(num_threads, cuda_enabled.size(), cuda_enabled.data(), cuda_blocks.data(),
			cuda_tpb.data(), opencl_enabled.size(), 0, opencl_enabled.data()));
	});

	ui->startbutton->hide();
	ui->stopbutton->show();

	setWindowTitle(tr("Contribute to a Pool - Mining"));

	Q_EMIT MinerStatusChanged(true);
}

void MinerSetup::on_stopbutton_clicked()
{
	miner_io_service->stop();

	setWindowTitle(tr("Stopping Miner...."));

	if (this->miner_io_thread.joinable())
		this->miner_io_thread.join();

	ui->stopbutton->hide();
	ui->startbutton->show();

	setWindowTitle(tr("Contribute to a Pool"));

	Q_EMIT MinerStatusChanged(false);
}

void MinerSetup::on_location_editTextChanged(QString l)
{
	if (l!="erpool.org")
	{
		ui->balance->setText(tr("unsupported miner pool, please view on there website"));
	}
	else
	{
		QString detailurl = QStringLiteral("http://%1.erpool.org/account/%2").arg(QAPP_COIN_SCHEME_NAME).arg(l);
		ui->balance->setText(tr("<html><head/><body><p>Pending Balance: <a href='%1'> (Detail)</a></p></body></html>").arg(detailurl));
		set_pending_balance("0.0");
	}
}

void MinerSetup::on_username_editTextChanged(QString l)
{
	if (ui->location->currentText()=="erpool.org")
	{
		if (IsValidDestinationString(ui->username->currentText().toStdString()))
		{
			QString detailurl = QStringLiteral("http://%1.erpool.org/account/%2").arg(QAPP_COIN_SCHEME_NAME).arg(l);
			QString htmltext = tr("<html><head/><body><p>Pending Balance: <a href=\"%1\"> (Detail)</a></p></body></html>").arg(detailurl);
			ui->balance_detail->setText(htmltext);
			set_pending_balance("0.0");
		}
		else
		{
			ui->balance_detail->setText(tr("Pending Balance:"));
			set_pending_balance("0.0");
		}
	}
}

void MinerSetup::on_refresh_clicked()
{
	ui->balance->setText(tr("querying the balance"));
	second_timer_interrupt();
}

static QWidget* TopLevelParentWidget(QWidget* widget)
{
	while (widget->parentWidget() != Q_NULLPTR) widget = widget->parentWidget();
	return widget;
}

void MinerSetup::dismiss_error()
{
	QMetaObject::invokeMethod(this, "dismiss_error_invoked", Qt::QueuedConnection);
}

void MinerSetup::openUrl(QString u)
{
	QDesktopServices::openUrl(u);
}

void MinerSetup::dismiss_error_invoked()
{
	if (message_widget)
	{
		message_widget->deleteLater();
	}
}

void MinerSetup::error_report(QString error_string, bool can_auto_dismiss)
{
	if (message_widget)
	{
		message_widget->setText(message_widget->text() + "\r\n" + error_string);
	}
	else
	{
		message_widget = new QLabel(error_string);
		message_widget->setStyleSheet("color: red");

		auto embeder = new OverlayDialogEmbeder(message_widget, TopLevelParentWidget(this));

		embeder->setProperty("greycolor", QColor(255, 255, 124, 200));

		embeder->installEventFilter(this);

		embeder->setAttribute(Qt::WA_DeleteOnClose, true);
		message_widget->show();
		embeder->show();
	}

	if (can_auto_dismiss)
	{
		QTimer::singleShot(5000, message_widget.data(), SLOT(deleteLater()));
	}
}

bool MinerSetup::eventFilter(QObject * watched, QEvent * ev)
{
	if (ev->type() == QEvent::MouseButtonPress)
	{
		static_cast<QWidget*>(watched)->close();
		return true;
	}

    return QWidget::eventFilter(watched, ev);
}
