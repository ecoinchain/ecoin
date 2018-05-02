// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>

#include "./greylay.h"

Greylay::Greylay(QWidget *parent)
	: ModalOverlay(parent)
{
	m_greycolor = QColor(57, 60, 61, 90);
}

Greylay::~Greylay()
{
}

void Greylay::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.fillRect(event->rect(), m_greycolor);
}
