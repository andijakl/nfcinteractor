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

Item {
    id: iaaAd

    height: iaaItem.height

    AdItem {
        id: iaaItem
        parameters: AdParameters {
            applicationId: "Mopius_NfcInteractor_OVI"
            usePositioning: false
        }
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        showText: false
    }

//    Connections {
//        target: adInterface
//        onNetworkNotAccessible:
//        {
//            quitDialog.open();
//        }
//        onNetworkAccessibilityChanged:
//        {
//            console.log("Network isOnline: " + adInterface.networkAccessible);
//        }
//    }

//    QueryDialog {
//        id: quitDialog
//        titleText: qsTr("No connectivity.")
//        message: qsTr("Please enable network connectivity and restart the app.\nIf you prefer to use the app in offline mode, purchase the Nfc Interactor Unlimited from the Nokia Store.")
//        acceptButtonText: "Quit"
//        onAccepted: Qt.quit()
//    }
}
