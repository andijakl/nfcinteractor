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

#include "snepmanager.h"

SnepManager::SnepManager(QObject *parent) :
    QObject(parent)
{
}

QByteArray SnepManager::wrapNdefInSnepPut(const QNdefMessage* ndefMessage)
{
    // Create SNEP header
    // ------------------
    QByteArray snepMsg;
    QDataStream snepStream(&snepMsg, QIODevice::WriteOnly);
    QByteArray rawMessage = ndefMessage->toByteArray();

    // Version (two 4-bit unsigned integers)
    snepStream << SNEP_VERSION;

    // Request Field (8-bit unsigned integer)
    snepStream << SNEP_REQ_PUT;

    // Size (32-bit unsigned integer)
    snepStream << (quint32)rawMessage.size();

    // Information (x bytes)
    snepMsg.append(rawMessage);

    qDebug() << "SNEP message: " << snepMsg;
    qDebug() << "SNEP total size: " << snepMsg.size();

    QString arrayContents = "";
    for (int i = 0; i < snepMsg.size(); ++i) {
        arrayContents.append(QString("0x") + QString::number(snepMsg.at(i), 16) + " ");
    }
    qDebug() << "SNEP Raw contents:\n" << arrayContents;

    return snepMsg;
}

QNdefMessage SnepManager::analyzeSnepMessage(QByteArray& rawMessage, QString& results)
{
    // TODO: Debug
    QString arrayContents = "";
    for (int i = 0; i < rawMessage.size(); ++i) {
        arrayContents.append(QString("0x") + QString::number(rawMessage.at(i), 16) + " ");
    }
    qDebug() << "Raw contents of SNEP message:\n" << arrayContents;

    // Version
    QDataStream snepStream(&rawMessage, QIODevice::ReadOnly);

    quint8 version;
    snepStream >> version;

    if (version != SNEP_VERSION)
    {
        const int majorVersion = version >> 4;      // Most significant nibble
        const int minorVersion = version & 0x0F;    // Least significant nibble
        results.append("Warning: Unsupported SNEP version (" + QString::number(majorVersion) + "." +
                       QString::number(minorVersion) + ")\n");

        // TODO: We could return the SNEP_RES_UNSUPPORTEDVERSION response here.
    }

    const int protocolHeaderSize = 6;   // version (1b) + command (1b) + length (4b) = 6 bytes

    // Command
    quint8 command;
    snepStream >> command;
    results.append("SNEP: " + convertSnepCommandToText(command) + "\n");

    // Length
    quint32 length;
    snepStream >> length;
    if (length > 0)
    {
        results.append("Length: " + QString::number(length) + " Bytes");

        QByteArray snepContents;
        snepStream >> snepContents;

        // Read contents
        if (command == SNEP_REQ_PUT) {
            // Read NDEF message and send back to to
            // NfcInfo::ndefMessageRead
            QNdefMessage containedNdef = QNdefMessage::fromByteArray(rawMessage.mid(protocolHeaderSize));

            return containedNdef;
        }
        else
        {
            // Read raw data
            results.append("Contents: ");
            results.append(rawMessage.mid(protocolHeaderSize) + "\n");
        }

    }

    // If success (writing), inform UI
    if (command == SNEP_RES_SUCCESS)
    {
        emit nfcSnepSuccess();
    }

    return QNdefMessage();
}

QByteArray SnepManager::createSnepSuccessResponse()
{
    QByteArray response(6, char(0));
    response[0] = SNEP_VERSION;
    response[1] = SNEP_RES_SUCCESS;
    // 2 - 5 (4b): 0x0 (length = 0, so no data afterwards)

    // TODO: Debug
    QString arrayContents = "";
    for (int i = 0; i < response.size(); ++i) {
        arrayContents.append(QString("0x") + QString::number(response.at(i), 16) + " ");
    }
    qDebug() << "Raw contents of SNEP response:\n" << arrayContents;

    return response;
}

QString SnepManager::convertSnepCommandToText(quint8 command)
{
    QString cmdTxt;
    switch (command)
    {
    case SNEP_REQ_CONTINUE:
        cmdTxt.append("Send remaining fragments");
        break;
    case SNEP_REQ_GET:
        cmdTxt.append("Return an NDEF message");
        break;
    case SNEP_REQ_PUT:
        cmdTxt.append("Accept an NDEF message");
        break;
    case SNEP_REQ_REJECT:
        cmdTxt.append("Do not send remaining fragments");
        break;
    case SNEP_RES_CONTINUE:
        cmdTxt.append("Continue sending remaining fragments");
        break;
    case SNEP_RES_SUCCESS:
        cmdTxt.append("Operation succeeded");
        break;
    case SNEP_RES_NOTFOUND:
        cmdTxt.append("Resource not found");
        break;
    case SNEP_RES_EXCESSDATA:
        cmdTxt.append("Resource exceeds data size limit");
        break;
    case SNEP_RES_BADREQUEST:
        cmdTxt.append("Malformed request not understood");
        break;
    case SNEP_RES_NOTIMPLEMENTED:
        cmdTxt.append("Unsupported functionality requested");
        break;
    case SNEP_RES_UNSUPPORTEDVERSION:
        cmdTxt.append("Unsupported protocol version");
        break;
    case SNEP_RES_REJECT:
        cmdTxt.append("Do not send remaining fragments");
        break;
    default:
        cmdTxt.append("Unknown response");
        break;
    }
    return cmdTxt;
}

