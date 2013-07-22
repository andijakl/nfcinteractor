/*
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Part of Qt NFC Setting sample application.
*/

/*!
 * \file
 * \brief Declaration of the NfcSettings class and related enumerations.
 *
 * This file contains the declaration of the NfcSettings class. The
 * NFC feature support, NFC mode and NFC error related enumerations
 * NfcSettings::NfcFeature, NfcSettings::NfcMode and NfcSettings::NfcError
 * are also declared here.
 */

#ifndef NFCSETTINGS_H
#define NFCSETTINGS_H

#include <QtCore/QObject>

class NfcSettingsPrivate;

/*!
 * \brief A class for querying NFC support and monitoring the current NFC mode.
 *
 * An instance of this class can be used to query whether a device supports Near
 * Field Communication, and to retrieve the current NFC mode setting value.
 * Support for NFC requires both the presence of NFC hardware and up to date
 * firmware.
 *
 * On devices that support NFC features, the NFC mode value indicates both
 * whether the NFC hardware is currently powered on and the mode it is operating
 * in. In addition to retrieving the mode setting on demand, a signal is
 * provided for getting a notification whenever the mode setting changes.
 *
 * The following code snippet shows a simple <code>QObject</code> derived class
 * declaration that is used as the basis for the usage examples below:
 *
 * \code
 * #ifndef MYCLASS_H
 * #define MYCLASS_H
 *
 * #include <QtCore/QObject>
 *
 * #include "nfcsettings.h"
 *
 * class MyClass : public QObject
 * {
 *     Q_OBJECT
 *
 * public:
 *     explicit MyClass(QObject *parent = 0);
 *     ~MyClass();
 *
 * private slots:
 *     void handleNfcModeChange(NfcSettings::NfcMode nfcMode);
 *     void handleNfcError(NfcSettings::NfcError nfcError, int error);
 *
 * private:
 *     NfcSettings *m_nfcSettings;
 * };
 *
 * #endif // MYCLASS_H
 * \endcode
 *
 * The following code snippet shows how to create a new NfcSettings instance,
 * query whether NFC is supported by the device, get the current NFC mode
 * setting value and how establish the signal-slot connections to receive mode
 * change notifications:
 *
 * \code
 * void MyClass::MyClass(QObject *parent) : QObject(parent)
 * {
 *     // Construct a new instance.
 *     m_nfcSettings = new NfcSettings(this);
 *
 *     // Retrieve the NFC feature support information.
 *     NfcSettings::NfcFeature nfcFeature = m_nfcSettings->nfcFeature();
 *
 *     if (nfcFeature == NfcSettings::NfcFeatureSupported) {
 *         // Connect signals for receiving mode change and error notifications.
 *         connect(m_nfcSettings, SIGNAL(nfcModeChanged(NfcSettings::NfcMode)), SLOT(handleNfcModeChange(NfcSettings::NfcMode)));
 *         connect(m_nfcSettings, SIGNAL(nfcErrorOccurred(NfcSettings::NfcError, int)), SLOT(handleNfcError(NfcSettings::NfcError, int)));
 *
 *         // Retrieve the initial value of the NFC mode setting.
 *         NfcSettings::NfcMode nfcMode = m_nfcSettings->nfcMode();
 *
 *         if (nfcMode != NfcSettings::NfcModeOn) {
 *             // NFC is supported but not turned on, prompt the user to enable it.
 *         }
 *     }
 *     else if (nfcFeature == NfcSettings::NfcFeatureSupportedViaFirmwareUpdate) {
 *         // Display message to user to update device firmware.
 *     } else {
 *         // Display message informing the user that NFC is not supported by this device.
 *     }
 * }
 * \endcode
 *
 * To receive mode change notifications, it is enough to implement a slot to
 * handle the nfcModeChanged() signal. However, it is recommended to also
 * implement a slot to handle the the nfcErrorOccurred() signal, as some error
 * situations may make the NFC mode reading unreliable, see nfcMode() and
 * nfcError() for details.
 *
 * The following code snippet shows the implementation of the
 * handleNfcModeChange() slot:
 *
 * \code
 * void MyClass::handleNfcModeChange(NfcSettings::NfcMode nfcMode)
 * {
 *     switch (nfcMode) {
 *     case NfcSettings::NfcModeNotSupported:
 *         // NFC is not currently supported. It is not possible to distinguish
 *         // whether a firmware update could enable NFC features based solely
 *         // on the value of the nfcMode parameter. The return value of
 *         // NfcSettings::nfcFeature() indicates whether a firmware update is
 *         // applicable to this device.
 *         break;
 *     case case NfcSettings::NfcModeUnknown:
 *         // NFC is supported, but the current mode is unknown at this time.
 *         break;
 *     case NfcSettings::NfcModeOn:
 *         // NFC hardware is supported, and currently switched on.
 *         break;
 *     case NfcSettings::NfcModeCardOnly:
 *         // NFC hardware is supported, and currently in card emulation mode.
 *         break;
 *     case NfcSettings::NfcModeOff:
 *         // NFC hardware is supported, and currently switched off.
 *         break;
 *     default:
 *         break;
 *     }
 * }
 * \endcode
 *
 * The nfcModeChanged() signal will not be emitted if there an error occurs while
 * obtaining the new mode value, or while attempting to resubscribe to receive
 * further notifications. In these situations the nfcErrorOccurred() signal will
 * be emitted instead.
 *
 * An example implementation of a slot for handling the nfcErrorOccurred()
 * signal is shown below:
 *
 * \code
 * void Widget::handleNfcError(NfcSettings::NfcError nfcError, int error)
 * {
 *     // The platform specific error code is ignored here.
 *     Q_UNUSED(error)
 *
 *     switch (nfcError) {
 *     case NfcSettings::NfcErrorFeatureSupportQuery:
 *         // Unable to query NFC feature support.
 *         break;
 *     case NfcSettings::NfcErrorSoftwareVersionQuery:
 *         // Unable to query device software version.
 *         break;
 *     case NfcSettings::NfcErrorModeChangeNotificationRequest:
 *         // Unable to request NFC mode change notifications.
 *         break;
 *     case NfcSettings::NfcErrorModeChangeNotification:
 *         // NFC mode change notification was received, but caused an error.
 *         break;
 *     case NfcSettings::NfcErrorModeRetrieval:
 *         // Unable to retrieve current NFC mode.
 *         break;
 *     default:
 *         break;
 *     }
 * }
 * \endcode
 *
 * The second parameter of the signal, <code>error</code>, is a platform
 * specific error code that can be used to determine the cause of the failure
 * in more detail.
 *
 * It is possible to try to reset the NfcSettings instance to recover from
 * transient error situations by calling the reset() function. However, this
 * must not be done directly from the slot connected to the nfcErrorOccurred()
 * signal, as calling reset() may cause further errors to be reported resulting
 * in endless recursion.
 *
 * The reset() call should be asynchronous, i.e. by starting a low priority
 * timer in the error handler slot instead of a direct call to reset().
 * Additionally there should be a limit on how many reset() calls are made. If
 * for example three reset() calls made at five second intervals do not clear
 * the error situation, the error is most likely non-recoverable.
 *
 * \note This class comprises the public part of the private implementation
 * design pattern, and as such it is the class intended to for use by clients of
 * this API. The corresponding platform specific implementation classes (named
 * NfcSettingsPrivate) are the classes containing the actual implementation of
 * the functionality. There are two versions of NfcSettingsPrivate, one for
 * Symbian devices (declared in nfcsettings_symbian_p.h) and a default
 * implementation for all other platforms (declared in nfcsettings_p.h). The
 * Symbian implementation is fully functional, the default implementation
 * reports NFC as not supported.
 *
 * \see NfcSettingsPrivate
 */
class NfcSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(NfcFeature)
    Q_ENUMS(NfcMode)
    Q_ENUMS(NfcError)

    /*!
     * \brief The current level of NFC feature support provided by the device.
     *
     * This read-only property provides information on the level of Near Field
     * Communication support currently provided by the device.  See nfcFeature()
     * for a more detailed discussion.
     *
     * \see NfcFeatute, nfcFeature()
     */
    Q_PROPERTY(NfcFeature nfcFeature READ nfcFeature)

    /*!
     * \brief The current NFC mode the device is operating in.
     *
     * This read-only property provides information on the current active NFC
     * mode, i.e. whether the NFC radio hardware is powered on and if so, is it
     * in normal operation or card-only mode. See nfcMode() for a more
     * detailed discussion.
     *
     * \see NfcMode, nfcMode(), nfcModeChanged()
     */
    Q_PROPERTY(NfcMode nfcMode READ nfcMode NOTIFY nfcModeChanged)

    /*!
     * \brief The error reason for the latest error that has occurred.
     *
     * This property provides information on the latest NFC setting related
     * error that has taken place. See nfcError() for a more detailed discussion.
     *
     * \see NfcError, nfcError(), nfcErrorOccurred(), reset()
     */
    Q_PROPERTY(NfcError nfcError READ nfcError RESET reset NOTIFY nfcErrorOccurred)

