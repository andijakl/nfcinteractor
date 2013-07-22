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

#include "nfcrecordmodel.h"

/*!
  \brief Constructor to create a new record model, whose items can be used
  to compose and define an NDEF message.
  */
NfcRecordModel::NfcRecordModel(QObject *parent) :
    QAbstractListModel(parent)
{
    // Merge default role names (if existing) with own role names
    NfcRecordItem* prototype = new NfcRecordItem(this);
    QHash<int, QByteArray> ownRoleNames = prototype->roleNames();
    QHash<int, QByteArray> defaultRoleNames = roleNames();

    setRoleNames(defaultRoleNames.unite(ownRoleNames));
    delete prototype;

    // Pass the list of record items to the Model->Ndef converter,
    // however, the ownership of the items will still stay with
    // the model of course.
    m_nfcModelToNdef = new NfcModelToNdef(m_recordItems, this);
    m_nfcRecordDefaults = new NfcRecordDefaults(this);
}

NfcRecordModel::~NfcRecordModel()
{
    qDeleteAll(m_recordItems);  // Required as we store pointers
    m_recordItems.clear();
}

void NfcRecordModel::setNfcStats(NfcStats *nfcStats)
{
    m_nfcStats = nfcStats;
    m_nfcModelToNdef->setNfcStats(nfcStats);
}

/*!
  \brief Convert all the record items currently stored in the model
  to a QNdefMessage.
  */
QNdefMessage* NfcRecordModel::convertToNdefMessage()
{
    return m_nfcModelToNdef->convertToNdefMessage();
}

bool NfcRecordModel::containsAdvMsg()
{
    foreach(NfcRecordItem* curItem, m_recordItems) {
        if (m_nfcStats->isAdvMsgType(curItem->messageType())) {
            return true;
        }
    }
    return false;
}

/*!
  \brief How many items are present in the record model.
  */
int NfcRecordModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_recordItems.size();
}

/*!
  \brief Get the appropriate data (specified by the role) from the references item.
  */
QVariant NfcRecordModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_recordItems.size())
        return QVariant();

    return m_recordItems.at(index.row())->data(role);
}

/*!
  \brief Search for the record \a item and return its index in the model.
  */
QModelIndex NfcRecordModel::indexFromItem(const NfcRecordItem *item) const
{
    Q_ASSERT(item);
    for(int row = 0; row < m_recordItems.size(); ++row) {
        if(m_recordItems.at(row) == item) return index(row);
    }
    return QModelIndex();
}

/*!
  \brief Relay the information that an item has been changed.

  Received through a signal from the individual item that has been changed,
  and further emitted by this class that data has been changed (to comply with
  Qt model requirements), and that the record items have been modified (custom).
  */
void NfcRecordModel::handleItemChange()
{
    NfcRecordItem* item = static_cast<NfcRecordItem*>(sender());
    QModelIndex index = indexFromItem(item);
    //qDebug() << "NfcRecordModel::handleItemChange(): " << m_recordItems.at(index.row())->currentText();
    if(index.isValid()) {
        //qDebug() << "NfcRecordModel::handleItemChange: emitting dataChanged signal";
        emit dataChanged(index, index);
        emit recordItemsModified();
    }
}

/*!
  \brief Append a new record item to the model, specifying the data directly and
  using int variables for type enums (for QML use).

  Casts the int values to enums and calls the 'real' addRecord() method.
  */
void NfcRecordModel::addRecord(const QString &title, const int messageType, const int recordContent, const QString &currentText, const bool removeVisible, const bool addVisible, const int recordId)
{
    addRecord(title, (NfcTypes::MessageType)messageType, (NfcTypes::RecordContent)recordContent, currentText, removeVisible, addVisible, recordId);
}

/*!
  \brief Append a new record item to the model, specifying the data directly and using enums.
  */
void NfcRecordModel::addRecord(const QString &title, const NfcTypes::MessageType messageType, const NfcTypes::RecordContent recordContent, const QString &currentText, const bool removeVisible, const bool addVisible, const int recordId)
{
    NfcRecordItem* newRecordItem = new NfcRecordItem(title, messageType, recordContent, currentText, removeVisible, addVisible, recordId, this);
    addRecordItem(newRecordItem);
}

