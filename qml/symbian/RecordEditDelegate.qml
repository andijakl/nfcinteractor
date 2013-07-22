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
import Qt.labs.components 1.1   // Needed for CheckableGroup on Symbian
import com.nfcinfo.types 1.0

Item {
    id: multiDelegate

    height: calculateHeight() //(recordContent == NfcTypes.RecordHeader) ? 40 : 190;//90;
    width: parent.width

    function calculateHeight() {
        switch (recordContent) {
        case NfcTypes.RecordHeader:
            // Header only = around 47 pixels height
            return 47;
        case NfcTypes.RecordSpAction:
        case NfcTypes.RecordGeoType:
        case NfcTypes.RecordTypeNameFormat:
        case NfcTypes.RecordSocialNetworkType:
            // Each option is around 50 pixels height
            return 37 + (selectOptions.length) * 50;
        default:
            // Header + 1 option (e.g., TextField)
            return 93;
        }
    }

    Rectangle {
        id: separator
        width: parent.width; height: 1; color: "gray"
        anchors.top: parent.top
        visible: (recordContent === NfcTypes.RecordHeader)
    }
    Rectangle {
        anchors.top:parent.top
        anchors.topMargin: 3
        anchors.left:parent.left
        anchors.leftMargin: 5
        width: 2
        height: parent.height - 3
        color: "cornflowerblue"
        visible: (recordContent !== NfcTypes.RecordHeader)
    }
    Button {
        id: removeRecordButton
        anchors.top: parent.top
        anchors.topMargin: (recordContent == NfcTypes.RecordHeader) ? customPlatformStyle.paddingSmall : 0
        anchors.right: parent.right
        iconSource: "delete.svg"
        onClicked: recordModel.removeRecord(index)
        visible: removeVisible
    }
    Button {
        id: helpRecordButton
        anchors.top: parent.top
        anchors.topMargin: (recordContent === NfcTypes.RecordHeader) ? customPlatformStyle.paddingSmall : 0
        anchors.right: removeRecordButton.left
        anchors.rightMargin: customPlatformStyle.paddingSmall
        iconSource: "info.svg"
        onClicked: composeTagPage.showRecordHelp(index, title, messageType, recordContent)
        visible: (recordContent === NfcTypes.RecordHeader)
    }
    Button {
        id: addInfoToRecordButton
        anchors.top: parent.top
        anchors.topMargin: (recordContent === NfcTypes.RecordHeader) ? customPlatformStyle.paddingSmall : 0
        anchors.right: helpRecordButton.left
        anchors.rightMargin: customPlatformStyle.paddingSmall
        iconSource: "add_large.svg"
        onClicked: composeTagPage.queryAddContentToRecord(index, messageType)
        visible: (recordContent === NfcTypes.RecordHeader && addVisible)
    }
    Text {
        id: titleDescription
        text: title
        anchors.leftMargin: (recordContent === NfcTypes.RecordHeader) ? 5 : 15
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: (recordContent === NfcTypes.RecordHeader) ? 11 : 7
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeLarge
        color: (recordContent === NfcTypes.RecordHeader) ? "lightskyblue" : "aliceblue";
    }

    // --------------------------------------------------------------------------------
    // Text editor content
    TextField {
        id: editText
        anchors.leftMargin: 15
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: 7
        text: (recordContent === NfcTypes.RecordHeader) ? "" : currentText;
        visible: (recordContent !== NfcTypes.RecordHeader
                  && recordContent !== NfcTypes.RecordSpAction
                  && recordContent !== NfcTypes.RecordGeoType
                  && recordContent !== NfcTypes.RecordTypeNameFormat
                  && recordContent !== NfcTypes.RecordSocialNetworkType)

        // ... button for file selection
        platformRightMargin: selectFileImg.visible ? selectFileImg.width + customPlatformStyle.paddingMedium : 0;
        Image {
            id: selectFileImg
            visible: recordContent === NfcTypes.RecordImageFilename
            anchors { top: parent.top; right: parent.right }
            height: parent.height; width: parent.height
            smooth: true
            fillMode: Image.PreserveAspectFit
            source: selectImgBtn.pressed ? "textfield_browse_pressed.svg"
                                : "textfield_browse.svg"

            MouseArea {
                id: selectImgBtn
                anchors.fill: parent
                onClicked: showFileDialog()
            }
        }

        onActiveFocusChanged: {
            // Note that this needs to be onActiveFocusChanged, not just onFocusChanged, which
            // wouldn't have the desired effect.
            recordView.currentIndex = index
        }

        onTextChanged: {
            // QML Delegate binding to C++ data model doesn't work so that changing the
            // contents in QML automatically updates the model. Therefore, the update
            // has to be triggered manually.
            //
            // Updating the model right here leads to a binding loop for whatever reason
            // Seems like when the model is updated, the onTextChanged slot is called 2 times:
            // Once with the old text, then with the new text. This causes some endless loop of changes.
            //
            // Solution: Don't update the model right away, but delay it using a timer, so
            // that the update happens outside of the onTextChanged. This seems to do the job.
            delayModelChangeTimer.start();
            //            nfcInfo.recordModel.setDataValue(index, editText.text, "currentText")
            //            console.log("QML: editText.text = " + editText.text)
            //            console.log("QML: currentText = " + currentText)
        }
    }

    // --------------------------------------------------------------------------------
    // File selection
    function showFileDialog() {
        fileDialogLoader.source = Qt.resolvedUrl("FileSelector.qml");
    }
    Loader {
        id: fileDialogLoader
        onStatusChanged: {
            if (status === Loader.Ready) {
                fileDialogLoader.item.titleText = "Select image..."
                fileDialogLoader.item.nameFilters = [ "*.png", "*.jpg", "*.jpeg", "*.gif" ]
                fileDialogLoader.item.open();
            }
        }
    }
    Connections {
        target: fileDialogLoader.item;
        onAccepted: {
            console.log("Accepted file: " + fileDialogLoader.item.selectedFileName);
            editText.text = fileDialogLoader.item.selectedFilePath;
            fileDialogLoader.sourceComponent = undefined;
        }
        onClickedOutside: { fileDialogLoader.sourceComponent = undefined; }
        onRejected: { fileDialogLoader.sourceComponent = undefined; }
    }

    // --------------------------------------------------------------------------------
    // Checkable options group
    CheckableGroup {
        id: selectGroup
        onSelectedValueChanged: delayModelChangeTimer.start();
    }
    Column {
        id: row
        spacing: customPlatformStyle.paddingSmall
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: 7
        anchors.leftMargin: 15
        visible: (recordContent === NfcTypes.RecordSpAction
                  || recordContent === NfcTypes.RecordGeoType
                  || recordContent === NfcTypes.RecordTypeNameFormat
                  || recordContent === NfcTypes.RecordSocialNetworkType)
        Repeater {
            model: selectOptions
            RadioButton {
                id: button1
                text: modelData
                platformExclusiveGroup: selectGroup
                checked: (index === selectedOption)
            }
        }
    }
    function getSelectedOptionIndex() {
        // QML CheckableGroup unfortunately only has a function to get the value
        // of the selected RadioButton, but not to get its index...
        // TODO: Report
        for (var i = 0; i < selectOptions.length; i++) {
            if (selectOptions[i] === selectGroup.selectedValue) {
                return i;
            }
        }
        return -1;
    }

    Timer {
        id: delayModelChangeTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            if (recordContent === NfcTypes.RecordSpAction
                    || recordContent === NfcTypes.RecordGeoType
                    || recordContent === NfcTypes.RecordTypeNameFormat
                    || recordContent === NfcTypes.RecordSocialNetworkType) {
                var selectedOptionIndex = getSelectedOptionIndex();
                if (selectedOptionIndex > -1) {
                    //nfcInfo.recordModel.setDataValue(index, selectGroup.selectedValue, "selectedOption");
                    nfcInfo.recordModel.setDataValue(index, getSelectedOptionIndex(), "selectedOption");
                }
            } else {
                // Changing the model will reset the text of the text edit item while the onTriggered is active.
                // This in turn will place the edit cursor at the end of the text. Therefore, cache the current
                // cursor position here, and restore it after the model has been changed.
                var cachedCursorPosition = editText.cursorPosition;
    //            console.log("QML Delayed timer Caching cursor pos: " + cachedCursorPosition);
    //            console.log("QML Delayed timer - setting value to model. New: " + editText.text + ", currentText: "+ currentText);
                nfcInfo.recordModel.setDataValue(index, editText.text, "currentText");
    //            console.log("QML Delayed timer end. New: " + editText.text + ", currentText: "+ currentText);
    //            console.log("QML Delayed timer Restoring cursor pos: " + cachedCursorPosition + "(currently set to: " + editText.cursorPosition + ")");
                editText.cursorPosition = cachedCursorPosition;
            }
        }
    }

}
