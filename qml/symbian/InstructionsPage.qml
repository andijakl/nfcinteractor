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
                pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
            }
        }
    }

    Item {
        id: instructionsItem
        anchors.fill: parent
        anchors.leftMargin: customPlatformStyle.paddingLarge
        anchors.rightMargin: customPlatformStyle.paddingLarge

        Flickable {
            id: instructionsFlickable
            width: parent.width
            height: parent.height
            anchors { left: parent.left; top: parent.top }
            contentHeight: instructionsContentItem.height
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: instructionsContentItem
                width: parent.width

                spacing: 7

                Text {
                    id: instructionsText1
                    text: qsTr("<strong>Nfc Interactor</strong>") + ((window.isAdvTagsPurchased() || window.isUnlimitedPurchased()) ? "\nUnlimited" : "");
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    wrapMode: Text.WordWrap
                    //color: customPlatformStyle.colorNormalLight
                    color: "aliceblue";
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeLarge
                }
                Image {
                    id: instructionsImage
                    anchors.horizontalCenter: parent.horizontalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "icon.png"
                    asynchronous: true
                }

                Text {
                    id: instructionsText2
                    text: qsTr("v4.2.0\n2011 - 2012 Andreas Jakl")
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    wrapMode: Text.WordWrap
                    color: customPlatformStyle.colorNormalLight
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeMedium
                }
                Button {
                    id: instructionsLinkButton
                    text: "NfcInteractor.com"
                    onClicked: Qt.openUrlExternally("http://www.nfcinteractor.com/r?ni");
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    id: instructionsText3
                    text: qsTr("\nTouch NFC tags to analyze their contents.\n\nTap the 'write' toolbar button to compose your own NDEF message (consisting of one or more records), then tap the finish flag to finalize the contents. Now touch a tag to write the message to the tag.\n\nIn the main view, tap the analyzed tag contents (blue text) to clone or edit a tag.\n\nChange peer to peer settings to configure device-to-device communication, including SNEP.")
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    wrapMode: Text.WordWrap
                    color: customPlatformStyle.colorNormalLight
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeMedium
                }
            }
        }

        ScrollBar {
            id: scrollBar
            height: parent.height
            anchors { top: instructionsFlickable.top; right: instructionsFlickable.right }
            flickableItem: instructionsFlickable
            interactive: false
            orientation: Qt.Vertical
        }

    }
}
