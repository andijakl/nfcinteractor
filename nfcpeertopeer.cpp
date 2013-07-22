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

#include "nfcpeertopeer.h"

NfcPeerToPeer::NfcPeerToPeer(QObject *parent) :
    QObject(parent),
    m_appSettings(NULL),
    m_reportingLevel(AppSettings::OnlyImportantReporting),
    m_nfcUri(NULL),
    m_nfcManager(NULL),
    m_nfcTarget(NULL),
    m_nfcServer(NULL),
    m_nfcClientSocket(NULL),
    m_nfcServerSocket(NULL),
    m_isResetting(false),
    m_isBusy(false)
{
    m_snepManager = new SnepManager(this);
    connect(m_snepManager, SIGNAL(nfcSnepSuccess()), this, SIGNAL(nfcSendNdefSuccess()));

#if defined(MEEGO_EDITION_HARMATTAN) && defined(USE_SNEP)
    m_snepManagerMeego = new SnepManagerMeego(this);
    connect(m_snepManagerMeego, SIGNAL(nfcSnepSuccess()), this, SIGNAL(nfcSendNdefSuccess()));
#endif
}

NfcPeerToPeer::~NfcPeerToPeer()
{
    delete[] m_nfcUri;
}

void NfcPeerToPeer::setAppSettings(AppSettings *appSettings)
{
    if (!m_appSettings) {
        m_appSettings = appSettings;
        // Copy the values of the settings to the currently used config.
        // No need to call applySettings() yet, as this method is only
        // called before the NFC system is initialized, so we don't
        // need to restart any servers.
        m_useConnectionLess = m_appSettings->useConnectionLess();
        m_nfcPort = m_appSettings->nfcPort();
        copyNfcUriFromAppSettings() ;
        m_connectClientSocket = m_appSettings->connectClientSocket();
        m_connectServerSocket = m_appSettings->connectServerSocket();
        m_sendThroughServerSocket = m_appSettings->sendThroughServerSocket();
    }
}

void NfcPeerToPeer::setNfcManager(QNearFieldManager *nfcManager)
{
    m_nfcManager = nfcManager;
}

void NfcPeerToPeer::applySettings()
{
    m_isResetting = true;
    m_isBusy = true;
    emit busyChanged();
    QTimer::singleShot(500, this, SLOT(doApplySettings()));
}

void NfcPeerToPeer::doApplySettings()
{
    if (!(m_isResetting && m_isBusy) || !m_appSettings) {
        // Allow running this method only if called through applySettings().
        // Also, don't run this method if m_appSettings isn't set.
        return;
    }

    // This setting doesn't need any restarting of services,
    // so can always be copied from the settings object
    // to the currently used config.
    m_sendThroughServerSocket = m_appSettings->sendThroughServerSocket();

    // Now check for individual changes that affect the
    // communication and in most cases requires restarting services.
    if (m_appSettings->useConnectionLess() != m_useConnectionLess)
    {
        // Switched between connection-oriented and connection-less mode
        m_useConnectionLess = m_appSettings->useConnectionLess();
        // Close all current connections
        resetAll();
        // Start services again
        initAndStartNfc();
    }
    else if (m_useConnectionLess && m_appSettings->nfcPort() != m_nfcPort)
    {
        // Connection-less mode
        // Changed port
        m_nfcPort = m_appSettings->nfcPort();
        // Close all current connections
        resetAll();
        // Start services again
        initAndStartNfc();
    }
    else if (!m_useConnectionLess)
    {
        // Connection-oriented mode
        bool restartOrShutdownServer = false;
        bool restartOrShutdownClient = false;
        // Connection-oriented: check if URI changed
        if (m_appSettings->nfcUri() != QString(m_nfcUri)) {
            // New URI - restart socket server
            copyNfcUriFromAppSettings();
            restartOrShutdownServer = true;
            restartOrShutdownClient = true;
        }
        if (m_appSettings->connectClientSocket() != m_connectClientSocket) {
            m_connectClientSocket = m_appSettings->connectClientSocket();
            restartOrShutdownClient = true;
        }
        if (m_appSettings->connectServerSocket() != m_connectServerSocket) {
            m_connectServerSocket = m_appSettings->connectServerSocket();
            restartOrShutdownServer = true;
        }
        if (restartOrShutdownServer || restartOrShutdownClient) {
            // We could also differentiate here which socket to
            // re-establish, based on the two boolean variables.
            // But for now, just reset everything.
            // Close all current connections
            resetAll();
            // Start services again
            initAndStartNfc();
        }
    }
    m_isResetting = false;
    m_isBusy = false;
    emit busyChanged();
    emit settingsApplied();
}

