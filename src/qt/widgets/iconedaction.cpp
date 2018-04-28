
#include "iconedaction.h"
#include "iconbutton.h"

IconedAction::IconedAction(std::array<QIcon, 2> _icons, QString txt, QWidget * parent)
	: QWidgetAction(parent)
    , text(txt)
	, icons(_icons)
{
	setIcon(icons[0]);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(update_icon_when_state_changed(bool)));
}

IconedAction::IconedAction(QIcon i, QString txt, QWidget* parent /*= nullptr*/)
	: QWidgetAction(parent)
	, text(txt)
    , icons({{i, i}})
{
	setIcon(icons[0]);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(update_icon_when_state_changed(bool)));
}


IconedAction::IconedAction(QString txt, QWidget* parent /*= nullptr*/)
	: QWidgetAction(parent)
	, text(txt)
{
	connect(this, SIGNAL(toggled(bool)), this, SLOT(update_icon_when_state_changed(bool)));
}

void IconedAction::setCheckable(bool checkable)
{
	QAction::setCheckable(checkable);

	if (!checkable)
	{
		setIcon(icons[0]);
	}
	else
	{
		setIcon(icons[1]);
	}
}

QWidget * IconedAction::createWidget(QWidget * parent)
{
	auto w = new iconbutton(icons, text, parent);
	connect(w, SIGNAL(clicked()), this, SLOT(trigger()));

	w->setIcon(icons[1]);
	return w;
}

void IconedAction::setIcon(const QIcon &icon)
{
	QAction::setIcon(icon);
	for (auto w : this->createdWidgets())
	{
		dynamic_cast<iconbutton*>(w)->setIcon(icon);
	}
}

void IconedAction::setStyleSheet(const QString& styleSheet)
{
	for (auto w : this->createdWidgets())
	{
		dynamic_cast<iconbutton*>(w)->setStyleSheet(styleSheet);
	}
}

void IconedAction::update_icon_when_state_changed(bool checked)
{
	setIcon(icons[checked? 0 : 1]);

	Q_EMIT stateChanged(checked);
}
