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
 * \brief Declaration of the Symbian NfcSettingsPrivate class.
 *
 * This file contains the declaration of the NfcSettingsPrivate class for
 * Symbian platforms.
 */

#ifndef NFCSETTINGS_SYMBIAN_P_H
#define NFCSETTINGS_SYMBIAN_P_H

#include <e32base.h>
#include <qglobal.h>

#include "nfcsettings.h"

class CRepository;

/*!
 * \brief The Symbian private implementation class for the NFC mode setting.
 *
 * This implementation class is used on Symbian platforms. The accessor methods
 * nfcMode() and nfcError() of this class are fully implemented and report
 * whether NFC is supported by the device and if so, what the current value of
 * the NFC mode setting is.
 *
 * On Symbian platforms the NFC mode setting is stored in the Central
 * Repository. NfcSettingsPrivate monitors changes to the NFC mode setting by
 * requesting to be notified whenever the value of the relevant Central
 * Repository key is modified. Changes to the setting are communicated to
 * clients of the public API class corresponding to this private implementation
 * by emitting its NfcSettings::nfcModeChanged() signal.
 *
 * \note This class is part of the private implementation design pattern, and as
 * such it is an internal implementation detail of the NfcSettings class. The
 * class NfcSettingsPrivate is documented here for completeness' sake, but must
 * not be used directly - all code using the NFC settings API should use the
 * public class NfcSettings instead.
 *
 * \see NfcSettings
 */
class NfcSettingsPrivate : public CActive
{
public:

    /*!
     * \brief C++ constructor.
     *
     * Constructs a new NfcSettingsPrivate instance. This class is
     * automatically instantiated by the public class NfcSettings as a part of
     * its own construction. No other instances of this class should be created.
     *
     * \param q Pointer to the public class instance that owns this private
     * instance. Ownership of the instance is not transferred.
     */
    explicit NfcSettingsPrivate( NfcSettings *q );

    /*!
     * \brief C++ destructor.
     *
     * Releases any resources allocated by this NfcSettingsPrivate instance.
     */
    virtual ~NfcSettingsPrivate();

    /*!
     * \brief Returns the current level of NFC feature support provided by the
     * device.
     *
     * Called by the public class when NfcSettings::nfcFeature() is called.
     *
     * \return The current level of NFC feature support of the device.
     * \see NfcSettings::NfcFeature, NfcSettings::nfcMode()
     */
    NfcSettings::NfcFeature nfcFeature() const;

    /*!
     * \brief Returns the current value of the NFC mode setting.
     *
     * Called by the public class when NfcSettings::nfcMode() is called.
     *
     * \return The current NFC mode setting value. If any errors have occurred,
     * this function always return NfcSettings::NfcModeUnknown.
     * \see NfcSettings::NfcMode, NfcSettings::nfcMode(),
     * NfcSettings::nfcModeChanged()
     */
    NfcSettings::NfcMode nfcMode() const;

   /*!
    * \brief Returns the error reason for the latest error that has occurred.
    *
    * Called by the public class when NfcSettings::nfcError() is called.
    *
    * \return The latest NFC error reason.
    * \see NfcSettings::NfcError, NfcSettings::nfcError(),
    * NfcSettings::nfcErrorOccurred()
    */
    NfcSettings::NfcError nfcError() const;

    /*!
     * \brief Resets this NfcSettings instance back to its initial state by
     * attempting to redetermine the availability of NFC hardware and retrieving
     * the NFC mode value.
     *
     * This function re-initializes this NfcSettings instance by verifying the
     * presence of NFC hardware and the Central Repository containing the mode
     * setting. If the Symbian Feature Manager reports that the device supports
     * NFC, the Central Repository containing the mode setting is opened and a
     * subscription is made to get notifications of changes to the mode setting.
     * The initial value of the mode setting is also retrieved.
     *
     * If the initialization cannot be successfully completed, the error is
     * reported to clients of the public class by emitting the
     * NfcSettings::nfcErrorOccurred() signal.
     */
    void reset();

protected:

    /*!
     * \brief From the base class <code>CActive</code>, cancels any pending
     * Central Repository notification requests.
     *
     * This function is called when <code>CActive::Cancel()</code> is called if
     * this active object is currently active. DoCancel() must not be called
     * manually.
     */
    virtual void DoCancel();

