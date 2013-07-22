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

#include "ndefnfcmimeimagerecord.h"

/*!
  \brief Construct a new Mime/Image record using the default type (png)
  and an empty payload.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord()
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, IMAGERECORD_DEFAULT_TYPE)
{
    setPayload(IMAGERECORD_DEFAULT_PAYLOAD);
}

/*!
  \brief Create a new Mime/Image record based on the record passed
  through the argument.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QNdefRecord &other)
    : QNdefRecord(other)
{
}

/*!
  \brief Create a new Mime/Image record using the specified mime type
  and an empty payload.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QByteArray &mimeType)
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, mimeType)
{
    setPayload(IMAGERECORD_DEFAULT_PAYLOAD);
}

/*!
  \brief Create a new Mime/Image record using the pixel data of the \a img,
  converted and encoded in the specified mime type (if supported).

  The mime type can either be a Qt image format or a mime type - the method
  will automatically convert to a mime type (\see setImage() ).
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QImage &img, const QByteArray &mimeType)
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, mimeType)
{
    setImage(img, mimeType);
}

/*!
  \brief Create a new Mime/Image record by reading the specified file.

  The mime type will be determined automatically.
  */
NdefNfcMimeImageRecord::NdefNfcMimeImageRecord(const QString &fileName)
    : QNdefRecord(IMAGERECORD_DEFAULT_TNF, IMAGERECORD_DEFAULT_TYPE)
{
    setImage(fileName);
}

/*!
  \brief Decode the payload and return it as a pixel image (QImage).

  If the payload is empty or Qt cannot parse the decode the image format,
  an empty image is returned.

  \return the decoded image if possible, or an empty image otherwise.
  */
QImage NdefNfcMimeImageRecord::image() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QImage();

    QImage img;
    if (img.loadFromData(p))
    {
        return img;
    }
    return QImage();
}

/*!
  \brief Get the raw data of the image.

  No decoding is performed, so this will retrieve the encoded version
  of the image (e.g., a png). This equals retrieving the payload.
  \return the encoded image / payload of the record.
  */
QByteArray NdefNfcMimeImageRecord::imageRawData() const
{
    return payload();
}

/*!
  \brief Save the image contained in this record to a file.

  \param fileName name of the image file to create.
  Note: do not specify the file extension - this will be added
  automatically, depending on the type of the image.
  Only specify the full path + name of the image file, e.g.:
  C:/nfc/myImg
  If the image is a PNG image, the resulting file name will be:
  C:/nfc/myImg.png

  \return file name, including the extension.
  */
QString NdefNfcMimeImageRecord::saveImageToFile(const QString& fileName) const
{
    // Do not use QImage::save(), as this would re-encode the image.
    // Instead, only determine the file extension and
    // save the byte array of the payload directly.
    QByteArray imgExtension = type().toLower();
    if (imgExtension.startsWith("image/")) {
        // Remove leading "image/" from the mime type so that only the image
        // type is left
        imgExtension = imgExtension.right(imgExtension.size() - 6);
    }
    QString fullFileName = fileName + "." + imgExtension;

    QFile imgFile(fullFileName);
    if (imgFile.open(QIODevice::WriteOnly)) {
        imgFile.write(payload());
        imgFile.close();
    } else {
        qDebug() << "Unable to open file for writing: " << imgFile.fileName();
        return QString();
    }

    return fullFileName;
}

/*!
  \brief Set the image (= payload) of the record to the byte array.
  This allows passing an encoded image (e.g., png) directly to the class.

  The method will attempt to identify the mime type of the raw image data
  and automatically modify the mime type stored in the record if successful.
  If this is not possible or the passed byte array is empty, the payload
  will not be set.
  \param imageRawData the encoded image data to use as payload of the record.
  */
