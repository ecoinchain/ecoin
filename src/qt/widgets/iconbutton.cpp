#include "iconbutton.h"
#include "qt/forms/ui_iconbutton.h"

iconbutton::iconbutton(std::array<QIcon, 2> _icons, QString txt, QWidget * parent)
	: QFrame(parent)
	, ui(new Ui::iconbutton)
{
	ui->setupUi(this);
	ui->text->setText(txt);
	
	//setStyleSheet("");
	mystyle = parent->palette();
}

iconbutton::~iconbutton()
{
    delete ui;
}

void iconbutton::setIcon(QIcon ico)
{
	ui->icon->setPixmap(ico.pixmap(ui->icon->size()));
}

void iconbutton::setActive(bool active)
{
	if (active)
	{
		setStyleSheet(QLatin1String("QFrame {\n"
			" \n"
			"	background-color: rgb(255, 255, 255);\n"
			"}"));
	}
	else
	{
		setStyleSheet(QLatin1String(""));
	}
}

void iconbutton::mousePressEvent(QMouseEvent *event)
{
	Q_EMIT clicked();
}
