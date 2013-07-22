/****************************************************************************
**
** Copyright (C) 2012-2013 Andreas Jakl.
** All rights reserved.
** Contact: Andreas Jakl (andreas.jakl@mopius.com)
**
** This file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#ifndef NFCSTATS_H
#define NFCSTATS_H

#include <QObject>
#include <QHash>
#include <QSettings>
#include <QDebug>
#include "nfctypes.h"

class NfcStats : public QObject
{
    Q_OBJECT
public:
    explicit NfcStats(QObject *parent = 0);
    
public:
    int tagReadCount();
    void incTagReadCount();

    QHash<NfcTypes::MessageType,int> msgWrittenCountAll();
    int msgWrittenCount(NfcTypes::MessageType msgType);
    Q_INVOKABLE int advMsgWrittenCount();
    bool isAdvMsgType(NfcTypes::MessageType msgType);

    void resetComposedMsgCount();
    void incComposedMsgCount(NfcTypes::MessageType msgType);
    void commitComposedToWrittenCount();

private:

    void incWrittenMsgCount(NfcTypes::MessageType msgType, const int numMsgPresent);

    void loadStats();
    void loadCount(QSettings *settings, const NfcTypes::MessageType msgType);
    void saveStats();
    void saveCount(QSettings* settings, const NfcTypes::MessageType msgType);

private:
    int m_tagReadCount;
    QHash<NfcTypes::MessageType,int> m_composedMsgCount;
    QHash<NfcTypes::MessageType,int> m_writtenMsgCount;
};

#endif // NFCSTATS_H
