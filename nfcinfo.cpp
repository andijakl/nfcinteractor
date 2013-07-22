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

#include "nfcinfo.h"

NfcInfo::NfcInfo(QObject *parent) :
    QObject(parent),
    m_nfcManager(NULL),
    m_cachedTarget(NULL),
    m_reportingLevel(AppSettings::OnlyImportantReporting),
    m_pendingWriteNdef(false),
    m_currentActivity(NfcUninitialized),
    m_writeOneTagOnly(true),
    m_cachedNdefMessage(NULL),
    m_cachedNdefMessageSize(0),
    m_cachedRequestType(NfcIdle),
    m_unlimitedAdvancedMsgs(true),
    m_harmattanPr10(false),
    m_usePeerToPeer(true),
    m_nfcPeerToPeer(NULL)
{
#if defined(MEEGO_EDITION_HARMATTAN)
    // Determine Harmattan FW version
    // PR 1.0 doesn't support activating read and write NDEF access at the same time,
    // so we need to switch between both modes depending on what the app intends to do.
    QSystemInfo* sysInfo = new QSystemInfo(this);
    if (sysInfo->version(QSystemInfo::Os) == "1.2" && sysInfo->version(QSystemInfo::Firmware).contains("10.2011.34")) {
        qDebug() << "Running Harmattan PR 1.0";
        m_harmattanPr10 = true;
    }
#endif

    // Record model and stats module
    m_nfcRecordModel = new NfcRecordModel(this);
    m_nfcStats = new NfcStats(this);
    m_nfcRecordModel->setNfcStats(m_nfcStats);
    connect(m_nfcRecordModel, SIGNAL(recordItemsModified()), this, SLOT(nfcRecordModelChanged()));

    // Target analyzer and Ndef parser
    m_nfcTargetAnalyzer = new NfcTargetAnalyzer(this);
    m_nfcNdefParser = new NfcNdefParser(m_nfcRecordModel, this);

    // Relay the signal when the private ndef parser found an image,
    // so that the QML UI can react to this.
    // Images are stored in the m_imgCache variable; both this and
    // the m_nfcNdefParser have a reference to it.
    connect(m_nfcNdefParser, SIGNAL(nfcTagImage(int)), this, SIGNAL(nfcTagImage(int)));

}

NfcInfo::~NfcInfo() {
    delete m_cachedNdefMessage;
}

void NfcInfo::initAndStartNfcAsync()
{
    QTimer::singleShot(50, this, SLOT(initAndStartNfc()));
}

/*!
  \brief Initialize the NFC access for NDEF targets and start target detection.

  This method emits nfcStatusUpdate signals containing the textual results of
  the operation.

  \return true if starting target detection was successful.
  */
bool NfcInfo::initAndStartNfc()
{
    bool success = false;
    if (!m_nfcPeerToPeer)
    {
        // Peer to peer for SNEP
        m_nfcPeerToPeer = new NfcPeerToPeer(this);
        m_nfcPeerToPeer->setAppSettings(m_appSettings);
        // TODO: Connect all signals of m_nfcPeerToPeer
        connect(m_nfcPeerToPeer, SIGNAL(rawMessage(QString)), this, SIGNAL(nfcInfoUpdate(QString)));
        connect(m_nfcPeerToPeer, SIGNAL(ndefMessage(QNdefMessage)), this, SLOT(ndefMessageRead(QNdefMessage)));
        connect(m_nfcPeerToPeer, SIGNAL(statusMessage(QString)), this, SIGNAL(nfcStatusUpdate(QString)));
        connect(m_nfcPeerToPeer, SIGNAL(nfcSendNdefSuccess()), this, SIGNAL(nfcTagWritten()));
    }

    if (m_nfcRecordModel->size() == 0) {
        nfcRecordModelChanged();
        // Populate write view with items (for development time only, comment out for release)
//        nfcRecordModel->addRecordItem(new NfcRecordItem("Smart Poster", NfcTypes::MsgSmartPoster, NfcTypes::RecordHeader, "", true, true, 1));
//        nfcRecordModel->addRecordItem(new NfcRecordItem("URI", NfcTypes::MsgSmartPoster, NfcTypes::RecordUri, "http://www.nokia.com/", false, false, 1));
    }

    // NfcInfo (this) is the parent; will automaically delete nfcManager
    if (!m_nfcManager) {
        m_nfcManager = new QNearFieldManager(this);
        m_nfcPeerToPeer->setNfcManager(m_nfcManager);
    }

    const bool nfcAvailable = m_nfcManager->isAvailable();
    if (nfcAvailable) {
        emit nfcStatusUpdate("Qt reports: NFC is available");
    } else {
        emit nfcStatusError("Qt reports: NFC is not available");
    }

    if (m_harmattanPr10) {
        // MeeGo Harmattan PR 1.0 only allows one target access mode to be active at the same time
        m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    } else {
        m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess | QNearFieldManager::NdefWriteTargetAccess | QNearFieldManager::TagTypeSpecificTargetAccess);
    }

    // Required for autostart tags
    m_nfcManager->registerNdefMessageHandler(this, SLOT(targetMessageDetected(QNdefMessage,QNearFieldTarget*)));

    // Get notified when the tag gets out of range
    connect(m_nfcManager, SIGNAL(targetLost(QNearFieldTarget*)),
            this, SLOT(targetLost(QNearFieldTarget*)));
    connect(m_nfcManager, SIGNAL(targetDetected(QNearFieldTarget*)),
            this, SLOT(targetDetected(QNearFieldTarget*)));

    if (m_usePeerToPeer) {
        m_nfcPeerToPeer->initAndStartNfc();
    }

    // Start detecting targets
    bool activationSuccessful = m_nfcManager->startTargetDetection();
    if (activationSuccessful) {
        emit nfcStatusUpdate("Successfully started target detection");
        success = true;
    } else {
        emit nfcStatusError("Error starting NFC target detection");
    }

    m_currentActivity = NfcIdle;
    emit nfcInitialized(success);
    return success;
}

