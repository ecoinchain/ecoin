
#include <QPainter>
#include "qbackgroundimageframe.h"

QBackgroundImageFrame::QBackgroundImageFrame(QWidget* parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
	: QFrame(parent, f)
{
}

void QBackgroundImageFrame::setBackgroundImage(const QIcon& img)
{
	this->bgimg = img;
}

void QBackgroundImageFrame::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QRect r;
	r.setSize(size());
	bgimg.paint(&painter, r, Qt::AlignLeft | Qt::AlignVCenter);
//	this->paintEngine()
}
