/*
*
* adinterface.h
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

#ifndef INNERINTERFACE_H
#define INNERINTERFACE_H

#include <QObject>
#include <QQueue>

class RequestQueue;
class QUrl;
class AdInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibilityChanged)
    friend class RequestQueue;

public:
    explicit AdInterface(QObject *parent = 0);

    bool networkAccessible() const;
    Q_INVOKABLE void requestAd(const QVariant &adItem);
    Q_INVOKABLE void openAd(const QUrl &adUrl);
signals:
    void networkAccessibilityChanged(bool accessibility);
    void networkNotAccessible();

private:
    void checkUserAgent();

    mutable QByteArray m_userAgent;

    RequestQueue* m_requestQueue;
};

#endif // INNERINTERFACE_H