/*!
  \brief Get a pointer to the NfcRecordModel instance used by this class.
*/
NfcRecordModel * NfcInfo::recordModel() const
{
    return m_nfcRecordModel;
}

void NfcInfo::setUnlimitedAdvancedMsgs(const bool unlimited)
{
    m_unlimitedAdvancedMsgs = unlimited;
}

/*!
  \brief Slot to be called whenever the contents of the record model changed,
  so that the contents can be converted to an NDEF message to see if the
  resulting size changed.

  If the NDEF message size does change, the method will emit the
  storedMessageSizeChanged signal, passing the byte-size of the message as
  parameter.
  */
void NfcInfo::nfcRecordModelChanged()
{
    // Calculate new message size
    QNdefMessage* message = recordModel()->convertToNdefMessage();
    QByteArray rawMessage = message->toByteArray();
    const int bytesize = rawMessage.size();
    delete message;
    emit storedMessageSizeChanged(bytesize);
}

/*!
  \brief Check if NFC is supported by the device and if it
  is activated.

  Note: currently, this is only implemented for Symbian using
  nfcsettings component (which uses native Symbian code to query
  the NFC status).

  \return true if and only if NFC is available and activated on Symbian.
  The nfcStatusError / nfcStatusUpdate signals contain more information
  about the actual status (e.g., if the device would support NFC but
  the user needs to activate it).
  */
bool NfcInfo::checkNfcStatus()
{
#ifdef Q_OS_SYMBIAN
    // Construct a new instance.
    nfcSettings = new NfcSettings(this);

    // Retrieve the NFC feature support information.
    NfcSettings::NfcFeature nfcFeature = nfcSettings->nfcFeature();

    if (nfcFeature == NfcSettings::NfcFeatureSupported) {
        // Connect signals for receiving mode change and error notifications.
        connect(nfcSettings, SIGNAL(nfcModeChanged(NfcSettings::NfcMode)), SLOT(handleNfcModeChange(NfcSettings::NfcMode)));
        connect(nfcSettings, SIGNAL(nfcErrorOccurred(NfcSettings::NfcError, int)), SLOT(handleNfcError(NfcSettings::NfcError, int)));

        // Retrieve the initial value of the NFC mode setting.
        NfcSettings::NfcMode nfcMode = nfcSettings->nfcMode();

        if (nfcMode != NfcSettings::NfcModeOn) {
            // NFC is supported but not switched on, prompt the user to enable it.
            emit nfcStatusError(tr("NFC hardware is available but currently switched off"));
            return false;
        } else {
            emit nfcStatusUpdate(tr("NFC is supported and switched on"));
            return true;
        }
    }
    else if (nfcFeature == NfcSettings::NfcFeatureSupportedViaFirmwareUpdate) {
        // Display message to user to update device firmware
        emit nfcStatusError(tr("Update device firmware to enable NFC support"));
        return false;
    } else {
        // Display message informing the user that NFC is not supported by this device.
        emit nfcStatusError(tr("NFC not supported by this device"));
        return false;
    }
#endif
    return false;
}

/*!
  \brief Set the image cache to use for storing images retrieved
  from tags.
  \a tagImageCache instance of the image cache. This
  class will not take ownership of the instance!
  */
void NfcInfo::setImageCache(TagImageCache *tagImageCache)
{
    m_nfcNdefParser->setImageCache(tagImageCache);
}

/*!
  \brief Called by the NearFieldManager whenever it finds a target.

  This method will create connections between the target and this class
  to be informed about its status. It also attempts to analyze the target
  and emits information through nfcStatusUpdate signals. If a write operation
  is pending, it will be written to the tag. Otherwise, the tag contents will be
  read (if possible).
  */
