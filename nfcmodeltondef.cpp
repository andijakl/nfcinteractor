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

#include "nfcmodeltondef.h"

/*!
  \brief Construct a new converter, based on the list of NfcRecordItems, as managed
  by the NfcRecordModel.
  */
NfcModelToNdef::NfcModelToNdef(QList<NfcRecordItem*> &nfcRecordItems, QObject *parent) :
    QObject(parent),
    m_recordItems(nfcRecordItems)
{
}

void NfcModelToNdef::setNfcStats(NfcStats *nfcStats)
{
    m_nfcStats = nfcStats;
}


/*!
  \brief Convert the current data in the record model to an NDEF message.
  */
QNdefMessage* NfcModelToNdef::convertToNdefMessage()
{
    QNdefMessage* ndefMessage = new QNdefMessage();

    NfcTypes::MessageType curMessageType;
    NfcTypes::RecordContent curRecordContent;
    int curRecordIndex = 0;
    if (m_nfcStats) {
        m_nfcStats->resetComposedMsgCount();
    }
    //qDebug() << "NfcModelToNdef::convertToNdefMessage() Record item array size: " << recordItems.size();
    // Go through array and convert the data to Ndef Records
    while(curRecordIndex < m_recordItems.size())
    {
        NfcRecordItem* curItem = m_recordItems[curRecordIndex];
        curRecordContent = curItem->recordContent();
        curMessageType = curItem->messageType();

        // If we find a new record header, check the message type
        // and convert this to a QNdefRecord, which is appended to the
        // ndefMessage.
        if (curRecordContent == NfcTypes::RecordHeader) {
            // Starting a new Ndef Record
            int parseEndIndex = -1;
            curMessageType = curItem->messageType();
            if (m_nfcStats) {
                m_nfcStats->incComposedMsgCount(curMessageType);
            }
            switch (curMessageType) {
            case NfcTypes::MsgSmartPoster:
                ndefMessage->append(*convertSpFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgUri:
                ndefMessage->append(*convertUriFromModel(curRecordIndex, parseEndIndex, true));
                break;
            case NfcTypes::MsgText:
                ndefMessage->append(*convertTextFromModel(curRecordIndex, parseEndIndex, true));
                break;
            case NfcTypes::MsgSms:
                ndefMessage->append(*convertSmsFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgBusinessCard:
                ndefMessage->append(*convertBusinessCardFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgSocialNetwork:
                ndefMessage->append(*convertSocialNetworkFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgGeo:
                ndefMessage->append(*convertGeoFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgStore:
                ndefMessage->append(*convertStoreFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgImage:
                ndefMessage->append(*convertImageFromModel(curRecordIndex, parseEndIndex, true));
                break;
            case NfcTypes::MsgCustom:
                ndefMessage->append(*convertCustomFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgLaunchApp:
                ndefMessage->append(*convertLaunchAppFromModel(curRecordIndex, parseEndIndex));
                break;
            case NfcTypes::MsgAndroidAppRecord:
                ndefMessage->append(*convertAndroidAppRecordFromModel(curRecordIndex, parseEndIndex));
                break;
            default:
                // MsgAnnotatedUrl, MsgCombination and a few others
                // are just templates to add multiple records
                // at once and don't exist as a type in the final model.
                qDebug() << "Warning: don't know how to handle this message type in NfcModelToNdef::convertToNdefMessage().";
                break;
            }

            if (parseEndIndex == curRecordIndex || parseEndIndex == -1) {
                // Record wasn't parsed
                // Skip it
                qDebug() << "Warning: unable to parse record " << curRecordIndex;
                curRecordIndex++;
            } else {
                // Parse end index is set - jump over the already parsed items.
                // parseEndIndex is the index of the next item to be looked at,
                // so no need to increment here.
                curRecordIndex = parseEndIndex;
            }
        } else {
            // It shouldn't be possible to have a different record type without
            // having a header item before, so this would be an error in the UI
            qDebug() << "Error in model: new message type without header item at index " << curRecordIndex << " (can be the case while deleting a record)";
            curRecordIndex++;
        }
    }
    return ndefMessage;
}

NdefNfcSpRecord* NfcModelToNdef::convertSpFromModel(const int startIndex, int& endIndex)
{
    NdefNfcSpRecord* newRecord = new NdefNfcSpRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgSmartPoster ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordUri: {
            newRecord->setUri(*convertUriFromModel(curIndex, curIndex, false));
            break; }
        case NfcTypes::RecordText:
        case NfcTypes::RecordTextLanguage: {
            newRecord->addTitle(*convertTextFromModel(curIndex, curIndex, false));
            break; }
        case NfcTypes::RecordSpAction: {
            NdefNfcSpRecord::NfcAction selectedAction = NdefNfcSpRecord::RFU;
            switch (curItem->selectedOption()) {
            case 0:
                selectedAction = NdefNfcSpRecord::DoAction;
                break;
            case 1:
                selectedAction = NdefNfcSpRecord::SaveForLater;
                break;
            case 2:
                selectedAction = NdefNfcSpRecord::OpenForEditing;
                break;
            }
            newRecord->setAction(selectedAction);
            curIndex ++;
            break; }
        case NfcTypes::RecordSpSize:
            newRecord->setSize(curItem->currentText().toUInt());
            curIndex ++;
            break;
        case NfcTypes::RecordSpType:
            newRecord->setMimeType(curItem->currentText());
            curIndex ++;
            break;
        case NfcTypes::RecordImageFilename: {
            newRecord->setImage(*convertImageFromModel(curIndex, curIndex, false));
            break; }
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    //qDebug() << "Sp payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    return newRecord;
}

QNdefNfcUriRecord* NfcModelToNdef::convertUriFromModel(const int startIndex, int& endIndex, bool expectHeader)
{
    QNdefNfcUriRecord* newRecord = new QNdefNfcUriRecord();
    int curIndex = startIndex;
    if (expectHeader) {
        if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgUri ||
                m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
            return newRecord;
        }
        // Start at the next item after the header
        curIndex = startIndex + 1;
    }
    if (curIndex >= m_recordItems.size())
        return newRecord;
    NfcRecordItem* curItem = m_recordItems[curIndex];
    newRecord->setUri(QUrl(curItem->currentText()));
    endIndex = curIndex + 1;
    return newRecord;
}


QNdefNfcTextRecord* NfcModelToNdef::convertTextFromModel(const int startIndex, int& endIndex, bool expectHeader)
{
    QNdefNfcTextRecord* newRecord = new QNdefNfcTextRecord();
    int curIndex = startIndex;
    if (expectHeader) {
        if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgText ||
                m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
            return newRecord;
        }
        // Start at the next item after the header
        curIndex = startIndex + 1;
    }

    bool reachedRecordEnd = false;
    bool foundText = false;
    bool foundLocale = false;
    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordText:
            if (foundText) {
                reachedRecordEnd = true;
            } else {
                newRecord->setText(curItem->currentText());
                foundText = true;
            }
            break;
        case NfcTypes::RecordTextLanguage:
            if (foundLocale) {
                reachedRecordEnd = true;
            } else {
                newRecord->setLocale(curItem->currentText());
                foundLocale = true;
            }
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        curIndex ++;
    }
    endIndex = curIndex;
    return newRecord;
}

NdefNfcMimeImageRecord* NfcModelToNdef::convertImageFromModel(const int startIndex, int& endIndex, bool expectHeader)
{
    NdefNfcMimeImageRecord* newRecord = new NdefNfcMimeImageRecord();
    int curIndex = startIndex;
    if (expectHeader) {
        if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgImage ||
                m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
            return newRecord;
        }
        // Start at the next item after the header
        curIndex = startIndex + 1;
    }
    if (curIndex >= m_recordItems.size())
        return newRecord;
    NfcRecordItem* curItem = m_recordItems[curIndex];
    if (newRecord->setImage(curItem->currentText())) {
        // Loading the image was successful
    } else {
        // Not successful in loading the image
        qDebug() << "Error loading the image for creating the payload.";
    }
    endIndex = curIndex + 1;

    return newRecord;
}


NdefNfcSmsRecord* NfcModelToNdef::convertSmsFromModel(const int startIndex, int& endIndex)
{
    NdefNfcSmsRecord* newRecord = new NdefNfcSmsRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgSms ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    // Need to find out all the info beforehand, in order to decide on
    // whether to write an URI record or a Smart Poster
    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordPhoneNumber:
            newRecord->setSmsNumber(curItem->currentText());
            curIndex ++;
            break;
        case NfcTypes::RecordSmsBody:
            newRecord->setSmsBody(curItem->currentText());
            curIndex ++;
            break;
        case NfcTypes::RecordText:
        case NfcTypes::RecordTextLanguage:
            newRecord->addTitle(*convertTextFromModel(curIndex, curIndex, false));
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;

    //qDebug() << "Sms payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    //qDebug() << "Is Sp: " << newRecord->isSp();
    return newRecord;
}


NdefNfcMimeVcardRecord* NfcModelToNdef::convertBusinessCardFromModel(const int startIndex, int& endIndex)
{
    NdefNfcMimeVcardRecord* newRecord = new NdefNfcMimeVcardRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgBusinessCard ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    QContact contact;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        QString value = curItem->currentText();
        const NfcTypes::RecordContent contentType = curItem->recordContent();
        switch (contentType) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordNamePrefix:
        case NfcTypes::RecordFirstName:
        case NfcTypes::RecordMiddleName:
        case NfcTypes::RecordLastName:
        case NfcTypes::RecordNameSuffix:
            contactSetDetail<QContactName>(contact, contentType, value);
            break;
        case NfcTypes::RecordNickname:
            contactSetDetail<QContactNickname>(contact, contentType, value);
            break;
        case NfcTypes::RecordPhoneNumber:
            contactSetDetail<QContactPhoneNumber>(contact, contentType, value);
            break;
        case NfcTypes::RecordEmailAddress:
            contactSetDetail<QContactEmailAddress>(contact, contentType, value);
            break;
        case NfcTypes::RecordContactUrl:
            contactSetDetail<QContactUrl>(contact, contentType, value);
            break;
        case NfcTypes::RecordOrganizationName:
        case NfcTypes::RecordOrganizationDepartment:
        case NfcTypes::RecordOrganizationRole:
        case NfcTypes::RecordOrganizationTitle:
            contactSetDetail<QContactOrganization>(contact, contentType, value);
            break;
        case NfcTypes::RecordBirthday:
            contactSetDetail<QContactBirthday>(contact, contentType, value);
            break;
        case NfcTypes::RecordNote:
            contactSetDetail<QContactNote>(contact, contentType, value);
            break;
        case NfcTypes::RecordImageFilename:
            contactSetDetail<QContactThumbnail>(contact, contentType, value);
            break;
        case NfcTypes::RecordCountry:
        case NfcTypes::RecordLocality:
        case NfcTypes::RecordPostOfficeBox:
        case NfcTypes::RecordPostcode:
        case NfcTypes::RecordRegion:
        case NfcTypes::RecordStreet:
            contactSetDetail<QContactAddress>(contact, contentType, value);
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        curIndex ++;
    }
    endIndex = curIndex;
    newRecord->setContact(contact);
    //qDebug() << "Contact payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    return newRecord;
}

template<class T> bool NfcModelToNdef::contactSetDetail(QContact& contact, const NfcTypes::RecordContent contentType, const QString& value)
{
    // Well, should work with the following way:
    //QContactName curDetail = contact.detail<QContactName>();
    // But then, the definitionName() of the contact detail isn't set
    // properly, causing the detail not to be written to the vCard.
    // Therefore the workaround with the template for the specific
    // detail class, and then a conversion to the generic QContactDetail
    // through its constructor.
    T curDetailSpecific = contact.detail<T>();
    QContactDetail curDetail(curDetailSpecific);
    const char* contactField;
    switch (contentType) {
    case NfcTypes::RecordNamePrefix:
        contactField = QContactName::FieldPrefix.latin1();
        break;
    case NfcTypes::RecordFirstName:
        contactField = QContactName::FieldFirstName.latin1();
        break;
    case NfcTypes::RecordMiddleName:
        contactField = QContactName::FieldMiddleName.latin1();
        break;
    case NfcTypes::RecordLastName:
        contactField = QContactName::FieldLastName.latin1();
        break;
    case NfcTypes::RecordNameSuffix:
        contactField = QContactName::FieldSuffix.latin1();
        break;
    case NfcTypes::RecordNickname:
        contactField = QContactNickname::FieldNickname.latin1();
        break;
    case NfcTypes::RecordPhoneNumber:
        contactField = QContactPhoneNumber::FieldNumber.latin1();
        break;
    case NfcTypes::RecordEmailAddress:
        contactField = QContactEmailAddress::FieldEmailAddress.latin1();
        break;
    case NfcTypes::RecordContactUrl:
        contactField = QContactUrl::FieldUrl.latin1();
        break;
    case NfcTypes::RecordOrganizationName:
        contactField = QContactOrganization::FieldName.latin1();
        break;
    case NfcTypes::RecordOrganizationDepartment:
        contactField = QContactOrganization::FieldDepartment.latin1();
        break;
    case NfcTypes::RecordOrganizationRole:
        contactField = QContactOrganization::FieldRole.latin1();
        break;
    case NfcTypes::RecordOrganizationTitle:
        contactField = QContactOrganization::FieldTitle.latin1();
        break;
    case NfcTypes::RecordBirthday:
        contactField = QContactBirthday::FieldBirthday.latin1();
        break;
    case NfcTypes::RecordNote:
        contactField = QContactNote::FieldNote.latin1();
        break;
    case NfcTypes::RecordImageFilename:
        contactField = QContactThumbnail::FieldThumbnail.latin1();
        break;
    case NfcTypes::RecordCountry:
        contactField = QContactAddress::FieldCountry.latin1();
        break;
    case NfcTypes::RecordLocality:
        contactField = QContactAddress::FieldLocality.latin1();
        break;
    case NfcTypes::RecordPostOfficeBox:
        contactField = QContactAddress::FieldPostOfficeBox.latin1();
        break;
    case NfcTypes::RecordPostcode:
        contactField = QContactAddress::FieldPostcode.latin1();
        break;
    case NfcTypes::RecordRegion:
        contactField = QContactAddress::FieldRegion.latin1();
        break;
    case NfcTypes::RecordStreet:
        contactField = QContactAddress::FieldStreet.latin1();
        break;
    default:
        qDebug() << "Warning: Unknown contact detail in NfcModelToNdef::contactSetDetail()";
        contactField = "Unknown";
        break;
    }
    if (contentType == NfcTypes::RecordImageFilename) {
        // Image-based detail
        QImage contactImg(value);
        if (!contactImg.isNull()) {
            // Succeeded in loading the image
            curDetail.setValue(contactField, contactImg);
        }
    } else {
        // Text-based detail
        curDetail.setValue(contactField, value);
    }
    return contact.saveDetail(&curDetail);
}

NdefNfcSocialRecord *NfcModelToNdef::convertSocialNetworkFromModel(const int startIndex, int &endIndex)
{
    NdefNfcSocialRecord* newRecord = new NdefNfcSocialRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgSocialNetwork ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordSocialNetworkType: {
            NdefNfcSocialRecord::NfcSocialType socialType = (NdefNfcSocialRecord::NfcSocialType)curItem->selectedOption();
            newRecord->setSocialType(socialType);
            curIndex ++;
            break; }
        case NfcTypes::RecordSocialNetworkName:
            newRecord->setSocialUserName(curItem->currentText());
            curIndex ++;
            break;
        case NfcTypes::RecordText:
        case NfcTypes::RecordTextLanguage:
            newRecord->addTitle(*convertTextFromModel(curIndex, curIndex, false));
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    //qDebug() << "Social payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    //qDebug() << "Is Social == Sp: " << newRecord->isSp();
    return newRecord;
}


NdefNfcGeoRecord *NfcModelToNdef::convertGeoFromModel(const int startIndex, int &endIndex)
{
    NdefNfcGeoRecord* newRecord = new NdefNfcGeoRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgGeo ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordGeoLatitude:
            newRecord->setLatitude(curItem->currentText().toDouble());
            curIndex ++;
            break;
        case NfcTypes::RecordGeoLongitude:
            newRecord->setLongitude(curItem->currentText().toDouble());
            curIndex ++;
            break;
        case NfcTypes::RecordText:
        case NfcTypes::RecordTextLanguage:
            newRecord->addTitle(*convertTextFromModel(curIndex, curIndex, false));
            break;
        case NfcTypes::RecordGeoType: {
            NdefNfcGeoRecord::NfcGeoType geoType = (NdefNfcGeoRecord::NfcGeoType)curItem->selectedOption();
            newRecord->setGeoType(geoType);
            curIndex ++;
            break; }
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    //qDebug() << "Geo payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    //qDebug() << "Is Geo == Sp: " << newRecord->isSp();
    return newRecord;
}


NdefNfcStoreLinkRecord *NfcModelToNdef::convertStoreFromModel(const int startIndex, int &endIndex)
{
    NdefNfcStoreLinkRecord* newRecord = new NdefNfcStoreLinkRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgStore ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordStoreNokia:
        case NfcTypes::RecordStoreSymbian:
        case NfcTypes::RecordStoreMeeGoHarmattan:
        case NfcTypes::RecordStoreSeries40:
        case NfcTypes::RecordStoreWindowsPhone:
        case NfcTypes::RecordStoreAndroid:
        case NfcTypes::RecordStoreiOS:
        case NfcTypes::RecordStoreBlackberry:
        case NfcTypes::RecordStoreCustomName:
            newRecord->addAppId(appStoreFromRecordContentType(curItem->recordContent()), curItem->currentText());
            curIndex ++;
            break;
        case NfcTypes::RecordText:
        case NfcTypes::RecordTextLanguage:
            newRecord->addTitle(*convertTextFromModel(curIndex, curIndex, false));
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    //qDebug() << "Store payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    //qDebug() << "Is Store == Sp: " << newRecord->isSp();
    return newRecord;
}

NdefNfcStoreLinkRecord::AppStore NfcModelToNdef::appStoreFromRecordContentType(const NfcTypes::RecordContent contentType)
{
    switch (contentType) {
    case NfcTypes::RecordStoreNokia:
        return NdefNfcStoreLinkRecord::StoreNokia;
        break;
    case NfcTypes::RecordStoreSymbian:
        return NdefNfcStoreLinkRecord::StoreSymbian;
        break;
    case NfcTypes::RecordStoreMeeGoHarmattan:
        return NdefNfcStoreLinkRecord::StoreMeeGoHarmattan;
        break;
    case NfcTypes::RecordStoreSeries40:
        return NdefNfcStoreLinkRecord::StoreSeries40;
        break;
    case NfcTypes::RecordStoreWindowsPhone:
        return NdefNfcStoreLinkRecord::StoreWindowsPhone;
        break;
    case NfcTypes::RecordStoreAndroid:
        return NdefNfcStoreLinkRecord::StoreAndroid;
        break;
    case NfcTypes::RecordStoreiOS:
        return NdefNfcStoreLinkRecord::StoreiOS;
        break;
    case NfcTypes::RecordStoreBlackberry:
        return NdefNfcStoreLinkRecord::StoreBlackberry;
        break;
    case NfcTypes::RecordStoreCustomName:
        return NdefNfcStoreLinkRecord::StoreCustomName;
        break;
    default:
        qDebug() << "Warning: Unknown record content type passed to NfcModelToNdef::appStoreFromRecordContentType.";
        return NdefNfcStoreLinkRecord::StoreNokia;
        break;
    }
}




QNdefRecord *NfcModelToNdef::convertCustomFromModel(const int startIndex, int &endIndex)
{
    QNdefRecord* newRecord = new NdefNfcGeoRecord();
    newRecord->setTypeNameFormat(QNdefRecord::ExternalRtd);
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgCustom ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;
    bool isEmptyRecord = true;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordTypeNameFormat: {
            QNdefRecord::TypeNameFormat recordTnf = (QNdefRecord::TypeNameFormat)curItem->selectedOption();
            newRecord->setTypeNameFormat(recordTnf);
            curIndex ++;
            break; }
        case NfcTypes::RecordTypeName: {
            newRecord->setType(curItem->currentText().toLatin1());
            curIndex ++;
            break; }
        case NfcTypes::RecordRawPayload:
            newRecord->setPayload(curItem->currentText().toLatin1());
            curIndex ++;
            isEmptyRecord = false;
            break;
        case NfcTypes::RecordId:
            newRecord->setId(curItem->currentText().toLatin1());
            curIndex ++;
            isEmptyRecord = false;
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }

    endIndex = curIndex;
    //qDebug() << "Custom payload: (" << newRecord->payload().count() << "): " << newRecord->payload();
    return newRecord;
}

NdefNfcLaunchAppRecord *NfcModelToNdef::convertLaunchAppFromModel(const int startIndex, int &endIndex)
{
    NdefNfcLaunchAppRecord* newRecord = new NdefNfcLaunchAppRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgLaunchApp ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordLaunchAppArguments:
            newRecord->setArguments(curItem->currentText());
            break;
        case NfcTypes::RecordLaunchAppWindows:
            newRecord->addPlatformAppId("Windows", curItem->currentText());
            break;
        case NfcTypes::RecordLaunchAppWindowsPhone:
            newRecord->addPlatformAppId("WindowsPhone", curItem->currentText());
            break;
        case NfcTypes::RecordLaunchAppPlatform:
            {
                QString platform = curItem->currentText();
                curIndex++;
                if (curIndex < m_recordItems.size()) {
                    QString appId = m_recordItems[curIndex]->currentText();
                    newRecord->addPlatformAppId(platform, appId);
                } else {
                    qDebug() << "Error in record model: LaunchApp platform without ID.";
                    reachedRecordEnd = true;
                }
                break;
            }
        case NfcTypes::RecordLaunchAppId:
            qDebug() << "Error in record model: LaunchApp App ID needs to be preceded by the platform name.";
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    return newRecord;
}

NdefNfcAndroidAppRecord *NfcModelToNdef::convertAndroidAppRecordFromModel(const int startIndex, int &endIndex)
{
    NdefNfcAndroidAppRecord* newRecord = new NdefNfcAndroidAppRecord();
    if (m_recordItems[startIndex]->messageType() != NfcTypes::MsgAndroidAppRecord ||
            m_recordItems[startIndex]->recordContent() != NfcTypes::RecordHeader) {
        return newRecord;
    }
    // Start at the next item after the header
    int curIndex = startIndex + 1;
    bool reachedRecordEnd = false;

    while (curIndex < m_recordItems.size()) {
        NfcRecordItem* curItem = m_recordItems[curIndex];
        switch (curItem->recordContent()) {

        case NfcTypes::RecordHeader:
            // Next record starts - quit!
            reachedRecordEnd = true;
            break;
        case NfcTypes::RecordAndroidPackageName:
            newRecord->setPackageName(curItem->currentText());
            curIndex ++;
            break;
        default:
            // Unknown record content that doesn't belong to this record
            reachedRecordEnd = true;
            break;
        }
        if (reachedRecordEnd)
            break;
        //curIndex ++;  // Already incremented by convert...() methods.
    }
    endIndex = curIndex;
    return newRecord;
}