/*!
  \brief Append a new record item to the model.
  */
void NfcRecordModel::addRecordItem(NfcRecordItem *newRecordItem)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    connect(newRecordItem, SIGNAL(dataChanged()), SLOT(handleItemChange()));
    m_recordItems.append(newRecordItem);
    endInsertRows();
    //qDebug() << "New item, message type = " << newRecordItem->messageType();
    emit recordItemsModified();
}

/*!
  \brief Insert a new record item at a specific position to the model.
  */
void NfcRecordModel::insertRecordItem(const int row, NfcRecordItem *newRecordItem)
{
    beginInsertRows(QModelIndex(), row, row);
    connect(newRecordItem, SIGNAL(dataChanged()), SLOT(handleItemChange()));
    m_recordItems.insert(row, newRecordItem);
    endInsertRows();
    //qDebug() << "New item, message type = " << newRecordItem->messageType();
    emit recordItemsModified();
}

/*!
  \brief Append the default info for a new message to the record model.

  Depending on the MessageType, this will add one or more record items
  with their default values to the record model.
  */
void NfcRecordModel::addCompleteRecordWithDefault(const int messageTypeInt)
{
    // Get next available record ID
    const int recordId = nextAvailableRecordId();
    const NfcTypes::MessageType messageType = (NfcTypes::MessageType) messageTypeInt;
    //qDebug() << "Add new complete record with default content for type: " << messageTypeInt;
    switch (messageType) {
    case NfcTypes::MsgSmartPoster:
        simpleAppendRecordHeaderItem(NfcTypes::MsgSmartPoster, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSmartPoster, NfcTypes::RecordUri, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSmartPoster, NfcTypes::RecordText, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSmartPoster, NfcTypes::RecordTextLanguage, false, recordId);
        break;
    case NfcTypes::MsgUri:
        simpleAppendRecordHeaderItem(NfcTypes::MsgUri, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgUri, NfcTypes::RecordUri, false, recordId);
        break;
    case NfcTypes::MsgText:
        simpleAppendRecordHeaderItem(NfcTypes::MsgText, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgText, NfcTypes::RecordText, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgText, NfcTypes::RecordTextLanguage, false, recordId);
        break;
    case NfcTypes::MsgSms:
        simpleAppendRecordHeaderItem(NfcTypes::MsgSms, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSms, NfcTypes::RecordPhoneNumber, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSms, NfcTypes::RecordSmsBody, false, recordId);
        break;
    case NfcTypes::MsgBusinessCard:
        simpleAppendRecordHeaderItem(NfcTypes::MsgBusinessCard, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgBusinessCard, NfcTypes::RecordFirstName, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgBusinessCard, NfcTypes::RecordLastName, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgBusinessCard, NfcTypes::RecordPhoneNumber, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgBusinessCard, NfcTypes::RecordEmailAddress, true, recordId);
        break;
    case NfcTypes::MsgSocialNetwork:
        simpleAppendRecordHeaderItem(NfcTypes::MsgSocialNetwork, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSocialNetwork, NfcTypes::RecordSocialNetworkType, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgSocialNetwork, NfcTypes::RecordSocialNetworkName, false, recordId);
        break;
    case NfcTypes::MsgGeo: {
        simpleAppendRecordHeaderItem(NfcTypes::MsgGeo, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgGeo, NfcTypes::RecordGeoType, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgGeo, NfcTypes::RecordGeoLatitude, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgGeo, NfcTypes::RecordGeoLongitude, false, recordId);
        break; }
    case NfcTypes::MsgStore: {
        simpleAppendRecordHeaderItem(NfcTypes::MsgStore, true, recordId);
        // Have a different default store type depending on if the app is executed
        // on Symbian or MeeGo
#ifdef MEEGO_EDITION_HARMATTAN
        simpleAppendRecordItem(NfcTypes::MsgStore, NfcTypes::RecordStoreMeeGoHarmattan, true, recordId);
#else
        simpleAppendRecordItem(NfcTypes::MsgStore, NfcTypes::RecordStoreSymbian, true, recordId);
#endif
        break; }
    case NfcTypes::MsgImage:
        simpleAppendRecordHeaderItem(NfcTypes::MsgImage, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgImage, NfcTypes::RecordImageFilename, false, recordId);
        break;
    case NfcTypes::MsgAnnotatedUrl:
        simpleAppendRecordHeaderItem(NfcTypes::MsgUri, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgUri, NfcTypes::RecordUri, false, recordId);
        simpleAppendRecordHeaderItem(NfcTypes::MsgText, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgText, NfcTypes::RecordText, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgText, NfcTypes::RecordTextLanguage, false, recordId);
        break;
    case NfcTypes::MsgCustom:
        simpleAppendRecordHeaderItem(NfcTypes::MsgCustom, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeNameFormat, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeName, true, recordId);
        // Always need to specify the payload - if not set explicitely to empty when constructing
        // a QNdefRecord, the payload might contain some random contents.
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordRawPayload, "", false, recordId);
        break;
    case NfcTypes::MsgCombination:
        simpleAppendRecordHeaderItem(NfcTypes::MsgCustom, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeNameFormat, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeName, true, recordId);
        simpleAppendRecordHeaderItem(NfcTypes::MsgUri, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgUri, NfcTypes::RecordUri, false, recordId);
        break;
    case NfcTypes::MsgNfcAutostart:
        simpleAppendRecordHeaderItem(NfcTypes::MsgCustom, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeNameFormat, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordTypeName, "nokia.com:nfcinteractor", true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgCustom, NfcTypes::RecordRawPayload, "", false, recordId);
        simpleAppendRecordHeaderItem(NfcTypes::MsgStore, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgStore, NfcTypes::RecordStoreCustomName, "ni", true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgStore, NfcTypes::RecordText, true,  recordId);
        simpleAppendRecordItem(NfcTypes::MsgStore, NfcTypes::RecordTextLanguage, false, recordId);
        break;
    case NfcTypes::MsgLaunchApp:
        simpleAppendRecordHeaderItem(NfcTypes::MsgLaunchApp, true, recordId);
        simpleAppendRecordItem(NfcTypes::MsgLaunchApp, NfcTypes::RecordLaunchAppArguments, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgLaunchApp, NfcTypes::RecordLaunchAppWindowsPhone, true, recordId);
        break;
    case NfcTypes::MsgAndroidAppRecord:
        simpleAppendRecordHeaderItem(NfcTypes::MsgAndroidAppRecord, false, recordId);
        simpleAppendRecordItem(NfcTypes::MsgAndroidAppRecord, NfcTypes::RecordAndroidPackageName, false, recordId);
        break;
    }
}

