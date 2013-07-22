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
import com.nfcinfo.types 1.0


Page {
    id: composeTagPage

    tools: composeToolbar

    Text {
        id: messageHeader
        text: "Ndef Message"
        font.bold: true
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: customPlatformStyle.fontSizeLarge + customPlatformStyle.paddingMedium
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeLarge
        color: customPlatformStyle.colorNormalLight
    }

    function updateHeader(ndefMessageSize) {
        messageHeader.text = "Ndef Message (size: " + ndefMessageSize + " bytes)";
    }

    Text {
        id: composeInstructions
        visible: recordView.count === 0
        text: qsTr("Empty message.\nUse the add ('plus') button in the toolbar to add one or more NDEF records.")
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors {top: messageHeader.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; }
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeMedium
        color: customPlatformStyle.colorNormalLight
        wrapMode: Text.WordWrap
    }

    ListView {
        id: recordView
        anchors {top: messageHeader.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; }
        model: nfcInfo.recordModel
        cacheBuffer: 200
        clip: true
        delegate: RecordEditDelegate { }
        focus: true
        visible: count > 0
    }

    // Note: VKB-Tricks with inputContext not needed on MeeGo


    // --------------------------------------------------------------------------------
    // Create a selection dialog with a title and list elements to choose from.
    Loader {
        // TODO: bug report - if setting loader to fill its parent, the first element
        // of the selection dialog will be invisible.
        // If centering the loader on the parent or not setting a size at all,
        // it works, but results in multiple binding loops reported in the debug output.
        //anchors.fill: parent
        // Need to dynamically load the dialog, as otherwise Symbian Components
        // would only check the size upon creation of the element and not update it
        // when we add components dynamically later on.
        // Therefore, we'll only (re)create the dialog on demand, fill it with items
        // right away and then show it.
        id: addContentDialogLoader
    }
    Component {
        id: addContentDialogComponent
        SelectionDialog {
            id: addContentToRecordDialog
            property int recordIndex
            property int messageType
            titleText: "Add to the record ..."
            selectedIndex: -1
            z: 1
            model: ListModel { }

            onAccepted: doAddContentToRecord(recordIndex, messageType, addContentToRecordDialog.model.get(addContentToRecordDialog.selectedIndex).type)
        }
    }

    function doAddContentToRecord(recordIndex, messageType, newRecordContent) {
        nfcInfo.recordModel.addContentToRecord(recordIndex, messageType, newRecordContent)
    }


    /*!
      \brief Show a dialog to add the possible content items to the specified record.
      */
    function queryAddContentToRecord(recordIndex, messageType) {
        addContentDialogLoader.sourceComponent = undefined;
        addContentDialogLoader.sourceComponent = addContentDialogComponent
        prepareAddContentDialog(recordIndex, messageType);

        // Check if we found a record to add - of not, show a message box to inform the user
        if (addContentDialogLoader.item.model.count === 0) {
            // "No further records can be added to this message"
            // Shouldn't get here, as the add button should be removed from
            // the record header when the last possible content item has been added.
            addContentDialogLoader.sourceComponent = undefined
        } else {
            // Show selection dialog - if accepted, it will call addInfoToRecord() to
            // actually add the content to the record.
            addContentDialogLoader.item.open();
        }
    }

    /*!
      \brief Setup the dialog which can be used to add content items to the specified record.

      If the model of the add content dialog is empty after executing this method, it is
      not possible to add further content items to the record.
      */
    function prepareAddContentDialog(recordIndex, messageType) {
        addContentDialogLoader.item.model.clear()
        addContentDialogLoader.item.recordIndex = recordIndex
        addContentDialogLoader.item.messageType = messageType
        // Determine which info can be added based on type and already visible elements
        // Helpful: http://xzis.me/2011/09/expose-lists-to-qml/
        // Bug about deleting the items: https://bugreports.qt.nokia.com/browse/QTBUG-20377
        // But all the items we create here have a parent as a QObject, so ok for now
        var contentList = nfcInfo.recordModel.possibleContentForRecord(recordIndex);
        for (var i = 0; i < contentList.length; i++) {
            addContentDialogLoader.item.model.append({"name": contentList[i].title(), "type": contentList[i].recordContentInt()})
        }
        addContentDialogLoader.item.selectedIndex = -1;
    }

    function openAddNewRecordDialog() {
        // Reset selected index to -1 so that not the previously
        // selected item is marked in the UI
        writeNdefSelection.selectedIndex = -1;
        writeNdefSelection.open();
    }

    SelectionDialog {
        id: writeNdefSelection
        titleText: "Choose record to add"
        selectedIndex: -1
        onAccepted: addRecordUi(writeNdefSelection.model.get(writeNdefSelection.selectedIndex).name)

        model: ListModel {
            ListElement { name: "Smart Poster" }
            ListElement { name: "URI" }
            ListElement { name: "Text" }
            ListElement { name: "SMS" }
            ListElement { name: "Business Card" }
            ListElement { name: "Social Network" }
            ListElement { name: "Geo" }
            ListElement { name: "App Store" }
            ListElement { name: "Image" }
            ListElement { name: "Annotated URL" }
            ListElement { name: "Custom" }
            ListElement { name: "Combination" }
            ListElement { name: "Nfc Interactor Autostart" }
            ListElement { name: "Windows LaunchApp" }
            ListElement { name: "Android Application Record" }
        }
    }

    function addRecordUi(recordType) {
        var newRecordType = -1;
        switch (recordType) {
        case "Smart Poster":
            newRecordType = NfcTypes.MsgSmartPoster;
            break;
        case "URI":
            newRecordType = NfcTypes.MsgUri;
            break;
        case "Text":
            newRecordType = NfcTypes.MsgText;
            break;
        case "SMS":
            newRecordType = NfcTypes.MsgSms;
            break;
        case "Business Card":
            newRecordType = NfcTypes.MsgBusinessCard;
            break;
        case "Social Network":
            newRecordType = NfcTypes.MsgSocialNetwork;
            break;
        case "Geo":
            newRecordType = NfcTypes.MsgGeo;
            break;
        case "App Store":
            newRecordType = NfcTypes.MsgStore;
            break;
        case "Image":
            newRecordType = NfcTypes.MsgImage;
            break;
        case "Annotated URL":
            newRecordType = NfcTypes.MsgAnnotatedUrl;
            break;
        case "Custom":
            newRecordType = NfcTypes.MsgCustom;
            break;
        case "Combination":
            newRecordType = NfcTypes.MsgCombination;
            break;
        case "Nfc Interactor Autostart":
            newRecordType = NfcTypes.MsgNfcAutostart;
            break;
        case "Windows LaunchApp":
            newRecordType = NfcTypes.MsgLaunchApp;
            break;
        case "Android Application Record":
            newRecordType = NfcTypes.MsgAndroidAppRecord;
            break;
        }

        //nfcInfo.recordModel.addCompleteRecordWithDefault(newRecordType);
        delayAddCompleteRecord.newRecordType = newRecordType;
        delayAddCompleteRecord.start();
    }

    Timer {
        id: delayAddCompleteRecord
        interval: 250
        running: false
        repeat: false
        property int newRecordType
        onTriggered: {
            nfcInfo.recordModel.addCompleteRecordWithDefault(newRecordType);
        }
    }


    property Dialog dialog
    function showRecordHelp(index, title, messageType, recordContent)
    {
        if (!dialog)
            dialog = helpDialogComponent.createObject(parent, {"titleText": title, "message": recordHelpText(messageType)})
        dialog.open()
    }
    function recordHelpText(messageType) {
        switch (messageType) {
        case NfcTypes.MsgSmartPoster:
            return qsTr("Combine a URI with additional meta-information like a title (optionally in multiple languages), a recommended action for the reader, and data about the linked content (size, MIME type).");
        case NfcTypes.MsgUri:
            return qsTr("Store a URI, which can for example be Internet address, SMS template, or a telephone number. Common URI types are stored more efficiently according to the NFC Forum specification.");
        case NfcTypes.MsgText:
            return qsTr("Write text in a specific language following the ISO/IANA code. Encoded with UTF-8. Usually used as a title in a Smart Poster and not as a stand-alone record.");
        case NfcTypes.MsgSms:
            return qsTr("Specify information about an SMS the user is supposed to send. Written as a URI record or Smart Poster (if additional information like the title is provided)");
        case NfcTypes.MsgBusinessCard:
            return qsTr("Store contact data using the vCard format. This record can contain a multitude of optional contact information. Note the usually high tag size requirement for storing business cards.");
        case NfcTypes.MsgSocialNetwork:
            return qsTr("Choose the social network you want to link to and enter your social network username to create a link to follow you. Record written as a URI record or Smart Poster (if additional information like the title is provided)");
        case NfcTypes.MsgGeo:
            return qsTr("Enter latitude and longitude specified in decimal degrees according to WSG 84. The URI is encoded either using the Geo URI standard (supported by MeeGo Harmattan), a link to Nokia Maps (supported by Symbian) or a generic redirection script hosted on NfcInteractor.com that works for all Nokia phones. Record written as a URI record or Smart Poster (if additional information like the title is provided)");
        case NfcTypes.MsgStore:
            return qsTr("Generate a download link to an app store. Creates a direct store link if one ID is specified, or a generic link using a platform detection script at nfcinteractor.com if more app ids are specified. For Nokia, either specify a generic Nokia Store ID if it's the same for all platforms, or instead specify an own ID for Symbian, MeeGo Harmattan and if needed Series 40. Record written as a URI record or Smart Poster (if additional information like the title is provided)");
        case NfcTypes.MsgImage:
            return qsTr("Store an image on a tag in any format. Note the images are usually very large and require a tag with enough writable space.");
        case NfcTypes.MsgCustom:
            return qsTr("Create a simple custom record by specifying the type name format, in most cases the type. Payload and ID are optional.");
        case NfcTypes.MsgLaunchApp:
            return qsTr("Directly launch an app by specifying the app ID for Windows and Windows Phone.");
        case NfcTypes.MsgAndroidAppRecord:
            return qsTr("Launch an app on Android by specifying its package name.");
        default:
            return qsTr("Sorry, no help available for this item.");
        }
    }

    Component {
        id: helpDialogComponent
        QueryDialog {
            id: helpDialog
            acceptButtonText: "Ok"
            // Don't call only destroy() on Harmattan, as it would
            // cause a segmentation fault when leaving the page. See:
            // https://bugreports.qt-project.org/browse/QTCOMPONENTS-1225
            onAccepted: helpDialog.destroy()
        }
    }

    // --------------------------------------------------------------------------------
    // Load NDEF message from file
    function openLoadMessageFromFileDialog() {
        fileDialogLoader.source = Qt.resolvedUrl("FileSelector.qml");
    }
    Loader {
        id: fileDialogLoader
        onStatusChanged: {
            if (status === Loader.Ready) {
                fileDialogLoader.item.titleText = "Open Ndef Message..."
                fileDialogLoader.item.nameFilters = "*.txt";
                fileDialogLoader.item.open();
            }
        }
    }
    Connections {
        target: fileDialogLoader.item;
        onAccepted: {
            nfcInfo.nfcEditTag(fileDialogLoader.item.selectedFilePath);
            fileDialogLoader.sourceComponent = undefined;
        }
        //onClickedOutside: { fileDialogLoader.sourceComponent = undefined; }  // non-existent on phone
        onRejected: { fileDialogLoader.sourceComponent = undefined; }
    }


    // --------------------------------------------------------------------------------
    // Save NDEF message to file
    function openSaveMessageToFileDialog()
    {
        fileNameDialog.open();
    }

    // According to the docs, CommonDialog should be available on MeeGo Harmattan
    // as well, but it isn't :(
    // -> copied the file from Qt Quick Components source to this project, renamed
    // to MyCommonDialog to avoid file name conflicts.
    MyCommonDialog {
        id: fileNameDialog
        titleText: "Save composed message"
        buttons: ButtonRow {
            style: ButtonStyle { }
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: "Save";
                onClicked: {
                    var fullFileName = nfcInfo.nfcSaveModelToFile(fileName.text)
                    fileNameDialog.close();
                    // MeeGo doesn't do forced linefeeds for wordwrap when the text is too long
                    // (it's using Text.WordWrap instead of Text.Wrap)
                    // -> separate the file name from the directory with a " ", to allow the
                    // dialog to insert a new line. Otherwise, the text wouldn't usually fit on
                    // the screen.
                    var fileNameStartsAt = fullFileName.lastIndexOf("/");
                    fullFileName = fullFileName.substring(0, fileNameStartsAt + 1) + " " + fullFileName.substring(fileNameStartsAt + 1, fullFileName.length)
                    console.log("Adapted file name: " + fullFileName)
                    successDialog.message = "Successfully saved message to file: " + fullFileName
                    successDialog.open();
                }
            }
        }

        content: Item {
            width: parent.width
            anchors.margins: customPlatformStyle.paddingMedium
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: dialogTitle.height + fileName.height + customPlatformStyle.paddingMedium * 3
            Text {
                id: dialogTitle
                width: parent.width
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                wrapMode: Text.WordWrap
                font.family: customPlatformStyle.fontFamilyRegular;
                font.pixelSize: customPlatformStyle.fontSizeMedium
                color: customPlatformStyle.colorNormalLight
                text: "Store the composed message to a file for later reuse:"
            }
            TextField {
                id: fileName
                anchors.top: dialogTitle.bottom
                anchors.topMargin: customPlatformStyle.paddingMedium
                anchors.left: parent.left
                anchors.right: parent.right
                width: parent.width
                placeholderText: "Name of file to create"
                focus: true
            }
        }
    }

    QueryDialog {
        id: successDialog
        titleText: "Saved"
        acceptButtonText: "Ok"
    }

    // --------------------------------------------------------------------------------
    // Toolbar
    onStatusChanged: {
        if (status === PageStatus.Activating) {
            toolLoadNdef.visible = settings.logNdefToFile();
            toolSaveNdef.visible = settings.logNdefToFile();
        }
    }

    ToolBarLayout {
        id: composeToolbar
        ToolIcon {
            iconId: "toolbar-back"
            onClicked: pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
        }
        ToolIcon {
            iconId: "toolbar-add"
            onClicked: openAddNewRecordDialog()
        }
        ToolIcon {
            id: toolLoadNdef
            iconId: "toolbar-directory"
            visible: settings.logNdefToFile()
            onClicked: openLoadMessageFromFileDialog()
        }
        ToolIcon {
            id: toolSaveNdef
            iconId: "toolbar-directory-move-to"
            visible: settings.logNdefToFile()
            onClicked: openSaveMessageToFileDialog()
        }
        ToolIcon {
            iconId: "toolbar-done"
            onClicked: {
                nfcInfo.nfcWriteTag(true);
                if (writeTagPageLoader.status === Loader.Ready) {
                    writeTagPageLoader.item.resetPage();
                    pageStack.push(writeTagPageLoader.item);
                }
            }
        }
    }
}
