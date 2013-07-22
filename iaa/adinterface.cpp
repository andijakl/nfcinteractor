/*
*
* adinterface.cpp
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

#include "adinterface.h"
#include "requestqueue.h"
#include <qplatformdefs.h>
#include <QEventLoop>
#include <QVariant>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QUrl>
#include <QDesktopServices>

#ifdef Q_OS_SYMBIAN
#include <APGTASK.H>
#include <APGCLI.H>
#include <EIKENV.H>
#include <cuseragent/cuseragent.h>
#endif

/*!
    \qmlclass AdInterface
    \ingroup com.inneractive
    \inherits QtObject
    The adInterface object is part of the \l {inneractive QML Components} module.

    AdInterface is exposed as a context property called "adInterface". It is not possible to
    instantiate a AdInterface object in QML.
*/

/*!
    \qmlproperty bool AdInterface::networkAccessible
    This property contains information of network accessibility.
*/

/*!
    \qmlsignal AdInterface::networkNotAccessible()
    This signal is emited after network connection fails or user doesn't allow
    connection to internet.
    For example:
    \code
        Connections {
            target: adInterface
            onNetworkNotAccessible: {
                //... show dialog to user about networkaccessibility and quit application...
            }
        }
    \endcode
*/
AdInterface::AdInterface(QObject *parent) :
    QObject(parent)
  , m_requestQueue(new RequestQueue(this))
{
    checkUserAgent();
}

bool AdInterface::networkAccessible() const
{
    return m_requestQueue->isOnline();
}

void AdInterface::requestAd(const QVariant &adItem)
{
    QObject *object = adItem.value<QObject*>();
    m_requestQueue->addToQueue(object);
}

// Opens url in native browser in Harmattan and Symbian
void AdInterface::openAd(const QUrl &adUrl)
{
#if defined(Q_OS_SYMBIAN)

    QString adUrlString(adUrl.toString());
    TUid browserUid = TUid::Uid(0x10008D39);
    adUrlString.prepend("4 ");
    TUid id( browserUid ); // S3
    TApaTaskList taskList( CEikonEnv::Static()->WsSession() );
    TApaTask task = taskList.FindApp( id );
    if ( task.Exists() ) {
      task.SendMessage( TUid::Uid( 0 ), TPtrC8((TUint8*)adUrlString.toUtf8().data()));
    } else {
      RApaLsSession appArcSession;
      TThreadId id;
      User::LeaveIfError( appArcSession.Connect() );
      appArcSession.StartDocument( TPtrC16(static_cast<const TUint16*>(adUrlString.utf16()), adUrlString.length()), browserUid, id);
      appArcSession.Close();
    }
#elif defined(MEEGO_EDITION_HARMATTAN)
    QProcess::startDetached(QString("/usr/bin/invoker --type=m /usr/bin/grob %1").arg(adUrl.toString()));
#else
    QDesktopServices::openUrl(adUrl);
#endif
}

void AdInterface::checkUserAgent()
{
#ifdef Q_OS_SYMBIAN
    CUserAgent *ua = 0;
    HBufC8 *uabuf = 0;
    QT_TRAP_THROWING(ua = CUserAgent::NewL());
    QT_TRAP_THROWING(uabuf = ua->UserAgentL());
    m_userAgent = QByteArray((char*)uabuf->Des().Ptr(),uabuf->Length());
    m_userAgent.append(" 3gpp-gba");
    delete ua;
    delete uabuf;
#elif defined(MEEGO_EDITION_HARMATTAN)
    m_userAgent = "Mozilla/5.0 (MeeGo; NokiaN9) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13";
#elif defined(Q_WS_MAEMO_5)
    m_userAgent = "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2.3pre) Gecko/20100723 Firefox/3.5 Maemo Browser 1.7.4.8 RX-51 N900";
#else
    m_userAgent = "Mozilla/5.0 Safari";
#endif
    m_requestQueue->setUserAgent(m_userAgent);
}
