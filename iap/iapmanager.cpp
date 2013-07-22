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

#include "iapmanager.h"

// To be able to transfer data this type with Qt::QueuedConnection
Q_DECLARE_METATYPE(IAPClient::ProductList)
Q_DECLARE_METATYPE(IAPClient::ProductDataList)

IapManager::IapManager(QObject *parent) :
    QObject(parent),
    m_client(NULL)
{
    // These meta types need to be registered by our app (?)
    // Otherwise, the IAP APIs will complain.
    qRegisterMetaType<IAPClient::ProductDataList>("IAPClient::ProductDataList");
    qRegisterMetaType<IAPClient::ProductDataHash>("IAPClient::ProductDataHash");
    // Don't create m_client instance just yet, only on demand.
    // Creating the instance directly connects to the Internet.
    // We only want the app to connect when the user wants
    // to interact with IAP, not every time it's started.
    // (also significantly adds to the app startup time otherwise).

}

void IapManager::addIapProduct(const QString& productId)
{
    m_iapProducts.append(new IapProduct(productId, this));
    // Check in our internal DB if the user already purchased the item
    iDbCheckProductPurchased(productId);
}

void IapManager::initIapEngine()
{
    // Ensure that the IAP manager has been instantiated already,
    // or create a new instance otherwise.
    checkIapClientInstance();
}

void IapManager::getProductData()
{
    // Ensure that the IAP manager has been instantiated already,
    // or create a new instance otherwise.
    checkIapClientInstance();

    // Request complete data for all items in the list
    // -> continue in slot productDataReceived()
    foreach(IapProduct* curProduct, m_iapProducts) {
        int requestId = m_client->getProductData(curProduct->productId());
        curProduct->setRequestId(requestId);
        if (requestId < 0) {
            qDebug() << "IAP: Error requesting product data of productId: " << curProduct->productId();
        }
    }
}

bool IapManager::isProductPurchased(const QString& productId)
{
    IapProduct* iapProduct = getIapProduct(productId);
    if (iapProduct) {
        return iapProduct->purchased();
    }
    return false;
}

QString IapManager::productPrice(const QString &productId)
{
    IapProduct* iapProduct = getIapProduct(productId);
    if (iapProduct) {
        return iapProduct->price();
    }
    return QString();
}

void IapManager::purchaseProduct(const QString& productId)
{
    // Ensure that the IAP manager has been instantiated already,
    // or create a new instance otherwise.
    checkIapClientInstance();

    // Check if the item is in the list of known items
    IapProduct* iapProduct = getIapProduct(productId);
    if (iapProduct) {
        // Send request to IAP to purchase item

        // The flag indicates if an in-app purchase becomes
        // a restoration because it has already been purchased.
        // ForcedAutomaticRestoration: The purchase automatically
        // becomes a restoration.
        const int requestId = m_client->purchaseProduct(productId, IAPClient::ForcedAutomaticRestoration);
        iapProduct->setRequestId(requestId);
        if (requestId < 0) {
            qDebug() << "IAP: Error requesting the purchase of product ID: " << productId;
        }
        // -> continue in slot purchaseCompleted() & purchaseFlowCompleted()
    } else {
        qDebug() << "IAP: Error starting purchase request: unknown product ID: " << productId;
    }
}

void IapManager::removePurchasedProduct(const QString &productId)
{
    iDbSetProductPurchased(productId, false);
}

void IapManager::restoreProducts()
{
    // Ensure that the IAP manager has been instantiated already,
    // or create a new instance otherwise.
    checkIapClientInstance();

    // Request from IAP all items that can be restored
    // -> continue in slot restorableProductsReceived()
    // Do not show the UI to check for restoring products.
    // (otherwise we'd have a UI pop up every time the
    // IAP page is shown).
    // If the user is not logged in, he will just have to
    // touch the items, which will then restore them anyway.
    int requestId = m_client->getRestorableProducts(IAPClient::OnlyInSilentAuthentication);
    if (requestId < 0) {
        qDebug() << "IAP: Error requesting restorable products";
    }
}

