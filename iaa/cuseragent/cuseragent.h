/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Declaration of the CUserAgent class.
*
*/


#ifndef  __CUSERAGENT_H
#define  __CUSERAGENT_H

// INCLUDES
#include <centralrepository.h>
#include <sysversioninfo.h>
#include <versioninfo.h>


// CLASS DECLARATION

/**
*  CUserAgent class.
*
*  @lib webutils.lib
*  @since 3.0
*/

enum TUserAgentStringKeys
    {
		ESymbianOSVersion,
		ES60Version,
		EHardwareType,
		EMIDP_CLDCVersion,
		EDefaultUserAgentString,
		EDefaultMMSUserAgentString,
		EDummy1,
		EDummy2,
		EURLUAProfile,
		EURL3GUAProfile
    };
		

class CUserAgent : public CBase
    {
    public:     // functions 

        /**
        * Destructor.
        */
        ~CUserAgent();
   
        /**
        * NewL
        */
        IMPORT_C static class CUserAgent* NewL();
        
        /**
        * Returns complete constructed user agent string.
        */
        IMPORT_C HBufC8* UserAgentL();
        
        /**
        * Returns fully contructed user agent string for MMS
        */
        IMPORT_C HBufC8* MMSUserAgentL();
        
        /**
        * Fetches the URL to user agent profile (2G and 3G)
        * @return void
        */
         IMPORT_C TInt URLToUserAgentProfileL(TDes8 &aUAProfURL, TDes8 &aUAProfURL3G);

        /**
        * Returns the Browser version 
        */
		IMPORT_C HBufC* BrowserVersionL();

        /**
        * Returns the Browser name and version
        */
        IMPORT_C HBufC* BrowserNameAndVersionL();
        
    private:    // functions
     
        /**
        * Symbian default constructor.
        */
		void ConstructL();

        /**
        * Default C++ constructor
        */
		CUserAgent();
		
	   /**
        * Reads useragent strings from shareddata / central repository
        */
		void ReadUserAgentStringL();
		void ReadMMSUserAgentStringL();
		void ReadURLUserAgentProfileL();
		
		bool IsUAStringEmpty(const TPtr &aUAString);
		TInt GetCenRepFragment(TUserAgentStringKeys aFragmentType, TDes& fragmentValue);
    void GetFragmentsL();
    void GetMMSFragmentsL();
    void ReadAndCreateUAStringL(TPtr &aUAStringPtr);
        
        
#ifdef BRDO_BROWSER_50_FF
        void GetFragmentsFromOSExtL();
        TInt GetOSExtFragment(TUserAgentStringKeys aFragmentType, TDes& fragmentValue, RFs& fs);
        /**
        * Gets the Version Values from osext layer / central repository
        */
        TInt GetOSVersion(TDes& aValue, RFs& aFs);
        TInt GetS60Version(TDes& aValue, RFs& aFs);
        TInt GetVersion(const SysVersionInfo::TVersionInfoType aType, TDes& aValue);
        TInt GetHardwareVersion(TDes& aHardwareType, RFs& aFs);
#endif //#ifdef BRDO_BROWSER_50_FF      

        void GetBrowserVersionL();
        void GetBrowserNameAndVersionL();
        
    private:        //data

        CRepository* iRepository;

        TBool   iUAStrQueried;
        TBool   iMMSUAStrQueried;
        TBool   iURLUAProfQueried;

        HBufC*  iBrowserCenrepStr;
        HBufC*  iMMSCenrepStr;
        HBufC*  iUAProfURL;
        HBufC*  iUAProfURL3G;
        
        TBuf<64> iSymbianVersion;
        TBuf<64> iS60Version;
        TBuf<64> iMIDPCLDCVersion;
        TBuf<64> iHardwareType;
        TBuf<64> iMMSS60Version;
        TBuf<64> iMMSMIDPVersion;
        TBuf<64> iMMSCLDCVersion;
        TBuf<64> iMMSHardwareType;

        HBufC*  iBrowserVersionStr;
        HBufC*  iBrowserNameAndVersionStr;

   };


#endif // __CUSERAGENT_H

// End of File