void NfcPeerToPeer::copyNfcUriFromAppSettings()
{
    QByteArray nfcUriBA = m_appSettings->nfcUri().toLatin1();
    delete[] m_nfcUri;
    m_nfcUri = new char[nfcUriBA.size() + 1];
    strcpy(m_nfcUri, nfcUriBA.data());
}

void NfcPeerToPeer::resetAll()
{
    qDebug(__PRETTY_FUNCTION__);
    if (m_nfcClientSocket) {
        // close() also calls disconnectFromService().
        m_nfcClientSocket->close();
        m_nfcClientSocket->deleteLater();
        m_nfcClientSocket = NULL;
    }
    if (m_nfcServerSocket) {
        m_nfcServerSocket->close();
        m_nfcServerSocket->deleteLater();
        m_nfcServerSocket = NULL;
    }
    if (m_nfcServer) {
        m_nfcServer->close();
        m_nfcServer->deleteLater();
        m_nfcServer = NULL;
    }
    qDebug() << "NfcPeerToPeer::resetAll() finished";
}

void NfcPeerToPeer::initAndStartNfc()
{
    qDebug(__PRETTY_FUNCTION__);
    if (!m_appSettings) {
        return;
    }

    // Create the NFC server which will listen for incoming connections
    // (for connection-oriented only, connectionless will bind the client
    // socket to the port).
    if (!m_useConnectionLess && m_connectServerSocket) {
        qDebug() << "Creating LLCP server";
        m_nfcServer = new QLlcpServer(this);
        connect(m_nfcServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

        qDebug() << "Starting to listen to " << m_nfcUri;
        m_nfcServer->listen(m_nfcUri);
    }

    // The NFC client socket
    initClientSocket();
}

void NfcPeerToPeer::initClientSocket()
{
    if (!m_useConnectionLess && !m_connectClientSocket) {
        // If using only one socket for connection-oriented, do not initialize
        // a client (second) socket here. The server will already wait for incoming
        // connections.
        return;
    }

    if (m_nfcClientSocket) {
        delete m_nfcClientSocket;
    }

    qDebug() << "Creating new client socket";
    // The NFC client socket (in case of connectionless the only one)
    m_nfcClientSocket = new QLlcpSocket(this);
    connect(m_nfcClientSocket, SIGNAL(readyRead()), this, SLOT(readTextClient()));
    connect(m_nfcClientSocket, SIGNAL(disconnected()), this, SLOT(clientSocketDisconnected()));
    connect(m_nfcClientSocket, SIGNAL(error(QLlcpSocket::SocketError)), this, SLOT(clientSocketError(QLlcpSocket::SocketError)));
    connect(m_nfcClientSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)), this, SLOT(clientSocketStateChanged(QLlcpSocket::SocketState)));
    //connect(nfcClientSocket, SIGNAL(bytesWritten(qint64)))

    if (m_useConnectionLess) {
        m_nfcClientSocket->bind(m_nfcPort);
    } else {
        qDebug() << "Connecting client socket to service";
        #if defined(MEEGO_EDITION_HARMATTAN)
            // On Harmattan, already connect the client socket to the URI.
            // On Symbian, the connection can also be opened when the target is found.
            m_nfcClientSocket->connectToService(0, m_nfcUri);
        #endif
    }
    qDebug() << "NfcPeerToPeer::initClientSocket() finished";
}

void NfcPeerToPeer::targetDetected(QNearFieldTarget *target)
{
    // Cache target
    m_nfcTarget = target;
    // Check if the target supports LLCP access
    QNearFieldTarget::AccessMethods accessMethods = target->accessMethods();
    if (accessMethods.testFlag(QNearFieldTarget::LlcpAccess))
    {
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        if (!m_useConnectionLess && m_connectClientSocket) {
            #ifndef MEEGO_EDITION_HARMATTAN
                // Connect to the service on Symbian
                // (on Harmattan, the connection was already established at the beginning)
                m_nfcClientSocket->connectToService(target, m_nfcUri);
                if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
                    emit statusMessage("Connecting to service...");
                }
            #endif
        }
    }
}

