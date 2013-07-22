#ifndef SNEPMANAGERMEEGO_H
#define SNEPMANAGERMEEGO_H

#include <QObject>
#include <QDebug>
#include <QNdefMessage>
#include <NDEFPusher.h>

QTM_USE_NAMESPACE

class SnepManagerMeego : public QObject
{
    Q_OBJECT
public:
    explicit SnepManagerMeego(QObject *parent = 0);
    ~SnepManagerMeego();
signals:
    void nfcSnepSuccess();
    
public slots:
    bool pushNdef(const QNdefMessage *message);

private slots:
    void pushServiceAvailable();
    void pushServiceUnavailable();
    void pushComplete(unsigned int id, NDEFPusher::RequestStatus status);
    
private:
    NDEFPusher* m_pusher;
    unsigned int m_requestId;
    bool m_pushAvailable;
};

#endif // SNEPMANAGERMEEGO_H
