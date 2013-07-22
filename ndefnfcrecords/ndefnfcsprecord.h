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

#ifndef NDEFNFCSPRECORD_H
#define NDEFNFCSPRECORD_H

#include <QString>
#include <QImage>
#include <QUrl>

#include <QNdefMessage>
#include <QNdefRecord>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcmimeimagerecord.h"
#include <QDebug>

#include <QTextCodec>

QTM_USE_NAMESPACE

// Header, Record name length ('Sp' = 2), Sp data length (0 for empty), 'Sp' record name
//byte[] emptySpRecord = { 0xD1, 0x02, 0x00, 'S', 'p' };
// "urn:nfc:wkt:Sp"

/*!
  \brief Handles the Smart Poster meta-record and is able to
  both parse and assemble a smart poster according to the specification.

  The class is able to handle the mandatory URI, any number of
  text records (for storing the Smart Poster title in multiple
  languages), the action, size, type and image.

  As only the URI is mandatory, various ...InUse() methods can
  be queried to see if an optional detail was set in the Smart
  Poster that was read from a tag.

  The action, size and type records used within the Smart Poster
  are only valid within its context according to the specification.
  Therefore, classes for those record types are defined within the
  context of the Smart Poster record class.

  Due to the more complex nature of the Smart Poster record, which
  consists of multiple records, this class parses the payload
  contents and creates instances of the various record classes
  found within the Smart Poster. However, any changes to details
  are instantly commited to the raw payload as well.

  \version 1.1.0
  */
class NdefNfcSpRecord : public QNdefRecord
{
    public:
    NdefNfcSpRecord();
    NdefNfcSpRecord(const QNdefRecord &other);
    virtual ~NdefNfcSpRecord();

private:
    void initializeData();
    void parseRecords();
    bool assemblePayload();
    void setPayloadAndParse(const QByteArray &payload, const bool parseNewPayload);

public:
    enum NfcAction {
        DoAction = 0,
        SaveForLater,
        OpenForEditing,
        RFU
    };

    //    byte[] actionRecord = { 0x11, 0x03, 0x01, 'a', 'c', 't', action };
    /*!
      \brief The Action record is a Local Type specific to the Smart Poster.

      It suggests a course of action that the device should do with the content.
      The NFC Local Type Name for the action is "act" (0x61 0x63 0x74).
      The action record is defined as having a local scope only, and therefore
      it has meaning only within a Smart Poster record. A lone "act"-record
      SHALL be considered an error.

      The device MAY ignore this suggestion. The default (i.e., the Action
      record is missing from the Smart Poster) is not defined. For example,
      the device might show a list of options to the user.

      (Information taken from NFC Forum Smart Poster NDEF record specifications)
      */
    class NdefNfcActRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(NdefNfcActRecord, QNdefRecord::NfcRtd, "act", QByteArray(1, char(0)));

