#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <array>

#include <QFrame>
#include <QIcon>

namespace Ui {
class iconbutton;
}

class iconbutton : public QFrame
{
    Q_OBJECT

public:
	explicit iconbutton(std::array<QIcon, 2> _icons, QString txt, QWidget * parent);
	~iconbutton();
	void setIcon(QIcon);

Q_SIGNALS:
	void clicked();

public Q_SLOTS:
	void setActive(bool active);


protected:
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::iconbutton *ui;
	QPalette mystyle;
};

#endif // ICONBUTTON_H
