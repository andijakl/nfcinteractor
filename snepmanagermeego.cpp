#include "snepmanagermeego.h"

SnepManagerMeego::SnepManagerMeego(QObject *parent) :
    QObject(parent),
    m_requestId(0),
    m_pushAvailable(false)
{
    m_pusher = new NDEFPusher(this);

    // This signal gets triggered when the Push service becomes available
    // or when it is available and its status was ecplicitly requested
    // by NDEFPusher::requestPushService() method.
    connect(m_pusher, SIGNAL(pushServiceAvailable()),
            this, SLOT(pushServiceAvailable()));

    // This signal gets triggered when the Push service becomes unavailable
    // or when it is unavailable and its status was ecplicitly requested
    // by NDEFPusher::requestPushService() method.
    connect(m_pusher, SIGNAL(pushServiceUnavailable()),
            this, SLOT(pushServiceUnavailable()));

    // This signal gets triggered when the push request completes
    connect(m_pusher, SIGNAL(pushComplete(unsigned int, enum NDEFPusher::RequestStatus)),
            this, SLOT(pushComplete(unsigned int, enum NDEFPusher::RequestStatus)));

    // Request push service
    m_pusher->requestPushService();
}

SnepManagerMeego::~SnepManagerMeego()
{
    m_pusher->cancelPush(m_requestId);
}

bool SnepManagerMeego::pushNdef(const QNdefMessage* message)
{
    if (!m_pushAvailable)
        return false;

    // Request NFC push for the Ndef message
    if (m_pusher->push(*message, m_requestId))
    {
        qDebug() << "Push request done. Please touch the target device to complete the push.";
        return true;
    }
    else
    {
        qDebug() << "Push request failed";
        return false;
    }
}

void SnepManagerMeego::pushServiceAvailable()
{
    // Push service is available.
    m_pushAvailable = true;
}

void SnepManagerMeego::pushServiceUnavailable()
{
    // Push service unavailable, cannot create a push request.
    m_pushAvailable = false;
    m_requestId = 0;
    qDebug() << "Push service unavailable.";
}

void SnepManagerMeego::pushComplete(unsigned int id, enum NDEFPusher::RequestStatus status)
{
    // The push request has been completed.
    Q_UNUSED(id);
    m_requestId = 0;

    if(status == NDEFPusher::REQUEST_OK)
    {
        emit nfcSnepSuccess();
        qDebug() << "Push request succeeded!";
    }
    else
    {
        qDebug() << "Push request failed!";
    }
}
