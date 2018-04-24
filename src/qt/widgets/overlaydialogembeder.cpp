// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>

#include "./overlaydialogembeder.h"

OverlayDialogEmbeder::OverlayDialogEmbeder(QDialog* child , QWidget *parent)
	: Greylay(parent)
{
	child->setParent(this);

	this->setLayout(new QHBoxLayout);

	this->layout()->addWidget(child);
}

OverlayDialogEmbeder::~OverlayDialogEmbeder()
{
}

