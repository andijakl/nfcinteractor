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

#include "nfcsettings_p.h"

NfcSettingsPrivate::NfcSettingsPrivate(NfcSettings *q) :
    QObject(0),
    q_ptr(q)
{
    // Empty implementation.
}

NfcSettingsPrivate::~NfcSettingsPrivate()
{
    // Empty implementation.
}

NfcSettings::NfcFeature NfcSettingsPrivate::nfcFeature() const
{
    return NfcSettings::NfcFeatureNotSupported;
}

NfcSettings::NfcMode NfcSettingsPrivate::nfcMode() const
{
    return NfcSettings::NfcModeNotSupported;
}

NfcSettings::NfcError NfcSettingsPrivate::nfcError() const
{
    return NfcSettings::NfcErrorNone;
}

void NfcSettingsPrivate::reset()
{
    // Empty implementation.
}
