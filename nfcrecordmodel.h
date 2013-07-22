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

#ifndef NFCRECORDMODEL_H
#define NFCRECORDMODEL_H

#include <QAbstractListModel>
#include "nfcrecorditem.h"
#include "nfcmodeltondef.h"
#include <QTimer>
#include <QDebug>
#include <QNdefMessage>
#include <QNdefRecord>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcrecords/ndefnfcsprecord.h"
#include "ndefnfcrecords/ndefnfcmimeimagerecord.h"
#include "ndefnfcrecords/ndefnfcmimevcardrecord.h"
#include "nfcstats.h"
#include "nfcrecorddefaults.h"

// Forward declarations
class NfcModelToNdef;

/*!
  \brief Stores and manages the editable data, which can be transformed
  to an NDEF message.

  The record model manages a list of NfcRecordItem instances, which
  define the various details that are combined to create a single
  NDEF message. The model can also contain data that will end up in
  several different NDEF records, which are all put together into
  one NDEF message in the end.

  Interaction with the data directly is encapsulated in the NfcRecordItem,
  but this class provides logic to add, remove and query items from the
  model, as well as tasks that go beyond the responsibilities of a single
  item - for example, finding which other record items can be added to a
  record present in the model (e.g., does the Smart Poster already have
  the optional Action item, or can it still be added?).
  */
class NfcRecordModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit NfcRecordModel(QObject *parent = 0);
    ~NfcRecordModel();

    void setNfcStats(NfcStats* nfcStats);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    // Needed so that the QML code can change the model.
    // setData() isn't invokable.
    // see: https://bugreports.qt.nokia.com/browse/QTBUG-7932
    Q_INVOKABLE void setDataValue(int index, const QVariant &value, const QString &role);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QNdefMessage* convertToNdefMessage();
    /*! Check if the message currently contained in the model contains an advanced message type. */
    bool containsAdvMsg();
    QModelIndex indexFromItem(const NfcRecordItem *item) const;

    int size() const;

public slots:
    void addCompleteRecordWithDefault(const int messageTypeInt);
    void addRecord(const QString &title, const int messageType, const int recordContent, const QString &currentText, const bool removeVisible, const bool addVisible, const int recordId);
    void addRecord(const QString &title, const NfcTypes::MessageType messageType, const NfcTypes::RecordContent recordContent, const QString &currentText, const bool removeVisible, const bool addVisible, const int recordId);
    void addRecordItem(NfcRecordItem *newRecordItem);

    void insertRecordItem(const int row, NfcRecordItem *newRecordItem);
    int simpleAppendRecordHeaderItem(const NfcTypes::MessageType messageType, const bool addVisible);
    void simpleAppendRecordHeaderItem(const NfcTypes::MessageType messageType, const bool addVisible, const int recordId);
    /** Creates and inserts a new record item, using defaults for some of the most common info (remove is visible, add is invisible). */
    void simpleInsertRecordItem(const int insertPosition, const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const QString &currentText, const bool removeVisible, const int parentRecordId);
    void simpleAppendRecordItem(const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const bool removeVisible, const int parentRecordId);
    void simpleAppendRecordItem(const NfcTypes::MessageType messageType, const NfcTypes::RecordContent contentType, const QString& currentText, const bool removeVisible, const int parentRecordId);

    void addContentToRecord(int recordIndex, int messageTypeInt, int newContentType);
    void addContentToLastRecord(NfcTypes::RecordContent contentType, const QString &currentText, const bool removeVisible);
    bool isContentInRecord(const int recordIndex, const NfcTypes::RecordContent searchForRecordContent);
    bool isContentInRecord(const int recordIndex, const int searchForRecordContent);

    void removeRecord(const int removeRecordIndex);
    void clear();

    int findHeaderForIndex(const int recordIndex);
    int nextAvailableRecordId();

    /**
      \brief Return a list of content items that can potentially be added
      to a record.
      \param recordIndex index of a header record, which will be searched.
      */
    //QList<NfcRecordItem*>*
    QList<QObject*> possibleContentForRecord(int recordIndex);

private slots:
    void handleItemChange();

private:
    int lastRecordContentIndex(const int recordIndex);
    void removeRecordFromModel(const int removeRecordIndex);
    void checkPossibleContentForRecord(QList<QObject*> &contentList, const bool onlyIfNotYetPresent, const int recordIndex, const NfcTypes::MessageType searchForMsgType, const NfcTypes::RecordContent searchForRecordContent, QString description = "");

signals:
    /*! Emitted whenever the model contents have been modified, e.g., by adding, removing or changing a record item. */
    void recordItemsModified();

private:
    /*! List of record items, which can be parsed to create an NDEF message. */
    QList<NfcRecordItem*> m_recordItems;
    /*! Converter to parse the record items and create an NDEF message. */
    NfcModelToNdef* m_nfcModelToNdef;
    /*! Count the number of tags read and messages written. (Not owned by this class) */
    NfcStats* m_nfcStats;
    /*! Default contents for record items. */
    NfcRecordDefaults* m_nfcRecordDefaults;
};

#endif // NFCRECORDMODEL_H
