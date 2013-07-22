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

#include "nfcndefparser.h"

NfcNdefParser::NfcNdefParser(NfcRecordModel* nfcRecordModel, QObject *parent) :
    QObject(parent),
    m_parseToModel(false),
    m_nfcRecordModel(nfcRecordModel)
{
}


/*!
  \brief Set the image cache to use for storing images retrieved
  from tags.
  \a tagImageCache instance of the image cache. This
  class will not take ownership of the instance!
  */
void NfcNdefParser::setImageCache(TagImageCache *tagImageCache)
{
    // Not owned by this class
    m_imgCache = tagImageCache;
}

void NfcNdefParser::setAppSettings(AppSettings *appSettings)
{
    m_appSettings = appSettings;
}

/*!
  \brief If enabled, additionally parse the contents of the NDEF
  message to the record model.
  */
void NfcNdefParser::setParseToModel(bool parseToModel)
{
    m_parseToModel = parseToModel;
}

/*!
  \brief Parse the NDEF \a message and return its contents in human-readable
  textual format.
  */
QString NfcNdefParser::parseNdefMessage(const QNdefMessage &message)
{
    if (message.isEmpty()) {
        return QString("No records in the Ndef message");
    }

    // Total string that will contain the parsed contents.
    QString tagContents;
    m_clipboardContents = ClipboardEmpty;

    // Message size
    QByteArray rawMessage = message.toByteArray();
    const int msgSize = rawMessage.size();
    if (msgSize > 0) {
        tagContents.append("Message size: " + QString::number(msgSize) + " bytes\n");
    }

    // Clear model if set to parse to the record model as well
    if (m_parseToModel) {
        m_nfcRecordModel->clear();
    }

    // Go through all records in the message
    const int recordCount = message.size();
    int numRecord = 1;
    foreach (const QNdefRecord &record, message)
    {
        if (recordCount > 1) {
            // More than one record in the message?
            // -> show which one we're parsing now.
            if (numRecord > 1) {
                tagContents.append("\n");
            }
            tagContents.append("Record " + QString::number(numRecord) + "/" + QString::number(recordCount) + "\n");
        }

        // Print generic information about the record
        tagContents.append("Type name: " + convertRecordTypeNameToString(record.typeNameFormat()) + "\n");
        tagContents.append("Record type: " + QString(record.type()) + " ");

        // Parse tag contents
        if (record.isRecordType<QNdefNfcUriRecord>())
        {
            // ------------------------------------------------
            // URI
            tagContents.append(parseUriRecord(QNdefNfcUriRecord(record)));
        }
        else if (record.isRecordType<QNdefNfcTextRecord>())
        {
            // ------------------------------------------------
            // Text
            tagContents.append(parseTextRecord(QNdefNfcTextRecord(record)));
        }
        else if (record.isRecordType<NdefNfcSpRecord>())
        {
            // ------------------------------------------------
            // Smart Poster (urn:nfc:wkt:Sp)
            tagContents.append(parseSpRecord(NdefNfcSpRecord(record)));
        }
        else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/"))
        {
            // The NdefNfcMimeImageRecord class handles many different
            // record types. To simplify the use, there is no extra
            // class for each individual image type (image/png, gif,
            // jpg, jpeg, etc.)
            // Therefore, the isRecordType<>() method can't be used
            // with the generic image handler class.
            // ------------------------------------------------
            // Image (any supported type)
            tagContents.append(parseImageRecord(NdefNfcMimeImageRecord(record)));
        }
        else if (record.isRecordType<NdefNfcMimeVcardRecord>())
        {
            // ------------------------------------------------
            // Mime type: vCard
            NdefNfcMimeVcardRecord vCardRecord(record);
            tagContents.append(parseVcardRecord(vCardRecord));
        }
        else if (record.isRecordType<NdefNfcLaunchAppRecord>())
        {
            // ------------------------------------------------
            // LaunchApp Record
            tagContents.append(parseLaunchAppRecord(NdefNfcLaunchAppRecord(record)));
        }
        else if (record.isRecordType<NdefNfcAndroidAppRecord>())
        {
            // ------------------------------------------------
            // Android Application Record
            tagContents.append(parseAndroidAppRecord(NdefNfcAndroidAppRecord(record)));
        }
        else if (record.typeNameFormat() == QNdefRecord::ExternalRtd) {
            // ------------------------------------------------
            // External type according to NFC RTD
            tagContents.append(parseCustomRecord(record));
        }
        else if (record.isEmpty())
        {
            // ------------------------------------------------
            // Empty
            tagContents.append("[Empty record]\n");
        }
        else
        {
            // ------------------------------------------------
            // Record type not parsed by this class
            tagContents.append("\nRaw payload: ");
            if (record.payload().isNull() || record.payload().isEmpty()) {
                tagContents.append("[Empty]");
            } else {
                tagContents.append(record.payload());
            }
            tagContents.append("\n");
        }
        numRecord++;
    }

    // If we found records that can be stored in a clipboard,
    // apply the contents now.
    setStoredClipboard();
    return tagContents;
}


