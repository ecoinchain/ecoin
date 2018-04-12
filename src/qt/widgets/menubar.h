#ifndef MENUBAR_H
#define MENUBAR_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class menubar;
}

class menubar : public QWidget
{
    Q_OBJECT

public:
    explicit menubar(QWidget *parent = 0);
    ~menubar();

	void setfileMenu(QMenu*);
	void setsettingMenu(QMenu*);
	void setHelpMenu(QMenu*);
private:
    Ui::menubar *ui;
};

#endif // MENUBAR_H
