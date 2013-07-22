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

#ifndef NDEFNFCLAUNCHAPPRECORD_H
#define NDEFNFCLAUNCHAPPRECORD_H

#include <QString>

#include <QNdefMessage>
#include <QNdefRecord>
#include <QDebug>

#include <QTextCodec>

QTM_USE_NAMESPACE

/*!
  \brief Handles the Windows LaunchApp record format.

  The Windows Proximity APIs can directly write LaunchApp tags using
  a specific API call. However, this doesn't allow flexibility
  in including this record together with other records on a tag,
  to allow for greater flexibility.

  A LaunchApp record created using this class can be put into a
  multi-record NDEF message. This allows creating a tag that contains
  both the Windows LaunchApp tag, as well as the Android Application
  Record.

  Note that for default handling by the OS, the Windows LaunchApp tag
  needs to be the first record in the message. Android recommends to put
  its Android Application Record (AAR) as the last record of the message.

  To create a LaunchApp tag using the Windows Proximity APIs, you have
  to pass a string containing the arguments and platform/app ID tuples
  in a special format (separated by tabs). The APIs then re-format the
  text into the actual payload of the record - meaning that the string
  you send to the Proximity LaunchApp Write API does NOT get directly
  written to the tag.

  In contrast, this class provides a more convenient way to set the
  arguments and to add any number of platforms; all using properties or
  methods, without the need to worry about formatting a special string.
  This class will then directly create the required raw payload that
  is suitable to be written to the tag.

  \version 1.0.0
 */
class NdefNfcLaunchAppRecord : public QNdefRecord
{
public:
    NdefNfcLaunchAppRecord();
    NdefNfcLaunchAppRecord(const QNdefRecord &other);
    virtual ~NdefNfcLaunchAppRecord();

    // Attention: this method is non-virtual in the base class!
    void setPayload(const QByteArray &payload);

public:
    QString arguments() const;
    void setArguments(const QString& arguments);

    void addPlatformAppId(const QString& platform, const QString& appId);
    int platformAppIdsCount() const;
    QHash<QString,QString> platformAppIds() const;

private:
    void initializeData();
    void parsePayloadToData();
    bool assemblePayload();
    void setPayloadAndParse(const QByteArray &payload, const bool parseNewPayload);

private:
    QString m_arguments;
    QHash<QString,QString> m_platformAppIds;
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcLaunchAppRecord, QNdefRecord::Uri, "windows.com/LaunchApp")

#endif // NDEFNFCLAUNCHAPPRECORD_H
