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

#ifndef NFCPEERTOPEER_H
#define NFCPEERTOPEER_H

#include <QObject>
#include <QDeclarativeView>
#include <QTimer>
#include <qnearfieldmanager.h>
#include <qllcpserver.h>
#include <qllcpsocket.h>
#include "appsettings.h"
#include "snepmanager.h"
#if defined(MEEGO_EDITION_HARMATTAN) && defined(USE_SNEP)
#include "snepmanagermeego.h"
#endif

QTM_USE_NAMESPACE   // Use Qt Mobility namespace


class NfcPeerToPeer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)

public:
    explicit NfcPeerToPeer(QObject *parent = 0);
    ~NfcPeerToPeer();

    void setAppSettings(AppSettings* appSettings);
    void setNfcManager(QNearFieldManager* nfcManager);

    bool isBusy() const;
signals:
    void rawMessage(const QString& nfcClientMessage);
    void ndefMessage(const QNdefMessage& nfcNdefMessage);
    void statusMessage(const QString& statusMessage);
    void nfcSendNdefSuccess();
    void settingsApplied();
    void busyChanged();

public slots:
    void applySettings();
    void initAndStartNfc();
    void sendText(const QString& text);
    void sendData(const QByteArray data);
    void sendNdefMessage(const QNdefMessage* message);
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);

private slots:
    void doApplySettings();

    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

    void handleNewConnection();
    void serverSocketError(QLlcpSocket::SocketError socketError);
    void serverSocketStateChanged ( QLlcpSocket::SocketState socketState );
    void serverSocketDisconnected ( );

    void readTextClient();
    void readTextServer();
    void clientSocketDisconnected();
    void clientSocketError(QLlcpSocket::SocketError socketError);
    void clientSocketStateChanged ( QLlcpSocket::SocketState socketState );

private:
    void resetAll();
    void copyNfcUriFromAppSettings();
    void initClientSocket();
    void readText(QLlcpSocket *socket, const bool isServerSocket);
    bool sendCachedText();
    QString convertTargetErrorToString(QNearFieldTarget::Error error);
    QString convertSocketStateToString(QLlcpSocket::SocketState socketState);
    QString convertSocketErrorToString(QLlcpSocket::SocketError socketError);


private:
    /*! Persistent storage of application settings. */
    AppSettings* m_appSettings;

    /*! Configures mainly how many messages are shown on the screen in the
      Nfc Info view. Also partly affects output to qDebug(). */
    AppSettings::ReportingLevel m_reportingLevel;

    char* m_nfcUri;
    int m_nfcPort;
    QNearFieldManager *m_nfcManager;
    SnepManager* m_snepManager;
    QNearFieldTarget *m_nfcTarget;
    QLlcpServer *m_nfcServer;
    QLlcpSocket *m_nfcClientSocket;
    QLlcpSocket *m_nfcServerSocket;
    QByteArray m_sendDataQueue;
    bool m_isResetting;
    // Use connection-less or connection-oriented LLCP.
    // In case of connection-less, will connect to: m_nfcPort
    // For connection-oriented, connects to: m_nfcUri
    // If set to use connection-less mode, the other three settings are not used
    // (sendThroughServerSocket, connectClientSocket, connectServerSocket), as the
    // connection only goes through a single socket anyway.
    // Note: the value of this one has to be set when constructing the class and
    // can't be changed afterwards! (as sockets need to be initialized differently).
    bool m_useConnectionLess;
    // Send text through the server socket or through the client socket.
    // Only used for connection-oriented mode.
    bool m_sendThroughServerSocket;
    // Connect the client socket once a suitable target is found.
    // Only used for connection-oriented mode.
    // Note: if sendThroughServerSocket is false, this setting has to be true.
    // Otherwise, the app would be unable to send data (can't send data through a
    // not-connected client socket).
    bool m_connectClientSocket;
    // Start a server to listen for incoming client connections.
    // Only used for connection-oriented mode.
    bool m_connectServerSocket;

    bool m_isBusy;

#if defined(MEEGO_EDITION_HARMATTAN) && defined(USE_SNEP)
    SnepManagerMeego* m_snepManagerMeego;
#endif
};

#endif // NFCPEERTOPEER_H
