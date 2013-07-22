/*
*
* requestqueue.h
* © Copyrights 2012 inneractive LTD, Nokia. All rights reserved
*
* This file is part of inneractiveAdQML.	
*
* inneractiveAdQML is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* inneractiveAdQML is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with inneractiveAdQML. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include <QObject>
#include <QQueue>
#include <QNetworkSession>

class QNetworkAccessManager;
class QNetworkReply;
class AdInterface;
class QNetworkSession;
class QNetworkConfigurationManager;
class RequestQueue : public QObject
{
    Q_OBJECT
public:
    explicit RequestQueue(AdInterface *parent = 0);
    ~RequestQueue();

    void setUserAgent(const QByteArray &ua) { m_userAgent = ua; }

    bool isOnline() const;
signals:

public slots:
    void addToQueue(QObject *object);

private:

private slots:
    void handleRequests();
    void adRequestFinished(QNetworkReply *req);
    void netSessionStateChanged(QNetworkSession::State state = QNetworkSession::Connected);

signals:
    void requestReady();

private:
    QQueue<QObject*> m_adItemQueue;
    QNetworkAccessManager *m_nam;
    QByteArray m_userAgent;
    bool m_requestRunning;
    QNetworkConfigurationManager *m_confman;
    QNetworkSession *m_nsession;
    bool m_onlineCheck;
    bool m_networkError;
};

#endif // REQUESTQUEUE_H
