
#include "iconedaction.h"
#include "iconbutton.h"

IconedAction::IconedAction(std::array<QIcon, 2> _icons, QString txt, QWidget * parent)
	: QWidgetAction(parent)
	, icons(_icons)
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

void IconedAction::update_icon_when_state_changed(bool checked)
{
	setIcon(icons[checked? 0 : 1]);

	for (auto w : this->createdWidgets())
	{
		dynamic_cast<iconbutton*>(w)->setIcon(icons[checked ? 0 : 1]);
		dynamic_cast<iconbutton*>(w)->setActive(checked);
	}
	
}
