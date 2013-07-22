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

#ifndef NDEFNFCMIMEIMAGERECORD_H
#define NDEFNFCMIMEIMAGERECORD_H

#include <QString>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QBuffer>
#include <QUrl>
#include <QNdefRecord>
#include <qndefmessage.h>
#include <qndefrecord.h>
#include <QDebug>

QTM_USE_NAMESPACE

// The default type name format - always Mime
#define IMAGERECORD_DEFAULT_TNF QNdefRecord::Mime
// The default image type, if an instance of this class
// is created without specifying an image type.
#define IMAGERECORD_DEFAULT_TYPE "image/png"
// The default payload of the image record - which is empty.
#define IMAGERECORD_DEFAULT_PAYLOAD QByteArray(0, char(0))

/*!
  \brief Handles all image-related MIME media-type constructs.

  Reads all images supported by Qt and is able to return those
  as a QImage instance. When writing the raw pixel data of a QImage
  to this record, you can choose the MIME type to use for encoding
  the image. Again, all image formats supported by Qt for writing
  can be used; in practical scenarios, mostly png, gif and jpeg
  will make sense.


  The NdefNfcMimeImageRecord class handles all supported images
  and therefore many different record types. To simplify the use,
  there is no extra class for each individual image type
  (image/png, gif, jpg, jpeg, etc.).
  Therefore, the isRecordType<>() method can't be used with this
  generic image handler class. Instead, check if the type name format
  is Mime and the record type starts with "image/" - if this is the
  case, create an instance of the NdefNfcMimeImageRecord based on
  the generic record. Example:

  if (record.typeNameFormat() == QNdefRecord::Mime &&
      record.type().startsWith("image/")) {
      NdefNfcMimeImageRecord imgRecord(record);
      QImage img = imgRecord.image();
  }

  \version 1.2.1
  */
class NdefNfcMimeImageRecord : public QNdefRecord
{
public:
    // "urn:nfc:mime:image/XXX"
    NdefNfcMimeImageRecord();
    NdefNfcMimeImageRecord(const QNdefRecord &other);
    NdefNfcMimeImageRecord(const QByteArray &mimeType);
    NdefNfcMimeImageRecord(const QImage &img, const QByteArray &mimeType);
    NdefNfcMimeImageRecord(const QString &fileName);

public:
    QByteArray format() const;
    QByteArray mimeType() const;

    QImage image() const;
    QByteArray imageRawData() const;
    QString saveImageToFile(const QString &fileName) const;

    bool setImage(QByteArray &imageRawData);
    bool setImage(const QString &fileName);
    bool setImage(const QImage &image, const QByteArray &mimeType);

    private:
    QByteArray checkImageFormat(const QByteArray &format);
};

// If you require the use of isRecordType() for images,
// create derived classes from this image record class,
// which specialize in the required formats and can then
// be registered like below.
/*
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImagePngRecord, QNdefRecord::Mime, "image/png")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageGifRecord, QNdefRecord::Mime, "image/gif")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageJpgRecord, QNdefRecord::Mime, "image/jpg")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeImageJpegRecord, QNdefRecord::Mime, "image/jpeg")
// etc.
*/

#endif // NDEFNFCMIMEIMAGERECORD_H
