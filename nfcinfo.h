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

#ifndef NFCINFO_H
#define NFCINFO_H

#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <QVariant>
#include <QUrl>
#include <QDeclarativeView>
#include <QSystemInfo>
#include <QTimer>

#include <QNearFieldManager>
#include <QNdefFilter>
#include <QNearFieldTarget>

#include "appsettings.h"

// NDEF
#include <QNdefMessage>
#include <QNdefRecord>
#include <QNdefNfcUriRecord>
#include <QNdefNfcTextRecord>
#include "ndefnfcrecords/ndefnfcmimeimagerecord.h"
#include "ndefnfcrecords/ndefnfcsprecord.h"
#include "ndefnfcrecords/ndefnfcmimevcardrecord.h"
#include "ndefnfcrecords/ndefnfcsmarturirecord.h"
#include "ndefnfcrecords/ndefnfcgeorecord.h"

// Analyze and parse targets
#include "nfctargetanalyzer.h"
#include "nfcndefparser.h"

#include "tagimagecache.h"

// Logging tags to files
#include <QDir>
#include <QFile>
#include <QDateTime>

// Record model for writing
#include "nfcrecordmodel.h"
#include "nfcrecorditem.h"

// Stats
#include "nfcstats.h"
#define ADV_MSG_WRITE_COUNT 10

// Peer to peer
#include "nfcpeertopeer.h"

#ifdef Q_OS_SYMBIAN
#include "nfcsettings.h"
#endif

QTM_USE_NAMESPACE

/*!
  \brief The NfcInfo class manages the interaction with the Nfc hardware,
  parses the tags and emits information signals for the UI.

  Following the use case of this application, the emitted signals
  contain mostly textual descriptions of the events or the information.
  Therefore, it is a good example to understand how the APIs are working
  and what kind of information you can get through the Qt Mobility APIs.

  Additionally, this class demonstrates how to parse tags and how to
  extract the relevant information - e.g., the parameters of a Smart Poster
  record, the image of a Mime record or the QContact coming from the versit
  document stored on a mime / text-vCard record.

  Furthermore, the class can also write NDEF messages to tags. The contents
  are edited by the user through the UI and stored in the model called
  NfcRecordModel. When writing, the model is converted to an actual NDEF message
  and written to the next available tag.

  The class will also handles errors that can occur and sends them out as
  signals for the UI to handle.
  */
class NfcInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(NfcRecordModel* recordModel READ recordModel NOTIFY recordModelChanged)

    /*! Current request type / status of the Nfc Interface. */
    enum NfcRequestStatus {
        NfcUninitialized,
        NfcIdle,
        NfcNdefReading,
        NfcNdefDeleting,
        NfcNdefWriting,
        NfcTargetAnalysis
    };

public:
    explicit NfcInfo(QObject *parent = 0);
    ~NfcInfo();

    void setImageCache(TagImageCache* tagImageCache);
    void setDeclarativeView(QDeclarativeView& view);
    void setAppSettings(AppSettings* appSettings);

signals:
    void nfcInitialized(const bool success);
    /*! Update on the NFC status (starting target detection, etc.) */
    void nfcStatusUpdate(const QString& nfcStatusText);
    /*! An important operation finished successfully (reading or writing tags) */
    void nfcStatusSuccess(const QString& nfcStatusSuccessText);
    /*! Error with NFC. */
    void nfcStatusError(const QString& nfcStatusErrorText);
    /*! Generic information about a new tag that was detected */
    void nfcInfoUpdate(const QString& nfcInfoText);
    /*! Information when the app starts interacting with a tag. */
    void nfcStartingTagInteraction();
    /*! Finished interacting with the tag. */
    void nfcStoppedTagInteraction();
    /*! Contents of the NDEF message found on the tag. */
    void nfcTagContents(const QString& nfcTagContents, const QString& nfcDataFileName);
    /*! The tag contained an image. The parameter contains
      the image id that can be used to fetch it from the
      tag image cache class. */
    void nfcTagImage(const int nfcImgId);
    /*! An error occured while interacting with the tag. */
    void nfcTagError(const QString& nfcTagError);
    /*! Written the message to the tag. */
    void nfcTagWritten();
    /*! An error occured while writing the tag. */
    void nfcTagWriteError(const QString& nfcTagError);
    /*! Written too many tags without having the unlimited version. */
    void nfcTagWriteExceeded();
    /*! Switched to a different operation mode. Using int for better compatibility to QML. */
    void nfcModeChanged(const int nfcNewMode);
    /*! The byte-size of the NDEF message resulting from the current NfcRecordModel changed. */
    void storedMessageSizeChanged(const int ndefMessageSize);
    /*! Signal when the record model has changed,
      needed for the NOTIFY part of the property of this class. */
    void recordModelChanged();