void NfcInfo::targetDetected(QNearFieldTarget *target)
{
    // Handle potential errors emitted by the target
    connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
            this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));
    connect(target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId)),
            this, SLOT(requestCompleted(QNearFieldTarget::RequestId)));
    connect(target, SIGNAL(ndefMessagesWritten()),
            this, SLOT(ndefMessageWritten()));

    m_currentActivity = NfcTargetAnalysis;
    // Cache the target in any case for future writing
    // (so that we can also write on tags that are empty as of now)
    m_cachedTarget = target;

    startedTagInteraction();
    // Check if the target includes a NDEF message
    bool targetHasNdefMessage = target->hasNdefMessage();
    if (targetHasNdefMessage) {
        emit nfcStatusUpdate("NDEF target detected");
    } else {
        emit nfcStatusUpdate("Target detected");
    }

    // Analyze the target and send the info to the UI
    emit nfcInfoUpdate(m_nfcTargetAnalyzer->analyzeTarget(target));
    m_currentActivity = NfcIdle;

    if (!m_pendingWriteNdef) {
        // Count number of tags read with the app
        // (don't count when in write mode)
        m_nfcStats->incTagReadCount();
    }


    // Check if we have NDEF access and can read or write to the tag
    QNearFieldTarget::AccessMethods accessMethods = target->accessMethods();
    // Work with NDEF both when NdefAccess or TagTypeSpecificAccess is set
    // This is due to the way the enum is defined (NdefAccess = 0x0,
    // TagTypeSpecificAccess = 0x1).
    // When testing for a flag, it is therefore impossible for a tag that has
    // tag type specific access to also report NdefAccess.
    // See: https://bugreports.qt-project.org/browse/QTMOBILITY-2024
    if (accessMethods.testFlag(QNearFieldTarget::NdefAccess) ||
            accessMethods.testFlag(QNearFieldTarget::TagTypeSpecificAccess)) {

#ifdef Q_OS_SYMBIAN
        // Bug workaround on Symbian: hasNdefMessage() always returns false
        // for a NFC Forum Tag Type 4, even if an NDEF message is present on the tag.
        // See: https://bugreports.qt.nokia.com/browse/QTMOBILITY-2018
        if (target->type() == QNearFieldTarget::NfcTagType4 && !targetHasNdefMessage) {
            targetHasNdefMessage = true;
        }
#endif
        // Is a write operation pending?
        if (!m_pendingWriteNdef)
        {
            // NO write operation pending, so read the tag if possible
            // If target has an NDEF message...
            if (targetHasNdefMessage)
            {
                m_currentActivity = NfcNdefReading;
                m_cachedRequestType = NfcNdefReading;
                // Target has NDEF messages: read them (asynchronous)
                connect(target, SIGNAL(ndefMessageRead(QNdefMessage)),
                        this, SLOT(ndefMessageRead(QNdefMessage)));
                m_cachedRequestId = target->readNdefMessages();
            } else {
                // No NDEF message detected
                qDebug() << "No NDEF message detected";
                emit nfcTagContents(tr("No NDEF message detected"), "");
                stoppedTagInteraction();
            }
        } else {
            // Write operation is pending, so attempt writing the message.
            if (m_harmattanPr10) {
                m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
            }
            writeCachedNdefMessage();
        }
    } else if (accessMethods.testFlag(QNearFieldTarget::LlcpAccess) && m_usePeerToPeer) {
        // Establish peer to peer connection
        m_nfcPeerToPeer->targetDetected(target);
        // Is a write operation pending?
        if (m_pendingWriteNdef)
        {
            writeCachedNdefMessage();
        }
    } else {
        // No NDEF access - no further interaction
        qDebug() << "No NDEF access to the target";
        stoppedTagInteraction();
    }

}

/*!
  \brief Slot needed for the registerNdefMessageHandler() method.

  This isn't used by the app directly, as it reads NDEF messages through the
  two-step process of first detecting targets, and then reading the messages.
  However, for registering an app for autostart on nfc tag touch, Qt Mobility
  requires to also use the registerNdefMessageHandler() handler, as this
  slot will get called when the app has been autostarted through a tag.
  */
void NfcInfo::targetMessageDetected(const QNdefMessage &message, QNearFieldTarget* target)
{
    // TODO: on Symbian, target isn't set -> check and maybe create a bug report.
    qDebug() << "NDEF Message detected (-> Autostart)!";
    emit nfcStatusUpdate("NDEF Message detected");
    if (target) {
        // Analyze the target and send the info to the UI
        emit nfcInfoUpdate(m_nfcTargetAnalyzer->analyzeTarget(target));
    }
    // Go through all records in the message
    ndefMessageRead(message);
#ifdef MEEGO_EDITION_HARMATTAN
    // MeeGo: raise the app to the foreground in case it was autostarted
    // by touching the tag AND it was already running in the background.
    // If we wouldn't do it, the app would receive the tag, but remain
    // in the background.
    if (m_declarativeView) {
        m_declarativeView->raise();
    }
#endif
}


/*!
  \brief Emits the nfcTagContents containing a textual description of the
  contents of the NDEF message.

  In case pictures are found, these are added to the image cache and the
  nfcTagImage signal is emitted together with the image ID.
  \param message the NDEF message to analyze.
  */
void NfcInfo::ndefMessageRead(const QNdefMessage &message)
{
    QString fileName = storeNdefToFile(QString(), message, true);
    emit nfcTagContents(m_nfcNdefParser->parseNdefMessage(message), fileName);
    stoppedTagInteraction();
}

/*!
  \brief Store a raw NDEF message to a file.

  The file will be stored inside the directory specified by AppSettings,
  which is further adapted by the collected parameter.

  \param fileName if specified, the filename to use. The ".txt" extension
  will be added to the file name automatically if it does not have a ".txt"
  extension already. If an empty QString is passed ("QString()"), a default
  file name will be created, based on the current date, time and (if available)
  the tag type.
  \param message the NDEF message to store as raw byte array in the file.
  \param collected influences the sub directory of the data directory.
  If set to true, it will go to the collected subdirectory for
  auto-collected/saved tags. If set to true, it will go to the subdirectory
  for manually saved messages.
  */
