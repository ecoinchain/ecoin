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
Q_SIGNALS:
	void quitRequested();
private Q_SLOTS:
	void on_quitButton_clicked();
private:
    Ui::menubar *ui;
};

#endif // MENUBAR_H