/*!
  \brief Update data in a specific record item.
  */
bool NfcRecordModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    //qDebug() << "NfcRecordModel::setData()";
    if (index.isValid() && role == Qt::EditRole) {
        m_recordItems.at(index.row())->setData(value, role);
        //emit dataChanged(index, index);  //  Not needed, emitted by the item
        return true;
    }
    return false;
}

/*!
  \brief Update data in a specific record item, with parameters of
  generic types that can be called from QML.

  The method needs to have a different name from setData(), so that the correct
  version is called from QML.
  */
void NfcRecordModel::setDataValue(int index, const QVariant &value, const QString &role)
{
    //qDebug() << "NfcRecordModel::setDataValue (before): index = " << index << ", value = " << value << ", role = " << role;
    //qDebug() << "Current text: " << m_recordItems.at(index)->currentText();
    if (index < 0 || index >= m_recordItems.size())
        return;

    m_recordItems.at(index)->setData(value, role);
    //qDebug() << "NfcRecordModel::setDataValue (after): " << m_recordItems.at(index)->currentText();
}

Qt::ItemFlags NfcRecordModel::flags ( const QModelIndex & index ) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

int NfcRecordModel::simpleAppendRecordHeaderItem(const NfcTypes::MessageType messageType, const bool addVisible)
{
    const int recordId = nextAvailableRecordId();
    simpleAppendRecordHeaderItem(messageType, addVisible, recordId);
    return recordId;
}

void NfcRecordModel::simpleAppendRecordHeaderItem(const NfcTypes::MessageType messageType, const bool addVisible, const int recordId)
{
    QString itemText = m_nfcRecordDefaults->itemHeaderTextDefault(messageType);

    addRecord(itemText, messageType, NfcTypes::RecordHeader, "", true, addVisible, recordId);
}


