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
import Qt.labs.folderlistmodel 1.0

SelectionDialog {
    id: fileSelectionDialog
    titleText: "Select file..."
    property string selectedFileName;
    property string selectedFilePath;
    property alias initialFolder: folderModel.folder
    property alias nameFilters: folderModel.nameFilters

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

    // Adapted copy of delegate from the SelectionDialog of the
    // Qt Quick Components for Harmattan, as the FolderListModel
    // uses "fileName" as the property name for the text, not the
    // default "modelData".
    property Style platformStyle: SelectionDialogStyle {}
    Component {
        id: fileDelegate

        Item {
            id: delegateItem
            property bool selected: index === selectedIndex;

            height: fileSelectionDialog.platformStyle.itemHeight
            anchors.left: parent.left
            anchors.right: parent.right


            MouseArea {
                id: delegateMouseArea
                anchors.fill: parent;
                onPressed: {
                    //deselectedIndex = selectedIndex;
                    selectedIndex = index;
                }
                onClicked:  {
                    selectedIndex = index
                    if (folderModel.isFolder(index)) {
                        folderModel.folder = filePath
                        // Reset selection to invisible after changing the contents of the dialog
                        selectedIndex = -1;
                    } else {
                        selectedFileName = fileName;
                        selectedFilePath = filePath;
                        // Cut away file:///, as this is confusing for the user and also the
                        // image / file loading classes don't find the file if "file:///" is
                        // part of the filename.
                        // Selected path in Harmattan starts with: "home/user/", which can't be
                        // found when opening a file. Therefore, leave one "/" at the beginning
                        // of the URL.
                        selectedFilePath = selectedFilePath.replace("file:///", "/");
                        accept();
                    }
                }
            }

            Rectangle {
                id: backgroundRect
                anchors.fill: parent
                color: delegateItem.selected ? fileSelectionDialog.platformStyle.itemSelectedBackgroundColor : fileSelectionDialog.platformStyle.itemBackgroundColor
            }

            BorderImage {
                id: background
                anchors.fill: parent
                // UI.CORNER_MARGINS == 22 in UIConstants.js
                border { left: 22; top: 22; right: 22; bottom: 22 }
                source: delegateMouseArea.pressed ? fileSelectionDialog.platformStyle.itemPressedBackground :
                                                    delegateItem.selected ? fileSelectionDialog.platformStyle.itemSelectedBackground :
                                                                            fileSelectionDialog.platformStyle.itemBackground
            }

            Text {
                id: itemText
                elide: Text.ElideRight
                color: delegateItem.selected ? fileSelectionDialog.platformStyle.itemSelectedTextColor : fileSelectionDialog.platformStyle.itemTextColor
                anchors.verticalCenter: delegateItem.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: fileSelectionDialog.platformStyle.itemLeftMargin
                anchors.rightMargin: fileSelectionDialog.platformStyle.itemRightMargin
                text: folderModel.isFolder(index) ? "<i>" + fileName + "/</i>" : fileName;
                font: fileSelectionDialog.platformStyle.itemFont
            }
        }
    }

}
