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

#include "ndefnfcstorelinkrecord.h"

/*!
  \brief Create an empty App Store Link record.

  Uses the web service hosted at nfcinteractor.com by default
  for multi-store links. See terms and conditions at
  nfcinteractor.com
  */
NdefNfcStoreLinkRecord::NdefNfcStoreLinkRecord() :
    m_webServiceUrl(DEFAULT_STORELINK_WEBSERVICE_URL)
{
    updatePayload();
}

/*!
  \brief Create an empty App Store Link record, using the specified
  web service for multi-store links.
  */
NdefNfcStoreLinkRecord::NdefNfcStoreLinkRecord(const QUrl &webServiceUrl) :
    m_webServiceUrl(webServiceUrl)
{
    updatePayload();
}

/*!
  \brief Get the current web service URL.

  By default, the web service hosted at nfcinteractor.com is used.
  See terms and conditions at nfcinteractor.com
  */
QUrl NdefNfcStoreLinkRecord::webServiceUrl() const
{
    return m_webServiceUrl;
}

/*!
  \brief Set the web service URL.

  The parameter should contain the complete URL. The script will then
  add the corresponding parameters to the URL.
  */
void NdefNfcStoreLinkRecord::setWebServiceUrl(const QUrl &webServiceUrl)
{
    m_webServiceUrl = webServiceUrl;
    updatePayload();
}

/*!
  \brief Add an app id for a specified app store.

  If you only add a single app id, the class will generate
  a direct store link. If you add more than one app store,
  it will by default use the nfcinteractor.com web service.
  */
void NdefNfcStoreLinkRecord::addAppId(const NdefNfcStoreLinkRecord::AppStore appStore, const QString &appId)
{
    m_appIds.insert(appStore, appId);
    updatePayload();
}

/*!
  \brief Retrieve the app id for the specified app store, if it has
  already been set.
  */
QString NdefNfcStoreLinkRecord::appId(NdefNfcStoreLinkRecord::AppStore appStore) const
{
    return m_appIds.value(appStore);
}

/*!
  \brief Format the payload and set it through the Smart URI base class.
  */
void NdefNfcStoreLinkRecord::updatePayload()
{
    QUrl tagStoreUri;
    const int numIds = m_appIds.size();
    // One app store only,
    // or same app id for Nokia Store on different platforms
    // -> Generate direct link
    if (numIds == 1) {
        // Get the platform and ID of the specified
        // app and generate a direct link
        QHash<AppStore, QString>::const_iterator i = m_appIds.constBegin();
        while (i != m_appIds.constEnd()) {
            // Only one element in the hash map, but still
            // need to iterate over it to get it.
            tagStoreUri = generateStoreLink(i.key(), i.value());
            i++;
        }
    } else if (numIds > 1) {
        // Multiple app stores
        // -> Use nfcinteractor.com script
        tagStoreUri = generateMultiStoreLink();
    }

    // No app id set
    //qDebug() << "Store URI before check: " << tagStoreUri;
    if (tagStoreUri.isEmpty()) {
        //qDebug() << "isEmpty";
        tagStoreUri.setUrl("http://store.ovi.com");
    }

    // Set link to base class
    NdefNfcSmartUriRecord::setUri(tagStoreUri);
}

/*!
  \brief Generate a direct link to the specified app store, using the specified UID.
  */
QUrl NdefNfcStoreLinkRecord::generateStoreLink(const NdefNfcStoreLinkRecord::AppStore appStore, const QString& appId)
{
    QUrl link;
    switch (appStore) {
    case StoreNokia:
    case StoreSymbian:
    case StoreMeeGoHarmattan:
    case StoreSeries40:
        link.setUrl("http://store.ovi.com/content/" + appId);
        break;
    case StoreWindowsPhone:
        link.setUrl("http://windowsphone.com/s?appId=" + appId);
        break;
    case StoreAndroid:
        link.setUrl("https://market.android.com/details?id=" + appId);
        break;
    case StoreiOS:
        link = "http://itunes.com/apps/" + appId;
        break;
    case StoreBlackberry:
        link = "http://appworld.blackberry.com/webstore/clientlaunch/" + appId;
        break;
    case StoreCustomName:
        link = m_webServiceUrl;
        link.addQueryItem("c", appId);
        break;
    }
    return link;
}

/*!
  \brief Create a multi-store link for all defined app stores, using the
  web service.
  */
QUrl NdefNfcStoreLinkRecord::generateMultiStoreLink()
{
    QUrl link(m_webServiceUrl);
    QHash<AppStore, QString>::const_iterator i = m_appIds.constBegin();
    while (i != m_appIds.constEnd()) {
        //cout << i.key() << ": " << i.value() << endl;
        link.addQueryItem(getWebCharForAppStore(i.key()), i.value());
        ++i;
    }
    return link;
}

/*!
  \brief Get the character used to abbreviate the app store for the web service.

  The web service adds the content IDs for the different platforms as GET parameters
  to the URL. The character used for the parameters can be retrieved using this method
  for the specified app store.
  */
QString NdefNfcStoreLinkRecord::getWebCharForAppStore(const NdefNfcStoreLinkRecord::AppStore appStore) {
    switch (appStore) {
    case StoreNokia:
        return "n";
        break;
    case StoreSymbian:
        return "s";
        break;
    case StoreMeeGoHarmattan:
        return "h";
        break;
    case StoreSeries40:
        return "f";
        break;
    case StoreWindowsPhone:
        return "w";
        break;
    case StoreAndroid:
        return "a";
        break;
    case StoreiOS:
        return "i";
        break;
    case StoreBlackberry:
        return "b";
        break;
    case StoreCustomName:
        return "n";
        break;
    }
    return QString();
}
