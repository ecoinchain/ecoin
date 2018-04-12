#include "menubar.h"
#include "qt/forms/ui_menubar.h"

menubar::menubar(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::menubar)
{
    ui->setupUi(this);
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

void menubar::on_quitButton_clicked()
{
	Q_EMIT quitRequested();
}
