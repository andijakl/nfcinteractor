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

#ifndef NDEFNFCSMSRECORD_H
#define NDEFNFCSMSRECORD_H

#include <QNdefMessage>
#include <QNdefRecord>
#include <QString>
#include <QUrl>
#include <QGeoCoordinate>
#include <QNdefNfcUriRecord>
#include "ndefnfcsprecord.h"
#include "ndefnfcsmarturirecord.h"

QTM_USE_NAMESPACE

/*!
  \brief Convenience class for formatting SMS information into
  an NDEF record, depending on added info either URI or Smart Poster.

  Touching a tag with SMS information stored on it should trigger
  a dialog in the phone to send the proposed SMS. This can for
  example be used to interact with information services that then
  reply by sending more info per SMS, or for payment services that
  send a premium SMS to purchase items or content.

  Add the recipient number and the SMS message body to the class,
  which takes care of properly encoding the information into the
  correct URI.

  As this class is based on the Smart URI base class, the
  payload is formatted as a URI record initially. When first
  adding Smart Poster information (like a title), the payload
  instantly transforms into a Smart Poster.

  \version 1.0.0
  */
class NdefNfcSmsRecord : public NdefNfcSmartUriRecord
{
public:
    NdefNfcSmsRecord();

public:
    QString smsBody() const;
    void setSmsBody(const QString& smsBody);

    QString smsNumber() const;
    void setSmsNumber(const QString& smsNumber);

private:
    void updatePayload();

private:
    QString m_smsBody;
    QString m_smsNumber;
};

#endif // NDEFNFCSMSRECORD_H