/*!
  \brief Create a textual description of the contents of the
  Uri (U) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseUriRecord(const QNdefNfcUriRecord& record)
{
    QString tagContents("[URI]\n");
    tagContents.append(record.uri().toString());
    storeClipboard(record.uri());
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgUri, false);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordUri, record.uri().toString(), false);
    }
    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  Text (T) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseTextRecord(const QNdefNfcTextRecord& record)
{
    QString tagContents("[Text]\n");
    // Add the text info to the string, parsed by an extra method
    // as the same content is also present for example in the Smart Poster.
    tagContents.append(textRecordToString(record));
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgText, false);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordText, record.text(), false);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordTextLanguage, record.locale(), false);
    }
    return tagContents;
}

/*!
  \brief Convert the details of a text record to a string.

  Seperated from parseTextRecord() as this is also used by the
  Smart Poster parser.
  */
QString NfcNdefParser::textRecordToString(const QNdefNfcTextRecord& textRecord)
{
    QString txt("Title: " + textRecord.text() + "\n");
    txt.append("Locale: " + textRecord.locale() + "\n");
    const QString textEncoding = textRecord.encoding() == QNdefNfcTextRecord::Utf8 ? "UTF-8" : "UTF-16";
    txt.append("Encoding: " + textEncoding + "\n");
    storeClipboard(textRecord.text(), textRecord.locale());
    return txt;
}

