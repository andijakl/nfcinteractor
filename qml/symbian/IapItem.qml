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

Item {
    property string productId
    property alias imageUrl: iapItemImage.source
    property int imageSize: 70
    property alias title: iapItemTitle.text
    property alias description: iapItemDescription.text
    property alias price: iapItemPrice.text
    property bool isPurchased: false

    anchors.left: parent.left
    anchors.right: parent.right
    width: parent.width
    height: iapItemTitle.height + iapItemDescription.height + iapItemPrice.height + customPlatformStyle.paddingMedium * 5 //childrenRect.height

    MouseArea {
        anchors.fill:parent
        onClicked: {
            if (!isPurchased) {
                iapManager.purchaseProduct(productId);
            }
        }
    }

    Image {
        id: iapItemImage
        width: imageSize
        height: imageSize
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: customPlatformStyle.paddingMedium
        asynchronous: true
        smooth: true
    }
    Text {
        id: iapItemTitle
        font.bold: true
        anchors.left: iapItemImage.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: customPlatformStyle.paddingMedium
        wrapMode: Text.WordWrap
        height: customPlatformStyle.fontSizeLarge
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeLarge
        color: isPurchased ? customPlatformStyle.colorNormalMid : customPlatformStyle.colorNormalLight;
    }
    Text {
        id: iapItemDescription
        anchors.left: iapItemImage.right
        anchors.right: parent.right
        anchors.top: iapItemTitle.bottom
        wrapMode: Text.WordWrap
        anchors.margins: customPlatformStyle.paddingMedium
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeMedium
        color: isPurchased ? customPlatformStyle.colorNormalMid : customPlatformStyle.colorNormalLight;
    }
    Text {
        id: iapItemPrice
        anchors.left: iapItemImage.right
        //anchors.right: parent.right
        anchors.top: iapItemDescription.bottom
        anchors.margins: customPlatformStyle.paddingMedium
        wrapMode: Text.WordWrap
        font.family: customPlatformStyle.fontFamilyRegular;
        font.italic: true
        font.pixelSize: customPlatformStyle.fontSizeMedium
        color: isPurchased ? customPlatformStyle.colorNormalMid : customPlatformStyle.colorNormalLight;
    }

    BusyIndicator {
        id: iapItemBusy
        visible: (iapItemPrice.text === qsTr("Checking..."))
        running: (iapItemPrice.text === qsTr("Checking..."))
        width: customPlatformStyle.fontSizeLarge
        height: customPlatformStyle.fontSizeLarge
        anchors.left: iapItemPrice.right
        anchors.leftMargin: customPlatformStyle.paddingMedium
        anchors.verticalCenter: iapItemPrice.verticalCenter
    }
}
