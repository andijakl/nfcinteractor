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


#include <centralrepository.h>
#include <featdiscovery.h>
#include <hal.h>

#include "nfcsettings_symbian_p.h"

// The following Central Repository UID, setting key and setting values are not
// available in the Nokia Symbian^3 SDK v1.0. Hence there is no header we could
// include, and we have to define the relevant values here.
const TUid KNfcCentralRepositoryUid = { 0x20021390 };
const TUint32 KNfcModeSetting = 0x01;
const TInt KNfcModeUnknown = -9999;
const TInt KNfcModeOn = 0;
const TInt KNfcModeCardOnly = 2;
const TInt KNfcModeOff = 1;
#define KFeatureIdNfc 117

// Device ID constant used to determine whether the current device is a
// Nokia C7-00 (RM-675), the only model for which NFC support is available
// as a firmware update.
const TInt KModelRm675 = 0x2002bf92;

NfcSettingsPrivate::NfcSettingsPrivate( NfcSettings *q ) :
    CActive( CActive::EPriorityLow ),
    q_ptr( q ),
    iNfcFeature( NfcSettings::NfcFeatureNotSupported ),
    iNfcMode( NfcSettings::NfcModeNotSupported ),
    iNfcError( NfcSettings::NfcErrorNone )
    {
    CActiveScheduler::Add( this );

    InitializeNfcFeatureSupport();
    }

NfcSettingsPrivate::~NfcSettingsPrivate()
    {
    Cancel();

    delete iRepository;
    }

NfcSettings::NfcFeature NfcSettingsPrivate::nfcFeature() const
    {
    return iNfcFeature;
    }

NfcSettings::NfcMode NfcSettingsPrivate::nfcMode() const
    {
    return iNfcMode;
    }

NfcSettings::NfcError NfcSettingsPrivate::nfcError() const
    {
    return iNfcError;
    }

void NfcSettingsPrivate::reset()
    {
    iNfcFeature = NfcSettings::NfcFeatureNotSupported;
    iNfcMode = NfcSettings::NfcModeNotSupported;
    iNfcError = NfcSettings::NfcErrorNone;

    InitializeNfcFeatureSupport();
    }

void NfcSettingsPrivate::DoCancel()
    {
    if ( iRepository )
        {
        iRepository->NotifyCancelAll();
        }
    }

void NfcSettingsPrivate::RunL()
    {
    const TInt status( iStatus.Int() );

    User::LeaveIfError( status );

    // A successfully completed CRepository::NotifyRequest() will set the key
    // of the modified repository value as the TRequestStatus value. As the
    // repository key is a TUint32, it is explicitly converted to a TInt
    // to compare it to the TInt status value. This supresses a GCCE compilation
    // warning. The value of KNfcModeSetting is 0x01, so it can be safely
    // converted to a TInt without any loss of information.
    if ( status == TInt(KNfcModeSetting) )
        {
        NotifyRequest();
        }
    }

TInt NfcSettingsPrivate::RunError( TInt aError )
    {
    ReportError( NfcSettings::NfcErrorModeChangeNotification, aError );

    return KErrNone;
    }

void NfcSettingsPrivate::InitializeNfcFeatureSupport()
    {
    TBool nfcAvailable = EFalse;
    TRAPD(error, nfcAvailable = CFeatureDiscovery::IsFeatureSupportedL(KFeatureIdNfc));

    if ( nfcAvailable )
        {
        iNfcFeature = NfcSettings::NfcFeatureSupported;

        if ( !iRepository )
            {
            TRAP( error, iRepository = CRepository::NewL(KNfcCentralRepositoryUid) );
            }

        if ( error == KErrNone )
            {
            Cancel();
            NotifyRequest();
            }
        else
            {
            ReportError( NfcSettings::NfcErrorModeRetrieval, error );
            }
        }
    else if ( ProductSupportsNfcViaFirmwareUpdate() )
        {
        iNfcFeature = NfcSettings::NfcFeatureSupportedViaFirmwareUpdate;
        }
    }

void NfcSettingsPrivate::NotifyRequest()
    {
    if ( !IsActive() && iRepository )
        {
        const TInt error( iRepository->NotifyRequest(KNfcModeSetting, iStatus) );

        if ( (error == KErrNone) && (iStatus == KRequestPending) )
            {
            SetActive();
            RetrieveSetting();
            }
        else if ( error != KErrNone )
            {
            ReportError( NfcSettings::NfcErrorModeChangeNotificationRequest, error );
            }
        }
    }

void NfcSettingsPrivate::RetrieveSetting()
    {
    if ( iRepository )
        {
        TInt nfcMode( KNfcModeUnknown );
        const TInt error( iRepository->Get(KNfcModeSetting, nfcMode) );

        if ( error == KErrNone )
            {
            switch ( nfcMode )
                {
                case KNfcModeUnknown:
                    iNfcMode = NfcSettings::NfcModeUnknown;
                    break;
                case KNfcModeOn:
                    iNfcMode = NfcSettings::NfcModeOn;
                    break;
                case KNfcModeCardOnly:
                    iNfcMode = NfcSettings::NfcModeCardOnly;
                    break;
                case KNfcModeOff:
                    iNfcMode = NfcSettings::NfcModeOff;
                    break;
                default:
                    iNfcMode = NfcSettings::NfcModeUnknown;
                    break;
                }

            if ( q_ptr )
                {
                emit q_ptr->nfcModeChanged( iNfcMode );
                }
            }
        else
            {
            ReportError( NfcSettings::NfcErrorModeRetrieval, error );
            }
        }
    }

void NfcSettingsPrivate::ReportError( NfcSettings::NfcError aNfcError, TInt aError )
    {
    iNfcError = aNfcError;

    // If NFC is supported, but the mode setting value cannot be retrieved,
    // reset the mode to NfcSettings::NfcModeUnknown. This prevents a situation
    // where nfcFeature() reports NFC as supported but the return value
    // of nfcMode() reports it as unsupported.
    if ( iNfcFeature == NfcSettings::NfcFeatureSupported )
        {
        iNfcMode = NfcSettings::NfcModeUnknown;
        }

    if ( q_ptr )
        {
        emit q_ptr->nfcErrorOccurred( iNfcError, aError );
        }
    }

TBool NfcSettingsPrivate::ProductSupportsNfcViaFirmwareUpdate()
    {
    TBool supportsNfcViaFirmwareUpdate( EFalse );
    TInt model( 0 );
    const TInt error( HAL::Get(HALData::EModel, model) );

    if ( error == KErrNone )
        {
        if ( model == KModelRm675 )
            {
            supportsNfcViaFirmwareUpdate = ETrue;
            }
        }
    else
        {
        ReportError( NfcSettings::NfcErrorSoftwareVersionQuery, error );
        }

    return supportsNfcViaFirmwareUpdate;
    }
