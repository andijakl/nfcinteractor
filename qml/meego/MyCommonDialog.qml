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

Dialog {
    id: genericDialog

    property string titleText: ""

    property Style platformStyle: SelectionDialogStyle {}

    //Deprecated, TODO Remove this on w13
    property alias style: genericDialog.platformStyle

    //private
    property bool __drawFooterLine: false

    title: Item {
        id: header
        height: genericDialog.platformStyle.titleBarHeight

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Item {
            id: labelField

            anchors.fill:  parent

            Item {
                id: labelWrapper
                anchors.left: labelField.left
                anchors.right: closeButton.left

                anchors.bottom:  parent.bottom
                anchors.bottomMargin: genericDialog.platformStyle.titleBarLineMargin

                //anchors.verticalCenter: labelField.verticalCenter

                height: titleLabel.height

                Label {
                    id: titleLabel
                    x: genericDialog.platformStyle.titleBarIndent
                    width: parent.width - closeButton.width
                    //anchors.baseline:  parent.bottom
                    font: genericDialog.platformStyle.titleBarFont
                    color: genericDialog.platformStyle.commonLabelColor
                    elide: genericDialog.platformStyle.titleElideMode
                    text: genericDialog.titleText
                }

            }

            Image {
                id: closeButton
                anchors.bottom:  parent.bottom
                anchors.bottomMargin: genericDialog.platformStyle.titleBarLineMargin-6
                //anchors.verticalCenter: labelField.verticalCenter
                anchors.right: labelField.right

                opacity: closeButtonArea.pressed ? 0.5 : 1.0

                source: "image://theme/icon-m-common-dialog-close"

                MouseArea {
                    id: closeButtonArea
                    anchors.fill: parent
                    onClicked:  {genericDialog.reject();}
                }

            }

        }

        Rectangle {
            id: headerLine

            anchors.left: parent.left
            anchors.right: parent.right

            anchors.bottom:  header.bottom

            height: 1

            color: "#4D4D4D"
        }

    }

    content: Item {id: contentField}

    buttons: Item {
         id: footer

         width: parent.width
         height: childrenRect.height

         //hack to make sure, we're evaluating the correct height
         Item {
             id: lineWrapper
             width: parent.width
             height: childrenRect.height
             y: 10

             Rectangle {
                 id: footerLine

                 anchors.left: parent.left
                 anchors.right: parent.right
                 anchors.top: parent.top
                 height: genericDialog.__drawFooterLine ? 1 : 0

                 color: "#4D4D4D"
             }
         }

         //ugly hack to assure, that we're always evaluating the correct height
         Item {id: dummy; anchors.fill:  parent}

     }

}
