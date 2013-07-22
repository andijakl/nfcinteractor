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

Page {
    tools: ToolBarLayout {
        ToolButton {
            flat: true
            iconSource: "toolbar-back";
            onClicked: {
                saveSettings();
                pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
            }
        }
    }

    // Internal use - for virtual keyboard handling
    property Item focusedItem

    // Settings
    property alias logNdefToFile: logNdefToFileEdit.checked
    property alias logNdefDir: logNdefDirEdit.text
    property alias deleteTagBeforeWriting: deleteTagBeforeWritingEdit.checked
    property alias useSnep: useSnepEdit.checked

    // - Raw peer to peer settings
    property alias useConnectionLess: connectionSwitch.checked
    property alias nfcPort: clPort.text
    property alias nfcUri: coUri.text
    property alias sendThroughServerSocket: coSendSocketSwitch.checked
    property alias connectClientSocket: coClientSocket.checked
    property alias connectServerSocket: coServerSocket.checked


    onStatusChanged: {
        if (status === PageStatus.Activating) {
            applySettingsToPage();
        }
    }

    function applySettingsToPage() {
        logNdefToFile = settings.logNdefToFile;
        logNdefDir = settings.logNdefDir;
        deleteTagBeforeWriting = settings.deleteTagBeforeWriting;
        useSnep = settings.useSnep;
        useConnectionLess = settings.useConnectionLess;
        nfcPort = settings.nfcPort;
        nfcUri = settings.nfcUri;
        sendThroughServerSocket = settings.sendThroughServerSocket;
        connectClientSocket = settings.connectClientSocket;
        connectServerSocket = settings.connectServerSocket;
    }

    function saveSettings() {
        console.log("Saving settings...");
        // Apply the new settings to the nfcPeerToPeer object
        settings.setLogNdefToFile(logNdefToFile);
        settings.setLogNdefDir(logNdefDir);
        settings.setDeleteTagBeforeWriting(deleteTagBeforeWriting);

        settings.setUseSnep(useSnep);
        settings.setUseConnectionLess(useConnectionLess);
        settings.setNfcPort(nfcPort);
        settings.setNfcUri(nfcUri);
        settings.setSendThroughServerSocket(sendThroughServerSocket);
        settings.setConnectClientSocket(connectClientSocket);
        settings.setConnectServerSocket(connectServerSocket);
        settings.saveSettings();

        nfcInfo.applySettings();
    }


    // ----------------------------------------------------------
    // Settings


    Row {
        id: settingsHeader
        spacing: customPlatformStyle.paddingMedium;
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Image {
            id: settingsImage
            source: "settings.svg"
            fillMode: Image.PreserveAspectFit
            asynchronous: true
        }

        Text {
            id: settingsHeaderText
            text: qsTr("Settings")
            verticalAlignment: Text.AlignVCenter
            height: settingsImage.height
            font.family: customPlatformStyle.fontFamilyRegular;
            color: customPlatformStyle.colorNormalLight
            font.pixelSize: customPlatformStyle.fontSizeLarge
        }
    }

    Rectangle {
        id: separator
        width: parent.width; height: 1; color: "gray"
        anchors.top: settingsHeader.bottom
    }


    Flickable {
        id: flickSettings

        clip: true
        contentHeight: settingsContentItem.height
        anchors.top: separator.bottom
        anchors.topMargin: customPlatformStyle.paddingMedium;
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: splitViewInput.top

        Column {
            id: settingsContentItem
            width: parent.width

            spacing: 7

            // --------------------------------------------------------------------------------
            // - Log NDEF messages to files
            Item {// 2-line text for checkbox not properly formated
                width: 1
                height: customPlatformStyle.paddingSmall;
            }
            CheckBox {
                id: logNdefToFileEdit
                checked: true
                text: "Save NDEF messages to files\n(required for cloning \& editing)"
                onClicked: {
                    logNdefDirEdit.enabled = logNdefToFileEdit.checked
                    logNdefDirTitle.color = (logNdefDirEdit.enabled) ? customPlatformStyle.colorNormalLight : customPlatformStyle.colorNormalMid;
                }
            }
            Item {// 2-line text for checkbox not properly formated
                width: 1
                height: customPlatformStyle.paddingSmall;
            }

            // --------------------------------------------------------------------------------
            // - Log directory
            Text {
                id: logNdefDirTitle
                text: qsTr("Saved NDEF directory")
                font.family: customPlatformStyle.fontFamilyRegular;
                color: customPlatformStyle.colorNormalLight
                font.pixelSize: customPlatformStyle.fontSizeMedium
            }

            TextField {
                id: logNdefDirEdit
                width: parent.width
                text: ""
                maximumLength: 255
                onActiveFocusChanged: {
                    if (activeFocus) {
                        focusedItem = logNdefDirEdit
                        moveToFocusedItem();
                    }
                }
            }

            // --------------------------------------------------------------------------------
            // - Delete tag before writing
            Item {// 2-line text for checkbox not properly formated
                width: 1
                height: customPlatformStyle.paddingSmall;
            }
            CheckBox {
                id: deleteTagBeforeWritingEdit
                checked: true
                text: "Delete/format tag before writing\n(use for factory empty tags)"
            }
            Item {// 2-line text for checkbox not properly formated
                width: 1
                height: customPlatformStyle.paddingSmall;
            }


            // --------------------------------------------------------------------------------
            Rectangle {
                id: separator2
                width: parent.width; height: 1; color: "gray"
            }
            Text {
                id: peertopeerHeaderText
                text: qsTr("Peer to peer")
                verticalAlignment: Text.AlignVCenter
                font.family: customPlatformStyle.fontFamilyRegular;
                color: customPlatformStyle.colorNormalLight
                font.pixelSize: customPlatformStyle.fontSizeLarge
            }

            // --------------------------------------------------------------------------------
            // - SNEP
            CheckBox {
                id: useSnepEdit
                checked: true
                text: "Use SNEP for peer-to-peer"
                onClicked: {
                    if (useSnepEdit.checked) {
                        // SNEP activated
                        useConnectionLess = false;
                        nfcUri = "urn:nfc:sn:snep";
                        connectClientSocket = true;
                        connectServerSocket = true;
                    } else {
                        // SNEP deactivated - no changes needed
                    }
                }
            }


            // --------------------------------------------------------------------------------
            // - No SNEP: Direct peer-to-peer settings
            Column {
                id: peerRawSettings
                visible: !useSnepEdit.checked
                spacing: customPlatformStyle.paddingMedium;
                width: parent.width


                Text {
                    id: connectionMode
                    text: qsTr("Connection Mode")
                    font.family: customPlatformStyle.fontFamilyRegular;
                    color: customPlatformStyle.colorNormalLight
                    font.pixelSize: customPlatformStyle.fontSizeMedium
                }

                // Switch: Connection oriented / less
                Row {
                     id: connectionRow
                     height: connectionSwitch.height
                     width: parent.width
                     spacing: customPlatformStyle.paddingMedium;

                     Switch {
                         id: connectionSwitch
                     }

                     Text {
                         width: connectionRow.width - connectionRow.spacing - connectionSwitch.width
                         height: connectionSwitch.height
                         verticalAlignment: Text.AlignVCenter
                         text: connectionSwitch.checked ? "Connection-less" : "Connection-oriented";
                         font.family: customPlatformStyle.fontFamilyRegular;
                         color: customPlatformStyle.colorNormalLight
                         font.pixelSize: customPlatformStyle.fontSizeMedium
                     }
                 }

                // --------------------------------------------------------------------------------
                // Connection-less
                Column {
                    id: clColumn
                    visible: connectionSwitch.checked
                    spacing: customPlatformStyle.paddingMedium;
                    width: parent.width

                    // - Port
                    Text {
                        text: qsTr("Port")
                        font.family: customPlatformStyle.fontFamilyRegular;
                        color: customPlatformStyle.colorNormalLight
                        font.pixelSize: customPlatformStyle.fontSizeMedium
                    }

                    TextField {
                        id: clPort
                        width: parent.width
                        text: "35"
                        validator: IntValidator{bottom: 1; top: 65536}
                        onActiveFocusChanged: {
                            if (activeFocus) {
                                focusedItem = clPort;
                                moveToFocusedItem();
                            }
                        }
                    }
                }

                // --------------------------------------------------------------------------------
                // Connection-oriented
                Column {
                    id: coColumn
                    visible: !connectionSwitch.checked
                    spacing: customPlatformStyle.paddingMedium;
                    width: parent.width

                    // - URI
                    Text {
                        text: qsTr("URI")
                        font.family: customPlatformStyle.fontFamilyRegular;
                        color: customPlatformStyle.colorNormalLight
                        font.pixelSize: customPlatformStyle.fontSizeMedium
                    }

                    TextField {
                        id: coUri
                        width: parent.width
                        text: "urn:nfc:sn:snep"
                        maximumLength: 255
                        onActiveFocusChanged: {
                            if (activeFocus) {
                                focusedItem = coUri;
                                moveToFocusedItem();
                            }
                        }
                    }

                    // - Connect Client Socket
                    CheckBox {
                        id: coClientSocket
                        checked: true
                        text: "Connect Client Socket"
                        onClicked: {
                            // Do not allow disabling both sockets
                            if (!coClientSocket.checked &&
                                    !coServerSocket.checked) {
                                coServerSocket.checked = true;
                            }
                            // Make sure sending is done through a connected socket
                            checkSendSocketSwitch();
                        }
                    }

                    // - Connect Server Socket
                    CheckBox {
                        id: coServerSocket
                        checked: true
                        text: "Connect Server Socket"
                        onClicked: {
                            // Do not allow disabling both sockets
                            if (!coServerSocket.checked &&
                                    !coClientSocket.checked) {
                                coClientSocket.checked = true;
                            }
                            // Make sure sending is done through a connected socket
                            checkSendSocketSwitch();
                        }
                    }


                    // - Send messages through: server or client socket
                    // (only if connect on this socket is enabled)
                    Text {
                        text: qsTr("Send messages through")
                        font.family: customPlatformStyle.fontFamilyRegular;
                        color: customPlatformStyle.colorNormalLight
                        font.pixelSize: customPlatformStyle.fontSizeMedium
                    }

                    Row {
                        id: coSendSocketRow
                        spacing: customPlatformStyle.paddingMedium;
                        width: parent.width
                        height: coSendSocketSwitch.height

                        // Checked: server socket
                        // Unchecked: client socket
                        Switch {
                            id: coSendSocketSwitch
                        }

                        Text {
                            width: coSendSocketRow.width - coSendSocketRow.spacing - coSendSocketSwitch.width
                            height: coSendSocketSwitch.height
                            verticalAlignment: Text.AlignVCenter
                            text: coSendSocketSwitch.checked ? "Server socket" : "Client socket"
                            font.family: customPlatformStyle.fontFamilyRegular;
                            color: customPlatformStyle.colorNormalLight
                            font.pixelSize: customPlatformStyle.fontSizeMedium
                        }
                    }
                }
            }

        }
    }


    function checkSendSocketSwitch() {
        if (!coClientSocket.checked &&
                !coSendSocketSwitch.checked) {
            // Client socket isn't connected, but the switch
            // instructs to use the client socket to send messages
            // -> change to use the server socket
            coSendSocketSwitch.checked = true;
        }
        if (!coServerSocket.checked &&
                coSendSocketSwitch.checked) {
            // Server socket isn't connected, but the switch
            // instructs to use the server socket to send messages
            // -> change to use the client socket
            coSendSocketSwitch.checked = false;
        }

        // Enable send socket choose switch only if both sockets
        // are connected
        coSendSocketSwitch.enabled = (coServerSocket.checked &&
                                      coClientSocket.checked);

    }

    // --------------------------------------------------------------------------------
    // Virtual Keyboard handling (VKB)
    // Resizes the Listview to avoid overlapping the edited item with the VKB.
    // Only needed on Symbian, MeeGo does that by itself.
    Item {
        // This element has the same size as the virtual keyboard and is used to
        // reduce the size of the recordView whenever the VKB becomes visible.
        // This is needed to ensure the edited item is visible and not hidden
        // by the VKB (through overlap).
        id: splitViewInput
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
    }
    Connections {
        target: inputContext;
        onVisibleChanged: adjustVkbHeight();
        onHeightChanged: adjustVkbHeight();
    }
    function adjustVkbHeight() {
        if (platform !== 2) {
            var prevSplitViewInputHeight = splitViewInput.height;
            // Only do this when not working with the Simulator, which doesn't draw the
            // VKB but still reserves the size for it.
            splitViewInput.height = (inputContext.visible) ? inputContext.height - tools.height : 0;
            if (prevSplitViewInputHeight !== splitViewInput.height) {
                // Note that the recordView.currentIndex requires the item to set itself
                // as the current index when the user starts editing an item.
                // (in onActiveFocusChanged in the TextField element of the delegate)
                //recordView.positionViewAtIndex(recordView.currentIndex, ListView.Visible);
            }
        }
    }
    function moveToFocusedItem() {
        flickSettings.contentY = focusedItem.y
        flickSettings.returnToBounds();
    }
}
