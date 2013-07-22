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

#include "nfcrecorditem.h"

/*!
  \brief Construct a new record item with the supplied information and an optional parent.
  */
NfcRecordItem::NfcRecordItem(const QString& title, NfcTypes::MessageType messageType,
                             NfcTypes::RecordContent recordContent, const QString& currentText,
                             bool removeVisible, bool addVisible, int recordId, QObject* parent) :
    QObject(parent),
    m_title(title),
    m_messageType(messageType),
    m_recordContent(recordContent),
    m_currentText(currentText),
    m_removeVisible(removeVisible),
    m_addVisible(addVisible),
    m_recordId(recordId)
{
}

//NfcRecordItem::NfcRecordItem(const NfcRecordItem &other) :
//    QObject(),
//    m_title(other.title()),
//    m_messageType(other.messageType()),
//    m_recordContent(other.recordContent()),
//    m_currentText(other.currentText()),
//    m_removeVisible(other.removeVisible()),
//    m_addVisible(other.addVisible()),
//    m_recordId(other.recordId())
//{
//    qDebug() << "[NfcRecordItem Copy Constructor]";
//}

/*!
  \brief Get all role names supported for this item, to be used by the model
  for generic setting and retrieving data.
  */
QHash<int, QByteArray> NfcRecordItem::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[MessageTypeRole] = "messageType";
    roles[RecordContentRole] = "recordContent";
    roles[CurrentTextRole] = "currentText";
    roles[SelectOptionsRole] = "selectOptions";
    roles[SelectedOptionRole] = "selectedOption";
    roles[RemoveVisibleRole] = "removeVisible";
    roles[AddVisibleRole] = "addVisible";
    roles[RecordIdRole] = "recordId";
    return roles;
}

/*!
  \brief Get the current data of the instance, based on the \a role.
  */
QVariant NfcRecordItem::data(int role) const
{
    switch (role) {
    case TitleRole:
        return title();
        break;
    case MessageTypeRole:
        return messageType();
        break;
    case RecordContentRole:
        return recordContent();
        break;
    case CurrentTextRole:
        return currentText();
        break;
    case SelectOptionsRole:
        return selectOptions();
        break;
    case SelectedOptionRole:
        return selectedOption();
        break;
    case RemoveVisibleRole:
        return removeVisible();
        break;
    case AddVisibleRole:
        return addVisible();
        break;
    case RecordIdRole:
        return recordId();
        break;
    }
    return QVariant();
}

//int NfcRecordItem::getIntForRoleString(const QString& role)
//{
//    return roleNames().key(role);
//}

/*!
  \brief Set data of the item using the role name as a string.
  */
void NfcRecordItem::setData(const QVariant& value, const QString& role)
{
    // Convert QString to enum
    int roleInt = roleNames().key(role.toAscii());
    setData(value, roleInt);
}

/*!
  \brief Set data of the item using the role name as an int (based on
  the enum, which is not supported by QML).
  */
void NfcRecordItem::setData(const QVariant& value, const int role)
{
    switch (role) {
    case TitleRole:
        setTitle(value.toString());
        break;
    case MessageTypeRole:
        setMessageType((NfcTypes::MessageType)value.toInt());
        break;
    case RecordContentRole:
        setRecordContent((NfcTypes::RecordContent)value.toInt());
        break;
    case CurrentTextRole:
        setCurrentText(value.toString());
        break;
    case SelectOptionsRole:
        if (value.canConvert<QVariantList>()) {
            setSelectOptions(value.value<QVariantList>());
        }
        break;
    case SelectedOptionRole:
        setSelectedOption(value.toInt());
        break;
    case RemoveVisibleRole:
        setRemoveVisible(value.toBool());
        break;
    case AddVisibleRole:
        setAddVisible(value.toBool());
        break;
    case RecordIdRole:
        setRecordId(value.toInt());
        break;
    }
}

/*!
  \brief Get the title text / header of the record item.
  */
QString NfcRecordItem::title() const
{
    return m_title;
}

/*!
  \brief Get the message type this item adds info to.
  */
NfcTypes::MessageType NfcRecordItem::messageType() const
{
    return m_messageType;
}

/*!
  \brief Get the record content - the detail of the NDEF record that is
  stored by this specific instance.
  */
NfcTypes::RecordContent NfcRecordItem::recordContent() const
{
    return m_recordContent;
}