    /*!
     * \brief From the base class <code>CActive</code>, handles completed
     * Central Repository notification requests.
     *
     * This function gets called whenever a Central Repository notification
     * request is completed, i.e. when the value of the NFC mode setting changes.
     * The new value of the setting is retrieved and the public class is
     * notified by emitting its NfcSettings::nfcModeChanged() signal.
     *
     * Prior to notifying the public class, a new notification request is made,
     * and NfcSettingsPrivate continues to wait for further mode setting
     * changes.
     */
    virtual void RunL();

    /*!
     * \brief From the base class <code>CActive</code>, handles Symbian leaves
     * that occur in RunL().
     *
     * This function is called if some code in RunL() causes a Symbian leave.
     * This happens if the Central Repository notification request is completed,
     * but the request status object indicates an error.
     *
     * All leaves are handled by notifying the clients of the public class by
     * emitting its NfcSettings::nfcErrorOccurred() signal with the NFC error
     * NfcSettings::NfcErrorModeChangeNotification and the platform specific
     * error code \a aError.
     *
     * \param aError The leave code generated by a leave that occurred in RunL().
     * \return Always return <code>KErrNone</code> to indicate that the leave
     * was successfully handled.
     */
    virtual TInt RunError( TInt aError );

private:

    /*!
     * \brief Verifies the presence of NFC hardware and the Central Repository
     * containing the mode setting.
     *
     * If the Symbian Feature Manager reports that the device supports NFC,
     * the Central Repository containing the mode setting is opened and a
     * subscription is made to get notifications of changes to the mode setting.
     *
     * If the initialization cannot be successfully completed, either an
     * NfcSettings::NfcErrorFeatureSupportQuery or
     * NfcSettings::NfcErrorSoftwareVersionQuery error is reported to the public
     * class.
     */
    void InitializeNfcFeatureSupport();

    /*!
     * \brief Requests the Central Repository to notify this class of changes
     * made to the NFC mode setting.
     *
     * If the notification request cannot be successfully completed, the error
     * NfcSettings::NfcErrorModeChangeNotificationRequest is reported to the
     * public class.
     */
    void NotifyRequest();

    /*!
     * \brief Reads the current value of the NFC mode setting from the Central
     * Repository.
     *
     * The Symbian specific NFC mode setting value is mapped to a value of the
     * NfcSettings::NfcMode enumeration and stored. The stored value can
     * subsequently be obtained by calling nfcMode().
     *
     * If the value cannot be successfully read, the error
     * NfcSettings::NfcErrorModeRetrieval is reported to the public class.
     */
    void RetrieveSetting();

    /*!
     * \brief Reports an error in the internal operation of this class to its
     * public counterpart.
     *
     * This is a utility function that stores the NFC error code and notifies
     * the public class of the error. The nfcError() function always returns
     * the error code of the latest error reported via a call to ReportError().
     *
     * \param aNfcError The general reason for the error being reported. The
     * value of this parameter indicates the stage at which the error took
     * place.
     * \param aError The Symbian error code relating to the error being
     * reported.
     */
    void ReportError( NfcSettings::NfcError aNfcError, TInt aError );

    /*!
     * \brief Tests whether the device can be updated to support NFC functionality.
     *
     * This function tests if the device has NFC hardware, and if so, whether
     * the installed firmware version is recent enough to provide support for
     * the hardware.
     *
     * \return ETrue if the device has NFC hardware, but requires a firmware
     * update to use it. EFalse otherwise (i.e. the there is no NFC hardware
     * or the firmware already supports NFC).
     */
    TBool ProductSupportsNfcViaFirmwareUpdate();

private:

    Q_DISABLE_COPY( NfcSettingsPrivate )

    /*! \brief Pointer to the public class instance, not owned. */
    NfcSettings *q_ptr;

    /*! \brief The level of NFC feature support provided by the device. */
    NfcSettings::NfcFeature iNfcFeature;

    /*! \brief The current effective NFC mode setting value. */
    NfcSettings::NfcMode iNfcMode;

    /*! \brief The most recent NFC mode related error reason. */
    NfcSettings::NfcError iNfcError;

    /*! \brief The Central Repository instance used to read the NFC mode setting value, owned. */
    CRepository* iRepository;
};

#endif // NFCSETTINGS_SYMBIAN_P_H