/*!
  \brief Create a textual description of the contents of the
  Smart Poster (Sp) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseSpRecord(const NdefNfcSpRecord& record)
{
    QString tagContents("[Smart Poster]\n");
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgSmartPoster, true);
    }

    // Uri
    tagContents.append("Uri: " + record.uri().toString() + "\n");
    if (m_parseToModel) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordUri, record.uri().toString(), false);
    }

    // Title
    tagContents.append("Title count: " + QString::number(record.titleCount()) + "\n");
    if (record.titleCount() > 0)
    {
        foreach (QNdefNfcTextRecord curTitle, record.titles()) {
            tagContents.append(textRecordToString(curTitle));
            if (m_parseToModel) {
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordText, curTitle.text(), true);
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordTextLanguage, curTitle.locale(), false);
            }
        }
    }

    // Action
    if (record.actionInUse())
    {
        QString spActionString = "Unknown";
        switch (record.action())
        {
        case NdefNfcSpRecord::DoAction:
            spActionString = "Do Action";
            break;
        case NdefNfcSpRecord::SaveForLater:
            spActionString = "Save for later";
            break;
        case NdefNfcSpRecord::OpenForEditing:
            spActionString = "Open for editing";
            break;
        case NdefNfcSpRecord::RFU:
            spActionString = "RFU";
            break;
        }
        tagContents.append("Action: " + spActionString + "\n");
        if (m_parseToModel) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordSpAction, QString::number((int)record.action()), true);
        }
    }

    // Size
    if (record.sizeInUse())
    {
        tagContents.append("Size: " + QString::number(record.size()) + "\n");
        if (m_parseToModel) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordSpSize, QString::number(record.size()), true);
        }
    }

    // Type
    if (record.mimeTypeInUse())
    {
        tagContents.append("Type: " + record.mimeType() + "\n");
        if (m_parseToModel) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordSpType, record.mimeType(), true);
        }
    }

    // Image
    if (record.imageInUse())
    {
        NdefNfcMimeImageRecord spImageRecord = record.image();
        QByteArray imgFormat = spImageRecord.format();
        if (!imgFormat.isEmpty()) {
            tagContents.append("Image format: " + imgFormat + "\n");
        }
        QImage spImage = spImageRecord.image();
        if (!spImage.isNull())
        {
            if (m_imgCache) {
                const int imgId = m_imgCache->addImage(spImage);
                qDebug() << "Stored image into cache, id: " << imgId;
                emit nfcTagImage(imgId);
            } else {
                qDebug() << "Image cache not set";
            }
        }
        if (m_parseToModel) {
            storeImageToFileForModel(spImageRecord, true);
        }
    }

    // Raw contents
    //tagContents.append(record.rawContents());
    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  image mime type record.

  The parsing works regardless of the actual image format used and
  supports all image formats available to Qt.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseImageRecord(const NdefNfcMimeImageRecord& record)
{
    QString tagContents("[Image]\n");
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgImage, true);
    }

    // Read image format (png, gif, jpg, etc.)
    QByteArray imgFormat = record.format();
    if (!imgFormat.isEmpty()) {
        tagContents.append("Format: " + imgFormat + "\n");
    }

    // Retrieve the image
    QImage img = record.image();
    if (!img.isNull()) {

        // Image size
        const QSize imgSize = img.size();
        tagContents.append("Width: " + QString::number(imgSize.width()) + ", height: " + QString::number(imgSize.height()));

        // Store the image in the cache to show it on the screen
        if (m_imgCache) {
            const int imgId = m_imgCache->addImage(img);
            qDebug() << "Stored image into cache, id: " << imgId;
            emit nfcTagImage(imgId);
        } else {
            tagContents.append("Error: Image cache not set\n");
        }
    }

    if (m_parseToModel) {
        storeImageToFileForModel(record, false);
    }

    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  VCard record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseVcardRecord(NdefNfcMimeVcardRecord& record)
{
    QString tagContents("[vCard]\n");
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgBusinessCard, true);
    }

    // Parse the list of contacts from the record
    QList<QContact> contacts = record.contacts();
    if (!contacts.isEmpty())
    {
        // At least one contact could be successfully retrieved
        // (will in most cases only be one in the NFC scenario)
        foreach (QContact curContact, contacts) {
            //const QString contactDisplayLabel = curContact.displayLabel();
            // Get a list containing all details of the contact, for easier display.
            QList<QContactDetail> details = curContact.details();

            foreach (QContactDetail curDetail, details) {
                // Go through all contact details
                const QString detailName = curDetail.definitionName();

                // Thumbnail
                if (detailName == QContactThumbnail::DefinitionName) {
                    // Special case: image
                    // Fetch the thumbnail and store it in the image cache.
                    QContactThumbnail contactThumb = curContact.detail<QContactThumbnail>();
                    QImage contactThumbImage = contactThumb.thumbnail();
                    if (!contactThumbImage.isNull())
                    {
                        if (m_imgCache) {
                            const int imgId = m_imgCache->addImage(contactThumbImage);
                            qDebug() << "Stored image into cache, id: " << imgId;
                            emit nfcTagImage(imgId);
                        } else {
                            qDebug() << "Image cache not set";
                        }
                        if (m_parseToModel) {
                            // Not optimal as the Qt Mobility contact only has a QImage,
                            // so we don't know the original encoding of the image.
                            // Instead, we need to re-encode. Using PNG as default here.
                            NdefNfcMimeImageRecord imgRecord(contactThumbImage, "PNG");
                            storeImageToFileForModel(imgRecord, true);
                        }
                    }
                }
                else    // Any other contact detail except image
                {
                    // Add the detail name and its contents to the description.
                    tagContents.append(detailName + ": ");

                    // We just add all values related to the detail converted to a string.
                    QVariantMap valueMap = curDetail.variantValues();

                    // Iterate over all contact details
                    QVariantMap::iterator i;
                    for (i = valueMap.begin(); i != valueMap.end(); ++i) {
                        tagContents.append(i.value().toString() + " ");
                        if (m_parseToModel) {
                            addContactDetailToModel(i.key(), i.value().toString());
                        }
                    }
                    tagContents.append("\n");
                }
            }
        }
    }
    else
    {
        // Error parsing the vCard into a QContact instance
        // Show the error message.s
        QString errorMsg = record.error();
        if (!errorMsg.isEmpty()) {
            tagContents.append(errorMsg);
        }
    }
    return tagContents;
}

/*!
  \brief Add a new item to the model, corresponding to the detailName
  (following Qt Mobility definition names) and with the specified
  detailValue.

  The method checks the known Qt Mobility contact detail definition names,
  converts that to the corresponding NfcTypes::RecordContent enum and
  adds an item with that enum and the parameter detailValue to the model.

  Method might be changed later to have the original QVariant as the second
  parameter, if the need arises to store stuff in another datatype than
  a string (e.g., for the birthday).

  \param detailName name of the Qt Mobility contact detail, following its
  DefinitionName.
  \param detailValue value corresponding to the detail definition name.
  */
