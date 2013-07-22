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

#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include <QtDeclarative>
#include <QDeclarativePropertyMap>
#ifdef Q_OS_SYMBIAN
//#include <QSystemScreenSaver>
#endif
#include "appsettings.h"
#include "nfcinfo.h"
#include "tagimagecache.h"
#include "nfctypes.h"
#include "nfcrecordmodel.h"
#include "nfcrecorditem.h"
#if defined(USE_IAP) && defined (Q_OS_SYMBIAN)
#include "iapmanager.h"

// Define content IDs
#define IAP_ID_ADV_TAGS "818679"
#define IAP_ID_REMOVE_ADS "818680"
#define IAP_ID_UNLIMITED "818769"
#endif

#if defined(USE_IAA)
//#include "iaa/adinterface.h"
#include <inneractiveplugin.h>
#endif


/*!
  \brief Get the height of a font, including its line spacing.

  This is required to vertically center the symbol to the text line
  in the Nfc Info view. The text can have a height of multiple lines,
  nevertheless, the icon needs to be centered to the first line and
  not the multiline text.

  Specially important to adapt to the font height changes between
  MeeGo Harmattan PR 1.1 and PR 1.2 (line spacing increased in PR 1.2,
  to increase text legibility).
  */
int getFontHeight(const QString& fontName, const int fontPixelSize) {
    QFont *font = new QFont(fontName);
    font->setPixelSize(fontPixelSize);
    QFontMetrics *fm = new QFontMetrics(*font);
    const int fontHeight = fm->height();
    delete fm;
    delete font;
    return fontHeight;
}

int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