public:

    /*!
     * \brief Enumeration listing all NFC feature support values.
     *
     * The values of this enumeration represent the levels of support for NFC
     * that a device can have.
     *
     * \see nfcFeature()
     */
    enum NfcFeature
    {
        NfcFeatureNotSupported,               /*!< The device does not support NFC. */
        NfcFeatureSupportedViaFirmwareUpdate, /*!< The device has NFC hardware, but requires a software update. */
        NfcFeatureSupported                   /*!< The device has full support for NFC features. */
    };

    /*!
     * \brief Enumeration listing all NFC mode setting values.
     *
     * The values of this enumeration represent the modes in which the NFC
     * hardware is operating. The modes are mutually exclusive, exactly one of
     * these modes is in effect at any given time.
     *
     * Not all modes are applicable on all devices and on all software versions,
     * e.g. the card emulation mode NfcSettings::NfcModeCardOnly may not be
     * supported by all NFC capable hardware. Similarly, the only mode reported
     * by devices that do not have NFC hardware is NfcSettings::NfcModeNotSupported.
     *
     * The mode value NfcSettings::NfcModeNotSupported is also used for situations
     * where NFC hardware is present, but the firmware version installed on the
     * device does not support it yet. In this case the function nfcFeature()
     * will return the value NfcSettings::NfcFeatureSupportedViaFirmwareUpdate
     * and NFC support may be enabled by simply updating the firmware.
     *
     * \see nfcFeature(), nfcMode(), nfcModeChanged()
     */
    enum NfcMode
    {
        NfcModeNotSupported, /*!< The device does not support NFC, hence no mode is applicable. */
        NfcModeUnknown,      /*!< The device supports NFC, but is unable to report the current mode. */
        NfcModeOn,           /*!< NFC is supported and the NFC radio is currently enabled. */
        NfcModeCardOnly,     /*!< NFC is supported and the NFC radio is currently in card emulation mode. */
        NfcModeOff           /*!< NFC is supported but the NFC radio is currently disabled. */
    };

    /*!
     * \brief Enumeration listing known NFC error states.
     *
     * The values of this enumeration represent the stages at which errors may
     * occur while trying to query the current NFC mode setting value.
     *
     * Not all errors are applicable on all devices. Devices not supporting
     * NFC will not report any of these errors, rather they will return
     * NfcModeNotSupported from nfcMode().
     *
     * \see nfcError(), nfcErrorOccurred()
     */
    enum NfcError
    {
        NfcErrorNone,                          /*!< No errors have occurred. */
        NfcErrorFeatureSupportQuery,           /*!< Trying to query whether the device has NFC hardware caused an error. */
        NfcErrorSoftwareVersionQuery,          /*!< Trying to obtain the device software version failed. */
        NfcErrorModeChangeNotificationRequest, /*!< Trying to subscribe to get NFC mode change notifications failed. */
        NfcErrorModeChangeNotification,        /*!< An error occurred while handling the NFC mode change notification. */
        NfcErrorModeRetrieval                  /*!< Trying to query the current NFC mode setting value caused an error. */
    };

    /*!
     * \brief C++ constructor.
     *
     * Constructs a new NfcSettings instance by determining the availability of
     * NFC hardware and retrieving the initial NFC mode value.
     *
     * Since no connections can be in place when the constructor is called, the
     * signals emitted by a new mode value, or by possible error scenarios cannot
     * be received by any listener. Instead the initial feature support, mode and
     * error values should be obtained by directly calling the corresponding
     * getter functions.
     *
     * The newly constructed NfcSettings instance is fully operational if
     * nfcFeature() returns NfcSettings::NfcFeatureSupported and nfcError()
     * returns NfcSettings::NfcErrorNone. All other return value combinations
     * indicate that NFC is either not supported, or an error took place while
     * trying to determine the availability of NFC hardware or the mode value.
     *
     * \param parent The object that is to be the parent of this NfcSettings
     * instance. If there is no parent object (i.e. 0 is passed in as the
     * parent), the newly created instance must be deleted by whoever allocated
     * it.
     * \see nfcFeature(), nfcMode(), nfcError()
     */
    explicit NfcSettings(QObject *parent = 0);

    /*!
     * \brief C++ destructor.
     *
     * Releases any resources allocated by this NfcSettings instance.
     */
    virtual ~NfcSettings();

    /*!
     * \brief Returns the current level of NFC feature support provided by the
     * device.
     *
     * This function returns the level of support current provided by the device
     * and its software. The value returned by this function cannot change at
     * runtime, as the only way to enable NFC support for a device that does not
     * currently have it, is via a firmware update.
     *
     * \return The current level of NFC feature support: NfcSettings::NfcFeatureNotSupported
     * if the device does not have NFC hardware, NfcSettings::NfcFeatureSupportedViaFirmwareUpdate
     * if NFC hardware is present but requires a software update to be usable, or
     * NfcSettings::NfcFeatureSupported is NFC features are fully supported.
     * \see NfcFeature
     */
    NfcFeature nfcFeature() const;

    /*!
     * \brief Returns the current value of the NFC mode setting.
     *
     * This function returns the NFC mode setting value that was most recently
     * retrieved without any errors. The "without errors" statement means that
     * if nfcError() returns anything other than NfcSettings::NfcErrorNone, the
     * return value of this function will be NfcSettings::NfcModeUnknown, as the
     * the latest NFC mode reading cannot be trusted to correctly reflect the
     * current active NFC mode.
     *
     * \return The current NFC mode setting value, or NfcSettings::NfcModeNotSupported
     * if the device does not have NFC hardware.
     * \see NfcMode, nfcModeChanged()
     */
    NfcMode nfcMode() const;

    /*!
     * \brief Returns the error reason for the latest error that has occurred.
     *
     * This function returns the reason for the latest NFC error that has taken
     * place. Error reason codes are not stored - if more than one error has
     * occurred since this function was last called, only the reason for the
     * latest error is returned. The nfcErrorOccurred() signal is, however,
     * sent for each error separately.
     *
     * If NfcSettings::NfcErrorFeatureSupportQuery is returned, this instance is
     * completely inoperative, nfcMode() will not return meaningful values and
     * no mode change signals will be fired. An NfcSettings object in this state
     * should either be discarded, or reinitialized by calling reset().
     *
     * If NfcSettings::NfcErrorModeChangeNotificationRequest is returned no
     * further mode change notifications will be delivered until reset() is
     * called to try and resubscribe to the change notification.
     *
     * If everything is working normally this function returns
     * NfcSettings::NfcErrorNone. Any other status means that the reading of the
     * NFC mode value is unreliable. In these cases nfcMode() will return
     * NfcSettings::NfcModeUnknown.
     *
     * Note that the device having no NFC hardware is not deemed an error. In
     * this case nfcError() returns NfcSettings::NfcErrorNone and nfcMode()
     * returns NfcSettings::NfcModeNotSupported.
     *
     * \return The latest NFC error reason.
     * \see NfcError, nfcErrorOccurred(), reset()
     */
    NfcError nfcError() const;