bool NfcNdefParser::addContactDetailToModel(const QString& detailName, const QString& detailValue)
{
    if (detailName == QContactName::FieldPrefix.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordNamePrefix, detailValue, true);
    } else if (detailName == QContactName::FieldFirstName.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordFirstName, detailValue, true);
    } else if (detailName == QContactName::FieldMiddleName.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordMiddleName, detailValue, true);
    } else if (detailName == QContactName::FieldLastName.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLastName, detailValue, true);
    } else if (detailName == QContactName::FieldSuffix.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordNameSuffix, detailValue, true);
    } else if (detailName == QContactNickname::FieldNickname.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordNickname, detailValue, true);
    } else if (detailName == QContactPhoneNumber::FieldNumber.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordPhoneNumber, detailValue, true);
    } else if (detailName == QContactEmailAddress::FieldEmailAddress.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordEmailAddress, detailValue, true);
    } else if (detailName ==QContactUrl::FieldUrl.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordContactUrl, detailValue, true);
    } else if (detailName == QContactOrganization::FieldName.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordOrganizationName, detailValue, true);
    } else if (detailName == QContactOrganization::FieldDepartment.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordOrganizationDepartment, detailValue, true);
    } else if (detailName == QContactOrganization::FieldRole.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordOrganizationRole, detailValue, true);
    } else if (detailName == QContactOrganization::FieldTitle.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordOrganizationTitle, detailValue, true);
    } else if (detailName == QContactBirthday::FieldBirthday.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordBirthday, detailValue, true);
    } else if (detailName == QContactNote::FieldNote.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordNote, detailValue, true);
//    } else if (detailName == QContactThumbnail::FieldThumbnail.latin1()) {
//        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordImageFilename, detailValue, true);
    } else if (detailName == QContactAddress::FieldCountry.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordCountry, detailValue, true);
    } else if (detailName == QContactAddress::FieldLocality.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLocality, detailValue, true);
    } else if (detailName == QContactAddress::FieldPostOfficeBox.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordPostOfficeBox, detailValue, true);
    } else if (detailName == QContactAddress::FieldPostcode.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordPostcode, detailValue, true);
    } else if (detailName == QContactAddress::FieldRegion.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordRegion, detailValue, true);
    } else if (detailName == QContactAddress::FieldStreet.latin1()) {
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordStreet, detailValue, true);
    } else {
        qDebug() << "Unknown contact field - can't convert to model: " << detailName << " (" << detailValue << ")";
        return false;
    }
    return true;
}

/*!
  \brief Create a textual description of the contents of the
  LaunchApp record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseLaunchAppRecord(const NdefNfcLaunchAppRecord& record)
{
    QString tagContents("[LaunchApp]\n");
    tagContents.append("Arguments: " + record.arguments() + "\n");
    tagContents.append("Defined platforms: " + QString::number(record.platformAppIdsCount()) + "\n");
    QHashIterator<QString, QString> i(record.platformAppIds());
    while (i.hasNext()) {
        i.next();
        tagContents.append("Platform: " + i.key() + "\n");
        tagContents.append("App ID: " + i.value() + "\n");
    }
    if (!record.id().isNull() && !record.id().isEmpty()) {
        tagContents.append("Record Id: " + record.id() + "\n");
    }
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgLaunchApp, true);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLaunchAppArguments, record.arguments(), false);
        i.toFront();
        while (i.hasNext()) {
            i.next();
            if (i.key() == "Windows") {
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLaunchAppWindows, i.value(), true);
            } else if (i.key() == "WindowsPhone") {
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLaunchAppWindowsPhone, i.value(), true);
            } else {
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLaunchAppPlatform, i.key(), true);
                m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordLaunchAppId, i.value(), false);
            }
        }
        if (!record.id().isNull() && !record.id().isEmpty()) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordId, record.id(), true);
        }
    }
    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  Android Application Record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseAndroidAppRecord(const NdefNfcAndroidAppRecord& record)
{
    QString tagContents("[Android Application Record]\n");
    tagContents.append("Package name: " + record.packageName() + "\n");
    if (!record.id().isNull() && !record.id().isEmpty()) {
        tagContents.append("Id: " + record.id() + "\n");
    }
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgAndroidAppRecord, true);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordAndroidPackageName, record.packageName(), false);
        if (!record.id().isNull() && !record.id().isEmpty()) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordId, record.id(), true);
        }
    }
    return tagContents;
}


/*!
  \brief Create a textual description of the contents of the
  external record type name format.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcNdefParser::parseCustomRecord(const QNdefRecord& record)
{
    QString tagContents("[External RTD]\n");
    //tagContents.append("Type: " + record.type() + "\n");  // Already parsed for every record
    tagContents.append("Payload (" + QString::number(record.payload().size()) + ")");
    if (!record.isEmpty()) {
        tagContents.append(": " + record.payload());
    }
    tagContents.append("\n");
    if (!record.id().isNull() && !record.id().isEmpty()) {
        tagContents.append("Id: " + record.id() + "\n");
    }
    if (m_parseToModel) {
        m_nfcRecordModel->simpleAppendRecordHeaderItem(NfcTypes::MsgCustom, true);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordTypeNameFormat, "4", false);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordTypeName, record.type(), false);
        m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordRawPayload, record.payload(), false);
        if (!record.id().isNull() && !record.id().isEmpty()) {
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordId, record.id(), true);
        }
    }
    return tagContents;
}



/*!
  \brief Return a textual description of the \a typeName of the NDEF record.
  */
