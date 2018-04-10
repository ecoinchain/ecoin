
#pragma once

#include <QIcon>
#include <QFrame>

class QBackgroundImageFrame : public QFrame
{
	Q_OBJECT
public:
	explicit QBackgroundImageFrame(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

	void setBackgroundImage(const QIcon&);

protected:
	virtual void paintEvent(QPaintEvent *event) override;

private:
	QIcon bgimg;
};