bool NdefNfcMimeImageRecord::setImage(QByteArray& imageRawData)
{
    if (imageRawData.isEmpty())
        return false;

    // Check image mime type
    QBuffer buffer(&imageRawData);  // compiler warning: taking address of temporary
    buffer.open(QIODevice::ReadOnly);
    QByteArray imgFormat = QImageReader::imageFormat(&buffer);

    // Check if Qt supports the image format (needed to set the mime type automatically)
    if (imgFormat.isEmpty())
        return false;

    // Add "image/" to the Qt image type in order to get to a mime type
    imgFormat = imgFormat.toLower();
    imgFormat = imgFormat.prepend("image/");
    setType(imgFormat);
    setPayload(imageRawData);
    return true;
}

/*!
  \brief Set the payload of the record by reading the specified image file.
  The mime type of the image will be determined automatically.

  \param fileName filename to load, including the complete path.
  */
bool NdefNfcMimeImageRecord::setImage(const QString& fileName)
{
    // Determine the mime type of the image file
    QByteArray imgFormat = QImageReader::imageFormat(fileName);
    //qDebug() << "Setting image as payload, determined mime type of " << fileName << ": " << imgFormat << "/" << imgFormat.length();

    // Check if Qt supports the image format (needed to set the mime type automatically)
    if (imgFormat.isEmpty()) {
        qDebug() << "Unable to determine mime type of image: " << fileName;
        return false;
    }

    // Read the file directly to the payload
    QFile imgFile(fileName);
    if (!imgFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to open the image file for reading: " << fileName;
        return false;
    }

    // Add "image/" to the Qt image type in order to get to a mime type
    imgFormat = imgFormat.toLower();
    imgFormat = imgFormat.prepend("image/");
    setType(imgFormat);

    setPayload(imgFile.readAll());
    imgFile.close();

    return true;
}

/*!
  \brief Encode the image data into the payload using the specified mime type.

  \param image the pixel image data to encode and set as the payload.
  \param mimeType the mime type can either be specified using a supported mime
  type or a Qt image format. The image format is automatically converted to a
  mime type.
  */
bool NdefNfcMimeImageRecord::setImage(const QImage &image, const QByteArray &mimeType)
{
    // See if we support writing the image in the specified type
    QByteArray checkedFormat = checkImageFormat(mimeType);
    if (checkedFormat.isEmpty())
        return false;

    QByteArray p;
    QBuffer buffer(&p);
    buffer.open(QIODevice::WriteOnly);
    bool success = image.save(&buffer, checkedFormat.constData()); // writes image into p in the selected format
    if (!success) {
        return false;
    }

    setPayload(p);
    return true;
}

/*!
  \brief Retrieve the Qt image format of the image data stored in the payload.

  \see QImageReader::imageFormat()
  \return the Qt image format of the payload.
  */
QByteArray NdefNfcMimeImageRecord::format() const
{
    QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QBuffer buffer(&p);  // compiler warning: taking address of temporary
    buffer.open(QIODevice::ReadOnly);
    return QImageReader::imageFormat(&buffer);
}

/*!
  \brief return the mime type of the image, as set in the record type.

  \return the mime type of the record.
  */
QByteArray NdefNfcMimeImageRecord::mimeType() const
{
    return type();
}

/*!
  \brief Check if the specified Qt image \a format / mime type is supported by Qt
  for encoding and return it as a Qt image format name.

  \param format either a Qt image format (e.g., "png") or a mime type
  (e.g., "image/png").
  \return if the image format is supported by Qt for encoding, the Qt image format
  name. If the type is not supported, an empty byte array is returned.
  */
QByteArray NdefNfcMimeImageRecord::checkImageFormat(const QByteArray& format)
{
    // Convert MIME types to Qt image type names
    QByteArray checkFormat = format.toUpper();
    if (checkFormat.startsWith("IMAGE/")) {
        // Remove leading "image/" from the mime type so that only the image
        // type is left
        checkFormat = checkFormat.right(checkFormat.size() - 6);
    }

    // Check if the image format is supported by Qt
    bool supported = false;
    foreach (QByteArray supportedFormat, QImageWriter::supportedImageFormats()) {
        if (checkFormat == supportedFormat.toUpper()) {
            supported = true;
            break;
        }
    }
    if (supported) {
        return checkFormat;
    } else {
        return QByteArray();
    }
}
