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
 * \brief Declaration of the non-Symbian NfcSettingsPrivate class.
 *
 * This file contains the declaration of the NfcSettingsPrivate class for
 * platforms other than Symbian.
 */

#ifndef NFCSETTINGS_P_H
#define NFCSETTINGS_P_H

#include <QtCore/QObject>

#include "nfcsettings.h"

/*!
 * \brief The default private implementation class for the NFC mode setting.
 *
 * This stub implementation class is used on platforms for which no more
 * specialized implementation is available. The accessor functions nfcMode() and
 * nfcError() of this class always report NFC as not supported.
 *
 * \note This class is part of the private implementation design pattern, and as
 * such it is an internal implementation detail of the NfcSettings class. The
 * class NfcSettingsPrivate is documented here for completeness' sake, but must
 * not be used directly - all code using the NFC settings API should use the
 * public class NfcSettings instead.
 *
 * \see NfcSettings
 */
class NfcSettingsPrivate : public QObject
{
    Q_OBJECT

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
     * \see reset()
     */
    explicit NfcSettingsPrivate(NfcSettings *q);

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
     * Called by the public class when NfcSettings::nfcFeature() is called. This
     * implementation is a stub and always reports NFC as not supported.
     *
     * \return The current level of NFC feature support of the device, always
     * returns NfcSettings::NfcFeatureNotSupported.
     * \see NfcSettings::NfcFeature, NfcSettings::nfcMode()
     */
    NfcSettings::NfcFeature nfcFeature() const;

    /*!
     * \brief Returns the current value of the NFC mode setting.
     *
     * Called by the public class when NfcSettings::nfcMode() is called. This
     * implementation is a stub and always reports NFC as not supported.
     *
     * \return The current NFC mode setting value, always returns
     * NfcSettings::NfcModeNotSupported.
     * \see NfcSettings::NfcMode, NfcSettings::nfcMode(),
     * NfcSettings::nfcModeChanged()
     */
    NfcSettings::NfcMode nfcMode() const;

    /*!
     * \brief Returns the error reason for the latest error that has occurred.
     *
     * Called by the public class when NfcSettings::nfcError() is called. This
     * implementation is a stub and cannot generate any errors.
     *
     * \return Always returns NfcSettings::NfcErrorNone.
     * \see NfcSettings::NfcError, NfcSettings::nfcError(),
     * NfcSettings::nfcErrorOccurred()
     */
    NfcSettings::NfcError nfcError() const;

    /*!
     * \brief Resets this NfcSettings instance back to its initial state by
     * attempting to redetermine the availability of NFC hardware and retrieving
     * the NFC mode value.
     *
     * Called by the public class when NfcSettings::reset() is called. This
     * is a no-op stub implementation.
     */
    void reset();

private:

    Q_DISABLE_COPY(NfcSettingsPrivate)

     /*! \brief Pointer to the public class instance, not owned. */
    NfcSettings *q_ptr;
};

#endif // NFCSETTINGS_P_H
