// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QWidget>

#include "./modaloverlay.h"

/** Modal overlay to display information about the chain-sync state */
class Greylay : public ModalOverlay
{
    Q_OBJECT

	Q_PROPERTY(QColor greycolor READ greycolor WRITE setGreycolor)

public:
    explicit Greylay(QWidget *parent);
    ~Greylay();

	QColor greycolor() { return m_greycolor; }
	void setGreycolor(QColor c) { m_greycolor = c; }

protected:
	virtual void paintEvent(QPaintEvent *event) override;
	QColor m_greycolor;
};
