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

#ifndef NFCMODELTONDEF_H
#define NFCMODELTONDEF_H

#include <QObject>
#include <QDebug>
#include "nfcrecordmodel.h"
#include "nfcrecorditem.h"
#include <QNdefMessage>
#include <QNdefRecord>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcrecords/ndefnfcsprecord.h"
#include "ndefnfcrecords/ndefnfcmimeimagerecord.h"
#include "ndefnfcrecords/ndefnfcsmarturirecord.h"
#include "ndefnfcrecords/ndefnfcgeorecord.h"
#include "ndefnfcrecords/ndefnfcsmsrecord.h"
#include "ndefnfcrecords/ndefnfcsocialrecord.h"
#include "ndefnfcrecords/ndefnfcstorelinkrecord.h"
#include "ndefnfcrecords/ndefnfclaunchapprecord.h"
#include "ndefnfcrecords/ndefnfcandroidapprecord.h"
#include "nfcstats.h"

// Contact handling
#include "ndefnfcrecords/ndefnfcmimevcardrecord.h"
#include <QContact>
#include <QContactDetail>
#include <QContactName>
#include <QContactNickname>
#include <QContactPhoneNumber>
#include <QContactEmailAddress>
#include <QContactUrl>
#include <QContactOrganization>
#include <QContactAddress>
#include <QContactNote>
#include <QContactBirthday>
#include <QContactThumbnail>

/*!
  \brief Builds an NDEF message based on the data stored in the NfcRecordModel.
  */
class NfcModelToNdef : public QObject
{
    Q_OBJECT

public:
    explicit NfcModelToNdef(QList<NfcRecordItem*> &nfcRecordItems, QObject *parent = 0);
    void setNfcStats(NfcStats* nfcStats);
    QNdefMessage * convertToNdefMessage();

private:
    NdefNfcSpRecord *convertSpFromModel(const int startIndex, int &endIndex);
    QNdefNfcUriRecord *convertUriFromModel(const int startIndex, int &endIndex, bool expectHeader = true);
    QNdefNfcTextRecord *convertTextFromModel(const int startIndex, int &endIndex, bool expectHeader = true);
    NdefNfcMimeImageRecord *convertImageFromModel(const int startIndex, int &endIndex, bool expectHeader = true);
    NdefNfcSmsRecord *convertSmsFromModel(const int startIndex, int& endIndex);
    NdefNfcMimeVcardRecord *convertBusinessCardFromModel(const int startIndex, int &endIndex);
    template<class T> bool contactSetDetail(QContact &contact, const NfcTypes::RecordContent contentType, const QString &value);
    NdefNfcSocialRecord *convertSocialNetworkFromModel(const int startIndex, int &endIndex);
    NdefNfcGeoRecord *convertGeoFromModel(const int startIndex, int& endIndex);
    NdefNfcStoreLinkRecord *convertStoreFromModel(const int startIndex, int &endIndex);
    NdefNfcStoreLinkRecord::AppStore appStoreFromRecordContentType(const NfcTypes::RecordContent contentType);
    QNdefRecord *convertCustomFromModel(const int startIndex, int &endIndex);
    NdefNfcLaunchAppRecord *convertLaunchAppFromModel(const int startIndex, int &endIndex);
    NdefNfcAndroidAppRecord *convertAndroidAppRecordFromModel(const int startIndex, int &endIndex);

private:
    QList<NfcRecordItem*> &m_recordItems;    // Not owned by this class
    /*! Count the number of tags read and messages written. (Not owned by this class) */
    NfcStats* m_nfcStats;

};

#endif // NFCMODELTONDEF_H