QString NfcInfo::storeNdefToFile(const QString& fileName, const QNdefMessage &message, const bool collected)
{
    QString fullFileName = "";
    if (m_appSettings && m_appSettings->logNdefToFile()) {
        // Store tag contents to the log file if enabled
        const QString writeDir = m_appSettings->logNdefDir(collected);
        QDir dir("/");
        dir.mkpath(writeDir);
        if (QDir::setCurrent(writeDir)) {
            if (fileName.isEmpty()) {
                // Generate file name
                QDateTime now = QDateTime::currentDateTime();
                fullFileName = now.toString("yyyy.MM.dd - hh.mm.ss");
                if (!m_nfcTargetAnalyzer->m_tagInfo.tagTypeName.isEmpty()) {
                    fullFileName.append(" - ");
                    fullFileName.append(m_nfcTargetAnalyzer->m_tagInfo.tagTypeName);
                }
                fullFileName.append(".txt");
            } else {
                fullFileName = fileName;
                if (fullFileName.right(4).toLower() != ".txt") {
                    fullFileName.append(".txt");
                }
            }
            QFile tagFile(fullFileName);
            if (tagFile.open(QIODevice::WriteOnly)) {
                tagFile.write(message.toByteArray());
                tagFile.close();
            } else {
                qDebug() << "Unable to open file for writing: " << tagFile.fileName();
            }
            fullFileName = writeDir + fullFileName;
        } else {
            emit nfcStatusError("Unable to open data directory (" + writeDir + ") - please check the application settings");
            qDebug() << "Unable to set current directory to: " << writeDir;
        }
    }
    return fullFileName;
}

/*!
  \brief Create the message for writing to the tag and attempt
  to write it.

  \param writeOneTagOnly automatically switch back to tag reading
  mode after writing one tag, or stay in tag writing mode and also
  write the same message to further targets.

  \return if it was already possible to write to the tag. If
  false is returned, the message is cached and will be written
  when a writable target is available. Only one message is cached;
  if this method is called a second time before the first message
  is actually written to a tag, the old message will be discarded
  and only the later one written to the tag.
  */
bool NfcInfo::nfcWriteTag(const bool writeOneTagOnly)
{
    // Convert the model into a NDEF message
    QNdefMessage* message = recordModel()->convertToNdefMessage();
    m_cachedNdefContainsAdvMsg = recordModel()->containsAdvMsg();

    // Set to writing mode
    emit nfcModeChanged(NfcTypes::nfcWriting);
    if (m_harmattanPr10) {
        m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    }

    QByteArray rawMessage = message->toByteArray();
    emit nfcStatusUpdate("Created message (size: " + QString::number(rawMessage.size()) + " bytes)");

    // Write the message (containing either a URL or plain text) to the target.
    if (m_cachedNdefMessage) { delete m_cachedNdefMessage; }
    m_cachedNdefMessage = message;
    m_cachedNdefMessageSize = m_cachedNdefMessage->toByteArray().size();
    m_pendingWriteNdef = true;
    m_writeOneTagOnly = writeOneTagOnly;
    return writeCachedNdefMessage();
}

/*!
  \brief Load an NDEF message from a file and attempt
  to write it.

  The specified file has to contain the raw and complete NDEF
  message contents.

  \param fileName data file that contains the complete, binary
  contents of an NDEF message.

  \param writeOneTagOnly automatically switch back to tag reading
  mode after writing one tag, or stay in tag writing mode and also
  write the same message to further targets.

  \return if it was already possible to write to the tag. If
  false is returned, the message is cached and will be written
  when a writable target is available. Only one message is cached;
  if this method is called a second time before the first message
  is actually written to a tag, the old message will be discarded
  and only the later one written to the tag.
  Additionally, false can also be returned if loading or parsing
  the file was not successful. In this case, the method will also
  emit nfcTagWriteError() with the error message, and does not
  switch the class to write mode.
  */
bool NfcInfo::nfcWriteTag(const QString& fileName, const bool writeOneTagOnly)
{
    QNdefMessage message = loadNdefFromFile(fileName);
    if (message.isEmpty()) {
        // Error while loading from the file - don't switch to writing mode.
        // Error info has already been emitted by loading method.
        return false;
    }

    // Set to writing mode
    emit nfcModeChanged(NfcTypes::nfcWriting);
    if (m_harmattanPr10) {
        m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    }

    // Write the message (containing either a URL or plain text) to the target.
    if (m_cachedNdefMessage) { delete m_cachedNdefMessage; }
    m_cachedNdefMessage = new QNdefMessage(message);
    m_cachedNdefMessageSize = m_cachedNdefMessage->toByteArray().size();
    m_pendingWriteNdef = true;
    m_writeOneTagOnly = writeOneTagOnly;

    return writeCachedNdefMessage();
}

/*!
  \brief Load tag contents from file name and put contents into
  the record model for editing.

  \param fileName data file that contains the complete, binary
  contents of an NDEF message.
  */
bool NfcInfo::nfcEditTag(const QString& fileName)
{
    QNdefMessage message = loadNdefFromFile(fileName);
    if (message.isEmpty()) {
        // Error while loading from the file - don't switch to writing mode.
        // Error info has already been emitted by loading method.
        return false;
    }
    // Parse contents of the message into the record model
    m_nfcNdefParser->setParseToModel(true);
    m_nfcNdefParser->parseNdefMessage(message);
    m_nfcNdefParser->setParseToModel(false);
    return true;
}

