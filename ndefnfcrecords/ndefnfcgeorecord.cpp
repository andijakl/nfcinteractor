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

#include "ndefnfcgeorecord.h"

/*!
  \brief Create an empty Geo record.
  */
NdefNfcGeoRecord::NdefNfcGeoRecord()
    : NdefNfcSmartUriRecord(),
      m_geoType(GeoUri),
      m_webServiceUrl(DEFAULT_GEOTAG_WEBSERVICE_URL)
{
    updatePayload();
}

/*!
  \brief Create a Geo record based on the record passed
  through the argument.
  */
NdefNfcGeoRecord::NdefNfcGeoRecord(const QGeoCoordinate &geoCoordinate)
    : NdefNfcSmartUriRecord(),
      m_geoType(GeoUri),
      m_webServiceUrl(DEFAULT_GEOTAG_WEBSERVICE_URL)
{
    setLocation(geoCoordinate);
}

/*!
  \brief Get the current web service URL.

  By default, the web service hosted at nfcinteractor.com is used.
  See terms and conditions at nfcinteractor.com
  */
QUrl NdefNfcGeoRecord::webServiceUrl() const
{
    return m_webServiceUrl;
}

/*!
  \brief Set the web service URL.

  The parameter should contain the complete URL, including the
  "?c=" part. The script will then add longitude and latitude,
  separated by a comma.
  */
void NdefNfcGeoRecord::setWebServiceUrl(const QUrl &webServiceUrl)
{
    m_webServiceUrl = webServiceUrl;
    updatePayload();
}

/*!
  \brief Get the current location / geo coordinate.
  */
QGeoCoordinate NdefNfcGeoRecord::location() const
{
    return m_geoCoordinate;
}

/*!
  \brief Set the location to the specified geo coordinate.
  */
void NdefNfcGeoRecord::setLocation(const QGeoCoordinate &geoCoordinate)
{
    m_geoCoordinate = geoCoordinate;
    updatePayload();
}

/*!
  \brief Set the latitude in decimal degrees; the value should be
  in the WSG84 datum.

  To be valid, the latitude must be between -90 to 90 inclusive.
  (\see QGeoCoordinate::setLatitude())
  */
void NdefNfcGeoRecord::setLatitude(const double latitude)
{
    m_geoCoordinate.setLatitude(latitude);
    updatePayload();
}

/*!
  \brief Set the longitude in decimal degrees; the value should be
  in the WSG84 datum.

  To be valid, the longitude must be between -180 to 180 inclusive.
  (\see QGeoCoordinate::setLongitude())
  */
void NdefNfcGeoRecord::setLongitude(const double longitude)
{
    m_geoCoordinate.setLongitude(longitude);
    updatePayload();
}

/*!
  \brief Get the current geo type.
  */
NdefNfcGeoRecord::NfcGeoType NdefNfcGeoRecord::geoType() const
{
    return m_geoType;
}

/*!
  \brief Change the geo type, updating the payload to the new type.
  */
void NdefNfcGeoRecord::setGeoType(const NdefNfcGeoRecord::NfcGeoType geoType)
{
    if (m_geoType != geoType) {
        // Changing the type
        m_geoType = geoType;
        // Adapt the payload
        updatePayload();
    }
}

/*!
  \brief Format the payload and set it through the Smart URI base class.
  */
void NdefNfcGeoRecord::updatePayload()
{
    QUrl uri;
    // The double -> string conversion used here is not locale-aware, so
    // should always get the right comma.
    const QString latString = QString::number(m_geoCoordinate.latitude());
    const QString longString = QString::number(m_geoCoordinate.longitude());
    switch(m_geoType) {
    case NokiaMaps:
        uri = DEFAULT_GEOTAG_NOKIAMAPS_URL + latString + "," + longString;
        break;
    case WebRedirect:
        uri = DEFAULT_GEOTAG_WEBSERVICE_URL + latString + "," + longString;
        break;
    case GeoUri:
    default:
        uri = "geo:" + latString + "," + longString;
        break;
    }

    NdefNfcSmartUriRecord::setUri(uri);
    //qDebug() << "Geo coordinates (lat, long): " << latString << "," << longString;
}

