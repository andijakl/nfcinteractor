# When switching between the unlimited version and the
# normal version, also call the corresponding .bat
# file if compiling for Harmattan, to change the
# names in the debian package files!
#DEFINES += UNLIMITED_VERSION

# When compiling for publishing the app, activate this
# to change to the 0x2... UID and the right UID for the
# Smart Installer. Using those UIDs requires a development
# certificate.
# For self signed versions, remove / comment the following line.
# Note: IAA on Symbian requires DEPLOY_VERSION to be set.
# Use a developer certificate during development for Symbian IAA.
DEFINES += DEPLOY_VERSION

# Use SNEP on MeeGo Harmattan (always activated on Symbian).
# Only works for N9 PR 1.2+, library not present on 1.0 & 1.1.
DEFINES += USE_SNEP

# Define for detecting Harmattan in .cpp files.
# Only needed for experimental / beta Harmattan SDKs.
# Will be defined by default in the final SDK.
exists($$QMAKE_INCDIR_QT"/../qmsystem2/qmkeys.h"):!contains(MEEGO_EDITION,harmattan): {
    MEEGO_VERSION_MAJOR     = 1
    MEEGO_VERSION_MINOR     = 2
    MEEGO_VERSION_PATCH     = 0
    MEEGO_EDITION           = harmattan
    DEFINES += MEEGO_EDITION_HARMATTAN
}

contains(DEFINES,UNLIMITED_VERSION) {
    contains(MEEGO_EDITION,harmattan) {
        TARGET = nfcinteractorunlimited
    } else {
        TARGET = nfcinteractor
    }
} else {
    TARGET = nfcinteractor
}
VERSION = 4.20.00


symbian {
    !contains(DEFINES,UNLIMITED_VERSION) {
        # In App Purchasing APIs only available on the Symbian platform
        DEFINES += USE_IAP
        # Enables test mode for IAP
        #DEFINES += IAP_TEST_MODE
        # In App Advertising
        DEFINES += USE_IAA
    }
}
contains(MEEGO_EDITION,harmattan) {
    # Unlimited version: disable USE_IAP and USE_IAA
    # Ad-supported limited version: enable USE_IAP and USE_IAA
    !contains(DEFINES,UNLIMITED_VERSION) {
        DEFINES += USE_IAP
        DEFINES += USE_IAA
    }
}

CONFIG += mobility qt-components
# In App Purchasing
contains(DEFINES,USE_IAP) {
    symbian:CONFIG += inapppurchase
}
MOBILITY += sensors connectivity systeminfo versit contacts location

# Define QMLJSDEBUGGER to allow debugging of QML in debug builds
# (This might significantly increase build time)
# DEFINES += QMLJSDEBUGGER

