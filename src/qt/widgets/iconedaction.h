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
	IconedAction(QIcon, QString txt, QWidget* parent = nullptr);
	IconedAction(QString txt, QWidget* parent = nullptr);

	void setCheckable(bool);

	QWidget* createWidget(QWidget *parent);

	void setIcon(const QIcon &icon);

Q_SIGNALS:
	void stateChanged(bool);

public Q_SLOTS:
#ifndef QT_NO_STYLE_STYLESHEET
	void setStyleSheet(const QString& styleSheet);
#endif

private Q_SLOTS:
	void update_icon_when_state_changed(bool);


private:
	QString text;
	std::array<QIcon, 2> icons;
};

#endif // ICONEDACTION_H