        NdefNfcSpRecord::NfcAction action() const;
        void setAction(const NdefNfcSpRecord::NfcAction &action);
    };

    /*!
      \brief The Size Record contains a four-byte, 32-bit, unsigned integer,
      which contains the size of object that the URI field refers to.

      Note that in practice this is limited to URLs (http://, ftp:// and similar).
      The Size Record's Local Type Name is "s".

      The size is expressed in network byte order (most significant byte first).
      For example, if Byte 0 contains 0x12, Byte 1 contains 0x34, Byte 2
      contains 0x56, and Byte 3 0x78, the size of the referred object
      is 0x12345678 bytes.

      The size record MAY be used by the device to determine whether it can
      accommodate the referenced file or not. For example, an NFC tag could
      trigger the download of an application to a cell phone. Using a combination
      of the Type Record and the Size Record, the mobile phone could determine
      whether it can accommodate such a program or not.

      The Size Record is for informational purposes only. Since the object size
      in the network may vary (for example, due to updates), this value should
      be used as a guideline only.

      The Size Record is optional to support.

      (Information taken from NFC Forum Smart Poster NDEF record specifications)
      */
    class NdefNfcSizeRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(NdefNfcSizeRecord, QNdefRecord::NfcRtd, "s", QByteArray(4, char(0)));

        quint32 size() const;
        void setSize(quint32 size);
    };

    //    The Payload of the Type Record is a UTF-8–formatted string that describes a MIME type [RFC 2046] which describes the type of the object that can be reached through the URI. (In practice this is limited to URLs only, much like the Size Record.)
    //    The Local Type Name for the Type Record is "t".
    //    The length of the payload string is the same as the length of the payload, so there is no need for separate length information or termination.
    //    The Type Record MAY be used by the device to determine whether it can process the referenced file or not. For example, an NFC tag could trigger a media file playback from an URL. If the Type Record references an unknown media type, the reader device (e.g. a cell phone) does not need to even initiate the playback.
    //    The Type Record is optional to support.
    /*!
      \brief The Payload of the Type Record is a UTF-8-formatted string
      that describes a MIME type [RFC 2046] which describes the type of the
      object that can be reached through the URI.

      (In practice this is limited to URLs only, much like the Size Record.)
      The Local Type Name for the Type Record is "t".

      The length of the payload string is the same as the length of the
      payload, so there is no need for separate length information or
      termination.

      The Type Record MAY be used by the device to determine whether it
      can process the referenced file or not. For example, an NFC tag
      could trigger a media file playback from an URL. If the Type Record
      references an unknown media type, the reader device (e.g. a cell phone)
      does not need to even initiate the playback.

      The Type Record is optional to support.

      (Information taken from NFC Forum Smart Poster NDEF record specifications)
      */
    class NdefNfcTypeRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(NdefNfcTypeRecord, QNdefRecord::NfcRtd, "t", QByteArray());

        QString mimeType() const;
        void setMimeType(const QString& mimeType);
    };

    // Attention: this method is non-virtual in the base class!
    void setPayload(const QByteArray &payload);

    QString rawContents() const;

    //    There is only one URI record per Smart Poster record. This is also the only mandatory record within a Smart Poster.
    //    The device is not required to support any particular URI protocol, but if the device does not support the referenced protocol, it MUST discard the entire Smart Poster record.
    QUrl uri() const;
    void setUri(const QUrl &uri);
    QNdefNfcUriRecord uriRecord() const;
    void setUri(const QNdefNfcUriRecord& newUri);

    //    The Title record is an instance of a Text RTD Record [TEXT]. There MAY be an arbitrary number of title records in the Smart Poster. However, there MUST NOT be two or more records with the same language identifier.
    //    The Title record SHOULD be shown to the user.
    void addTitle(const QNdefNfcTextRecord& newTitle);
    void setTitleList(QList<QNdefNfcTextRecord> newTitleList);
    int titleCount() const;
    QList<QNdefNfcTextRecord> titles() const;
    QNdefNfcTextRecord title(const int index) const;

    bool actionInUse() const;
    NfcAction action() const;
    void setAction(const NfcAction &action);

    bool sizeInUse() const;
    quint32 size() const;
    void setSize(const quint32 size);

    bool mimeTypeInUse() const;
    QString mimeType() const;
    void setMimeType(const QString& mimeType);

    bool imageInUse() const;
    NdefNfcMimeImageRecord image() const;
    void setImage(const NdefNfcMimeImageRecord& imageRecord);

    bool hasSpData() const;

private:
    QNdefNfcUriRecord *m_recordUri;
    QList<QNdefNfcTextRecord> m_recordTitleList;
    NdefNfcActRecord* m_recordAction;
    NdefNfcSizeRecord* m_recordSize;
    NdefNfcTypeRecord* m_recordMimeType;
    NdefNfcMimeImageRecord* m_recordImage;
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcSpRecord, QNdefRecord::NfcRtd, "Sp")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcSpRecord::NdefNfcActRecord, QNdefRecord::NfcRtd, "act")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcSpRecord::NdefNfcSizeRecord, QNdefRecord::NfcRtd, "s")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcSpRecord::NdefNfcTypeRecord, QNdefRecord::NfcRtd, "t")


#endif // NDEFNFCSPRECORD_H