QString NfcInfo::nfcSaveModelToFile(const QString &fileName)
{
    QString savedFileName = storeNdefToFile(fileName, *m_nfcRecordModel->convertToNdefMessage(), false);
    if (!savedFileName.isEmpty()) {
        emit nfcStatusSuccess("Stored NDEF message to " + savedFileName);
    } else {
        emit nfcStatusError("Unable to save NDEF message");
    }
    return savedFileName;
}

QNdefMessage NfcInfo::loadNdefFromFile(const QString& fileName)
{
    qDebug() << "Load tag: " << fileName;
    if (fileName.isEmpty()) {
        emit nfcTagWriteError("No file name specified");
        return QNdefMessage();
    }
    // Load the NDEF message from the specified file name
    QFile tagFile(fileName);
    if (!tagFile.open(QIODevice::ReadOnly)) {
        emit nfcTagWriteError("Unable to open file: " + fileName);
        return QNdefMessage();
    }
    QByteArray rawMessage = tagFile.readAll();
    tagFile.close();
    if (rawMessage.isEmpty()) {
        // Check if reading the file was successful
        emit nfcTagWriteError("Unable to load file: " + fileName);
        return QNdefMessage();
    }
    emit nfcStatusUpdate("Loaded message (size: " + QString::number(rawMessage.size()) + " bytes)");
    QNdefMessage message = QNdefMessage::fromByteArray(rawMessage);
    if (message.isEmpty()) {
        // Unable to create an NDEF message from the file
        emit nfcTagWriteError("Unable to create NDEF message from file: " + fileName);
        return QNdefMessage();
    }
    return message;
}

/*!
  \brief Stop waiting to write a tag, and switch back to reading mode.
  */
void NfcInfo::nfcStopWritingTags()
{
    m_pendingWriteNdef = false;
    m_writeOneTagOnly = false;
    emit nfcModeChanged(NfcTypes::nfcReading);
    if (m_harmattanPr10) {
        m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    }
}

/*!
  \brief Attempt to write the currently cached message to the tag.

  \return true if it was possible to send the request to the tag.
  */
bool NfcInfo::writeCachedNdefMessage()
{
    bool success = false;
    if (m_pendingWriteNdef && m_cachedNdefMessage)
    {
        if (!m_unlimitedAdvancedMsgs) {
            qDebug() << "Advanced messages written: " << m_nfcStats->advMsgWrittenCount();
        }
        if (m_cachedNdefContainsAdvMsg &&
                !m_unlimitedAdvancedMsgs &&
                m_nfcStats->advMsgWrittenCount() > ADV_MSG_WRITE_COUNT) {
            // Not allowed to write more advanced tags in trial mode
            // Setting success to true will trigger the signal that the
            // tag interaction is stopped.
            success = true;
            // Don't switch to reading mode by default, as the app would still stay
            // in the write page and then no longer show any writing issues,
            // but instead succeed in reading the tag.
            if (!m_writeOneTagOnly) {
                // If writing only one tag, deactivate the writing mode again.
                m_pendingWriteNdef = false;
                emit nfcModeChanged(NfcTypes::nfcReading);
                if (m_harmattanPr10) {
                    m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
                }
            }
            emit nfcTagWriteExceeded();
        } else {
            if (m_cachedTarget)
            {
                // Check target access mode
                QNearFieldManager::TargetAccessModes accessModes = m_nfcManager->targetAccessModes();
                // Writing access is active - we should be able to write
                if (m_cachedTarget->accessMethods().testFlag(QNearFieldTarget::LlcpAccess) &&
                        m_usePeerToPeer && m_nfcPeerToPeer)
                {
                    // -----------------------------------------------------
                    // Peer to peer (SNEP)
                    m_nfcPeerToPeer->sendNdefMessage(m_cachedNdefMessage);
                }
                else if (accessModes.testFlag(QNearFieldManager::NdefWriteTargetAccess))
                {
                    // -----------------------------------------------------
                    // NDEF Access
                    m_currentActivity = NfcNdefWriting;
                    if (m_appSettings->deleteTagBeforeWriting() && m_cachedRequestType != NfcNdefDeleting) {
                        // Write an empty message first
                        m_cachedRequestType = NfcNdefDeleting;
                        emit nfcStatusUpdate("Writing empty message to the tag");
                        // TODO: check if we need to add an empty record, or if
                        // formatting is also done like this.
                        m_cachedRequestId = m_cachedTarget->writeNdefMessages(QList<QNdefMessage>() << (QNdefMessage()));
                    } else {
                        qDebug() << "Writing message: " << m_cachedNdefMessage->toByteArray();
                        // Either the empty message was already written, or
                        // configuration is not set to delete the message first.
                        m_cachedRequestType = NfcNdefWriting;
                        emit nfcStatusUpdate("Writing message to the tag");
                        m_cachedRequestId = m_cachedTarget->writeNdefMessages(QList<QNdefMessage>() << (*m_cachedNdefMessage));
                    }
                    success = true;
                    if (!m_writeOneTagOnly && m_cachedRequestType != NfcNdefDeleting) {
                        // If writing only one tag, deactivate the writing mode again.
                        m_pendingWriteNdef = false;
                        emit nfcModeChanged(NfcTypes::nfcReading);
                        if (m_harmattanPr10) {
                            m_nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
                        }
                    }

                }
                else
                {
                    // -----------------------------------------------------
                    // Not in right mode / not right target
                    emit nfcStatusUpdate("Please touch the tag again to write the message");
                }
            } else {
                // Can't write - no cached target available
                emit nfcStatusUpdate("Please touch a tag to write the message");
            }
        }
    }
    if (!success) {
        // Didn't start a request to write a message - finished interacting
        // with the tag
        stoppedTagInteraction();
    }
    return success;
}

