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

import QtQuick 1.1
import com.nokia.symbian 1.1 // Symbian Qt Quick components

import Nfc 1.0
import com.nfcinfo.types 1.0

PageStackWindow {
    id: window
    initialPage: nfcInfoPage
    showStatusBar: true
    showToolBar: true

    NfcInfoPage {
        id: nfcInfoPage
    }

    // IAP
    // --------------------------------------------------------------
    Loader {
        id: iapLoader
        anchors.fill: parent
        onStatusChanged: {
            if (status === Loader.Ready) {
                // Make sure we navigate first to the main page
                // (e.g., if using the button on the write tag page)
                pageStack.pop(null, true);
                pageStack.push(iapLoader.item);
            }
        }
    }
    function loadIapPage() {
        if (iapLoader.status === Loader.Ready) {
            // Already loaded IAP page
            // Make sure we navigate first to the main page
            // (e.g., if using the button on the write tag page)
            pageStack.pop(null, true);
            pageStack.push(iapLoader.item);
        } else {
            // Load IAP page.
            // Will push itself to the page stack when it's ready
            iapLoader.source = Qt.resolvedUrl("IapPage.qml");
        }
    }
    function isAdvTagsPurchased() {
        return useIap ? iapManager.isProductPurchased(iapIdAdvTags) : true;
    }
    function isRemoveAdsPurchased() {
        return useIap ? iapManager.isProductPurchased(iapIdRemoveAds) : true;
    }
    function isUnlimitedPurchased() {
        return useIap ? iapManager.isProductPurchased(iapIdUnlimited) : true;
    }
    function setUnlimitedAdvancedMsgs(unlimited) {
        nfcInfo.setUnlimitedAdvancedMsgs(unlimited);
    }
    function setRemoveAds(removeAds) {
        if (removeAds) {
            nfcInfoPage.hideAds();
        } else {
            nfcInfoPage.loadAds();
        }
    }

    // --------------------------------------------------------------

    Loader {
        id: composeTagPageLoader
        anchors.fill: parent
    }
    Loader {
        id: writeTagPageLoader
        anchors.fill: parent
    }
    Loader {
        id: instructionsLoader
        anchors.fill: parent
    }
    Loader {
        id: settingsPageLoader
        anchors.fill: parent
    }

    // Add a message to the model
    function logMessage(text, color, img, nfcDataFileName)
    {
        nfcInfoPage.logMessage(text, color, img, nfcDataFileName)
    }

    // Add an image to the model (message with empty text but custom image)
    function logImage(imgId)
    {
        nfcInfoPage.logMessage("", "white", ("image://nfcimageprovider/"+imgId), "")
    }

    Connections {
        target: nfcInfo

        onNfcInitialized: {
            timerPages.restart();
        }
        onNfcStatusUpdate: {
            logMessage(nfcStatusText, "aliceblue", "nfcSymbolInfo.png");
        }
        onNfcStatusSuccess: {
            logMessage(nfcStatusSuccessText, "aliceblue", "nfcSymbolSuccess.png");
        }
        onNfcStatusError: {
            logMessage(nfcStatusErrorText, "coral", "nfcSymbolError.png");
        }
        onNfcInfoUpdate: {
            logMessage(nfcInfoText, "aquamarine", "nfcSymbolTag.png");
        }
        onNfcTagContents: {
            logMessage(nfcTagContents, "lightskyblue", "nfcSymbolTag.png", nfcDataFileName);
        }
        onNfcTagImage: {
            logImage(nfcImgId);
        }
        onNfcTagError: {
            logMessage(nfcTagError, "coral", "nfcSymbolError.png");
        }

        onNfcStartingTagInteraction: {
            if (writeTagPageLoader.status === Loader.Ready &&
                    writeTagPageLoader.item.status === PageStatus.Active) {
                // Active page is writing page - start busy animation
                writeTagPageLoader.item.startWriting();
            } else if (nfcInfoPage.status === PageStatus.Active) {
                nfcInfoPage.showHideBusy(true);
            }
        }

        onNfcStoppedTagInteraction: {
            // Hide the busy animation in any case - the user might have
            // switched away from the page while tag reading was active.
            nfcInfoPage.showHideBusy(false);
            if (writeTagPageLoader.status === Loader.Ready) {
                writeTagPageLoader.item.cancelWriting();
            }
        }

        onNfcTagWritten: {
            logMessage("Message written to the tag / device", "aliceblue", "nfcSymbolSuccess.png");
            if (writeTagPageLoader.status === Loader.Ready) {
                writeTagPageLoader.item.tagWritten();
            }
        }
        onNfcTagWriteError: {
            logMessage(nfcTagError, "coral", "nfcSymbolError.png");
            if (writeTagPageLoader.status === Loader.Ready) {
                writeTagPageLoader.item.tagWriteError(nfcTagError);
            }
        }
        onNfcTagWriteExceeded: {
            if (writeTagPageLoader.status === Loader.Ready) {
                writeTagPageLoader.item.tagWriteExceeded();
            }
        }

        onNfcModeChanged: {
            if (nfcNewMode === 0) {
                // Reading; 1 would be writing, but the NfcInfo class will
                // send out more detailed textual signals about the write status,
                // so we won't print a message here to keep the amount of on-screen text down.
                logMessage("Switched back to tag reading mode", "aliceblue", "nfcSymbolInfo.png");
            }
        }

        onStoredMessageSizeChanged: {
            if (composeTagPageLoader.status === Loader.Ready) {
                composeTagPageLoader.item.updateHeader(ndefMessageSize)
            }
        }
    }

	
    Component.onCompleted: {
        // Start loading the sub-pages
        timerInit.restart();
    }

    Timer {
        id: timerInit
        interval: 100
        repeat: false
        onTriggered: {
            nfcInfo.initAndStartNfcAsync();
        }
    }

    Timer {
        id: timerPages
        interval: 100
        repeat: false
        onTriggered: {
            instructionsLoader.source = Qt.resolvedUrl("InstructionsPage.qml");
            writeTagPageLoader.source = Qt.resolvedUrl("WriteTagPage.qml");
            composeTagPageLoader.source = Qt.resolvedUrl("ComposeTagPage.qml");
            settingsPageLoader.source = Qt.resolvedUrl("SettingsPage.qml");
        }
    }
}
