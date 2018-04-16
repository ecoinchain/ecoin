
#include "recentrequestsdelegates.h"
#include <QLabel>
#include <QPainter>

RecentRequestsDelegates::RecentRequestsDelegates(QObject *parent /*= Q_NULLPTR*/)
	: QItemDelegate(parent)
	, action_icon_view(":/icons/eye")
	, action_icon_delete(":/icons/remove")
{

}

QWidget * RecentRequestsDelegates::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto r = new QLabel("hello", parent);
	r->show();
	return r;
}

QSize RecentRequestsDelegates::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(50, 20);
}

void RecentRequestsDelegates::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QRect r1, r2;

	r1 = option.rect.normalized();
	r2 = option.rect.normalized();

	r1.setRight(option.rect.normalized().center().rx());
	r2.setLeft(option.rect.normalized().center().rx());

	painter->save();
	action_icon_view.paint(painter, r1);
	action_icon_delete.paint(painter, r2);
	painter->restore();
}