/*!
  \brief This method should be called by the code of this class whenever the
  app is starting interaction with a tag.

  This also emits the nfcStartingTagInteraction() signal, so that the UI
  can for example show a busy animation, informing the user that he should
  keep the phone close to the tag until tag interaction is finished.
  */
void NfcInfo::startedTagInteraction() {
    if (!m_nfcTagInteractionActive) {
        m_nfcTagInteractionActive = true;
        emit nfcStartingTagInteraction();
        qDebug() << "*** Starting tag interaction...";
    }
}

/*!
  \brief This method should be called by the code of this class whenever the
  app is stopping interaction with a tag.

  This also emits the nfcStoppedTagInteraction() signal, so that the UI
  can for example stop the busy animation, informing the user that it's now
  safe to remove the phone from the tag again.
  */
void NfcInfo::stoppedTagInteraction() {
    if (m_nfcTagInteractionActive) {
        m_nfcTagInteractionActive = false;
        emit nfcStoppedTagInteraction();
        qDebug() << "*** Stopped tag interaction...";
    }
}

/*!
  \brief Apply the new app settings to the NFC manager classes.
  */
void NfcInfo::applySettings()
{
    if (m_nfcPeerToPeer) {
        m_nfcPeerToPeer->applySettings();
    }
}

/*!
  \brief Slot for handling when the target was lost (usually when
  it gets out of range.
  */
void NfcInfo::targetLost(QNearFieldTarget *target)
{
    if (m_nfcPeerToPeer) {
        m_nfcPeerToPeer->targetLost(target);
    }
    m_cachedTarget = NULL;
    target->deleteLater();
    stoppedTagInteraction();
    emit nfcStatusUpdate("Target lost");
}

/*!
  \brief Slot for handling an error with a request to the target.

  Emits the nfcTagError signal containing a description of the error,
  or the nfcTagWriteError signal if the error occured during writing,
  including a more detailed analysis of what could have gone wrong
  (as the Qt Mobility APIs don't usually report a reason).
  */
void NfcInfo::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)
{
    QString errorText("Error: " + convertTargetErrorToString(error));
    qDebug() << errorText;
    if (id == m_cachedRequestId && m_cachedRequestType == NfcNdefDeleting) {
        // Writing the empty message failed - try to write the full message
        // before reporting the error
        writeCachedNdefMessage();
        errorText.append("\n\nFailed to write empty message - attempting to write full NDEF message.");
        emit nfcTagWriteError(errorText);
    } else if (id == m_cachedRequestId && m_cachedRequestType == NfcNdefWriting) {
        m_cachedRequestType = NfcIdle;
        if (!m_pendingWriteNdef) {
            m_currentActivity = NfcIdle;
        }
        errorText.append("\n\nUnable to write message.");

        if (m_nfcTargetAnalyzer->m_tagInfo.combinedWriteAccess() == NearFieldTargetInfo::NfcAccessForbidden) {
            errorText.append("\nTag is write-protected.");
        } else if (m_nfcTargetAnalyzer->m_tagInfo.combinedWriteAccess() == NearFieldTargetInfo::NfcAccessUnknown) {
            errorText.append("\nTag write access unknown.");
        }
        // Compare tag size to message size
        const int tagWritableSize = m_nfcTargetAnalyzer->m_tagInfo.tagWritableSize;
        // Check if the app was successful in determining the tag size
        if (tagWritableSize > 0 && m_cachedNdefMessageSize > 0) {
            // Known tag size - we can do a proper check
            if (m_cachedNdefMessageSize > tagWritableSize) {
                // Message was too large for the target.
                errorText.append("\n\nMessage (" + QString::number(m_cachedNdefMessageSize) + " bytes) and control data were probably too large for the available tag size (" + QString::number(tagWritableSize) + " bytes).");
            }
        } else if (tagWritableSize <= 0 && m_cachedNdefMessageSize > 0 && m_cachedTarget) {
            // Don't know the tag size - print a warning for typical tag sizes
            // that we have have to guess
            // This happens if the tag has issues, if Qt Mobility APIs
            // didn't allow reading the size (on Symbian Anna or Harmattan),
            // or if this app doesn't support reading the tag size.
            int memorySizeWarning;
            switch (m_cachedTarget->type()) {
            case QNearFieldTarget::NfcTagType2:
                memorySizeWarning = GUESS_TYPE2_SIZE;
                break;
            case QNearFieldTarget::NfcTagType3:
                memorySizeWarning = GUESS_TYPE3_SIZE;
                break;
            case QNearFieldTarget::NfcTagType4:
                memorySizeWarning = GUESS_TYPE4_SIZE;
                break;
            case QNearFieldTarget::MifareTag:
                memorySizeWarning = GUESS_MIFARE_SIZE;
                break;
            case QNearFieldTarget::NfcTagType1:
            default:
                memorySizeWarning = GUESS_TYPE1_SIZE;
                break;
            }
            if (m_cachedNdefMessageSize > memorySizeWarning) {
                errorText.append("\n\nMessage (" + QString::number(m_cachedNdefMessageSize) + " bytes) plus control data might be too large for the " + m_nfcTargetAnalyzer->convertTagTypeToString(m_cachedTarget->type()) + " target?");
            }
        }
        emit nfcTagWriteError(errorText);
        stoppedTagInteraction();
    } else if (id == m_cachedRequestId && m_cachedRequestType == NfcNdefReading) {
        // Error while reading the tag
        emit nfcTagError(errorText);
        stoppedTagInteraction();
    } else {
        // Filter errors when not reading or writing
        // E.g., during analysis, the Qt Mobility APIs might not support some tag-specific
        // access, resulting in InvalidParametersError or UnsupportedError.
        // The analysis part will correctly handle those cases, no need to spam the user
        // with error messages.
        if (m_reportingLevel == AppSettings::FullReporting ||
                (error != QNearFieldTarget::InvalidParametersError &&
                 error != QNearFieldTarget::UnsupportedError)) {
            emit nfcTagError(errorText);
        }
    }
}