/*!
  \brief Add a new record item to an existing message, using default types.

  Uses int values for specifying the message and content type, so that it
  can be called from QML.

  If adding for example a text, this method will also add the language.
  */
void NfcRecordModel::addContentToRecord(int recordIndex, int messageTypeInt, int contentTypeInt)
{
    if (recordIndex < 0 || recordIndex > m_recordItems.count())
        return;

    // Search the end of the record, so that we can add the new info at the end
    const int parentRecordId = m_recordItems[recordIndex]->recordId();
    //qDebug() << "Parent record index: " << recordIndex << ", id: " << parentRecordId;
    const int insertPosition = lastRecordContentIndex(recordIndex);
    const NfcTypes::MessageType messageType = (NfcTypes::MessageType) messageTypeInt;
    const NfcTypes::RecordContent contentType = (NfcTypes::RecordContent) contentTypeInt;
    switch (contentType) {
    case NfcTypes::RecordText:
    {
        // Add two items for the text record, therefore handle it here
        // and not in the generic simpleInsertRecordItem() method.
        // (text is always together with language)
        simpleInsertRecordItem(insertPosition, messageType, NfcTypes::RecordTextLanguage, QString(), false, parentRecordId);
        simpleInsertRecordItem(insertPosition, messageType, NfcTypes::RecordText, QString(), true, parentRecordId);
        break;
    }
    case NfcTypes::RecordLaunchAppPlatform:
        // Add two items for the launch app platform item, therefore handle it here
        // and not in the generic simpleInsertRecordItem() method.
        // (a platform is always together with the ID)
        simpleInsertRecordItem(insertPosition, messageType, NfcTypes::RecordLaunchAppId, QString(), false, parentRecordId);
        simpleInsertRecordItem(insertPosition, messageType, NfcTypes::RecordLaunchAppPlatform, QString(), true, parentRecordId);
        break;
    case NfcTypes::RecordHeader:
        // Shouldn't be possible to add a header as content to an existing record
        qDebug() << "Error: attempt to add a header as a content for another record";
        break;
    default:
    {
        // Any other item: add it using the default values
        simpleInsertRecordItem(insertPosition, messageType, contentType, QString(), true, parentRecordId);
        break;
    }

    }
    // Check if another content item can potentially be added to the record
    if (recordIndex >= 0) {
        QList<QObject*> possibleContent = possibleContentForRecord(recordIndex);
        if (possibleContent.size() == 0) {
            // It is possible to add content items to the record
            // -> enable the add button for the header
            m_recordItems[recordIndex]->setAddVisible(false);
        }
    }
}

void NfcRecordModel::addContentToLastRecord(NfcTypes::RecordContent contentType, const QString& currentText, const bool removeVisible)
{
    if (m_recordItems.size() == 0) {
        // Empty model - no previous item to copy the message type from,
        // therefore, we can't add another item to a non-existing message.
        // At least the header of a record should already be present
        // in the model before calling this method.
        qDebug() << "No previous item found in model in NfcRecordModel::addContentToLastRecord()";
        return;
    }
    const int prevRecordIndex = m_recordItems.size() - 1;
    const int prevRecordId = m_recordItems[prevRecordIndex]->recordId();
    const NfcTypes::MessageType prevMessageType = m_recordItems[prevRecordIndex]->messageType();
    simpleAppendRecordItem(prevMessageType, contentType, currentText, removeVisible, prevRecordId);
}

/*!
  \brief Add a single item to the model using its defaults at the specified position.
  Send a QString() to use the default contents, or specify the contents through
  the parameter.

  Used by addContentToRecord().
  */
