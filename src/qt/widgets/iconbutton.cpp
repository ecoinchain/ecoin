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
		setStyleSheet(QStringLiteral("QFrame#iconbutton {\n"
			" \n"
			"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(240, 240, 240, 255), stop:1 rgba(255, 255, 255, 255));\n"
			"}"));
	}
	else
	{
		setStyleSheet(QStringLiteral("QFrame#iconbutton {\n"
			" \n"
			"background-color: rgb(240, 240, 240);\n"
			"}"));
	}
}

void iconbutton::mousePressEvent(QMouseEvent *event)
{
	Q_EMIT clicked();
}