void IapManager::productDataReceived(int requestId, QString status, IAPClient::ProductDataHash productData)
{
    // Go through list of items and add additional metadata
    // to the items.
    if(status.compare("OK", Qt::CaseSensitive) == 0) {
        // Getting product data OK
        // Get ID of the product data item we got additional info for
#ifdef IAP_TEST_MODE
        // Debug only - in test mode, the APIs unfortunately don't return
        // the product ID we requested info for. So just set one of our
        // own. Note that then of course the UI will only show a €5 price
        // for this one item, while the others will remain in "checking" state.
        const QString productId = "818680";
#else
        const QString productId = productData.value("id").toString();
#endif
        qDebug() << "IAP: Product data received for " << productId;
        IapProduct* iapProduct = getIapProduct(productId);
        if (iapProduct) {
            // Found a matching product in our own "database"
            // Update the metadata
            iapProduct->setPrice(productData.value("price").toString());
            iapProduct->setRequestFinished();
            qDebug() << "IAP: price applied to internal list";
            emit productDataAvailable(productId);
        }
    } else {
        // Error
        qDebug() << "IAP: Error receiving product data: " << status;
    }
}

void IapManager::purchaseCompleted(int requestId, QString status, QString purchaseTicket)
{
    // Get the product that was purchased
    IapProduct* iapProduct = getProductWithRequestId(requestId);
    if (iapProduct) {
        iapProduct->setRequestFinished();
        // Check if purchase was successful
        if(status.compare("OK", Qt::CaseSensitive) == 0) {
            // IAP successful!
            // Store activation of feature to internal DB
            iDbSetProductPurchased(iapProduct->productId(), true);
            // Send out signal that the item is purchased
            emit productPurchased(iapProduct->productId());
        } else {
            // IAP failed!
            if (status.compare("Cancel", Qt::CaseSensitive) == 0) {
                qDebug() << "IAP error: User cancelled purchase";
                // No need to inform the user through a singal, as he cancelled it
            } else {
                qDebug() << "IAP error: " << status;
                emit iapError("In App Purchase failed. Please try again");
            }
        }
    } else {
        qDebug() << "IAP error: Didn't find cached product ID in internal DB";
        emit iapError("Didn't find cached product ID in internal DB");
    }

    Q_UNUSED(purchaseTicket)
}

void IapManager::purchaseFlowFinished(int requestId)
{
    // This signal is emitted when IAP shows no more any UI after purchase,
    // and the client is in control of the whole UI.

    // Nothing to do here for our app.
}

void IapManager::restorableProductsReceived(int requestId, QString status, IAPClient::ProductDataList items)
{
    // Check status of the request
    if(status.compare("OK", Qt::CaseSensitive) == 0) {
        // Check if any item in our list is restorable
        // If yes -> send request to restore the item
        foreach (IAPClient::ProductDataHash curProductData, items) {
            const QString restorableProductId = curProductData.value("id").toString();
            IapProduct* restorableProduct = getIapProduct(restorableProductId);
            if (restorableProduct) {
                // Found a valid item in our list that can be restored
                const int requestId = m_client->restoreProduct(restorableProductId);
                restorableProduct->setRequestId(requestId);
                if (requestId < 0) {
                    qDebug() << "IAP: Error requesting restorable products";
                }
            } else {
                qDebug() << "IAP: Unable to find restorable product ID in internal list: " << restorableProductId;
            }
        }
    } else {
        // IAP failed!
        if (status.compare("Cancel", Qt::CaseSensitive) == 0) {
            qDebug() << "IAP error: User cancelled purchase";
            // No need to inform the user through a singal, as he cancelled it
        } else {
            qDebug() << "IAP restoration request error: " << status;
            emit iapError("Restoration query failed. Please try again.");
        }
    }
}

