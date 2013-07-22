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

#include "nfcrecorddefaults.h"

NfcRecordDefaults::NfcRecordDefaults(QObject *parent) :
    QObject(parent)
{
}

QString NfcRecordDefaults::itemHeaderTextDefault(const NfcTypes::MessageType messageType)
{
    QString itemText = "";
    switch (messageType) {
    case NfcTypes::MsgSmartPoster:
        itemText = "Smart Poster Record";
        break;
    case NfcTypes::MsgUri:
        itemText = "URI Record";
        break;
    case NfcTypes::MsgText:
        itemText = "Text Record";
        break;
    case NfcTypes::MsgSms:
        itemText = "SMS Record";
        break;
    case NfcTypes::MsgBusinessCard:
        itemText = "Business Card";
        break;
    case NfcTypes::MsgSocialNetwork:
        itemText = "Social Network";
        break;
    case NfcTypes::MsgGeo:
        itemText = "Geo Record";
        break;
    case NfcTypes::MsgStore:
        itemText = "Store Record";
        break;
    case NfcTypes::MsgImage:
        itemText = "Image Record";
        break;
    case NfcTypes::MsgCustom:
        itemText = "Custom Record";
        break;
    case NfcTypes::MsgLaunchApp:
        itemText = "LaunchApp";
        break;
    case NfcTypes::MsgAndroidAppRecord:
        itemText = "Android App Record";
        break;
    default:
        qDebug() << "Unknown message type " << messageType << " in simpleAddRecordHeaderItem()";
        break;
    }

    return itemText;
}


/*!
  \brief Get default values for a new record item that is part of a specific message type.
  */
void NfcRecordDefaults::itemContentDefault(const NfcTypes::MessageType msgType, const NfcTypes::RecordContent contentType, QString& defaultTitle, QString& defaultContents)
{
    defaultTitle = "";
    defaultContents = "";
    switch (contentType) {
    case NfcTypes::RecordUri:
        defaultTitle = "URI";
        defaultContents = "http://nokia.com";
        break;
    case NfcTypes::RecordText: {
        if (msgType == NfcTypes::MsgText) {
            defaultTitle = "Text";
        } else {
            defaultTitle = "Title text";
        }
        switch (msgType) {
        case NfcTypes::MsgSms:
            defaultContents = "Send SMS";
            break;
        case NfcTypes::MsgSocialNetwork:
            defaultContents = "Follow me";
            break;
        case NfcTypes::MsgGeo:
            defaultContents = "View location";
            break;
        case NfcTypes::MsgStore:
            defaultContents = "Download app";
            break;
        default:
            defaultContents = "Nokia";
        }
        break; }
    case NfcTypes::RecordTextLanguage:
        defaultTitle = "Language";
        defaultContents = "en";
        break;
    case NfcTypes::RecordSmsBody:
        defaultTitle = "SMS Body";
        defaultContents = "Hello";
        break;
    case NfcTypes::RecordSpAction:
        defaultTitle = "Action";
        // Selection item - no default contents string
        break;
    case NfcTypes::RecordImageFilename:
        defaultTitle = "Image";
        break;
    case NfcTypes::RecordSpType:
        defaultTitle = "Type of linked content";
        defaultContents = "text/html";
        break;
    case NfcTypes::RecordSpSize:
        defaultTitle = "Size of linked content";
        defaultContents = "0";
        break;
    case NfcTypes::RecordTypeNameFormat:
        defaultTitle = "Type Name Format";
        // Selection item - no default contents string
        break;
    case NfcTypes::RecordTypeName:
        defaultTitle = "Type Name";
        defaultContents = "nokia.com:custom";
        break;
    case NfcTypes::RecordId:
        defaultTitle = "Id";
        break;
    case NfcTypes::RecordRawPayload:
        defaultTitle = "Raw Payload";
        defaultContents = "Nokia";
        break;
    case NfcTypes::RecordSocialNetworkType:
        defaultTitle = "Social Network";
        break;
    case NfcTypes::RecordSocialNetworkName:
        defaultTitle = "User name / ID";
        defaultContents = "mopius";
        break;
        // ----------------------------------------------------------------
        // Contacts
    case NfcTypes::RecordNamePrefix:
        defaultTitle = "Name Prefix";
        break;
    case NfcTypes::RecordFirstName:
        defaultTitle = "First name";
        defaultContents = "Joe";
        break;
    case NfcTypes::RecordMiddleName:
        defaultTitle = "Middle name";
        break;
    case NfcTypes::RecordLastName:
        defaultTitle = "Last name";
        defaultContents = "Bloggs";
        break;
    case NfcTypes::RecordNameSuffix:
        defaultTitle = "Name suffix";
        defaultContents = "";
        break;
    case NfcTypes::RecordNickname:
        defaultTitle = "Nickname";
        break;
    case NfcTypes::RecordEmailAddress:
        defaultTitle = "Email address";
        defaultContents = "joe.bloggs@nokia.com";
        break;
    case NfcTypes::RecordPhoneNumber:
        defaultTitle = "Phone number";
        defaultContents = "+1234";
        break;
    case NfcTypes::RecordContactUrl:
        defaultTitle = "URL";
        defaultContents = "http://developer.nokia.com/";
        break;
    case NfcTypes::RecordOrganizationName:
        defaultTitle = "Organization name";
        break;
    case NfcTypes::RecordOrganizationDepartment:
        defaultTitle = "Organization department";
        break;
    case NfcTypes::RecordOrganizationRole:
        defaultTitle = "Organization role";
        break;
    case NfcTypes::RecordOrganizationTitle:
        defaultTitle = "Organization title";
        break;
    case NfcTypes::RecordBirthday:
        defaultTitle = "Birthday (YYYY-MM-DD)";
        defaultContents = "1980-03-25";
        break;
    case NfcTypes::RecordNote:
        defaultTitle = "Note";
        break;
    case NfcTypes::RecordCountry:
        defaultTitle = "Country";
        break;
    case NfcTypes::RecordLocality:
        defaultTitle = "Locality";
        break;
    case NfcTypes::RecordPostOfficeBox:
        defaultTitle = "Post Office Box";
        break;
    case NfcTypes::RecordPostcode:
        defaultTitle = "Postcode";
        break;
    case NfcTypes::RecordRegion:
        defaultTitle = "Region";
        break;
    case NfcTypes::RecordStreet:
        defaultTitle = "Street";
        break;
        // ----------------------------------------------------------------
        // Geo
    case NfcTypes::RecordGeoType:
        defaultTitle = "Geo Tag Type";
        // Selection item - no default contents string
        break;
    case NfcTypes::RecordGeoLatitude:
        defaultTitle = "Latitude (dec deg., WGS-84)";
        defaultContents = "60.17";
        break;
    case NfcTypes::RecordGeoLongitude:
        defaultTitle = "Longitude (dec deg., WGS-84)";
        defaultContents = "24.829";
        break;
        // ----------------------------------------------------------------
        // Store
    case NfcTypes::RecordStoreNokia:
        defaultTitle = "Nokia Store generic ID";
        // TODO: add id of Nfc Interactor by default, once known
        break;
    case NfcTypes::RecordStoreSymbian:
        defaultTitle = "Symbian ID in Nokia Store";
        defaultContents = "184295";
        break;
    case NfcTypes::RecordStoreMeeGoHarmattan:
        defaultTitle = "MeeGo ID in Nokia Store";
        defaultContents = "214283";
        break;
    case NfcTypes::RecordStoreSeries40:
        defaultTitle = "Series 40 ID in Nokia Store";
        break;
    case NfcTypes::RecordStoreWindowsPhone:
        defaultTitle = "Windows Phone Marketplace ID";
        break;
    case NfcTypes::RecordStoreAndroid:
        defaultTitle = "Android Marketplace ID";
        break;
    case NfcTypes::RecordStoreiOS:
        defaultTitle = "iOS App Store ID";
        break;
    case NfcTypes::RecordStoreBlackberry:
        defaultTitle = "BlackBerry App World ID";
        break;
    case NfcTypes::RecordStoreCustomName:
        defaultTitle = "Registered app name";
        defaultContents = "ni";
        break;
    // ----------------------------------------------------------------
    // LaunchApp
    case NfcTypes::RecordLaunchAppArguments:
        defaultTitle = "Arguments";
        defaultContents = "user=default";
        break;
    case NfcTypes::RecordLaunchAppWindows:
        defaultTitle = "Windows proximity app ID";
        defaultContents = "";
        break;
    case NfcTypes::RecordLaunchAppWindowsPhone:
        defaultTitle = "Windows Phone Product ID";
        defaultContents = "{5e506af4-4586-4c90-bc5f-428b12cf48bc}";
        break;
    case NfcTypes::RecordLaunchAppPlatform:
        defaultTitle = "Custom platform name";
        defaultContents = "";
        break;
    case NfcTypes::RecordLaunchAppId:
        defaultTitle = "Custom platform app ID";
        defaultContents = "";
        break;
    // ----------------------------------------------------------------
    // Android Application Record
    case NfcTypes::RecordAndroidPackageName:
        defaultTitle = "Package name";
        defaultContents = "com.twitter.android";
        break;
    default:
        qDebug() << "Warning: don't have defaults for requested content type in NfcRecordModel::getDefaultsForRecordContent().";
        break;
    }
}