void NfcRecordModel::simpleInsertRecordItem(const int insertPosition, const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const QString& currentText, const bool removeVisible, const int parentRecordId)
{
    QString defaultTitle;
    QString defaultContents;
    m_nfcRecordDefaults->itemContentDefault(messageType, contentType, defaultTitle, defaultContents);
    // If contents for this item are specified, use those.
    // Otherwise, use the defaults.
    if (!currentText.isNull()) {
        defaultContents = currentText;
    }

    NfcRecordItem* newRecordItem = new NfcRecordItem(defaultTitle, messageType, contentType, defaultContents, removeVisible, false, parentRecordId, this);
    if (contentType == NfcTypes::RecordSpAction ||
            contentType == NfcTypes::RecordGeoType ||
            contentType == NfcTypes::RecordTypeNameFormat ||
            contentType == NfcTypes::RecordSocialNetworkType) {
        // Selection item - also add the selection options to the item
        int defaultSelectedItem = 0;
        QVariantList selectionItems = m_nfcRecordDefaults->itemSelectionDefault(contentType, defaultSelectedItem);
        // Check if the parameter contained a number for the default selected item
        if (!defaultContents.isNull()) {
            bool ok = false;
            int specifiedDefault = defaultContents.toInt(&ok);
            if (ok) {
                defaultSelectedItem = specifiedDefault;
            }
        }
        newRecordItem->setSelectOptions(selectionItems);
        newRecordItem->setSelectedOption(defaultSelectedItem);
    }
    insertRecordItem(insertPosition, newRecordItem);
}

/*!
  \brief append a single item to the model using its defaults at the specified position.
  */
void NfcRecordModel::simpleAppendRecordItem(const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const bool removeVisible, const int parentRecordId)
{
    simpleInsertRecordItem(rowCount(), messageType, contentType, QString(), removeVisible, parentRecordId);
}

/*!
  \brief append a single item to the model using its default title and the given content at the specified position.
  */
void NfcRecordModel::simpleAppendRecordItem(const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const QString& currentText, const bool removeVisible, const int parentRecordId)
{
    simpleInsertRecordItem(rowCount(), messageType, contentType, currentText, removeVisible, parentRecordId);
}


/*!
  \brief Return the model index after the last entry of the specified record.

  This can be used to insert a new content item at the end of the record.
  */
int NfcRecordModel::lastRecordContentIndex(const int recordIndex) {
    const int searchForRecordId = m_recordItems[recordIndex]->recordId();
    int curSearchIndex = recordIndex;
    while (curSearchIndex < m_recordItems.count())
    {
        if (m_recordItems[curSearchIndex]->recordId() != searchForRecordId) {
            return curSearchIndex;
        }
        curSearchIndex++;
    }
    // Reached the end of the model, so this record was the last
    // Return the end index of the list (== count)
    return curSearchIndex;
}


/*!
  \brief Check if the specified record content item is already part of the message.

  Uses int values instead of enums for compatibility to QML.
  */
bool NfcRecordModel::isContentInRecord(const int recordIndex, const int searchForRecordContent) {
    return isContentInRecord(recordIndex, (NfcTypes::RecordContent)searchForRecordContent);
}

/*!
  \brief Check if the record UI already contains the specified content item.
  \a recordIndex index of the parent record, whose children should be searched
  \a searchForRecordContent record content type to check if available in the parent record.
  \return true if the specified record content type is already present in the parent record.
  */
bool NfcRecordModel::isContentInRecord(const int recordIndex, const NfcTypes::RecordContent searchForRecordContent) {
    if (recordIndex < 0 || recordIndex > m_recordItems.size())
        return true;

    int curIndex = recordIndex;
    int recordId = m_recordItems[recordIndex]->recordId();
    while (curIndex < m_recordItems.size()) {
        if (m_recordItems[curIndex]->recordId() != recordId) {
            // Found a content item already belonging to a different record ID
            // -> no, the specified record doesn't contain the UI for the
            // specified record content yet
            return false;
        }
        if (m_recordItems[curIndex]->recordContent() == searchForRecordContent) {
            // Found the content item we're looking for!
            return true;
        }
        curIndex ++;
    }
    return false;
}

/*!
  \brief Return the model index of the header record item for the given index.
  If the specified index is already a header, the same index is returned.
  */
int NfcRecordModel::findHeaderForIndex(const int recordIndex) {
    for (int curIndex = recordIndex; curIndex >= 0; curIndex --) {
        if (m_recordItems[curIndex]->recordContent() == NfcTypes::RecordHeader) {
            return curIndex;
        }
    }
    // If the UI doesn't contain an error, it shouldn't be possible not to find
    // a header item
    return -1;
}
/*!
  \brief Return the next unused record id that can be used to add a new record to the UI.
  This method assumes that record ids are sorted, and will return the id of the last
  item in the model plus 1.
  */