/*!
  \brief Slot called by Qt Mobility when a request has been completed.

  This method emits the nfcStatusUpdate signal to log the event in the
  user interface. In case the request was of a kind that contains a
  response, information about the response will also be emitted through
  another signal of the same type. In case the response is a byte array,
  it will be printed as a string of hex characters.
  */
void NfcInfo::requestCompleted(const QNearFieldTarget::RequestId &id)
{
    QString message;
    bool showAnotherTagWriteMsg = false;
    if (id == m_cachedRequestId) {
        bool noStatusChange = false;
        switch (m_cachedRequestType) {
        case NfcIdle: {
            message = "Active request completed.";
            m_currentActivity = NfcIdle;
            break; }

        case NfcNdefReading: {
            message = "Read request completed.";
            m_currentActivity = NfcIdle;
            break; }

        case NfcNdefDeleting: {
            // requestCompleted() is not called on Symbian, only
            // ndefMessageWritten().
            // On the n9, both requestCompleted() and ndefMessageWritten()
            // are called after the empty message has been written.
            // -> only start writing the "real" message from one place,
            // so that we don't start two simultaneous write requests on the N9.
            // Also, don't change the status of the class to say that
            // we stopped tag interaction.
            // Plus, the cached request type needs to remain at deleting,
            // so that the write method knows that it should proceed to the
            // second step and write the real message now.
            noStatusChange = true;
            break; }

        case NfcNdefWriting: {
            message = "Write request completed.";
            if (m_pendingWriteNdef) {
                // Writing tags is still pending - means the user can write another
                // tag with the same contents.
                showAnotherTagWriteMsg = true;
            } else {
                m_currentActivity = NfcIdle;  // Read or write request finished
            }
            break; }

        default: {
            message = "Request completed.";
            break; }
        }
        if (!message.isEmpty()) {
            qDebug() << message;
            if (!(m_cachedRequestType == NfcNdefWriting && m_reportingLevel == AppSettings::OnlyImportantReporting)) {
                // Writing success will already be reported by the nfcTagWritten method
                // (this method is called as well on MeeGo, resulting in 2x status updates).
                // Therefore, for writing, do not print this status mesesage if
                // reporting is set to only important.
                emit nfcStatusSuccess(message);
            }
        }
        if (!noStatusChange) {
            m_cachedRequestType = NfcIdle;
            stoppedTagInteraction();
        }
    } else {
        message = "Request completed.";
        if (m_reportingLevel == AppSettings::DebugReporting) {
            qDebug() << message;
        }
    }
    // This is already done by the nfcTagWritten() method.
    //    if (showAnotherTagWriteMsg) {
    //        // Emit the message that the user can write another tag.
    //        // (after we emitted the message that the previous write request completed).
    //        emit nfcStatusUpdate("Touch another tag to write again.");
    //    }

    // Request the response in case we're in debug reporting mode
    // Usually, if the response is important, it will be handled directly
    // by the requesting method.
    if (m_reportingLevel == AppSettings::DebugReporting) {
        // Print the response of the tag to the debug output
        // in case debug reporting is active.
        if (m_cachedTarget)
        {
            QVariant response = m_cachedTarget->requestResponse(id);
            if (response.isValid()) {
                if (response.type() == QVariant::ByteArray) {
                    //emit nfcStatusUpdate("Response (" + QString(response.typeName()) + ")");
                    qDebug() << "Response (" << QString(response.typeName()) << ")";
                } else {
                    //emit nfcStatusUpdate("Response (" + QString(response.typeName()) + "): " + response.toString());
                    qDebug() << "Response (" << QString(response.typeName()) << "): " << response.toString();
                }
                if (response.type() == QVariant::ByteArray) {
                    QByteArray p = response.toByteArray();
                    QString arrayContents = "";
                    for (int i = 0; i < p.size(); ++i) {
                        arrayContents.append(QString("0x") + QString::number(p.at(i), 16) + " ");
                    }
                    qDebug() << "Raw contents of response:\n" << arrayContents;
                }
            }
        }
    }
}