/*!
  \brief Get possible selection items for a new record item.

  \param contentType record content type to get the possible selection items for
  \param defaultSelectedItem which item should be selected by default
  */
QVariantList NfcRecordDefaults::itemSelectionDefault(const NfcTypes::RecordContent contentType, int& defaultSelectedItem)
{
    QVariantList selectionItems;
    defaultSelectedItem = 0;
    switch (contentType) {
    case NfcTypes::RecordSpAction:
        selectionItems << "Do the action";
        selectionItems << "Save for later";
        selectionItems << "Open for editing";
        break;
    case NfcTypes::RecordGeoType:
        selectionItems << "geo: URI scheme";
        selectionItems << "Nokia Maps link";
        selectionItems << "Generic redirect NfcInteractor.com";
        break;
    case NfcTypes::RecordSocialNetworkType:
        selectionItems << "Twitter";
        selectionItems << "LinkedIn";
        selectionItems << "Facebook";
        selectionItems << "Xing";
        // vKontakte.ru
        selectionItems << QString::fromWCharArray(L"\x0412\x041A\x043E\x043D\x0442\x0430\x043A\x0442\x0435");
        selectionItems << "Foursquare";
        selectionItems << "Skype";
        break;
    case NfcTypes::RecordTypeNameFormat:
        selectionItems << "Empty";
        selectionItems << "NfcRtd";
        selectionItems << "Mime";
        selectionItems << "Uri";
        selectionItems << "ExternalRtd";
        selectionItems << "Unknown";
        defaultSelectedItem = 4;
        break;
    default:
        qDebug() << "Warning: don't have defaults for requested content type in NfcRecordModel::getDefaultSelectionItemsForRecordContent().";
        break;
    }
    return selectionItems;
}
