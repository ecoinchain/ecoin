#include "iconbutton.h"
#include "qt/forms/ui_iconbutton.h"

iconbutton::iconbutton(std::array<QIcon, 2> _icons, QString txt, QWidget * parent)
	: QFrame(parent)
	, ui(new Ui::iconbutton)
{
	ui->setupUi(this);
	ui->button->setText(txt);

#ifdef Q_OS_MAC
	double iconscale = 1.0;
#else
	double iconscale = logicalDpiX() / 96.0;
#endif

	ui->button->setIconSize(QSize(28,28)* iconscale);
	layout()->setContentsMargins(10 * iconscale, 10 * iconscale, 10 * iconscale, 10 * iconscale);
}

iconbutton::~iconbutton()
{
    delete ui;
}

void iconbutton::setIcon(QIcon ico)
{
	ui->button->setIcon(ico);
}

void iconbutton::on_button_clicked(bool)
{
	Q_EMIT clicked();
}

void iconbutton::mousePressEvent(QMouseEvent *event)
{
	Q_EMIT clicked();
	QFrame::mousePressEvent(event);
}
