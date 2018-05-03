
#pragma once

#include <QItemDelegate>

class RecentRequestsDelegates : public QItemDelegate
{
	Q_OBJECT
public:
	explicit RecentRequestsDelegates(QObject *parent = Q_NULLPTR);

	virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
	QIcon action_icon_view;
	QIcon action_icon_delete;
};