int NfcRecordModel::nextAvailableRecordId() {
    const int numRecords = m_recordItems.count();
    if (numRecords > 0) {
        return m_recordItems[numRecords - 1]->recordId() + 1;
    } else {
        return 0;
    }
}


/*!
  \brief Remove the specified record / record content item from the UI.
  */
void NfcRecordModel::removeRecord(const int removeRecordIndex) {
    const NfcTypes::RecordContent recordContent = m_recordItems[removeRecordIndex]->recordContent();
    const int recordId = m_recordItems[removeRecordIndex]->recordId();
    const int recordHeaderIndex = findHeaderForIndex(removeRecordIndex);

    removeRecordFromModel(removeRecordIndex);
    // If it was a header, also remove all its children with the same record id
    // (or until the next header is found)
    if (recordContent == NfcTypes::RecordHeader) {
        while (m_recordItems.size() > removeRecordIndex) {
            if (m_recordItems[removeRecordIndex]->recordContent() == NfcTypes::RecordHeader) {
                break;
            } else if (m_recordItems[removeRecordIndex]->recordId() == recordId) {
                removeRecordFromModel(removeRecordIndex);
            }
        }
    } else {
        if (recordContent == NfcTypes::RecordText) {
            // If we just deleted a text entry, also delete the following text language (if present)
            if (m_recordItems.count() > removeRecordIndex && m_recordItems[removeRecordIndex]->recordContent() == NfcTypes::RecordTextLanguage)
            {
                removeRecordFromModel(removeRecordIndex);
            }
        } else if (recordContent == NfcTypes::RecordLaunchAppPlatform) {
            // If we just deleted a text entry, also delete the following text language (if present)
            if (m_recordItems.count() > removeRecordIndex && m_recordItems[removeRecordIndex]->recordContent() == NfcTypes::RecordLaunchAppId)
            {
                removeRecordFromModel(removeRecordIndex);
            }
        }

        // Check if it is possible to add items to the record now
        // If yes, we might need to re-enable the add button of the header.
        if (recordHeaderIndex >= 0) {
            QList<QObject*> possibleContent = possibleContentForRecord(recordHeaderIndex);
            if (possibleContent.size() > 0) {
                // It is possible to add content items to the record
                // -> enable the add button for the header
                m_recordItems[recordHeaderIndex]->setAddVisible(true);
            }
        }
    }
    // Note: Don't remove record if all its content is empty

}

/*!
  \brief Actually modify the model to remove an individual record from the model.

  Should usually be called from within a more intelligent method like
  removeRecord(), which knows if removing an item should also trigger
  removing related items, or if the visibility of the add button needs changed.
  */
void NfcRecordModel::removeRecordFromModel(const int removeRecordIndex)
{
    if (m_recordItems.size() > removeRecordIndex) {
        beginRemoveRows(QModelIndex(), removeRecordIndex, removeRecordIndex);
        m_recordItems.removeAt(removeRecordIndex);
        endRemoveRows();
        emit recordItemsModified();
    }
}

/*!
  \brief Remove all records from this model.
  */
void NfcRecordModel::clear()
{
    if (m_recordItems.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_recordItems.size());
        m_recordItems.clear();
        endRemoveRows();
        emit recordItemsModified();
    }
}

/*!
  \brief Count how many items are in the record model.
  */
int NfcRecordModel::size() const
{
    return m_recordItems.size();
}

/*!
  \brief Return a list of record items that can possibly be added to the
  messageType / NDEF record.

  For example, the custom record can only define the payload once. If it
  is already present, it should no longer be offered to the user to add
  the payload record item.

  In contrast, a text item can in many cases be added as many times as needed,
  as Smart Posters support text in different languages, so that the reader
  can choose which language to show.
  */