void IapManager::restorationCompleted(int requestId, QString status, QString purchaseTicket)
{
    // Check status of the request
    if(status.compare("OK", Qt::CaseSensitive) == 0) {
        // An individual item has been successfully restored
        IapProduct* restoredProduct = getProductWithRequestId(requestId);
        if (restoredProduct) {
            restoredProduct->setRequestFinished();
            iDbSetProductPurchased(restoredProduct->productId(), true);
            emit productRestored(restoredProduct->productId());
        } else {
            qDebug() << "IAP: Unable to find restored product ID in internal list. Request ID: " << requestId;
        }
    } else {
        // IAP failed!
        if (status.compare("Cancel", Qt::CaseSensitive) == 0) {
            qDebug() << "IAP error: User cancelled purchase";
            // No need to inform the user through a singal, as he cancelled it
        } else {
            // Possible issues that could be relevant:
            // RestorationNotSupported
            // RestorationNotAllowed
            // RestorationLimitExceeded
            // RestorationDeviceLimitExceeded
            qDebug() << "IAP restoration error: " << status;
            emit iapError("Restoration of product failed. Please try again.");
        }
    }
    Q_UNUSED(purchaseTicket);
}

void IapManager::checkIapClientInstance()
{
    // Create IAPClient instance only on demand
    if (!m_client) {
        qDebug() << "Creating IAPClient...";
        m_client = new IAPClient(this);
        // Get product data
        bool success = connect(m_client, SIGNAL(productDataReceived(int,QString,IAPClient::ProductDataHash)),
                this, SLOT(productDataReceived(int,QString,IAPClient::ProductDataHash)), Qt::QueuedConnection);
        if (!success)
            qDebug() << "IAP: Failed to connect productDataReceived() signal to slot";

        // Purchase product
        connect(m_client, SIGNAL(purchaseCompleted( int , QString, QString)),
                this, SLOT(purchaseCompleted( int , QString, QString)),Qt::QueuedConnection);
        connect(m_client, SIGNAL(purchaseFlowFinished(int)),
                this, SLOT(purchaseFlowFinished(int)), Qt::QueuedConnection);

        // Product restoration
        connect(m_client, SIGNAL(restorableProductsReceived (int, QString, IAPClient::ProductDataList)),
                this, SLOT(restorableProductsReceived (int, QString, IAPClient::ProductDataList)),
                        Qt::QueuedConnection);
        connect(m_client, SIGNAL(restorationCompleted (int , QString , QString)),
                this, SLOT(restorationCompleted (int , QString , QString)), Qt::QueuedConnection);
    }
}

IapProduct* IapManager::getIapProduct(const QString& productId)
{
    foreach(IapProduct* curProduct, m_iapProducts) {
        if (curProduct->productId() == productId) {
            return curProduct;
        }
    }
    return NULL;
}

IapProduct* IapManager::getProductWithRequestId(const int requestId)
{
    foreach(IapProduct* curProduct, m_iapProducts) {
        if (curProduct->requestId() == requestId) {
            return curProduct;
        }
    }
    return NULL;
}

bool IapManager::iDbCheckProductPurchased(const QString& productId)
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);
    const bool itemPurchased = settings.value(productId, false).toBool();
    // Apply the purchased status also to the item in the list
    IapProduct* iapProduct = getIapProduct(productId);
    if (iapProduct) {
        // Apply the purchase status from the internal settings database
        // to the iapItem in the list.
        iapProduct->setPurchased(itemPurchased);
    } else {
        qDebug() << "IAP: Didn't find product id in list when checking the purchase status in the internal DB.";
    }
    return itemPurchased;
}

void IapManager::iDbSetProductPurchased(const QString& productId, const bool itemPurchased)
{
    QSettings settings(SETTINGS_ORG, SETTINGS_APP, this);

    // Apply the purchased status also to the item in the list
    IapProduct* iapProduct = getIapProduct(productId);
    if (iapProduct) {
        // Store purchase status to the item in our list
        iapProduct->setPurchased(itemPurchased);
        // Save to internal settings database
        settings.setValue(productId, itemPurchased);
    } else {
        qDebug() << "IAP: Didn't find product id in list when setting purchase status.";
    }
}

