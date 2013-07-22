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

#ifndef NDEFNFCANDROIDAPPRECORD_H
#define NDEFNFCANDROIDAPPRECORD_H

#include <QString>

#include <QNdefMessage>
#include <QNdefRecord>
#include <QDebug>

#include <QTextCodec>

QTM_USE_NAMESPACE

/*!
  \brief Creates the Android-specific Android Application Record.
  Through specifying the package name, this record directly launches an
  app on an Android phone (4.0+). If the app isn't installed on the phone,
  it will open the store and search for the app.

  To pass custom data to the app, you would typically add other records
  to the NDEF message.

  If creating a multi-record NDEF message, it's recommended to put this
  record to the end of the message.

  \see http://developer.android.com/guide/topics/connectivity/nfc/nfc.html#aar
  \version 1.0.0
  */
class NdefNfcAndroidAppRecord : public QNdefRecord
{
public:
    // "android.com:pkg"
    Q_DECLARE_NDEF_RECORD(NdefNfcAndroidAppRecord, QNdefRecord::ExternalRtd, "android.com:pkg", QByteArray());

public:
    QString packageName() const;
    void setPackageName(const QString& packageName);
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcAndroidAppRecord, QNdefRecord::ExternalRtd, "android.com:pkg")

#endif // NDEFNFCANDROIDAPPRECORD_H