public slots:

    /*!
     * \brief Determines the availability of NFC hardware and retrieves the
     * initial NFC mode value.
     *
     * If the device supports NFC and no errors occur during the initialization,
     * the initial value of the mode setting is retrieved.
     *
     * It is valid to call reset() more than once. This will try to clear any
     * existing error condition and attempt to reinitialize the NfcSettings
     * instance to a pristine state. Calling reset() again can be used as a way
     * to recover from intermittent runtime errors that may occur for example
     * when there is a temporary low memory situation.
     *
     * \warning reset() must not be called from a slot connected to the
     * nfcErrorOccurred() signal. This is because a non-clearable error condition
     * will cause the slot to be called again, resulting in endless recursion.
     *
     * \see NfcSettings, nfcError(), nfcErrorOccurred()
     */
    void reset();

signals:

    /*!
     * \brief Signals a change in the NFC mode setting.
     *
     * This signal is sent whenever a change in the NFC mode setting is detected.
     *
     * \note The nfcModeChanged() signal will never send the enumerated value
     * NfcSettings::NfcModeNotSupported as the value of the \a nfcMode parameter.
     * As the level of NFC support cannot change at runtime, no nfcModeChanged()
     * signals are emitted if the initial mode value is NfcSettings::NfcModeNotSupported.
     * Regardless of this, it is a good practice to implement the corresponding
     * slot to handle the NfcSettings::NfcModeNotSupported value as well, in
     * case the slot is also used as a normal function in the application.
     *
     * \param nfcMode The current value of the NFC mode setting.
     * \see NfcMode, nfcMode()
     */
    void nfcModeChanged(NfcSettings::NfcMode nfcMode);

    /*!
     * \brief Signals an error in some the NFC feature or mode related operation.
     *
     * This signal is sent whenever an error occurs while trying to query support
     * for NFC hardware, retrieve the current value of the NFC mode setting, or
     * subscribing to get NFC mode setting change notifications.
     *
     * Error states are permanent in that they don't get cleared until reset()
     * is explicitly called. A call to reset() may return the NfcSettings
     * instance to normal operation, but there are error states from which it
     * is not possible to recover.
     *
     * \param nfcError The general reason for the error. The value of this
     * parameter indicates the stage at which the error took place. E.g. the
     * value NfcSettings::NfcErrorFeatureSupportQuery means that an error occured
     * while trying to query whether the device has NFC hardware,
     * NfcSettings::NfcErrorModeRetrieval means NFC is supported, but there was
     * a problem retrieving the current value of the mode setting.
     * \param error A platform specific error code giving a more detailed
     * description of the error that took place. For example
     * <code>KErrNoMemory</code> (-4) on Symbian devices if there is not enough
     * memory to perform the requested operation.
     * \see NfcError, nfcError(), reset()
     */
    void nfcErrorOccurred(NfcSettings::NfcError nfcError, int error);

private:

    Q_DISABLE_COPY(NfcSettings)

    /*! \brief Private implementation class. */
    friend class NfcSettingsPrivate;

    /*! \brief Pointer to the private class instance, owned. */
    NfcSettingsPrivate *d_ptr;
};

#endif // NFCSETTINGS_H