OTHER_FILES += \
    qml/images/*.svg \
    qml/images/*.png

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    nfcinfo.cpp \
    nearfieldtargetinfo.cpp \
    nfctargetanalyzer.cpp \
    tagimagecache.cpp \
    nfcrecordmodel.cpp \
    nfcrecorddefaults.cpp \
    nfcrecorditem.cpp \
    nfcmodeltondef.cpp \
    nfcndefparser.cpp \
    nfcstats.cpp \
    appsettings.cpp \
    nfcpeertopeer.cpp \
    snepmanager.cpp \
    ndefnfcrecords/ndefnfcsprecord.cpp \
    ndefnfcrecords/ndefnfcmimeimagerecord.cpp \
    ndefnfcrecords/ndefnfcmimevcardrecord.cpp \
    ndefnfcrecords/ndefnfcgeorecord.cpp \
    ndefnfcrecords/ndefnfcsmarturirecord.cpp \
    ndefnfcrecords/ndefnfcsmsrecord.cpp \
    ndefnfcrecords/ndefnfcsocialrecord.cpp \
    ndefnfcrecords/ndefnfcstorelinkrecord.cpp \
    ndefnfcrecords/ndefnfcandroidapprecord.cpp \
    ndefnfcrecords/ndefnfclaunchapprecord.cpp

HEADERS += \
    nfcinfo.h \
    nearfieldtargetinfo.h \
    nfctargetanalyzer.h \
    tagimagecache.h \
    nfcrecordmodel.h \
    nfcrecorddefaults.h \
    nfcrecorditem.h \
    nfcmodeltondef.h \
    nfcndefparser.h \
    nfcstats.h \
    nfctypes.h \
    appsettings.h \
    nfcpeertopeer.h \
    snepmanager.h \
    ndefnfcrecords/ndefnfcsprecord.h \
    ndefnfcrecords/ndefnfcmimeimagerecord.h \
    ndefnfcrecords/ndefnfcmimevcardrecord.h \
    ndefnfcrecords/ndefnfcgeorecord.h \
    ndefnfcrecords/ndefnfcsmarturirecord.h \
    ndefnfcrecords/ndefnfcsmsrecord.h \
    ndefnfcrecords/ndefnfcsocialrecord.h \
    ndefnfcrecords/ndefnfcstorelinkrecord.h \
    ndefnfcrecords/ndefnfcandroidapprecord.h \
    ndefnfcrecords/ndefnfclaunchapprecord.h

simulator {
    # The simulator uses the QML and images from Symbian,
    # as it doesn't have support for simulating Qt Quick Components for
    # MeeGo Harmattan.
    OTHER_FILES += \
        qml/symbian/*.qml

    qmlFolder.source = qml/symbian/*
    qmlFolder.target = qml
    QML_IMPORT_PATH += qml/symbian

    OTHER_FILES += \
        qml/images-symbian/*.qml
    imagesFolderSymbian.source = qml/images-symbian/*
    imagesFolderSymbian.target = qml
    DEPLOYMENTFOLDERS += imagesFolderSymbian
}

symbian {
    DEPLOYMENT.display_name = "NfcInteractor"
    contains(DEFINES,DEPLOY_VERSION) {
        TARGET.UID3 = 0x2005CE03
    } else {
        TARGET.UID3 = 0xE0D154A1
        # On device debugging workaround for https://bugreports.qt-project.org/browse/QTBUG-25678
        MMP_RULES += "DEBUGGABLE_UDEBONLY"
    }

    # Allow network access on Symbian
    TARGET.CAPABILITY += NetworkServices LocalServices

    # add NfcSettings support
    include(nfcsettings/nfcsettings.pri)

    OTHER_FILES += \
        qml/symbian/*.qml

    qmlFolder.source = qml/symbian/*
    qmlFolder.target = qml
    QML_IMPORT_PATH += qml/symbian

    # Symbian specific images
    OTHER_FILES += \
        qml/images-symbian/*.qml
    imagesFolderSymbian.source = qml/images-symbian/*
    imagesFolderSymbian.target = qml
    DEPLOYMENTFOLDERS += imagesFolderSymbian

    # Demo images and tags for writing to tags
    demoImages.sources = ./demoimages/minimal.gif ./demoimages/minimal.png ./demoimages/nokia.png ./demoimages/Autostart.txt ./demoimages/SpNokia.txt
    demoImages.path = e:/nfc/
    DEPLOYMENT += demoImages

    # Smart Installer package's UID
    # This UID is from the protected range and therefore the package will
    # fail to install if self-signed. By default qmake uses the unprotected
    # range value if unprotected UID is defined for the application and
    # 0x2002CCCF value if protected UID is given to the application
    contains(DEFINES,DEPLOY_VERSION) {
        DEPLOYMENT.installer_header = 0x2002CCCF
    }
	
    # Autostart
    ndefhandler.sources = ndefhandler_nfcinteractor.xml
    ndefhandler.path = c:/private/2002AC7F/import/
    DEPLOYMENT += ndefhandler

    # Localisation support.
    CODECFORTR = UTF-8
    TRANSLATIONS += loc/$${TARGET}_en.ts \
                    loc/$${TARGET}_ar.ts \
                    loc/$${TARGET}_zh_HK.ts \
                    loc/$${TARGET}_zh_CN.ts \
                    loc/$${TARGET}_zh_TW.ts \
                    loc/$${TARGET}_cs.ts \
                    loc/$${TARGET}_da.ts \
                    loc/$${TARGET}_nl.ts \
                    loc/$${TARGET}_en_US.ts \
                    loc/$${TARGET}_fi.ts \
                    loc/$${TARGET}_fr.ts \
                    loc/$${TARGET}_fr_CA.ts \
                    loc/$${TARGET}_de.ts \
                    loc/$${TARGET}_el.ts \
                    loc/$${TARGET}_he.ts \
                    loc/$${TARGET}_hi.ts \
                    loc/$${TARGET}_hu.ts \
                    loc/$${TARGET}_id.ts \
                    loc/$${TARGET}_it.ts \
                    loc/$${TARGET}_ms.ts \
                    loc/$${TARGET}_no.ts \
                    loc/$${TARGET}_pl.ts \
                    loc/$${TARGET}_pt.ts \
                    loc/$${TARGET}_pt_BR.ts \
                    loc/$${TARGET}_ro.ts \
                    loc/$${TARGET}_ru.ts \
                    loc/$${TARGET}_sk.ts \
                    loc/$${TARGET}_es.ts \
                    loc/$${TARGET}_es_419.ts \
                    loc/$${TARGET}_sv.ts \
                    loc/$${TARGET}_th.ts \
                    loc/$${TARGET}_tr.ts \
                    loc/$${TARGET}_uk.ts \
                    loc/$${TARGET}_vi.ts

    translationfiles.source = loc/*.qm
    DEPLOYMENTFOLDERS += translationfiles

    vendorName = "Andreas Jakl"
    vendorinfo = \
        "; Localised Vendor name" \
        "%{$$addLanguageDependentPkgItem(vendorName)}" \
        " " \
        "; Unique Vendor name" \
        ":\"$$vendorName\"" \
        " "

    deployment_vendor.pkg_prerules += vendorinfo
    DEPLOYMENT += deployment_vendor

    # In App Purchasing
    contains(DEFINES,USE_IAP) {
        message(Using In-App Purchasing)

        # *** including inappbilling below
        LIBS += -liapclientapi
        # NOTE: this is enabling stdlib to prevent error 'undefined symbol __aeabi_atexit'
        LIBS += -lusrt2_2

        # capabilities required for IAP API
        TARGET.CAPABILITY += NetworkServices

        iap_dependency.pkg_prerules = "; Has dependency on IAP component" \
                                      "(0x200345C8), 0, 2, 6, {\"IAP\"}"
        DEPLOYMENT += iap_dependency

        SOURCES += \
            iap/iapmanager.cpp \
            iap/iapproduct.cpp

        HEADERS += \
            iap/iapmanager.h \
            iap/iapproduct.h

        DEPENDPATH += iap

        # IAP API files to include in package
        addIapFiles.sources = ./iap_data/IAP_VARIANTID.txt
        addIapFiles.path = ./
        DEPLOYMENT += addIapFiles

        # For testing In-App Purchase without Nokia Store
        contains(DEFINES, IAP_TEST_MODE) {
            message(In-App Purchase API in TEST_MODE)
            addIapTestFiles.sources = ./iap_data/TEST_MODE.txt
            addIapTestFiles.path = .
            DEPLOYMENT += addIapTestFiles
        }
    }
    contains(DEFINES,USE_IAA) {
        message(Using In-App Advertising)

        include(iaa/component.pri)
        iaa_components.source = iaa/inneractive/*
        iaa_components.target = qml
        DEPLOYMENTFOLDERS += iaa_components
        QML_IMPORT_PATH += iaa/inneractive
    }
}

contains(MEEGO_EDITION,harmattan) {
    # Temp
    DEFINES += MEEGO_EDITION_HARMATTAN

    # LibNDEFpush for SNEP sending
    # Needs installation to the SDK - see release_notes.txt for instructions
    contains(DEFINES,USE_SNEP) {
        CONFIG += link_pkgconfig
        PKGCONFIG += libndefpush-1.0
        SOURCES += snepmanagermeego.cpp
        HEADERS += snepmanagermeego.h
    } else {
        # Speed up launching on MeeGo/Harmattan when using applauncherd daemon
        # For some reason doesn't work when including the Lib NDEF Push library
        CONFIG += qdeclarative-boostable
    }

    OTHER_FILES += qtc_packaging/debian_harmattan/changelog \
        qtc_packaging/debian_harmattan/compat \
        qtc_packaging/debian_harmattan/control \
        qtc_packaging/debian_harmattan/copyright \
        qtc_packaging/debian_harmattan/manifest.aegis \
        qtc_packaging/debian_harmattan/README \
        qtc_packaging/debian_harmattan/rules \
        qtc_packaging/debian_harmattan/$${TARGET}.postinst \
        qtc_packaging/debian_harmattan/$${TARGET}.prerm

    OTHER_FILES += \
        qml/meego/*.qml

    qmlFolder.source = qml/meego/*
    qmlFolder.target = qml
    QML_IMPORT_PATH += qml/meego

    # Demo images for writing to tags - operation not permitted
    #demoImages.files = ./demoimages/minimal.gif ./demoimages/minimal.png ./demoimages/nokia.png
    #demoImages.path = /home/user/MyDocs/nfc/
    #INSTALLS += demoImages

    # Don't use nfcinfo_harmattan.desktop. Otherwise,
    # the NDEF Autostart handler won't find the desktop file and
    # will not be able to auto-launch this app on tag-touch.
    # See: https://bugreports.qt.nokia.com/browse/QTMOBILITY-1848
    harmattandesktopfile.files = $${TARGET}.desktop
    harmattandesktopfile.path = /usr/share/applications
    INSTALLS += harmattandesktopfile

    # To avoid conflicts, recommended to name this file according to the
    # full service name instead of just the app name.
    # See: https://bugreports.qt.nokia.com/browse/QTMOBILITY-1848
    ndefhandler_service.files = com.nokia.qtmobility.nfc.$${TARGET}.service
    ndefhandler_service.path = /usr/share/dbus-1/services/
    INSTALLS += ndefhandler_service
	
    launchericon.files = nfcinteractor80.png splash-nfcinteractor-l.png splash-nfcinteractor-p.png
    launchericon.path = /opt/$${TARGET}/
    INSTALLS += launchericon

    contains(DEFINES,USE_IAA) {
        message(Using In-App Advertising)

        include(iaa/component.pri)
        iaa_components.source = iaa/inneractive/*
        iaa_components.target = qml
        DEPLOYMENTFOLDERS += iaa_components
        QML_IMPORT_PATH += iaa/inneractive
    }
}

commonFolder.source = qml/common/*
commonFolder.target = qml
imagesFolder.source = qml/images/*
imagesFolder.target = qml
DEPLOYMENTFOLDERS += qmlFolder commonFolder imagesFolder

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