void NfcPeerToPeer::targetLost(QNearFieldTarget */*target*/)
{
    if (!m_useConnectionLess && m_nfcClientSocket) {
        // Connection-oriented
        m_nfcClientSocket->disconnectFromService();
    } else {
        // Connection-less
#ifdef Q_OS_SYMBIAN
        // Delete and create a new client socket. Otherwise, we'd get an error from
        // the socket and it wouldn't work for new connections anymore.
        initClientSocket();
#endif
    }
    // Don't delete target here, it's owned by NfcInfo
}


void NfcPeerToPeer::handleNewConnection()
{
    if (!m_connectServerSocket)
        return;

    if (m_nfcServerSocket) {
        m_nfcServerSocket->deleteLater();
    }

    // The socket is a child of the server and will therefore be deleted automatically
    m_nfcServerSocket = m_nfcServer->nextPendingConnection();

    connect(m_nfcServerSocket, SIGNAL(readyRead()), this, SLOT(readTextServer()));
    connect(m_nfcServerSocket, SIGNAL(error(QLlcpSocket::SocketError)), this, SLOT(serverSocketError(QLlcpSocket::SocketError)));
    connect(m_nfcServerSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)), this, SLOT(serverSocketStateChanged(QLlcpSocket::SocketState)));
    connect(m_nfcServerSocket, SIGNAL(disconnected()), this, SLOT(serverSocketDisconnected()));

    if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
        emit statusMessage("New server socket connection");
    }
    sendCachedText();
}

void NfcPeerToPeer::readTextClient()
{
    readText(m_nfcClientSocket, false);
}

void NfcPeerToPeer::readTextServer()
{
    readText(m_nfcServerSocket, true);
}

void NfcPeerToPeer::readText(QLlcpSocket* socket, const bool isServerSocket)
{
    if (!socket)
        return;

    bool hasDatagramWaiting = socket->hasPendingDatagrams();
    if (hasDatagramWaiting)
    {
        // Connection-less
        qint64 datagramSize = socket->pendingDatagramSize();
        char* rawData = new char[datagramSize];
        socket->readDatagram ( rawData, datagramSize );

        // Check if data is NDEF formatted
        QNdefMessage containedNdef = QNdefMessage::fromByteArray(rawData);
        if (containedNdef.count() > 0) {
            // NDEF message found
            qDebug() << "Raw NDEF message received (" << containedNdef.count() << " records)";
            emit ndefMessage(containedNdef);
        }
        else
        {
            // No NDEF message found - output raw data
            QString data = QString::fromUtf8(rawData, datagramSize);
            QString dataLength;
            dataLength.setNum(datagramSize);
            QString message = (isServerSocket ? "Server" : "Client");
            message.append(" (" + dataLength + "): " + data);
            emit rawMessage(message);
        }
        delete rawData;
    }
    else
    {
        // Connection-oriented
        // Parse SNEP
        qDebug() << "Received peer-to-peer data";
        QByteArray rawData = socket->readAll();
        if (m_appSettings->useSnep()) {
            QString snepAnalysis;
            QNdefMessage containedNdef = m_snepManager->analyzeSnepMessage(rawData, snepAnalysis);
            emit rawMessage(snepAnalysis);
            if (containedNdef.count() > 0) {
                // NDEF message
                qDebug() << "SNEP NDEF message received (" << containedNdef.count() << " records)";
                emit ndefMessage(containedNdef);

                // Send back success response
                emit sendData(m_snepManager->createSnepSuccessResponse());
            } else {
                qDebug() << "No / empty NDEF message contained";
            }
        } else {
            // Check if data is NDEF formatted
            QNdefMessage containedNdef = QNdefMessage::fromByteArray(rawData);


            // TODO: Debug
            QString arrayContents = "";
            for (int i = 0; i < rawData.size(); ++i) {
                arrayContents.append(QString("0x") + QString::number(rawData.at(i), 16) + " ");
            }
            qDebug() << "Raw contents of message:\n" << arrayContents;


            if (containedNdef.count() > 0) {
                // NDEF message found
                qDebug() << "Raw NDEF message received (" << containedNdef.count() << " records)";
                emit ndefMessage(containedNdef);
            }
            else
            {
                // No NDEF message found - output raw data
                QString data = QString::fromUtf8(rawData.constData(), rawData.size());
                QString message = (isServerSocket ? "Server" : "Client");
                message.append(": " + data);
                emit rawMessage(message);
            }
        }
    }
}

