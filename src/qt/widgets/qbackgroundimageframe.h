
#pragma once

#include <QIcon>
#include <QFrame>

class QBackgroundImageFrame : public QFrame
{
	Q_OBJECT
	Q_PROPERTY(QIcon backgroundImage READ backgroundImage WRITE setBackgroundImage)
	Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

public:
	explicit QBackgroundImageFrame(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

	void setBackgroundImage(const QIcon&);
	QIcon backgroundImage() const { return bgimg; } ;

	Qt::Alignment alignment() const { return m_alignment; };
	void setAlignment(Qt::Alignment v) { m_alignment = v; };

protected:
	virtual void paintEvent(QPaintEvent *event) override;

private:
	QIcon bgimg;
	Qt::Alignment m_alignment;
};
