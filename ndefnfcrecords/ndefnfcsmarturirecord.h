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

#ifndef NDEFNFCSMARTURIRECORD_H
#define NDEFNFCSMARTURIRECORD_H

#include <QNdefMessage>
#include <QNdefRecord>
#include "ndefnfcsprecord.h"
#include <QNdefNfcUriRecord>

QTM_USE_NAMESPACE

#define SMARTURI_URI_RECORD_TYPE "U"
#define SMARTURI_SP_RECORD_TYPE "Sp"

/*!
  \brief Smart class that uses the smallest possible NDEF record type
  for storing the requested information - either a simple URI record,
  or a Smart Poster record.

  At construction and when only setting the URI, this class will stay a
  URI record. As soon as you set a Smart Poster feature, the payload and
  type will transform into a Smart Poster. This transformation will not
  be reversed if you remove Smart Poster information again.

  If you use the type() and payload() methods provided by this class, it
  will look like a URI record again. However, as the methods of the base
  class are not virtual, accessing the payload() through a variable of type
  QNdefRecord would return a Smart Poster payload after such information
  has been added.

  Note: the QNdefRecord base class doesn't define its methods as virtual.
  Therefore, make sure you only modify the data in this class if your
  object is of the correct NdefNfcSmartUriRecord type.

  \version 1.0.0
  */
class NdefNfcSmartUriRecord : public NdefNfcSpRecord
{
public:
    // "urn:nfc:wkt:U" / "urn:nfc:wkt:Sp"
    NdefNfcSmartUriRecord();
    NdefNfcSmartUriRecord(const QNdefRecord &other);

    QByteArray type() const;
    QByteArray payload() const;

    /*!
      \brief Returns true if the record contains any information that
      would mandate a Smart Poster record.
      If this record only contains a URI, retrieving its type or payload
      will return a URI record instead of a Smart Poster.
      */
    bool isSp() const;

    void setUri(const QUrl &uri);
    void setUri(const QNdefNfcUriRecord& newUri);

    void addTitle(const QNdefNfcTextRecord& newTitle);
    void setTitleList(QList<QNdefNfcTextRecord> newTitleList);
    void setAction(const NfcAction &action);
    void setSize(const quint32 size);
    void setMimeType(const QString& mimeType);
    void setImage(const NdefNfcMimeImageRecord& imageRecord);

private:
    void changeTypeToSp();
};

#endif // NDEFNFCSMARTURIRECORD_H
