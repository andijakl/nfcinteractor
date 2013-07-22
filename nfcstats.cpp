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

#include "nfcstats.h"

NfcStats::NfcStats(QObject *parent) :
    QObject(parent)
{
    loadStats();
}

int NfcStats::tagReadCount()
{
    return m_tagReadCount;
}

void NfcStats::incTagReadCount()
{
    m_tagReadCount++;
    saveStats();
}

QHash<NfcTypes::MessageType, int> NfcStats::msgWrittenCountAll()
{
    return m_writtenMsgCount;
}

int NfcStats::msgWrittenCount(NfcTypes::MessageType msgType)
{
    return m_writtenMsgCount.value(msgType, 0);
}

int NfcStats::advMsgWrittenCount()
{
    return m_writtenMsgCount.value(NfcTypes::MsgSms, 0) +
            m_writtenMsgCount.value(NfcTypes::MsgBusinessCard, 0) +
            m_writtenMsgCount.value(NfcTypes::MsgSocialNetwork, 0) +
            m_writtenMsgCount.value(NfcTypes::MsgGeo, 0) +
            m_writtenMsgCount.value(NfcTypes::MsgStore, 0) +
            m_writtenMsgCount.value(NfcTypes::MsgCustom, 0);
}

bool NfcStats::isAdvMsgType(NfcTypes::MessageType msgType)
{
    return (msgType == NfcTypes::MsgSms ||
            msgType == NfcTypes::MsgBusinessCard ||
            msgType == NfcTypes::MsgSocialNetwork ||
            msgType == NfcTypes::MsgGeo ||
            msgType == NfcTypes::MsgStore ||
            msgType == NfcTypes::MsgCustom);
}

void NfcStats::resetComposedMsgCount()
{
    m_composedMsgCount.clear();
}

void NfcStats::incComposedMsgCount(NfcTypes::MessageType msgType)
{
    // Update the count in the hash by one
    // (using 0 as the default if this tag hasn't been
    // written yet)
    const int count = m_composedMsgCount.value(msgType, 0) + 1;
    // Insert into the hash
    // (overwrites an existing value)
    m_composedMsgCount.insert(msgType, count);
}

void NfcStats::commitComposedToWrittenCount()
{
    // Loop over composed messages present in the current NDEF message,
    // and store them to the actually written messages
    QHash<NfcTypes::MessageType,int>::const_iterator i = m_composedMsgCount.constBegin();
    while (i != m_composedMsgCount.constEnd()) {
        incWrittenMsgCount(i.key(), i.value());
        ++i;
    }
    saveStats();
}

void NfcStats::incWrittenMsgCount(NfcTypes::MessageType msgType, const int numMsgPresent)
{
    const int count = m_writtenMsgCount.value(msgType, 0) + numMsgPresent;
    m_writtenMsgCount.insert(msgType, count);
}


void NfcStats::loadStats()
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);

    m_tagReadCount = settings.value("TagReadCount", 0).toInt();
    // Ok, we could loop over the enum, treating it as an int
    // But let's rather add them individually, for safety reasons
    // in case something changes in the future.
    loadCount(&settings, NfcTypes::MsgSmartPoster);
    loadCount(&settings, NfcTypes::MsgUri);
    loadCount(&settings, NfcTypes::MsgText);
    loadCount(&settings, NfcTypes::MsgSms);
    loadCount(&settings, NfcTypes::MsgBusinessCard);
    loadCount(&settings, NfcTypes::MsgSocialNetwork);
    loadCount(&settings, NfcTypes::MsgGeo);
    loadCount(&settings, NfcTypes::MsgStore);
    loadCount(&settings, NfcTypes::MsgImage);
    loadCount(&settings, NfcTypes::MsgCustom);
}

void NfcStats::loadCount(QSettings* settings, const NfcTypes::MessageType msgType)
{
    m_writtenMsgCount.insert(msgType, settings->value(QString("msg") + QString::number((int)msgType), 0).toInt());
}

void NfcStats::saveStats()
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);

    settings.setValue("TagReadCount", m_tagReadCount);
    //qDebug() << "Save tag read count: " << m_tagReadCount;
    saveCount(&settings, NfcTypes::MsgSmartPoster);
    saveCount(&settings, NfcTypes::MsgUri);
    saveCount(&settings, NfcTypes::MsgText);
    saveCount(&settings, NfcTypes::MsgSms);
    saveCount(&settings, NfcTypes::MsgBusinessCard);
    saveCount(&settings, NfcTypes::MsgSocialNetwork);
    saveCount(&settings, NfcTypes::MsgGeo);
    saveCount(&settings, NfcTypes::MsgStore);
    saveCount(&settings, NfcTypes::MsgImage);
    saveCount(&settings, NfcTypes::MsgCustom);
}

void NfcStats::saveCount(QSettings* settings, const NfcTypes::MessageType msgType)
{
    //qDebug() << "Save written message count: " << QString("msg") << QString::number((int)msgType) << m_writtenMsgCount.value(msgType, 0);
    settings->setValue(QString("msg") + QString::number((int)msgType), m_writtenMsgCount.value(msgType, 0));
}
