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
import com.nokia.meego 1.0

Page {
    id: writeTagPage

    tools: ToolBarLayout {
        ToolIcon {
            iconId: "toolbar-back"
            onClicked: {
                nfcInfo.nfcStopWritingTags();
                pageStack.depth <= 1 ? Qt.quit() : pageStack.pop();
            }
        }
    }

    function resetPage() {
        writeStatusAnimation.complete();
        writeStatusBgFlash.complete();
        writeStatusContainer.visible = false;
        writeStatusGroup.visible = false;
        writeStatusBg.visible = false;
        separator.visible = false;
        upgradeAdvTagsButton.visible = false;
    }

    function startWriting() {
        resetPage();
        showHideBusy(true);
    }

    function cancelWriting() {
        if (busyContainer.visible) {
            showHideBusy(false);
        }
    }

    function showHideBusy(showBusy) {
        busyContainer.visible = showBusy;
        busySpinner.running = showBusy;
    }

    function startAnimation() {
        writeStatusContainer.visible = true;
        writeStatusGroup.visible = true;
        writeStatusAnimation.restart();
        writeStatusBg.visible = true;
        writeStatusBgFlash.restart();
        separator.visible = true;
    }

    function tagWritten() {
        showHideBusy(false);
        writeStatusText.text = "Success";
        writeStatusText.color = "palegreen";
        writeStatusDetails.text = "Message written to the tag / device.\nTouch another tag to write again, or go back to the edit page.";
        writeStatusImage.source = "okLarge.svg";
        writeStatusBg.gradient = writeSuccessGradient;
        separator.gradient = writeSuccessGradient;
        startAnimation();
    }

    function tagWriteExceeded() {
        var errorMsg;
        if (platform === 0) {
            // Symbian - In App Purchasing
            errorMsg = qsTr("Please purchase the 'Advanced Tags' upgrade to write an unlimited number of advanced tag formats!");
        } else {
            // Harmattan - Full version in the Nokia Store
            errorMsg = qsTr("Please purchase the unlimited version of the Nfc Interactor from the Nokia Store to write an unlimited number of advanced messages to tags!");
        }
        upgradeAdvTagsButton.visible = true;
        tagWriteError(errorMsg);
    }

    function tagWriteError(errorMsg) {
        showHideBusy(false);
        writeStatusText.text = "Failed to write tag";
        writeStatusText.color = "lightcoral";
        writeStatusDetails.text = errorMsg;
        writeStatusImage.source = "errorLarge.svg";
        writeStatusBg.gradient = writeFailureGradient;
        separator.gradient = writeFailureGradient;
        startAnimation();
    }

    Gradient {
        id: writeSuccessGradient
        GradientStop {
            position: 0.00;
            color: "#1b3e11";
        }
        GradientStop {
            position: 1.00;
            color: "#4baa29";
        }
    }
    Gradient {
        id: writeFailureGradient
        GradientStop {
            position: 0.00;
            color: "#3e1111";
        }
        GradientStop {
            position: 1.00;
            color: "#aa2828";
        }
    }

    Rectangle {
        id: writeStatusBg
        anchors.fill: parent
        opacity: 0.0
        SequentialAnimation {
            id: writeStatusBgFlash
            PropertyAnimation {
                target: writeStatusBg
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 10
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: writeStatusBg
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 1000
                easing.type: Easing.OutQuad
            }
        }
    }


    // -------------------------------------------------------------------------
    // Instructions

    Image {
        id: writeInstructionsImage
        source: "nfcTouchTag.svg"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: customPlatformStyle.paddingMedium
    }
    Text {
        id: writeInstructions
        text: "Please touch a tag to write the NDEF message"
        //horizontalAlignment: Text.AlignHCenter
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeLarge
        color: customPlatformStyle.colorNormalLight
        anchors.top: parent.top
        anchors.topMargin: customPlatformStyle.paddingLarge
        anchors.left: writeInstructionsImage.right
        anchors.leftMargin: customPlatformStyle.paddingLarge
        anchors.right: parent.right
        anchors.rightMargin: customPlatformStyle.paddingMedium
        wrapMode: Text.WordWrap
    }


    Rectangle {
        id: separator
        width: parent.width; height: 10;
        gradient: writeSuccessGradient
        opacity: 0.3
        anchors.top: writeInstructionsImage.bottom
        visible: false
    }

    // -------------------------------------------------------------------------
    // Working indicator
    Item {
        id: busyContainer
        anchors.top: separator.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: false
        BusyIndicator {
            id: busySpinner
            running: false
            platformStyle: BusyIndicatorStyle { size: "large" }
            anchors.top: parent.top
            anchors.topMargin: customPlatformStyle.paddingLarge
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            id: busyText
            text: "Writing...\n         "
            font.family: customPlatformStyle.fontFamilyRegular;
            font.pixelSize: customPlatformStyle.fontSizeLarge
            color: customPlatformStyle.colorNormalLight
            anchors.top: busySpinner.bottom
            anchors.topMargin: customPlatformStyle.paddingMedium
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // -------------------------------------------------------------------------
    // Status

    Item {
        id: writeStatusContainer
        anchors.top: separator.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: customPlatformStyle.paddingLarge
        anchors.rightMargin: customPlatformStyle.paddingLarge
        visible: false

        Flickable {
            id: writeStatusFlickable
            width: parent.width
            anchors.fill: parent
            contentWidth: parent.width
            contentHeight: writeStatusGroup.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            Column {
                id: writeStatusGroup
                width: parent.width
                spacing: customPlatformStyle.paddingMedium

                PropertyAnimation {
                    id: writeStatusAnimation
                    target: writeStatusGroup
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 500
                    easing.type: Easing.InOutQuad
                }
                Text {
                    id: writeStatusText
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeLarge
                    color: customPlatformStyle.colorNormalLight
                    height: customPlatformStyle.fontSizeLarge + customPlatformStyle.paddingLarge
                    verticalAlignment: Text.AlignBottom
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
                Text {
                    id: writeStatusDetails
                    horizontalAlignment: Text.AlignHCenter
                    font.family: customPlatformStyle.fontFamilyRegular;
                    font.pixelSize: customPlatformStyle.fontSizeMedium
                    color: customPlatformStyle.colorNormalLight
                    width: parent.width
                    wrapMode: Text.WordWrap
                }

                Button {
                    id: upgradeAdvTagsButton
                    visible: false
                    text: "Upgrade the App"
                    //iconSource: "buy.svg";    // When using the icon, the text is left-aligned, which doesn't look good here
                    onClicked: Qt.openUrlExternally("http://www.nfcinteractor.com/dl.php?c=nfcinteractormeegoupgrade")
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Image {
                    id: writeStatusImage
                    source: "okLarge.svg"
                    anchors.horizontalCenter: parent.horizontalCenter
                    fillMode: Image.PreserveAspectFit
                }


            }

        }
    }


}
