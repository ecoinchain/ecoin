
#include <QPushButton>

#include "menubar.h"
#include "qt/forms/ui_menubar.h"

menubar::menubar(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::menubar)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
	double iconscale = 1.0;
#else
	double iconscale = logicalDpiX() / 96.0;
#endif
	ui->quitButton->icon();
	ui->quitButton->setIconSize(QSize(24, 24) * iconscale);
	ui->quitButton->setIcon(QIcon(":/icons/quit"));
	ui->quitButton->setStyleSheet("");

	ui->quitButton->installEventFilter(this);
}

menubar::~menubar()
{
    delete ui;
}

void menubar::setfileMenu(QMenu* m)
{
	ui->fileButton->setMenu(m);
}

void menubar::setsettingMenu(QMenu* m)
{
	ui->settingsButton->setMenu(m);
}

void menubar::setHelpMenu(QMenu* m)
{
	ui->helpButton->setMenu(m);
}

bool menubar::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->quitButton)
	{
		if (event->type() == QEvent::HoverEnter)
		{
			ui->quitButton->setIcon(QIcon(":/icons/colored_quit"));
		}
		else if (event->type() == QEvent::HoverLeave)
		{
			ui->quitButton->setIcon(QIcon(":/icons/quit"));
		}
	}

	return QWidget::eventFilter(watched, event);
}

void menubar::on_quitButton_clicked()
{
	Q_EMIT quitRequested();
}
