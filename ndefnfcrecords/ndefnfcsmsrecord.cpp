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

#include "ndefnfcsmsrecord.h"

/*!
  \brief Create an empty Sms record.
  */
NdefNfcSmsRecord::NdefNfcSmsRecord()
    : NdefNfcSmartUriRecord()
{
    updatePayload();
}

/*!
  \brief Return the current body of the short message.
  */
QString NdefNfcSmsRecord::smsBody() const
{
    return m_smsBody;
}

/*!
  \brief Set the body of the short message.

  Note: the URI content has to be encoded in UTF-8, which should
  be done by the Qt Mobility URI class. No further transformation
  is necessary for the SMS use case (e.g., to 7-bit US-ASCII, as
  would be recommended for Internet URLs).
  */
void NdefNfcSmsRecord::setSmsBody(const QString &smsBody)
{
    m_smsBody = smsBody;
    updatePayload();
}

/*!
  \brief Retrieve the number  the reading phone is supposed to send
  the short message to.
  */
QString NdefNfcSmsRecord::smsNumber() const
{
    return m_smsNumber;
}

/*!
  \brief Set the number the reading phone is supposed to send
  the short message to.

  Recommended to store in international format, e.g., +431234...
  */
void NdefNfcSmsRecord::setSmsNumber(const QString &smsNumber)
{
    m_smsNumber = smsNumber;
    updatePayload();
}

/*!
  \brief Format the payload and set it through the Smart URI base class.
  */
void NdefNfcSmsRecord::updatePayload()
{
    // Create URI string
    QString tagSmsUri = "sms:" + m_smsNumber;
    if (!m_smsBody.isEmpty()) {
        tagSmsUri.append("?body=" + m_smsBody);
    }

    NdefNfcSmartUriRecord::setUri(tagSmsUri);
}
