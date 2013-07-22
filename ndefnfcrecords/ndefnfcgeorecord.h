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

#ifndef NDEFNFCGEORECORD_H
#define NDEFNFCGEORECORD_H

#include <QNdefMessage>
#include <QNdefRecord>
#include <QString>
#include <QUrl>
#include <QGeoCoordinate>
#include <QNdefNfcUriRecord>
#include "ndefnfcsprecord.h"
#include "ndefnfcsmarturirecord.h"

QTM_USE_NAMESPACE

#define DEFAULT_GEOTAG_NOKIAMAPS_URL "http://m.ovi.me/?c="
// Web server is using a rule to rewrite "m" to "m.php"
#define DEFAULT_GEOTAG_WEBSERVICE_URL "http://NfcInteractor.com/m?c="

/*!
  \brief Store longitude and latitude on a tag, to allow the user
  to view a map when touching the tag.

  Geo tags are not standardized by the NFC forum, therefore,
  this class supports three different types of writing the location
  to a tag.

  1. GeoUri: write URI based on the "geo:" URI scheme, as specified
  by RFC5870, available at: http://geouri.org/
  Currently supported by MeeGo Harmattan and Android, not recognized
  by Symbian at the moment.

  2. NokiaMaps: write URI based on a Nokia Maps link, following the
  "http://m.ovi.me/?c=..." scheme of the Nokia/Ovi Maps Rendering API.
  Depending on the target device, the phone / web service should then
  redirect to the best maps representation.
  On Symbian, the phone will launch the Nokia Maps client. On a
  desktop computer, the full Nokia Maps web experience will open.
  On other phones, the HTML 5 client may be available.
  On MeeGo Harmattan, currently only a static Maps image is shown,
  making this alternative not suitable for the Nokia N9.

  3. WebRedirect: uses the web service at NfcInteractor.com to
  check the OS of the phone, and then redirect to the best way
  of showing maps to the user. On MeeGo Harmattan, this would be
  the GeoUri (1) scheme; on Symbian, the NokiaMaps-link (2).
  Essentially, this allows opening the Maps client on all currently
  available NFC capable Nokia phones.
  Note the limitations and terms of use of the web service. For
  real world deployment, outside of development and testing, it's
  recommended to host the script on your own web server.
  Find more information at nfcinteractor.com.
  If you host the web service on your own URI, you can change the
  location using setWebServiceUrl().

  As this class is based on the Smart URI base class, the
  payload is formatted as a URI record initially. When first
  adding Smart Poster information (like a title), the payload
  instantly transforms into a Smart Poster.

  \version 1.0.0
  */
class NdefNfcGeoRecord : public NdefNfcSmartUriRecord
{
public:
    // "urn:nfc:wkt:U" / "urn:nfc:wkt:Sp"
    NdefNfcGeoRecord();
    NdefNfcGeoRecord(const QGeoCoordinate& geoCoordinate);

public:
    /*!
      \brief The type of link to generate for the specified coordinates.

      For more details on the different types, see the general class documentation.
      */
    enum NfcGeoType {
        GeoUri = 0,
        NokiaMaps,
        WebRedirect
    };
public:
    QUrl webServiceUrl() const;
    void setWebServiceUrl(const QUrl& webServiceUrl);
    QGeoCoordinate location() const;
    void setLocation(const QGeoCoordinate& geoCoordinate);
    void setLatitude(const double latitude);
    void setLongitude(const double longitude);

    NfcGeoType geoType() const;
    void setGeoType(const NfcGeoType geoType);

private:
    void updatePayload();

private:
    QGeoCoordinate m_geoCoordinate;
    NfcGeoType m_geoType;
    QUrl m_webServiceUrl;
};

#endif // NDEFNFCGEORECORD_H