QList<QObject*> NfcRecordModel::possibleContentForRecord(int recordIndex)
{
    QList<QObject*> possibleContent;
    if (recordIndex < 0 || recordIndex > m_recordItems.size() ||
            m_recordItems[recordIndex]->recordContent() != NfcTypes::RecordHeader)
        return possibleContent;

    const NfcTypes::MessageType messageType = m_recordItems[recordIndex]->messageType();
    switch (messageType) {
    case NfcTypes::MsgUri:
    case NfcTypes::MsgText:
    case NfcTypes::MsgImage:
    case NfcTypes::MsgAnnotatedUrl:
    case NfcTypes::MsgCombination:
    case NfcTypes::MsgNfcAutostart:
        // No content can be added/removed to these records
        break;
    case NfcTypes::MsgSmartPoster:
    {
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordUri);
        checkPossibleContentForRecord(possibleContent, false, recordIndex, messageType, NfcTypes::RecordText);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordSpAction);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordImageFilename);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordSpType);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordSpSize);
        break;
    }
    case NfcTypes::MsgBusinessCard:
    {
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordNamePrefix);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordFirstName);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordMiddleName);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordLastName);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordNameSuffix);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordNickname);

        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordEmailAddress);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordPhoneNumber);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordContactUrl);

        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordOrganizationName);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordOrganizationDepartment);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordOrganizationRole);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordOrganizationTitle);

        //RecordBirthday, // TODO: not implemented yet, would require a date editor field
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordNote);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordImageFilename);

        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordCountry);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordLocality);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordPostOfficeBox);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordPostcode);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordRegion);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStreet);
        break;
    }
    case NfcTypes::MsgSms:
    case NfcTypes::MsgGeo:
    case NfcTypes::MsgSocialNetwork:
    {
        // Smart URI types - can convert to a Smart Poster record, by default URI record.
        checkPossibleContentForRecord(possibleContent, false, recordIndex, messageType, NfcTypes::RecordText);
        break;
    }
    case NfcTypes::MsgStore:
    {
        // Smart URI types - can convert to a Smart Poster record, by default URI record.
        checkPossibleContentForRecord(possibleContent, false, recordIndex, messageType, NfcTypes::RecordText);
        if (!isContentInRecord(recordIndex, NfcTypes::RecordStoreSymbian) &&
                !isContentInRecord(recordIndex, NfcTypes::RecordStoreMeeGoHarmattan) &&
                !isContentInRecord(recordIndex, NfcTypes::RecordStoreSeries40)) {
            // Generic Nokia Store link only available when no Symbian, MeeGo Harmattan or Series 40 specific ID is here
            checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreNokia);
        }
        if (!isContentInRecord(recordIndex, NfcTypes::RecordStoreNokia)) {
            // Symbian, MeeGo Harmattan and Series 40 links only possible if no generic
            // Nokia app id is present
            checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreSymbian);
            checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreMeeGoHarmattan);
            checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreSeries40);
        }
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreWindowsPhone);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreAndroid);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreiOS);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreBlackberry);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordStoreCustomName);
        break;
    }
    case NfcTypes::MsgCustom:
    {
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordTypeName);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordRawPayload);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordId);
        break;
    }
    case NfcTypes::MsgLaunchApp:
    {
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordLaunchAppArguments);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordLaunchAppWindows);
        checkPossibleContentForRecord(possibleContent, true, recordIndex, messageType, NfcTypes::RecordLaunchAppWindowsPhone);
        checkPossibleContentForRecord(possibleContent, false, recordIndex, messageType, NfcTypes::RecordLaunchAppPlatform);
        break;
    }
    case NfcTypes::MsgAndroidAppRecord:
        break;
    }
    //qDebug() << "Number of possible additional content items (C++): " << possibleContent.size();
    return possibleContent;
}



/*!
  \brief Add an item to the model of the content item dialog.
  \param onlyIfNotYetPresent check the current record and only allow adding the new
  content item if the record doesn't already show the UI for that content item.
  */
void NfcRecordModel::checkPossibleContentForRecord(QList<QObject*> &contentList, const bool onlyIfNotYetPresent, const int recordIndex, const NfcTypes::MessageType searchForMsgType, const NfcTypes::RecordContent searchForRecordContent, QString description)
{
    if (!onlyIfNotYetPresent || !isContentInRecord(recordIndex, searchForRecordContent)) {
        if (description.isEmpty()) {
            // No description provided - use the defaulf for this record content type
            QString defaultContents;
            m_nfcRecordDefaults->itemContentDefault(searchForMsgType, searchForRecordContent, description, defaultContents);
        }
        contentList.append(new NfcRecordItem(description, searchForMsgType, searchForRecordContent, QString(), false, false, -1, this));
    }
}