/*!
  \brief Slot called by Qt Mobility when an NDEF message was successfully
  written to a tag.

  Emits an nfcStatusUpdate signal to log this in the user interface.
  On MeeGo, both the requestCompleted() method and this method will be called
  when writing a tag.
  */
void NfcInfo::ndefMessageWritten()
{
    if (m_cachedRequestType == NfcNdefDeleting) {
        // Start writing the full message, now that
        // the empty message has been written
        emit nfcStatusSuccess("Empty message written.");
        // Need to start the second write request with a delay from
        // a separate thread; wouldn't work otherwise.
        QTimer::singleShot(50, this, SLOT(writeCachedNdefMessage()));
        return;
    }
    // Store the composed message type count to the actual written count
    m_nfcStats->commitComposedToWrittenCount();
    emit nfcTagWritten();
    stoppedTagInteraction();

    if (m_pendingWriteNdef) {
        // Writing tags is still pending - means the user can write another
        // tag with the same contents.
        // If the class is only supposed to write to one tag,
        // writeCachedNdefMessage() would have changed m_pendingWriteNdef to false.
        emit nfcStatusUpdate("Touch another tag to write again.");
    } else {
        m_currentActivity = NfcIdle;
    }
}

/*!
  \brief Return a textual description of the \a error.
  */
QString NfcInfo::convertTargetErrorToString(QNearFieldTarget::Error error)
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
        errorString = "Generic communication error"; //"Invalid parameters were passed to a tag type specific function.";
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

/*!
  \brief Store a pointer to the declarative view.

  Needed for raising the app to the foreground on MeeGo in the
  autostart scenario when the app is already active in the background.
  */
void NfcInfo::setDeclarativeView(QDeclarativeView& view)
{
    m_declarativeView = &view;
}

void NfcInfo::setAppSettings(AppSettings *appSettings)
{
    m_appSettings = appSettings;
    m_appSettings->setParent(this);
    if (m_nfcNdefParser) {
        m_nfcNdefParser->setAppSettings(m_appSettings);
    }
    if (m_nfcPeerToPeer) {
        m_nfcPeerToPeer->setAppSettings(m_appSettings);
    }
}

#ifdef Q_OS_SYMBIAN
/*!
  \brief Slot will be executed by the nfcsettings component whenever
  the NFC availability changes in the phone.

  This can also happen at runtime, for example when the user switches
  on NFC through the settings app. Emits an nfcStatusError or nfcStatusUpdate
  signal with the new information.
  */
void NfcInfo::handleNfcModeChange(NfcSettings::NfcMode nfcMode)
{
    switch (nfcMode) {
    case NfcSettings::NfcModeNotSupported:
        // NFC is not currently supported. It is not possible to distinguish
        // whether a firmware update could enable NFC features based solely
        // on the value of the nfcMode parameter. The return value of
        // NfcSettings::nfcFeature() indicates whether a firmware update is
        // applicable to this device.
        emit nfcStatusError(tr("NFC is not currently supported"));
        break;
    case NfcSettings::NfcModeUnknown:
        // NFC is supported, but the current mode is unknown at this time.
        emit nfcStatusError(tr("NFC is supported, but the current mode is unknown at this time"));
        break;
    case NfcSettings::NfcModeOn:
        // NFC is supported and switched on.
        emit nfcStatusUpdate(tr("NFC is supported and switched on"));
        break;
    case NfcSettings::NfcModeCardOnly:
        // NFC hardware is available and currently in card emulation mode.
        emit nfcStatusError(tr("NFC hardware is available and currently in card emulation mode"));
        break;
    case NfcSettings::NfcModeOff:
        // NFC hardware is available but currently switched off.
        emit nfcStatusError(tr("NFC hardware is available but currently switched off"));
        break;
    default:
        break;
    }
}

/*!
  \brief Handle any error that might have occured when checking
  the NFC support on the phone.
  */
void NfcInfo::handleNfcError(NfcSettings::NfcError nfcError, int error)
{
    // The platform specific error code is ignored here.
    Q_UNUSED(error)

    switch (nfcError) {
    case NfcSettings::NfcErrorFeatureSupportQuery:
        // Unable to query NFC feature support.
        emit nfcStatusError(tr("Unable to query NFC feature support"));
        break;
    case NfcSettings::NfcErrorSoftwareVersionQuery:
        // Unable to query device software version.
        emit nfcStatusError(tr("Unable to query device software version"));
        break;
    case NfcSettings::NfcErrorModeChangeNotificationRequest:
        // Unable to request NFC mode change notifications.
        emit nfcStatusError(tr("Unable to request NFC mode change notifications"));
        break;
    case NfcSettings::NfcErrorModeChangeNotification:
        // NFC mode change notification was received, but caused an error.
        emit nfcStatusError(tr("NFC mode change notification was received, but caused an error"));
        break;
    case NfcSettings::NfcErrorModeRetrieval:
        // Unable to retrieve current NFC mode.
        emit nfcStatusError(tr("Unable to retrieve current NFC mode"));
        break;
    default:
        break;
    }
}



#endif
