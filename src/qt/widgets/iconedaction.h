#ifndef ICONEDACTION_H
#define ICONEDACTION_H

#include <array>
#include <QIcon>
#include <QString>
#include <QWidget>

#include <QWidgetAction>

class IconedAction : public QWidgetAction
{
	Q_OBJECT
public:
    IconedAction(std::array<QIcon,2>, QString txt, QWidget* parent = nullptr);

	void setCheckable(bool);

	QWidget* createWidget(QWidget *parent);

private Q_SLOTS:
	void update_icon_when_state_changed(bool);

private:
	QString text;
	std::array<QIcon, 2> icons;
};

#endif // ICONEDACTION_H
