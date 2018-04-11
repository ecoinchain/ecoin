
#include <QMouseEvent>
#include "recentrequestsview.h"

RecentRequestsView::RecentRequestsView(QWidget *parent)
    : QTableView(parent)
{

}

void RecentRequestsView::mousePressEvent(QMouseEvent *event)
{
	if (this->columnAt(event->x()) == model()->columnCount() - 1)
	{
		QModelIndex itemidx = this->indexAt(event->pos());
		QRect r = this->visualRect(itemidx);

		QPoint item_pos = event->pos() - r.topLeft();
		event->accept();
		Q_EMIT CellClicked(itemidx, item_pos);
		return;
	}

	return QTableView::mousePressEvent(event);
}
