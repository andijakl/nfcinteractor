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
import Qt.labs.folderlistmodel 1.0

SelectionDialog {
    id: fileSelectionDialog
    titleText: "Select file..."
    property string selectedFileName;
    property string selectedFilePath;
    property alias initialFolder: folderModel.folder
    property alias nameFilters: folderModel.nameFilters
    property variant myButtonTexts: ["Up", "Cancel"]

    model: FolderListModel {
        id: folderModel
        showOnlyReadable: true
        showDirs: true
        showDotAndDotDot: true              // Bug: doesn't work on Symbian
        sortField: FolderListModel.Name
        folder: "file:///" + settings.logNdefDir()
        //folder: "file://"
        //nameFilters: ["*.*"]
        nameFilters: [ "*.png", "*.jpg", "*.jpeg", "*.gif" ]
    }


    delegate: fileDelegate

    Component {
        id: fileDelegate

        MenuItem {
            platformInverted: root.platformInverted
            text: folderModel.isFolder(index) ? "<i>" + fileName + "/</i>" : fileName;

            onClicked: {
                selectedIndex = index
                if (folderModel.isFolder(index)) {
                    folderModel.folder = filePath
                } else {
                    selectedFileName = fileName;
                    selectedFilePath = filePath;
                    // Cut away file:///, as this is confusing for the user and also the
                    // image / file loading classes don't find the file if "file:///" is
                    // part of the filename.
                    selectedFilePath = selectedFilePath.replace("file:///", "");
                    accept();
                }
            }
        }
    }

    onButtonClicked: {
        if (index === 0) {
            // Up
            var folder = new String(folderModel.folder);
            // Only navigate up if not already at the top level
            if (folder !== "file://") {
                if (folder.lastIndexOf('/') === folder.length - 1) {
                    // "/" at the end of the path name - remove last "/"
                    // Normally, path names are without "/" at the end, only
                    // when at directory level it seems to be there
                    // (e.g., for "file:///D:/")
                    folder = folder.substring(0, folder.lastIndexOf('/'));
                }
                // Remove last part of the directory and set as new folder
                // for the file model
                folderModel.folder = folder.substring(0, folder.lastIndexOf('/'));
            }
        } else {
            // Cancel
            reject();
        }
    }

    // ---------------------------------------------------------------------------
    // Symbian: need a way to navigate up in the folder model.
    // Due to a bug, the showDotAndDotDot doesn't work on a Symbian phone,
    // only on the simulator and MeeGo Harmattan.
    // So, we're adding a button to go one directory up.
    // However, the default behavior of the button is to instantly
    // close the dialog. As we don't want that, the following is pretty
    // much a copy of the base CommonDialog, so that we can create our
    // own buttons that are identical to the default ones, only that they
    // don't close the dialog.
    Component.onCompleted: {
        for (var i = buttonRow.children.length; i > 0; --i) {
            buttonRow.children[i - 1].destroy()
        }
        for (var j = 0; j < myButtonTexts.length; ++j) {
            var button = buttonComponent.createObject(buttonRow)
            button.text = myButtonTexts[j]
            button.index = j
        }
    }
    buttons: Item {
        id: buttonContainer

        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true

        width: parent.width
        height: myButtonTexts.length ? privateStyle.toolBarHeightLandscape + 2 * platformStyle.paddingSmall : 0

        Row {
            id: buttonRow
            objectName: "buttonRow"
            anchors.centerIn: parent
            spacing: platformStyle.paddingMedium
        }
    }
    Component {
        id: buttonComponent
        ToolButton {
            property int index

            width: internal.buttonWidth()
            height: privateStyle.toolBarHeightLandscape

            onClicked: {
                if (status === DialogStatus.Open) {
                    buttonClicked(index)
                }
            }
        }
    }

    QtObject {
        id: internal

        function buttonWidth() {
            switch (myButtonTexts.length) {
                case 0: return 0
                case 1: return Math.round((privateStyle.dialogMaxSize - 3 * platformStyle.paddingMedium) / 2)
                default: return (buttonContainer.width - (myButtonTexts.length + 1) *
                    platformStyle.paddingMedium) / myButtonTexts.length
            }
        }
    }

}
