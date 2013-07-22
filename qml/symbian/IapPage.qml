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
    id: iapPage
    tools: ToolBarLayout {
        ToolButton {
            flat: true
            iconSource: "toolbar-back";
            onClicked: pageStack.depth <= 1 ? Qt.quit() : pageStack.pop()
        }
//        ToolButton {
//            flat: true
//            visible: iapTestMode
//            text: "Restore"
//            onClicked: iapManager.restoreItems()
//        }
        ToolButton {
            flat: true
            visible: iapTestMode
            iconSource: "toolbar-delete";
            onClicked: {
                iapManager.removePurchasedProduct(iapIdAdvTags);
                iapManager.removePurchasedProduct(iapIdRemoveAds);
                iapManager.removePurchasedProduct(iapIdUnlimited);
                applyPurchaseStatus(iapIdAdvTags);
                applyPurchaseStatus(iapIdRemoveAds);
                applyPurchaseStatus(iapIdUnlimited);
            }
        }
    }

    Connections {
        target: iapManager
        onProductDataAvailable: {
            console.log("QML: Product data received");
            applyPurchasePrice(productId);
        }
        onProductPurchased: {
            applyPurchaseStatus(productId);
        }
        onProductRestored: {
            applyPurchaseStatus(productId);
        }
        onIapError: {
            // TODO
        }
    }

    function applyPurchaseStatus(productId) {
        // Get purchase status from the item
        var isPurchased = iapManager.isProductPurchased(productId);
        // Update the price on the IAP-page
        applyPurchasePrice(productId);
        var applyToItem = getQmlItemForProductId(productId);
        applyToItem.isPurchased = isPurchased;
        // Check which product was purchased and apply actions in the app
        if (productId === iapIdAdvTags) {
            // Advanced tags
            window.setUnlimitedAdvancedMsgs(isPurchased);
        } else if (productId === iapIdRemoveAds) {
            // Remove ads
            window.setRemoveAds(isPurchased);
        } else if (productId === iapIdUnlimited) {
            if (isPurchased) {
                // Unlimited (includes all other IAP items)
                // When Unlimited is purchased, remove ads and set unlimited tag writing.
                // Note that when the Unlimited isn't purchased, we shouldn't deactivate
                // the others here, as they might still have been individually purchased.
                window.setUnlimitedAdvancedMsgs(isPurchased);
                window.setRemoveAds(isPurchased);
            }
        }
        // Apply the status also to other items that might be affected
        // (both individuals purchased -> mark unlimited as purchased.
        //  unlimited purchased -> mark both individuals as purchased)
        applyMetaPurchaseStatus();
    }

    function applyMetaPurchaseStatus() {
        if (iapManager.isProductPurchased(iapIdAdvTags) && iapManager.isProductPurchased(iapIdRemoveAds)) {
            // Purchased both advanced tags and remove ads
            // -> Also apply unlimited IAP item
            getQmlItemForProductId(iapIdUnlimited).isPurchased = true;
            applyPurchasePrice(iapIdUnlimited);
        } else if (iapManager.isProductPurchased(iapIdUnlimited)) {
            // Unlimited is purchased:
            // -> also mark the other items as purchased
            getQmlItemForProductId(iapIdAdvTags).isPurchased = true;
            applyPurchasePrice(iapIdAdvTags);
            getQmlItemForProductId(iapIdRemoveAds).isPurchased = true;
            applyPurchasePrice(iapIdRemoveAds);
        }
    }

    function applyPurchasePrice(productId) {
        var price = iapManager.productPrice(productId);
        var isPurchased = iapManager.isProductPurchased(productId);
        if (iapManager.isProductPurchased(iapIdUnlimited)) {
            // If unlimited is purchased, all the others should appear
            // as purchased as well
            // (even if they are not in the backend, but that doesn't matter).
            isPurchased = true;
        } else if (productId === iapIdUnlimited &&
                   iapManager.isProductPurchased(iapIdAdvTags) &&
                   iapManager.isProductPurchased(iapIdRemoveAds)) {
            // Processing unlimited item:
            // If all others are already purchased, set this unlimited to be
            // purchased as well
            isPurchased = true;
        }

        var applyToItem = getQmlItemForProductId(productId);
        console.log("Price: " + price + ", isPurchased: " + isPurchased + ", item: " + applyToItem);

        if (isPurchased) {
            applyToItem.price = qsTr("Activated (already purchased)");
        } else {
            applyToItem.price = price;
        }
    }

    function getQmlItemForProductId(productId) {
        switch (productId) {
        case iapIdAdvTags:
            return iapAdvTagsItem;
        case iapIdRemoveAds:
            return iapRemoveAdsItem;
        case iapIdUnlimited:
            return iapUnlimitedItem;
        default:
            console.log("No QML item available for product id " + productId);
            break;
        }
    }

    function allItemPurchased() {
        return ((iapManager.isProductPurchased(iapIdAdvTags) && iapManager.isProductPurchased(iapIdRemoveAds)) ||
                iapManager.isProductPurchased(iapIdUnlimited));
    }

    Text {
        id: messageHeader
        text: qsTr("Purchase Upgrades")
        font.bold: true
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        wrapMode: Text.WordWrap
        height: customPlatformStyle.fontSizeLarge + customPlatformStyle.paddingMedium
        font.family: customPlatformStyle.fontFamilyRegular;
        font.pixelSize: customPlatformStyle.fontSizeLarge
        color: "aliceblue";
    }

    // -------------------------------------------------------------
    // Purchasable Items
    Flickable {
        anchors.top: messageHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentHeight: iapAdvTagsItem.height + iapRemoveAdsItem.height + iapUnlimitedItem.height
        contentWidth: parent.width
        clip: true

        IapItem {
            id: iapAdvTagsItem
            productId: iapIdAdvTags
            anchors.top: parent.top
            imageUrl: "iapAdvTags.png"
            title: qsTr("Advanced Tag Writing")
            description: qsTr("Write an unlimited number of advanced message types, like business cards, geo tags or social network tags.")
            price: qsTr("Checking...")
        }
        IapItem {
            id: iapRemoveAdsItem
            productId: iapIdRemoveAds
            anchors.top: iapAdvTagsItem.bottom
            imageUrl: "iapRemoveAds.png"
            title: qsTr("Remove Ads")
            description: qsTr("Use the application without ads, also enabling the use in offline mode and without data connection when roaming.")
            price: qsTr("Checking...")
        }
        IapItem {
            id: iapUnlimitedItem
            productId: iapIdUnlimited
            anchors.top: iapRemoveAdsItem.bottom
            imageUrl: "iapUnlimited.png"
            title: qsTr("Unlimited")
            description: qsTr("Combines the other items into one purchase: remove ads to use Nfc Interactor in offline mode, and write an unlimited number of advanced tags!")
            price: qsTr("Checking...")
        }
    }


    Component.onCompleted: {
        // Check if the products have been purchased already
        applyPurchaseStatus(iapIdAdvTags);
        applyPurchaseStatus(iapIdRemoveAds);
        applyPurchaseStatus(iapIdUnlimited);
        // Let the page transition animations finish, and then
        // put the IAP APIs into use (which will stall the app
        // briefly as the IAP Client APIs initialize themselves).
        initIapTimer.restart();
    }

    Timer {
        id: initIapTimer
        interval: 500   // Give enough time for page transitions to finish
        repeat: false
        onTriggered: {
            // Connect to the IAP service and check the product data
            console.log("QML: Initializing IAP engine");
            iapManager.initIapEngine();
            // Start timer to check products
            checkProductsTimer.restart();
        }
    }

    Timer {
        id: checkProductsTimer
        interval: 1000   // Give enough time for page transitions to finish
        repeat: false
        onTriggered: {
            // Connect to the IAP service and check the product data
            console.log("QML: Checking product data");
            iapManager.getProductData();
            // Check if we can restore any items
            iapManager.restoreProducts();
        }
    }

}
