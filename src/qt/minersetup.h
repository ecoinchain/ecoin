// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <amount.h>

#include <QWidget>
#include <memory>

class PlatformStyle;

namespace Ui {
    class MinerSetup;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class MinerSetup : public QWidget
{
    Q_OBJECT

public:
    explicit MinerSetup(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~MinerSetup();
private:
	Ui::MinerSetup* ui;
};