void NfcPeerToPeer::sendText(const QString& text)
{
    sendData(text.toUtf8());
}

void NfcPeerToPeer::sendData(const QByteArray data)
{
    bool textQueuedBefore = m_sendDataQueue.isEmpty() ? false : true;
    m_sendDataQueue = data;
    if (!sendCachedText()) {
        if (textQueuedBefore) {
            emit statusMessage("Enqueued message replaced");
        } else {
            emit statusMessage("Message enqueued");
        }
    }
}

void NfcPeerToPeer::sendNdefMessage(const QNdefMessage *message)
{
#if defined(MEEGO_EDITION_HARMATTAN) && defined(USE_SNEP)
    if (m_snepManagerMeego) {
        m_snepManagerMeego->pushNdef(message);
    }
#else
    if (m_appSettings && m_appSettings->useSnep()) {
        // Wrap in SNEP protocol
        m_sendDataQueue = m_snepManager->wrapNdefInSnepPut(message);
    } else {
        // Directly write NDEF to stream
        m_sendDataQueue = message->toByteArray();
    }
    if (!sendCachedText()) {
        emit statusMessage("Message enqueued");
    }
#endif
}

bool NfcPeerToPeer::sendCachedText()
{
    qDebug(__PRETTY_FUNCTION__);
    if (!m_sendDataQueue.isEmpty()) {

        if (m_useConnectionLess) {
            // Connection-less
            if (m_nfcClientSocket) {
                // Connection-less doesn't have a server, only uses the client socket
                m_nfcClientSocket->writeDatagram(m_sendDataQueue.data(), (qint64)m_sendDataQueue.size(), m_nfcTarget, m_nfcPort);
                m_sendDataQueue.clear();
                emit statusMessage("Datagram sent");
                emit nfcSendNdefSuccess();
                return true;
            }
        }
        else {
            // Connection-oriented
            bool messageSent = false;
            if (m_sendThroughServerSocket && m_nfcServerSocket && m_nfcServerSocket->isOpen() && m_nfcServerSocket->isWritable()) {
                m_nfcServerSocket->write(m_sendDataQueue);
                messageSent = true;
            } else if (!m_sendThroughServerSocket && m_nfcClientSocket && m_nfcClientSocket->isOpen() && m_nfcClientSocket->state() == QLlcpSocket::ConnectedState){
                m_nfcClientSocket->write(m_sendDataQueue);
                messageSent = true;
            }
            if (messageSent) {
                m_sendDataQueue.clear();
                QString messageText = (m_appSettings && m_appSettings->useSnep() ? "SNEP message sent" : "NDEF message sent");
                emit statusMessage(messageText);
                if (m_appSettings && !m_appSettings->useSnep())
                {
                    // SNEP: success response from other phone will trigger written signal
                    // Without SNEP (= here), send success when we managed to send the data through the socket.
                    emit nfcSendNdefSuccess();
                }
                return true;
            }
            qDebug() << "Connection not ready for sending message";
        }
    } else {
        qDebug() << "NfcPeerToPeer::sendCachedText(): No text cached";
    }
    return false;
}

void NfcPeerToPeer::clientSocketDisconnected()
{
    if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
        emit statusMessage("Client socket disconnected");
    }
#ifdef MEEGO_EDITION_HARMATTAN
    if (!m_isResetting && !m_useConnectionLess && m_connectClientSocket) {
        m_nfcClientSocket->connectToService(0, m_nfcUri);
    }
#endif
}

void NfcPeerToPeer::serverSocketDisconnected()
{
    if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
        emit statusMessage("Server socket disconnected");
    }
#ifdef MEEGO_EDITION_HARMATTAN
    if (!m_isResetting && m_nfcServerSocket) {
        m_nfcServerSocket->deleteLater();
        m_nfcServerSocket = NULL;
    }
#endif
}

void NfcPeerToPeer::serverSocketError(QLlcpSocket::SocketError socketError)
{
    emit statusMessage("Server socket error: " + convertSocketErrorToString(socketError));
}

