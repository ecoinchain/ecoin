
#include <QPainter>
#include "qbackgroundimageframe.h"

QBackgroundImageFrame::QBackgroundImageFrame(QWidget* parent /*= Q_NULLPTR*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
	: QFrame(parent, f)
{
	m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
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
	bgimg.paint(&painter, r, m_alignment);
}