/*!
  \brief Get the record content - the detail of the NDEF record that is
  stored by this specific instance.

  The record content is actually stored as an enum, but to communicate with
  QML, this is casted to an int.
  */
int NfcRecordItem::recordContentInt() const
{
    return (int)m_recordContent;
}

/*!
  \brief The current text of the record item.

  Used by most record items (exception being for example the selection item)
  */
QString NfcRecordItem::currentText() const
{
    return m_currentText;
}

/*!
  \brief Whether this record item can be removed again from the message.
  */
bool NfcRecordItem::removeVisible() const
{
    return m_removeVisible;
}

/*!
  \brief Whether a sub-item can be added after this record item to the
  same message type, providing more detail.

  This is usually only the case for header items. Could be combined
  with the message header type to save this boolean in the future, if
  there is no need for a separate addVisible() when more different
  item editors are added.
  */
bool NfcRecordItem::addVisible() const
{
    return m_addVisible;
}

/*!
  \brief Get the record id this record item belongs to.
  */
int NfcRecordItem::recordId() const
{
    return m_recordId;
}



/*!
  \brief Set the title text / header of the record item.
  */
void NfcRecordItem::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit dataChanged();
    }
}

/*!
  \brief Set the message type this item adds info to.
  */
void NfcRecordItem::setMessageType(const NfcTypes::MessageType messageType)
{
    if (m_messageType != messageType) {
        m_messageType = messageType;
        emit dataChanged();
    }
}

/*!
  \brief Set the record content - the detail of the NDEF record that is
  stored by this specific instance.
  */
void NfcRecordItem::setRecordContent(const NfcTypes::RecordContent recordContent)
{
    if (m_recordContent != recordContent) {
        m_recordContent = recordContent;
        emit dataChanged();
    }
}

/*!
  \brief Set the current text of the record item.

  Used by most record items (exception being for example the selection item)
  */
void NfcRecordItem::setCurrentText(const QString &currentText)
{
    if (m_currentText != currentText) {
        m_currentText = currentText;
        //qDebug() << "NfcRecordItem::setCurrentText: changed to: " << m_currentText;
        emit dataChanged();
    }
}

/*!
  \brief Whether this record item can be removed again from the message.
  */
void NfcRecordItem::setRemoveVisible(const bool removeVisible)
{
    if (m_removeVisible != removeVisible) {
        m_removeVisible = removeVisible;
        emit dataChanged();
    }
}

/*!
  \brief Whether a sub-item can be added after this record item to the
  same message type, providing more detail.

  This is usually only the case for header items. Could be combined
  with the message header type to save this boolean in the future, if
  there is no need for a separate addVisible() when more different
  item editors are added.
  */
void NfcRecordItem::setAddVisible(const bool addVisible)
{
    if (m_addVisible != addVisible) {
        m_addVisible = addVisible;
        emit dataChanged();
    }
}

/*!
  \brief Set the record id this record item belongs to.
  */
void NfcRecordItem::setRecordId(const int recordId)
{
    if (m_recordId != recordId) {
        m_recordId = recordId;
        emit dataChanged();
    }
}

/*!
  \brief Get the list of option items / radio buttons for this record item,
  in case it is of a type that uses a selection instead of a text field.
  */
QVariantList NfcRecordItem::selectOptions() const
{
    return m_selectOptions;
}

/*!
  \brief Set the list of option items / radio buttons for this record item,
  in case it is of a type that uses a selection instead of a text field.
  */
void NfcRecordItem::setSelectOptions(const QVariantList selectOptions)
{
    if (m_selectOptions != selectOptions) {
        m_selectOptions = selectOptions;
        emit dataChanged();
    }
}

/*!
  \brief Get the currently selected option / radio button for this record item,
  in case it is of a type that uses a selection instead of a text field.
  */
int NfcRecordItem::selectedOption() const
{
    return m_selectedOption;
}

/*!
  \brief Set the currently selected option / radio button for this record item,
  in case it is of a type that uses a selection instead of a text field.
  */
void NfcRecordItem::setSelectedOption(const int selectedOption)
{
    if (m_selectedOption != selectedOption) {
        m_selectedOption = selectedOption;
        emit dataChanged();
    }
    //qDebug() << "Nfc Record Item - New selected option: " << m_selectedOption;
}

