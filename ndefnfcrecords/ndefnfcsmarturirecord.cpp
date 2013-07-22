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

#include "ndefnfcsmarturirecord.h"

/*!
  \brief Create an empty Smart URI record.

  Will by default be a URI record type, until the Sp-specific
  info is added.

  Note that the payload will not be corresponding to this until
  the URI has been added, as a URI / Smart Poster record is not
  legal without an URI.
  */
NdefNfcSmartUriRecord::NdefNfcSmartUriRecord()
    : NdefNfcSpRecord()
{
    setType(SMARTURI_URI_RECORD_TYPE);
}


/*!
  \brief Return the current type of the Smart Uri record:
  either U for URI or Sp for Smart Poster when Sp-related
  info has been added.

  Warning: type() isn't defined as virtual in the base
  class, therefore this method is only executed when
  the pointer is of the correct type!
  */
QByteArray NdefNfcSmartUriRecord::type() const
{
    if (NdefNfcSpRecord::hasSpData()) {
        // Smart Poster record
        return NdefNfcSpRecord::type();
    }
    // URI record
    return SMARTURI_URI_RECORD_TYPE;
}

/*!
  \brief Get the payload of the record - either a Smart Poster,
  or a URI if no Sp-specific info has been added yet.

  Warning: payload() isn't defined as virtual in the base
  class, therefore this method is only executed when
  the pointer is of the correct type!
  */
QByteArray NdefNfcSmartUriRecord::payload() const
{
    if (NdefNfcSpRecord::hasSpData()) {
        // Complete Smart Poster payload
        return NdefNfcSpRecord::payload();
    }
    // URI record payload only, as otherwise embedded
    // in the Smart Poster.
    return NdefNfcSpRecord::uriRecord().payload();
}

/*!
  \brief Return if the information stored in this class
  needs a Smart Poster, or if the (smaller) URI record is used.
  */
bool NdefNfcSmartUriRecord::isSp() const
{
    return type() == SMARTURI_SP_RECORD_TYPE;
}

/*!
  \brief Set the URI to the specified URI.

  This method is especially important to be used from this
  class (note: non-virtual!), as it overrides the normal
  Smart Poster payload of the base class with the smaller
  URI payload if no Sp-info has been added yet.
  */
void NdefNfcSmartUriRecord::setUri ( const QUrl & uri )
{
    if (NdefNfcSpRecord::hasSpData()) {
        // It's already a Smart Poster, so use the methods
        // of the Sp base class
        NdefNfcSpRecord::setUri(uri);
    } else {
        NdefNfcSpRecord::setUri(uri);
        // It's still a URI record - so set the payload as a
        // URI record as well and override the Smart Poster.
        QNdefNfcUriRecord uriRecord;
        uriRecord.setUri(uri);
        QNdefRecord::setPayload(uriRecord.payload());
    }
}

/*!
  \brief Set the URI to the specified URI.

  This method is especially important to be used from this
  class (note: non-virtual!), as it overrides the normal
  Smart Poster payload of the base class with the smaller
  URI payload if no Sp-info has been added yet.
  */
void NdefNfcSmartUriRecord::setUri(const QNdefNfcUriRecord &newUri)
{
    if (NdefNfcSpRecord::hasSpData()) {
        // It's already a Smart Poster, so use the methods
        // of the Sp base class
        NdefNfcSpRecord::setUri(newUri);
    } else {
        NdefNfcSpRecord::setUri(newUri);
        // It's still a URI record - so set the payload as a
        // URI record as well and override the Smart Poster.
        QNdefRecord::setPayload(newUri.payload());
    }
}

void NdefNfcSmartUriRecord::addTitle(const QNdefNfcTextRecord &newTitle)
{
    changeTypeToSp();
    NdefNfcSpRecord::addTitle(newTitle);
}

void NdefNfcSmartUriRecord::setTitleList(QList<QNdefNfcTextRecord> newTitleList)
{
    changeTypeToSp();
    NdefNfcSpRecord::setTitleList(newTitleList);
}

void NdefNfcSmartUriRecord::setAction(const NdefNfcSpRecord::NfcAction &action)
{
    changeTypeToSp();
    NdefNfcSpRecord::setAction(action);
}

void NdefNfcSmartUriRecord::setSize(const quint32 size)
{
    changeTypeToSp();
    NdefNfcSpRecord::setSize(size);
}

void NdefNfcSmartUriRecord::setMimeType(const QString &mimeType)
{
    changeTypeToSp();
    NdefNfcSpRecord::setMimeType(mimeType);
}

void NdefNfcSmartUriRecord::setImage(const NdefNfcMimeImageRecord &imageRecord)
{
    changeTypeToSp();
    NdefNfcSpRecord::setImage(imageRecord);
}

/*!
  \brief Force this class to become a Smart Poster.

  Note: this method alone doesn't alter the payload if it's currently
  forced to be a URI record payload. You'd need to call a method
  of the Smart Poster base class that updates the payload.
  */
void NdefNfcSmartUriRecord::changeTypeToSp()
{
    setType(SMARTURI_SP_RECORD_TYPE);
}