#ifdef Q_OS_SYMBIAN
    // Translation for NFC Status messages
    QString locale = QLocale::system().name();

    QTranslator translator;
    translator.load(QString("nfcinteractor_") + locale);
    app->installTranslator(&translator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
#endif

    // Create a variable that stores the platform, to customize the
    // screen appearance in QML accordingly.
#if defined(Q_OS_SYMBIAN)
    int platformId = 0;
#elif defined(MEEGO_EDITION_HARMATTAN)
    int platformId = 1;
#elif defined(QT_SIMULATOR)
    int platformId = 2;
#else
    int platformId = 3;
#endif


    // Register the Nfc interaction classes to the QML environment
    qmlRegisterType<NfcInfo>("Nfc", 1, 0, "NfcInfo");
    qmlRegisterType<NfcRecordModel>("Nfc", 1, 0, "NfcRecordModel");
    qmlRegisterType<NfcRecordItem>("Nfc", 1, 0, "NfcRecordItem");
    qmlRegisterUncreatableType<NfcTypes>("com.nfcinfo.types", 1, 0,
                                                 "NfcTypes", "This exports NfcTypes enums to QML");

    // Setup QML Viewer
    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.rootContext()->setContextProperty("platform", platformId);

    // On MeeGo, only allow activating SNEP if compiled with the library support
    // (not permitted otherwise).
    bool useSnep = true;
#if defined(MEEGO_EDITION_HARMATTAN) && !defined(USE_SNEP)
    useSnep = false;
#endif
    viewer.rootContext()->setContextProperty("allowSnep", QVariant(useSnep));

    AppSettings* settings = new AppSettings();  // Ownership will be transferred to NfcInfo*
    viewer.rootContext()->setContextProperty("settings", settings);

    // Symbian has style constants defined in platformStyle, MeeGo has a
    // different system.
    // To minimize QML code differences between both versions, define
    // a customPlatformStyle here, used by both versions.
    // (Unfortunately it's not possible to use the original
    // "platformStyle" on Symbian and set a "platformStyle" on MeeGo,
    // because the Window already has platformStyle property, which
    // would have preference over setting a context property from here)
    QScopedPointer<QDeclarativePropertyMap> platformStyle(new QDeclarativePropertyMap());
    platformStyle->insert("paddingSmall", QVariant(4));
    platformStyle->insert("paddingMedium", QVariant(8));
    platformStyle->insert("paddingLarge", QVariant(12));
    platformStyle->insert("fontSizeSmall", QVariant(18));
    platformStyle->insert("colorNormalLight", QVariant(QColor(255, 255, 255)));
    platformStyle->insert("colorNormalMid", QVariant(QColor(153, 153, 153)));
#if defined(MEEGO_EDITION_HARMATTAN)
    qDebug() << "MeeGo UI platform style";
    platformStyle->insert("fontFamilyRegular", QVariant(QString("Nokia Pure Text")));
    platformStyle->insert("fontSizeMedium", QVariant(22));
    platformStyle->insert("fontHeightMedium", QVariant(getFontHeight(QString("Nokia Pure Text"), 22)));
    platformStyle->insert("fontSizeLarge", QVariant(26));
#else
    qDebug() << "Symbian UI platform style";
    platformStyle->insert("fontFamilyRegular", QVariant(QFont().defaultFamily()));
    platformStyle->insert("fontSizeMedium", QVariant(20));
    platformStyle->insert("fontHeightMedium", QVariant(getFontHeight(QFont().defaultFamily(), 20)));
    platformStyle->insert("fontSizeLarge", QVariant(22));
#endif
    viewer.rootContext()->setContextProperty("customPlatformStyle", platformStyle.data());

    // Register the image provider class to QML
    TagImageCache *tagImageCache = new TagImageCache();
    viewer.engine()->addImageProvider(QLatin1String("nfcimageprovider"), tagImageCache);

#ifdef USE_IAP
    // In App Purchasing
    qDebug() << "Using IAP";
    viewer.rootContext()->setContextProperty("useIap", QVariant(true));
    #if defined(Q_OS_SYMBIAN)
        // In App Purchasing APIs on Symbian
        QScopedPointer<IapManager> iapManager(new IapManager());
        // Add the known items to the list
        // Also retrieves if the user has already purchased the items
        // from the internal DB, to prevent the need to go online
        // every time.
        iapManager->addIapProduct(IAP_ID_ADV_TAGS);
        iapManager->addIapProduct(IAP_ID_REMOVE_ADS);
        iapManager->addIapProduct(IAP_ID_UNLIMITED);
        viewer.rootContext()->setContextProperty("iapManager", iapManager.data());
        viewer.rootContext()->setContextProperty("iapIdAdvTags", QVariant(IAP_ID_ADV_TAGS));
        viewer.rootContext()->setContextProperty("iapIdRemoveAds", QVariant(IAP_ID_REMOVE_ADS));
        viewer.rootContext()->setContextProperty("iapIdUnlimited", QVariant(IAP_ID_UNLIMITED));
    #else
        // Unlimited version on Harmattan
        // No IAP APIs available -> separate product on Nokia Store
    #endif
#else
    viewer.rootContext()->setContextProperty("useIap", QVariant(false));
#endif

#ifdef IAP_TEST_MODE
    qDebug() << "IAP Test Mode";
    viewer.rootContext()->setContextProperty("iapTestMode", QVariant(true));
#else
    viewer.rootContext()->setContextProperty("iapTestMode", QVariant(false));
#endif

#ifdef USE_IAA
    // In App Advertising
    qDebug() << "Using IAA";
    #if defined(Q_OS_SYMBIAN) && defined(USE_IAP)
        // Define the pointer here; creating it in the if would instantly
        // destroy it again after the if ends, but we obviously need to
        // keep the instance alive till the app exits.
        // Use a scoped pointer so that we only need to create the instance
        // when the user hasn't purchased the remove ads upgrade, to speed
        // up app startup time if he has.
        //QScopedPointer<AdInterface> adI;
        // Symbian & using IAP - only start the ad interface if the user
        // didn't already purchase the remove ads upgrade
        if (!iapManager->isProductPurchased(IAP_ID_REMOVE_ADS)) {
            // IAA Upgrade not purchased - use IAA
            //AdInterface adI;
            //adI.reset(new AdInterface());
            //viewer.rootContext()->setContextProperty("adInterface", adI.data());
            inneractivePlugin::initializeEngine(viewer.engine());
            viewer.rootContext()->setContextProperty("useIaa", QVariant(true));
        } else {
            // IAA Upgrade purchased - don't use IAA
            viewer.rootContext()->setContextProperty("useIaa", QVariant(false));
        }
    #else
        // [Symbian and no IAP] or Harmattan
        //QScopedPointer<AdInterface> adI(new AdInterface());
        //viewer.rootContext()->setContextProperty("adInterface", adI.data());
        inneractivePlugin::initializeEngine(viewer.engine());
        viewer.rootContext()->setContextProperty("useIaa", QVariant(true));
    #endif
#else
    // IAA not activated
    viewer.rootContext()->setContextProperty("useIaa", QVariant(false));
#endif

    QScopedPointer<NfcInfo> nfcInfo(new NfcInfo());
    // Give the ndef manager a pointer to our declarative view,
    // so that it can raise the app to the foreground when an
    // autostart tag is touched and the app is already running.
    nfcInfo->setDeclarativeView(viewer);
    nfcInfo->setAppSettings(settings);
    nfcInfo->setImageCache(tagImageCache);
    // Pass the record model back to the QML
    // Could be easier to just make the model a property?
    viewer.rootContext()->setContextProperty("recordModel", nfcInfo->recordModel());
#ifdef USE_IAP
#ifdef Q_OS_SYMBIAN
    // Symbian: set unlimited tags according to whether it has been purchased already
    nfcInfo->setUnlimitedAdvancedMsgs(iapManager->isProductPurchased(IAP_ID_ADV_TAGS) || iapManager->isProductPurchased(IAP_ID_UNLIMITED));
#else
    // Harmattan: set unlimited tags according to whether IAP is enabled in the .pro file.
    // (if it is enabled, limit tag writing. If USE_IAP isn't set, use unlimited tag writing)
    nfcInfo->setUnlimitedAdvancedMsgs(false);
#endif
#endif

    // Make the nfcPeerToPeer class known in the QML world
    viewer.rootContext()->setContextProperty("nfcInfo", nfcInfo.data());

    // Finally, load the main QML file to the viewer.
    viewer.setMainQmlFile(QLatin1String("qml/main.qml"));

    viewer.showExpanded();

    //nfcInfo->initAndStartNfcAsync();

//#ifdef Q_OS_SYMBIAN
//    // Prevent screensaver from kicking in on Symbian
//    QSystemScreenSaver *screensaver = new QSystemScreenSaver ( &viewer );
//    screensaver->setScreenSaverInhibit();
//#endif

    return app->exec();
}
