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

#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) :
    QObject(parent),
    m_logNdefToFile(true),
    m_logNdefDir(DEFAULT_NDEF_LOG_DIR),
    m_deleteTagBeforeWriting(false),
#if defined(MEEGO_EDITION_HARMATTAN) && !defined(USE_SNEP)
    m_useSnep(false),   // MeeGo: not allowed to use SNEP port
#else
    m_useSnep(true),
#endif
    m_useConnectionLess(false),
    m_nfcUri(LLCP_CONNECTIONORIENTED_SERVICENAME),
    m_nfcPort(LLCP_CONNECTIONLESS_PORT),
    m_sendThroughServerSocket(false),
    m_connectClientSocket(true),
    m_connectServerSocket(true)
{
    loadSettings();
}

void AppSettings::setUseConnectionLess(const bool useConnectionLess)
{
    if (useConnectionLess != m_useConnectionLess) {
        m_useConnectionLess = useConnectionLess;
    }
}

bool AppSettings::useConnectionLess() const
{
    return m_useConnectionLess;
}

void AppSettings::setNfcUri(const QString &nfcUri)
{
    if (nfcUri != m_nfcUri) {
        m_nfcUri = nfcUri;
    }
}

QString AppSettings::nfcUri() const
{
    return m_nfcUri;
}

void AppSettings::setNfcPort(const int nfcPort)
{
    if (nfcPort != m_nfcPort) {
        m_nfcPort = nfcPort;
    }
}

int AppSettings::nfcPort() const
{
    return m_nfcPort;
}

void AppSettings::setSendThroughServerSocket(const bool sendThroughServerSocket)
{
    if (sendThroughServerSocket != m_sendThroughServerSocket) {
        m_sendThroughServerSocket = sendThroughServerSocket;
    }
}

bool AppSettings::sendThroughServerSocket() const
{
    return m_sendThroughServerSocket;
}

void AppSettings::setConnectClientSocket(const bool connectClientSocket)
{
    if (connectClientSocket != m_connectClientSocket) {
        m_connectClientSocket = connectClientSocket;
    }
}

bool AppSettings::connectClientSocket() const
{
    return m_connectClientSocket;
}

void AppSettings::setConnectServerSocket(const bool connectServerSocket)
{
    if (connectServerSocket != m_connectServerSocket) {
        m_connectServerSocket = connectServerSocket;
    }
}

bool AppSettings::connectServerSocket() const
{
    return m_connectServerSocket;
}

void AppSettings::setLogNdefToFile(const bool logNdefToFile)
{
    if (logNdefToFile != m_logNdefToFile) {
        m_logNdefToFile = logNdefToFile;
    }
}

bool AppSettings::logNdefToFile() const
{
    return m_logNdefToFile;
}

void AppSettings::setLogNdefDir(const QString &logNdefDir)
{
    if (logNdefDir != m_logNdefDir) {
        m_logNdefDir = logNdefDir;
    }
}

QString AppSettings::logNdefDir() const
{
    return m_logNdefDir;
}

QString AppSettings::logNdefDir(const bool collected)
{
    return m_logNdefDir + (collected ? COLLECTED_LOG_DIR : SAVED_LOG_DIR);
}

void AppSettings::setDeleteTagBeforeWriting(const bool deleteTagBeforeWriting)
{
    if (deleteTagBeforeWriting != m_deleteTagBeforeWriting) {
        m_deleteTagBeforeWriting = deleteTagBeforeWriting;
    }
}

bool AppSettings::deleteTagBeforeWriting() const
{
    return m_deleteTagBeforeWriting;
}

void AppSettings::setUseSnep(const bool useSnep)
{
    if (useSnep != m_useSnep) {
        m_useSnep = useSnep;
    }
}

bool AppSettings::useSnep() const
{
    return m_useSnep;
}

void AppSettings::saveSettings()
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);
    settings.setValue("settingsversion", SETTINGS_VERSION);
    settings.setValue("logNdefToFile", m_logNdefToFile);
    settings.setValue("logNdefDir", m_logNdefDir);
    settings.setValue("deleteTags", m_deleteTagBeforeWriting);
    settings.setValue("useSnep", m_useSnep);
    settings.setValue("useConnectionLess", m_useConnectionLess);
    settings.setValue("nfcUri", m_nfcUri);
    settings.setValue("nfcPort", m_nfcPort);
    settings.setValue("sendThroughServerSocket", m_sendThroughServerSocket);
    settings.setValue("connectClientSocket", m_connectClientSocket);
    settings.setValue("connectServerSocket", m_connectServerSocket);
}

void AppSettings::loadSettings()
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);
    if (settings.value("settingsversion", -1) == SETTINGS_VERSION) {
        // Only load settings from version SETTINGS_VERSION with this code
        m_logNdefToFile = settings.value("logNdefToFile", true).toBool();
        m_logNdefDir = settings.value("logNdefDir", DEFAULT_NDEF_LOG_DIR).toString();
        m_deleteTagBeforeWriting = settings.value("deleteTags", false).toBool();
#if defined(MEEGO_EDITION_HARMATTAN) && !defined(USE_SNEP)
        m_useSnep = settings.value("useSnep", false).toBool();      // MeeGo: not allowed to use SNEP port
#else
        m_useSnep = settings.value("useSnep", true).toBool();
#endif
        m_useConnectionLess = settings.value("useConnectionLess", true).toBool();
        m_nfcUri = settings.value("nfcUri", LLCP_CONNECTIONORIENTED_SERVICENAME).toString();
        m_nfcPort = settings.value("nfcPort", LLCP_CONNECTIONLESS_PORT).toInt();
        m_sendThroughServerSocket = settings.value("sendThroughServerSocket", true).toBool();
        m_connectClientSocket = settings.value("connectClientSocket", true).toBool();
        m_connectServerSocket = settings.value("connectServerSocket", true).toBool();
    }
}

