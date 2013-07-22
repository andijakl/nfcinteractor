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

#include "ndefnfcmimevcardrecord.h"

/*!
  \brief Return a list of all the contacts contained in the vCard record.

  In case the contents of the payload can not be parsed into valid
  QContact instances, an empty list is returned. You can then retrieve
  the error message through error().
  */
QList<QContact> NdefNfcMimeVcardRecord::contacts()
{
    QByteArray p = payload();

    // Is the payload empty?
    if (p.isEmpty())
        return QList<QContact>();

    // Create a buffer so that the versit reader can read from the byte array
    QBuffer buffer(&p);  // compiler warning: taking address of temporary
    buffer.open(QBuffer::ReadOnly);

    // Read the versit document synchronously - for NFC, we will usually only
    // have one contact with as few details as possible.
    QVersitReader reader;
    reader.setDevice(&buffer);
    reader.startReading(); // Remember to check the return value
    reader.waitForFinished();

    // As a final step, use the contact importer to create a list of
    // QContact instances based on the versit documents.
    QList<QVersitDocument> inputDocuments = reader.results();
    QVersitContactImporter importer;
    if (importer.importDocuments(inputDocuments))
    {
        // Successful - return the list of contacts
        return importer.contacts();
    }
    else
    {
        // Error converting the versit documents into QContact instances.
        // Assemble the error message.
        QString errorMessage;
        QMap<int, QVersitContactImporter::Error>::const_iterator iterator = importer.errorMap().constBegin();
        while(iterator != importer.errorMap().constEnd()){
            switch (iterator.value()){
            case QVersitContactImporter::InvalidDocumentError:
                errorMessage += QString("Index %1:").arg(iterator.key());
                errorMessage += "One of the documents is not a vCard";
                break;
            case QVersitContactImporter::EmptyDocumentError:
                errorMessage += QString("Index %1:").arg(iterator.key());
                errorMessage += "One of the documents is empty";
                break;
            default:
                errorMessage += QString("Index %1:").arg(iterator.key());
                errorMessage += "Unknown error";
            }
            ++iterator;
        }
        cachedErrorText = "Error while reading vCard: " + errorMessage;
        qDebug() << cachedErrorText;
        return QList<QContact>();
    }
}

/*!
  \brief Convenience function if only one contact should be stored
  in the message.

  \see setContact()
  */
bool NdefNfcMimeVcardRecord::setContact(const QContact &contact, QVersitDocument::VersitType versitType)
{
    return setContact(QList<QContact>() << contact, versitType);
}

/*!
  \brief Store the list of contacts into the payload of the vCard record.

  In case there is an error while creating versit documents based on the
  QContact instances, the method will return false and you can retrieve
  the error message through error().

  \param contacts a list of contacts to be stored as a vCard.
  \param versitType version of the versit document to be created. Defaults to v3.
  \return true if the payload was successfully set.
  */
bool NdefNfcMimeVcardRecord::setContact(const QList<QContact> contacts, QVersitDocument::VersitType versitType)
{
    // Export the contacts into a versit document
    QVersitContactExporter contactExporter;
    if (!contactExporter.exportContacts(contacts, versitType))
    {
        // Error exporting - create an error message and return false.
        QString errorMessage;
        QMap<int, QVersitContactExporter::Error>::const_iterator iterator = contactExporter.errorMap().constBegin();
        while(iterator != contactExporter.errorMap().constEnd()){
            switch (iterator.value()){
            case QVersitContactExporter::EmptyContactError:
                errorMessage += QString("Index %1:").arg(iterator.key());
                errorMessage += "One of the contacts was empty";
                break;
            case QVersitContactExporter::NoNameError:
                errorMessage += QString("Index %1:").arg(iterator.key());
                errorMessage += "One of the contacts has no QContactName field";
                break;
            default:
                errorMessage += QString("Index %1:%2 ").arg(iterator.key())
                        .arg("Unknown error");
            }
            ++iterator;
        }
        cachedErrorText = "Error while writing vCard: " + errorMessage;
        qDebug() << cachedErrorText;
        return false;
    }

    // Exporting the contacts to a versit document was successful.
    // Retrieve the versit documents.
    QList<QVersitDocument> versitDocuments = contactExporter.documents();

    // Create an array to store the payload.
    QByteArray p;
    QBuffer buffer(&p);
    buffer.open(QIODevice::WriteOnly);

    // Create a versit writer which will serialize the versit document
    // into our byte array.
    QVersitWriter writer;
    writer.setDevice(&buffer);

    // Handle the writing synchronously - for NFC, we will usually only
    // have one contact with as few details as possible.
    writer.startWriting(versitDocuments);
    writer.waitForFinished();

    // Check if there was an error writing the contact.
    const QVersitWriter::Error writeError = writer.error();
    if (writeError == QVersitWriter::NoError)
    {
        // No error - commit the byte array to the payload of the base record class.
        setPayload(p);
    }
    else
    {
        // Error - create the error message.
        QString errorMessage;

        switch (writeError)
        {
        case QVersitWriter::UnspecifiedError:
            errorMessage += "The most recent operation failed for an undocumented reason";
            break;
        case QVersitWriter::IOError:
            errorMessage += "The most recent operation failed because of a problem with the device";
            break;
        case QVersitWriter::OutOfMemoryError:
            errorMessage += "The most recent operation failed due to running out of memory";
            break;
        case QVersitWriter::NotReadyError:
            errorMessage += "The most recent operation failed because there is an operation in progress";
            break;
        default:
            errorMessage += "Unknown error";
            break;
        }
        cachedErrorText = "Error while serializing vCard: " + errorMessage;
        qDebug() << cachedErrorText;
        return false;
    }
    return true;
}

/*!
  \brief Returns the error message in case there was a problem
  parsing the record into a QContact or assembling the payload.
  */
QString NdefNfcMimeVcardRecord::error()
{
    return cachedErrorText;
}



