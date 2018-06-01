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

	QFontMetrics fm = this->fontMetrics();
	ui->username->setMinimumSize(QSize(fm.width("6KtbGLHHTqskwx6nS28mTXkERW15uVtLT1   "), 0));
	ui->stopbutton->hide();

	// fill CPU{id} checkbox.
	int number_of_cpus = std::thread::hardware_concurrency();

	for (int i = 0; i < number_of_cpus; i++)
	{
		auto checkbox = new QCheckBox(ui->cpu_select_group);

		checkbox->setText(QString("CPU%1").arg(i));

		checkbox->setCheckState(Qt::Checked);

		ui->cpu_select_group_layout->addWidget(checkbox, i/2 , i%2, 1, 1);

		checkboxies.push_back(checkbox);
	}

	ui_update_timer.setInterval(std::chrono::milliseconds(500));
	connect(&ui_update_timer, SIGNAL(timeout()), this, SLOT(timer_interrupt()));
	ui_update_timer.start();

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

	SendCoinsRecipient info(address, "mining_receive_address", {}, tr("address to receive mining payment"));

    /* Store request for later reference */
	model->getRecentRequestsTableModel()->addNewRequest(info);
}

MinerSetup::~MinerSetup()
{
	miner_io_service.stop();
	delete ui;
	if (miner_io_thread.joinable())
		miner_io_thread.join();
}

void MinerSetup::start_mining(std::string host, std::string port,
	std::string user, std::string password, std::vector<std::unique_ptr<ISolver>> i_solvers)
{
	miner_io_service.reset();

	ZcashMiner miner(&speed, std::move(i_solvers));
	ZcashStratumClient sc{
		miner_io_service, &miner, host, port, user, password, 0, 0
	};

	sc.set_report_error([this](std::string error)
	{
		QMetaObject::invokeMethod(this, "error_report", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdString(error)));
	});

	sc.on_target_change([this](std::string newtarget)
	{
		QMetaObject::invokeMethod(this, "on_target_change", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdString(newtarget)));
	});

	miner.onSolutionFound([&](const EquihashSolution& solution, const std::string& jobid) {
		return sc.submit(&solution, jobid);
	});

	miner_io_service.run();

	miner.stop();
	speed.Reset();
}

void MinerSetup::timer_interrupt()
{
	double allshares = speed.GetShareSpeed() * 60;

	ui->hashrate->setText(QString("%1/s").arg(speed.GetSolutionSpeed()));
	ui->acceptedshare->setText(QString("%1/min").arg(allshares));
}

void MinerSetup::on_startbutton_clicked()
{
	int num_threads = std::accumulate(checkboxies.begin(), checkboxies.end(), 0, [](int sum, QCheckBox* box){ return box->checkState() == Qt::Unchecked ?  sum  : sum + 1;});

	std::string location;
	std::string user = ui->username->currentText().toStdString();
	if (ui->location->currentText() == "币易官方矿池")
		location = "47.97.167.150:3333";
	else
		location = ui->location->currentText().toStdString();

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

	setWindowTitle(tr("YeeMiner - Mining"));
}

void MinerSetup::on_stopbutton_clicked()
{
	miner_io_service.stop();

	setWindowTitle(tr("Stopping Miner...."));

	if (this->miner_io_thread.joinable())
		this->miner_io_thread.join();

	ui->stopbutton->hide();
	ui->startbutton->show();

	setWindowTitle(tr("YeeMiner"));

	ui->difficultlty->setText(tr("N/A"));
}

static QWidget* TopLevelParentWidget(QWidget* widget)
{
	while (widget->parentWidget() != Q_NULLPTR) widget = widget->parentWidget();
	return widget;
}

void MinerSetup::error_report(QString error_string)
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

void MinerSetup::on_target_change(QString newtarget)
{
	ui->difficultlty->setText(newtarget);
}

