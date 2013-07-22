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

Page {

    function logMessage(text, color, img, nfcDataFileName)
    {
        nfcDataFileName = typeof nfcDataFileName !== 'undefined' ? nfcDataFileName : "";
        messageModel.append( {"infoMsg": text, "textColor": color, "image": img, "nfcDataFileName": nfcDataFileName} )
    }

    ListModel {
        id: messageModel
        // Initial help message
        ListElement {
            infoMsg: "Please touch NFC tags to analyze them";
            textColor: "aliceblue";
            image: "nfcSymbolInfo.png"
        }
    }

    Component.onCompleted: {
        loadAds();
        messageView.contentY = 0;
    }

    ListView {
        id: messageView
        model: messageModel
        clip: true
        delegate: listDelegate
        highlight: highlight
        highlightFollowsCurrentItem: true
        focus: true
        currentIndex: -1
        //anchors.fill: parent
        anchors {
            top: iaaLoader.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        anchors.topMargin: customPlatformStyle.paddingLarge

        // Automatically scroll down when a new element is added
        onCountChanged: positionViewAtEnd();
        Component.onCompleted: {
            messageView.contentY = 0;
        }
    }

    // Delegate for showing an individual line of the model
    Component {
        id: listDelegate
        Item {
            width: parent.width
            height: Math.max(infoImg.height, infoTxt.paintedHeight) + customPlatformStyle.paddingMedium
            Item {
                id: row
                width: parent.width
                Image {
                    id: infoImg
                    source: image
                    fillMode: Image.PreserveAspectFit
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: Math.ceil((customPlatformStyle.fontHeightMedium - infoImg.sourceSize.height) / 2)
                    asynchronous: true
                }
                Text {
                    id: infoTxt
                    text: infoMsg
                    color: textColor
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeMedium
                    wrapMode: Text.Wrap
                    anchors.left: infoImg.right
                    anchors.leftMargin: customPlatformStyle.paddingMedium
                    anchors.right: parent.right
                    anchors.rightMargin: customPlatformStyle.paddingSmall
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // Clicked on an item
                    if (messageModel.get(index).nfcDataFileName && settings.logNdefToFile) {
                        itemMenu.open()
                        messageView.currentIndex = index
                        console.log(nfcDataFileName);
                    }
                }
            }
        }
    }

    ContextMenu {
        id: itemMenu
        MenuLayout {
            MenuItem {
                // Open in compose tag view
                // Only edits supported contents
                text: "Edit"
                onClicked: {
                    nfcInfo.nfcEditTag(messageModel.get(messageView.currentIndex).nfcDataFileName);
                    pageStack.push(composeTagPageLoader.item)
                }
            }
            MenuItem {
                // Go right to write mode to create a 1:1 copy of the tag
                text: "Clone"
                onClicked: {
                    nfcInfo.nfcWriteTag(messageModel.get(messageView.currentIndex).nfcDataFileName, true);
                    if (writeTagPageLoader.status === Loader.Ready) {
                        writeTagPageLoader.item.resetPage();
                        pageStack.push(writeTagPageLoader.item);
                    }
                }
            }
        }
    }
    Component {
        id: highlight
        Rectangle {
            color: "steelblue"
            opacity: 0.5
        }
    }

    // In App Advertising
    // -------------------------------------------------------------------------
    Loader {
        id: iaaLoader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    function loadAds() {
        if (!(window.isRemoveAdsPurchased() || window.isUnlimitedPurchased())) {
            iaaLoader.source = Qt.resolvedUrl("IaaAd.qml");
        }
    }
    function hideAds() {
        iaaLoader.sourceComponent = undefined
        iaaLoader.height = 0
    }

    // -------------------------------------------------------------------------
    // Working indicator
    function showHideBusy(showBusy) {
        busySpinner.visible = showBusy;
        busySpinner.running = showBusy;
    }
    BusyIndicator {
        id: busySpinner
        visible: false
        running: false
        width: 50
        height: 50
        anchors.bottom: parent.bottom
        anchors.bottomMargin: customPlatformStyle.paddingMedium
        anchors.right: parent.right
        anchors.rightMargin: customPlatformStyle.paddingMedium
    }

    // -------------------------------------------------------------------------
    // Toolbar
    tools: (!useIap ||
            (window.isAdvTagsPurchased() && window.isRemoveAdsPurchased() && !iapTestMode) ||
            (window.isUnlimitedPurchased() && !iapTestMode)) ? toolBarNfcInfoPage : toolBarNfcInfoPageIap;
    // Just setting a ToolButton to invisible would still leave its
    // space free in the ToolBarLayout. Therefore, we need to define
    // two separate ToolBarLayouts and switch depending on if IAP is enabled
    // or not.
    ToolBarLayout {
        id: toolBarNfcInfoPage
        ToolButton {
            flat: true
            iconSource: "toolbar-back";
            onClicked: pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
        }
        ToolButton {
            flat: true
            iconSource: "info.svg";
            visible: instructionsLoader.status === Loader.Ready
            onClicked: pageStack.push(instructionsLoader.item)
        }
        ToolButton {
            flat: true
            iconSource: "toolbar-settings";
            visible: settingsPageLoader.status === Loader.Ready
            onClicked: {
                pageStack.push(settingsPageLoader.item)
            }
        }
        ToolButton {
            flat: true
            iconSource: "create_message.svg";
            visible: composeTagPageLoader.status === Loader.Ready
            onClicked: {
                composeTagPageLoader.item.updateToolbar()
                pageStack.push(composeTagPageLoader.item)
            }
        }
    }
    ToolBarLayout {
        id: toolBarNfcInfoPageIap
        ToolButton {
            flat: true
            iconSource: "toolbar-back";
            onClicked: pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
        }
        ToolButton {
            flat: true
            iconSource: "info.svg";
            visible: instructionsLoader.status === Loader.Ready
            onClicked: pageStack.push(instructionsLoader.item)
        }
        ToolButton {
            flat: true
            iconSource: "toolbar-settings";
            visible: settingsPageLoader.status === Loader.Ready
            onClicked: {
                pageStack.push(settingsPageLoader.item)
            }
        }
        ToolButton {
            flat: true
            iconSource: "buy.svg";
            onClicked: loadIapPage();
        }
        ToolButton {
            flat: true
            iconSource: "create_message.svg";
            visible: composeTagPageLoader.status === Loader.Ready
            onClicked: {
                composeTagPageLoader.item.updateToolbar()
                pageStack.push(composeTagPageLoader.item)
            }
        }
    }

}