public slots:
    void initAndStartNfcAsync();
    bool checkNfcStatus();
    bool nfcWriteTag(const bool writeOneTagOnly);
    bool nfcWriteTag(const QString &fileName, const bool writeOneTagOnly);
    bool nfcEditTag(const QString &fileName);
    QString nfcSaveModelToFile(const QString &fileName);
    void nfcStopWritingTags();
    NfcRecordModel* recordModel() const;
public:
    Q_INVOKABLE void setUnlimitedAdvancedMsgs(const bool unlimited);
    Q_INVOKABLE void applySettings();

private slots:
    bool initAndStartNfc();
    void nfcRecordModelChanged();
    void targetDetected(QNearFieldTarget *target);
    void targetMessageDetected(const QNdefMessage &message, QNearFieldTarget *target);
    void ndefMessageRead(const QNdefMessage &message);
    bool writeCachedNdefMessage();
    void ndefMessageWritten();
    void requestCompleted(const QNearFieldTarget::RequestId & id);
    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
    void targetLost(QNearFieldTarget *target);

private:
    QString storeNdefToFile(const QString &fileName, const QNdefMessage &message, const bool collected);
    QNdefMessage loadNdefFromFile(const QString &fileName);

    QString convertTargetErrorToString(QNearFieldTarget::Error error);

    void startedTagInteraction();
    void stoppedTagInteraction();

#ifdef Q_OS_SYMBIAN
private slots:
    // Check for NFC Support
    void handleNfcError(NfcSettings::NfcError nfcError, int error);
    void handleNfcModeChange(NfcSettings::NfcMode nfcMode);
private:
    NfcSettings* nfcSettings;
#endif

private:
    /*! Responsible for finding tags and interacting with them. */
    QNearFieldManager *m_nfcManager;
    /*! Stores the editable records of the compose tag view. */
    NfcRecordModel* m_nfcRecordModel;
    /*! Current NFC target in proximity. */
    QNearFieldTarget *m_cachedTarget;

    /*! Analyze the tag type platform targets, getting low level information about
      the tags themselves. */
    NfcTargetAnalyzer *m_nfcTargetAnalyzer;
    /*! Analyze the NDEF contents and return the record contents in text format. */
    NfcNdefParser *m_nfcNdefParser;

    /*! Configures mainly how many messages are shown on the screen in the
      Nfc Info view. Also partly affects output to qDebug(). */
    AppSettings::ReportingLevel m_reportingLevel;

    /*! Set to true if the application requested to write an NDEF message
      to the tag on the next opportunity. */
    bool m_pendingWriteNdef;
    /*! Set to true if the app is currently interacting with a tag. */
    bool m_nfcTagInteractionActive;
    /*! Current activity / status of the class, e.g., reading or analyzing
      a tag. The activity can consist of multiple individual requests. */
    NfcRequestStatus m_currentActivity;
    /*! If set to true, the message will only be written to the next tag,
      then the class will read tags again.
      If set to false, the message will be written to every future tag
      touched, until nfcStopWriteTag() is called. */
    bool m_writeOneTagOnly;
    /*! The cached NDEF message that is to be written to the tag. */
    QNdefMessage* m_cachedNdefMessage;
    /*! Save the size of the message that is queued to write, to make
      it easier to compare it to the tag size if writing fails. */
    int m_cachedNdefMessageSize;
    /*! Currently active request ID for tracking the requests
      to the NFC interface. Only used for main read & write requests.
      Finishing them will stop NFC interactivity. */
    QNearFieldTarget::RequestId m_cachedRequestId;

    /*! Status of the concrete currently active request.
      In contrast to the m_currentActivity, this is related to only
      one request and not the whole class status, where for example the
      analysis phase can include multiple individual requests. */
    NfcRequestStatus m_cachedRequestType;

    /*! Is unlimited advanced tag writing enabled. */
    bool m_unlimitedAdvancedMsgs;
    /*! Whether the cached message that is waiting to be written contains
      an advanced record. */
    bool m_cachedNdefContainsAdvMsg;
    /*! Statistics for reading and writing tags. */
    NfcStats* m_nfcStats;

    /*! Persistent storage of application settings. */
    AppSettings* m_appSettings;

    /*! Needed on MeeGo Harmattan to raise the app to the foreground when
      it's autostarted. */
    QDeclarativeView* m_declarativeView;
    /*! Running on Harmattan PR 1.0? Then, need to switch between reading
      and writng NDEF messages from/to tags, as this FW can't have both
      modes activated at the same time. This has been improved in PR 1.1+. */
    bool m_harmattanPr10;

    bool m_usePeerToPeer;
    NfcPeerToPeer* m_nfcPeerToPeer;
};

#endif // NFCINFO_H
