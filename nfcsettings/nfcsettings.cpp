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

#include "nfcsettings.h"

#if defined(Q_OS_SYMBIAN)
#include "nfcsettings_symbian_p.h"
#else
#include "nfcsettings_p.h"
#endif

NfcSettings::NfcSettings(QObject *parent) :
    QObject(parent),
    d_ptr(0)
{
    d_ptr = new NfcSettingsPrivate(this);
}

NfcSettings::~NfcSettings()
{
    delete d_ptr;
}

NfcSettings::NfcFeature NfcSettings::nfcFeature() const
{
    return d_ptr->nfcFeature();
}

NfcSettings::NfcMode NfcSettings::nfcMode() const
{
    return d_ptr->nfcMode();
}

NfcSettings::NfcError NfcSettings::nfcError() const
{
    return d_ptr->nfcError();
}

void NfcSettings::reset()
{
    d_ptr->reset();
}
