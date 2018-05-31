// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QResizeEvent>
#include <QPropertyAnimation>

#include "qt/widgets/modaloverlay.h"

ModalOverlay::ModalOverlay(QWidget *parent)
	: QWidget(parent)
{
    if (parent)
	{
        parent->installEventFilter(this);
        raise();
    }
}

ModalOverlay::~ModalOverlay()
{
}

bool ModalOverlay::eventFilter(QObject * obj, QEvent * ev) {
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            QResizeEvent * rev = static_cast<QResizeEvent*>(ev);
            resize(rev->size());
			raise();
			Q_EMIT parentResized(rev->size());
        }
        else if (ev->type() == QEvent::ChildAdded) {
			raise();
		}
	}

	if (ev->type() == QEvent::KeyPress)
	{
		QKeyEvent * keyevent = static_cast<QKeyEvent*>(ev);

		if (keyevent->key() == Qt::Key_Escape)
		{
			close();
			deleteLater();
			return true;
		}
	}
    return QWidget::eventFilter(obj, ev);
}

//! Tracks parent widget changes
bool ModalOverlay::event(QEvent* ev)
{
	if (ev->type() == QEvent::ParentAboutToChange)
	{
		if (parent()) parent()->removeEventFilter(this);
	}
	else if (ev->type() == QEvent::ParentChange)
	{
		if (parent())
		{
			parent()->installEventFilter(this);
			raise();
		}
	}
	else if (ev->type() == QEvent::KeyPress)
	{
		QKeyEvent * keyevent = static_cast<QKeyEvent*>(ev);

		if (keyevent->key() == Qt::Key_Escape)
		{
			close();
			deleteLater();
			return true;
		}
	}
	return QWidget::event(ev);
}