QString NfcNdefParser::convertRecordTypeNameToString(const QNdefRecord::TypeNameFormat typeName)
{
    QString typeNameString = "Unknown";
    switch (typeName)
    {
    case QNdefRecord::Empty:
        typeNameString = "Empty NDEF record (does not contain a payload)";
        break;
    case QNdefRecord::NfcRtd:
        typeNameString = "NFC RTD Specification";
        break;
    case QNdefRecord::Mime:
        typeNameString = "RFC 2046 (Mime)";
        break;
    case QNdefRecord::Uri:
        typeNameString = "RFC 3986 (Url)";
        break;
    case QNdefRecord::ExternalRtd:
        typeNameString = "External type name";
        break;
    case QNdefRecord::Unknown:
        typeNameString = "Unknown record type; should be treated similar to content with MIME type 'application/octet-stream' without further context";
        break;
    }
    return typeNameString;
}

void NfcNdefParser::storeClipboard(const QString& text, const QString& locale)
{
    if (m_clipboardContents == ClipboardUri ||
            m_clipboardContents == ClipboardTextEn)  {
        // Don't override clipboard if it already stores an URI or an English text
        return;
    }
    // Empty clipboard so far, or nothing with higher priority stored -> store new
    // text to clipboard
    m_clipboardText = text;
    // Set the contents to either contain text, or English text
    // (which has higher priority than others; one language has to have
    // highest priority if there are multiple languages stored on a tag,
    // and English makes most sense).
    m_clipboardContents = (locale.toLower() == "en") ? ClipboardTextEn : ClipboardText;
}

void NfcNdefParser::storeClipboard(const QUrl& uri)
{
    if (m_clipboardContents == ClipboardUri) {
        // Already have an URI stored in the clipboard cache
        return;
    }
    // Otherwise, storing the URI on the clipboard has higher priority than text
    m_clipboardContents = ClipboardUri;
    m_clipboardText = uri.toString();
}

void NfcNdefParser::setStoredClipboard()
{
    if (m_clipboardContents == ClipboardEmpty)
        return;

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        // We have new contents for sure - clear the clipboard first
        clipboard->clear();
        clipboard->setText(m_clipboardText);
    }
}

/*!
  \brief Save the image of an NDEF record to a file and add
  an item to the record model with the filename.

  \param imgRecord image record to extract the image from.
  \param removeVisible if the remove button should be visible
  in the view of the model. Recommended to set to false for an
  image only record, or to true for a record where the image
  is optional (e.g., Smart Poster).
  */
void NfcNdefParser::storeImageToFileForModel(const NdefNfcMimeImageRecord& imgRecord, const bool removeVisible)
{
    if (m_appSettings && m_appSettings->logNdefToFile()) {
        // Save image to file
        QDir dir("/");
        QString fileName = "";
        dir.mkpath(m_appSettings->logNdefDir());
        if (QDir::setCurrent(m_appSettings->logNdefDir())) {
            // Create image name based on current system time
            QDateTime now = QDateTime::currentDateTime();
            fileName = now.toString("yyyy.MM.dd - hh.mm.ss");
            fileName.append(" - image");
            QString imgFullFileName = imgRecord.saveImageToFile(fileName);
            qDebug() << "Saved image to file: " << m_appSettings->logNdefDir() + imgFullFileName;

            // Put image name to model
            m_nfcRecordModel->addContentToLastRecord(NfcTypes::RecordImageFilename, m_appSettings->logNdefDir() + imgFullFileName, removeVisible);
        }
    }
}
