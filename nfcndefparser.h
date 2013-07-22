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

#ifndef NFCNDEFPARSER_H
#define NFCNDEFPARSER_H

#include <QObject>
#include <QDebug>
#include "nfcrecordmodel.h"

// Clipboard
#include <QApplication>
#include <QClipboard>

// NDEF
#include <QNdefMessage>
#include <QNdefRecord>
#include <QNdefNfcUriRecord>
#include <QNdefNfcTextRecord>
#include "ndefnfcrecords/ndefnfcmimeimagerecord.h"
#include "ndefnfcrecords/ndefnfcsprecord.h"
#include "ndefnfcrecords/ndefnfcmimevcardrecord.h"
#include "ndefnfcrecords/ndefnfcandroidapprecord.h"

// Image handling
#include <QImage>
#include "tagimagecache.h"

// VCard reading
#include <QContact>
#include <QContactDetail>
#include <QVariantMap>
#include <QContactThumbnail>

// Logging tags/images to files
#include <QDir>
#include <QFile>
#include <QDateTime>


QTM_USE_NAMESPACE

/*!
  \brief Parse the contents of an NDEF message and return the contents
  of known records in human-readable textual form.

  The output contains information about all the records contained
  in the message. More details are reported for records of type:
  Uri, Text, Smart Poster, Image and Mime/vCard.

  The NfcTargetAnalyzer has a similar task, but returns the general
  tag information in textual form.
  */
class NfcNdefParser : public QObject
{
    Q_OBJECT
public:
    explicit NfcNdefParser(NfcRecordModel *nfcRecordModel, QObject *parent = 0);
    void setImageCache(TagImageCache* tagImageCache);
    void setAppSettings(AppSettings* appSettings);

signals:
    /*! \brief The tag contained an image.
      The parameter contains the image id that can be used
      to fetch it from the tag image cache class. */
    void nfcTagImage(const int nfcImgId);

public:
    /*! \brief Parse the NDEF message and return its contents
      as human-readable text. */
    QString parseNdefMessage(const QNdefMessage &message);

    void setParseToModel(bool parseToModel);
    private:
    QString parseUriRecord(const QNdefNfcUriRecord &record);
    QString parseTextRecord(const QNdefNfcTextRecord &record);
    QString textRecordToString(const QNdefNfcTextRecord &textRecord);
    QString parseSpRecord(const NdefNfcSpRecord &record);
    QString parseImageRecord(const NdefNfcMimeImageRecord &record);
    QString parseVcardRecord(NdefNfcMimeVcardRecord &record);
    QString parseLaunchAppRecord(const NdefNfcLaunchAppRecord &record);
    QString parseAndroidAppRecord(const NdefNfcAndroidAppRecord &record);

    bool addContactDetailToModel(const QString &detailName, const QString &detailValue);

    QString parseCustomRecord(const QNdefRecord &record);

    QString convertRecordTypeNameToString(const QNdefRecord::TypeNameFormat typeName);

private:
    void storeImageToFileForModel(const NdefNfcMimeImageRecord &imgRecord, const bool removeVisible);
    void storeClipboard(const QString &text, const QString &locale);
    void storeClipboard(const QUrl &uri);
    void setStoredClipboard();

private:
    /*! Used for storing images found on the tags. */
    TagImageCache* m_imgCache;    // Not owned by this class
    bool m_parseToModel;
    /*! Stores the editable records of the compose tag view.
      Not owned by this class.*/
    NfcRecordModel* m_nfcRecordModel;

    /*! Content that's currently stored in m_clipboardText.
      Used to prioritize what's going to be written to the clipboard,
      in the following priority order:
      Uri > Text (English) > Text (any other language). */
    enum ClipboardContents {
        ClipboardEmpty,
        ClipboardUri,
        ClipboardText,
        ClipboardTextEn // Give preference to English text for storing on the clipboard
    };
    ClipboardContents m_clipboardContents;
    QString m_clipboardText;

    /*! Persistent storageof application settings. Not owned by this class. */
    AppSettings* m_appSettings;

};

#endif // NFCNDEFPARSER_H
