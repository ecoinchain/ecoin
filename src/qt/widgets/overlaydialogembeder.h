// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#ifndef OVERLAY_DIALOG_EMBEDER_HPP
#define OVERLAY_DIALOG_EMBEDER_HPP

#include <QWidget>

#include "qt/widgets/greylay.h"

/** Modal overlay to display information about the chain-sync state */
class OverlayDialogEmbeder : public Greylay
{
    Q_OBJECT
public:
    explicit OverlayDialogEmbeder(QWidget* embed_child, QWidget *parent);
    ~OverlayDialogEmbeder();
};

#endif
