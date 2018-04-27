// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>

#include "./overlaydialogembeder.h"

OverlayDialogEmbeder::OverlayDialogEmbeder(QWidget* child , QWidget *parent)
	: Greylay(parent)
{
	child->setParent(this);
    
    QVBoxLayout* l = new QVBoxLayout;

	this->setLayout(l);

	l->addWidget(child);
    l->addSpacing(0);

	if (parent) {
		QSize parentsize = parent->size();
		setGeometry(0, 0, parentsize.width(), parentsize.height());
	}
	QMargins marg(30,50,20,50);

#ifdef Q_OS_MAC
	double iconscale = 1.0;
#else
	double iconscale = logicalDpiX() / 96.0;
#endif

	setContentsMargins(marg * iconscale);
}

OverlayDialogEmbeder::~OverlayDialogEmbeder()
{
}

