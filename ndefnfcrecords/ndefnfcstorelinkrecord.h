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

#ifndef NDEFNFCSTORELINKRECORD_H
#define NDEFNFCSTORELINKRECORD_H

#include <QNdefMessage>
#include <QNdefRecord>
#include <QString>
#include <QUrl>
#include <QNdefNfcUriRecord>
#include "ndefnfcsprecord.h"
#include "ndefnfcsmarturirecord.h"

QTM_USE_NAMESPACE

// Web server is using a rule to rewrite "dl" to "dl.php"
#define DEFAULT_STORELINK_WEBSERVICE_URL "http://nfcinteractor.com/dl"

/*!
  \brief Create an app store link to download the app, either direct
  or for multiple stores using the nfcinteractor.com web service.

  Tags to download an app are useful in many cases, either as
  the sole record on a tag, or in combination with a custom record
  in a single NDEF message. This would then allow your app to
  auto-start when already installed on the phone (through the
  custom record your app is registered for), or if the app isn't
  installed, the phone will ignore the record and instead parse
  this store record and send the user to download the app.

  IF you only add the app ID for a single app store, the class
  will write the direct store link to the tag. This causes some
  phones to directly open the store (e.g., on the Nokia N9).

  If you want your tag to work for multiple platforms, you can use
  the web service at nfcinteractor.com to pass multiple ids for
  different platforms. The web service will then detect the
  operating system of the phone, and redirect it to the correct
  app store link.

  The web service also supports specifying a name instead of passing
  different IDs, to make further maintenance easier (StoreCustomName).
  See the instructions at nfcinteractor.com how to host the service, in
  order to add your own names to the script. You can change the URL
  of the webservice using setWebServiceUrl() or during construction
  of the class.

  As this class is based on the Smart URI base class, the
  payload is formatted as a URI record initially. When first
  adding Smart Poster information (like a title), the payload
  instantly transforms into a Smart Poster.

  \version 1.0.0
  */
class NdefNfcStoreLinkRecord : public NdefNfcSmartUriRecord
{
public:
    NdefNfcStoreLinkRecord();
    NdefNfcStoreLinkRecord(const QUrl& webServiceUrl);

public:
    /*! Which app store to link to. */
    enum AppStore {
        StoreNokia = 0,
        StoreSymbian,
        StoreMeeGoHarmattan,
        StoreSeries40,
        StoreWindowsPhone,
        StoreAndroid,
        StoreiOS,
        StoreBlackberry,
        StoreCustomName
    };

public:
    QUrl webServiceUrl() const;
    void setWebServiceUrl(const QUrl& webServiceUrl);
    void addAppId(const AppStore appStore, const QString& appId);
    QString appId(AppStore appStore) const;

private:
    void updatePayload();
    QUrl generateStoreLink(const AppStore appStore, const QString& appId);
    QUrl generateMultiStoreLink();
    QString getWebCharForAppStore(const NdefNfcStoreLinkRecord::AppStore appStore);

private:
    QUrl m_webServiceUrl;
    QHash<AppStore,QString> m_appIds;
};

#endif // NDEFNFCSTORELINKRECORD_H
