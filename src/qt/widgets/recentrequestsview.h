#pragma once

#ifndef RECENTREQUREST_VIEW_HPP
#define RECENTREQUREST_VIEW_HPP

#include <QObject>
#include <QTableView>

class RecentRequestsView : public QTableView
{
    Q_OBJECT
public:
    explicit RecentRequestsView(QWidget *parent = nullptr);

Q_SIGNALS:
	void CellClicked(const QModelIndex &index, QPoint clickpos);

public Q_SLOTS :

private:
protected:
	virtual void mousePressEvent(QMouseEvent *event) override;

};

#endif