void NfcPeerToPeer::clientSocketError(QLlcpSocket::SocketError socketError)
{
    // While resetting the sockets, error smight occur. Hide them from the UI.
    if (!m_isResetting) {
#if defined(MEEGO_EDITION_HARMATTAN) && defined(USE_SNEP)
        if (socketError == QLlcpSocket::SocketAccessError) {
            // Ignore the error, as sending won't be handled by our LLCP implementation,
            // but instead the LibNDEFpush library
        } else {
            emit statusMessage("Client socket error: " + convertSocketErrorToString(socketError));
        }
#else
        emit statusMessage("Client socket error: " + convertSocketErrorToString(socketError));
#endif
    } else {
        qDebug() << "Client socket error: " + convertSocketErrorToString(socketError);
    }
}

void NfcPeerToPeer::serverSocketStateChanged(QLlcpSocket::SocketState socketState)
{
    if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
        emit statusMessage("Server socket state: " + convertSocketStateToString(socketState));
    }
}

void NfcPeerToPeer::clientSocketStateChanged(QLlcpSocket::SocketState socketState)
{
    if (m_reportingLevel != AppSettings::OnlyImportantReporting) {
        emit statusMessage("Client socket state: " + convertSocketStateToString(socketState));
    }
    if (socketState == QLlcpSocket::ConnectedState) {
        sendCachedText();
    }
}

void NfcPeerToPeer::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &/*id*/)
{
    emit statusMessage("Error: " + convertTargetErrorToString(error));
}


QString NfcPeerToPeer::convertTargetErrorToString(QNearFieldTarget::Error error)
{
    QString errorString = "Unknown";
    switch (error)
    {
    case QNearFieldTarget::NoError:
        errorString = "No error has occurred.";
        break;
    case QNearFieldTarget::UnsupportedError:
        errorString = "The requested operation is unsupported by this near field target.";
        break;
    case QNearFieldTarget::TargetOutOfRangeError:
        errorString = "The target is no longer within range.";
        break;
    case QNearFieldTarget::NoResponseError:
        errorString = "The target did not respond.";
        break;
    case QNearFieldTarget::ChecksumMismatchError:
        errorString = "The checksum has detected a corrupted response.";
        break;
    case QNearFieldTarget::InvalidParametersError:
        errorString = "Invalid parameters were passed to a tag type specific function.";
        break;
    case QNearFieldTarget::NdefReadError:
        errorString = "Failed to read NDEF messages from the target.";
        break;
    case QNearFieldTarget::NdefWriteError:
        errorString = "Failed to write NDEF messages to the target.";
        break;
    case QNearFieldTarget::UnknownError:
        errorString = "Unknown error.";
        break;
    }
    return errorString;
}

QString NfcPeerToPeer::convertSocketStateToString(QLlcpSocket::SocketState socketState)
{
    QString stateText = "Unknown";
    switch (socketState)
    {
    case QLlcpSocket::UnconnectedState:
        stateText = "The socket is not connected.";
        break;
    case QLlcpSocket::ConnectingState:
        stateText = "The socket has started establishing a connection.";
        break;
    case QLlcpSocket::ConnectedState:
        stateText = "A connection is established.";
        break;
    case QLlcpSocket::ClosingState:
        stateText = "The socket is about to close.";
        break;
    case QLlcpSocket::BoundState:
        stateText = "The socket is bound to a local port (for servers).";
        break;
    case QLlcpSocket::ListeningState:
        stateText = "The socket is listening for incoming connections (for internal use).";
        break;
    }
    return stateText;
}

QString NfcPeerToPeer::convertSocketErrorToString(QLlcpSocket::SocketError socketError)
{
    QString errorText = "Unknown";
    switch (socketError)
    {
    case QLlcpSocket::UnknownSocketError:
        errorText = "An unidentified error has occurred.";
        break;
    case QLlcpSocket::RemoteHostClosedError:
        errorText = "The remote host closed the connection.";
        break;
    case QLlcpSocket::SocketAccessError:
        errorText = "The socket operation failed because the application lacked the required privileges.";
        break;
    case QLlcpSocket::SocketResourceError:
        errorText = "The local system ran out of resources (e.g., too many sockets).";
        break;
    }
    return errorText;
}


bool NfcPeerToPeer::isBusy() const
{
    return m_isBusy;
}
