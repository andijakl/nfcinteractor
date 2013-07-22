Nfc Interactor
http://www.nfcinteractor.com
-------------------------------------------------------------------------------

View low level information about NFC tags and their contents, and write your own tags with a dynamic NDEF message editor UI.

Touch an NFC tag to analyze its format and its contents. Nfc Interactor parses and shows many different NDEF messages and extends the offering of Qt through reusable convenience classes for handling additional record types: Smart Poster, Mime/Image, vCard, SMS, Social Networks, Geo-Tags, Smart URI, Windows LaunchApp, Android Application Record and custom records.

Compose your own NDEF messages by combining one or more NDEF records using the dynamic editor interface. Various templates assist you, allowing you to concentrate on the content instead of the format. While editing, Nfc Interactor will always show you the current size. Once your message is finalized, write it to as many tags as you like.

Send and receive NDEF messages through a peer-to-peer connection between devices, either using standardized SNEP or a raw LLCP connection with freely defined settings.

Nfc Interactor is written in Qt and works on Nokia Anna / Belle as well as on MeeGo Harmattan (Nokia N9). As an open source example project, it also showcases the use of Qt Quick Components, as well as the integration of C++ with QML.


Note: this version of Nfc Interactor replaces the previous Java ME-based NFCinteractor. The new Qt-version is more powerful and introduces a wealth of new features, plus improved reading support. The Java ME-based project is still useful as a code example and has therefore been renamed to Nfc Creator and is available here: https://projects.developer.nokia.com/nfccreator . This Qt-based Nfc Interactor also succeeds the "Nfc Info" project, which was only capable of reading tags.


FEATURES
-------------------------------------------------------------------------------
Tag reading:
- Tag type (NFC Forum Tag Type 1, 2, 3, 4 and Mifare)
- Tag UID
- Tag version
- Tag memory size, static vs. dynamic memory (for tag types 1 + 2**)
- Lock bits and Capability Container access level (for static memory tag types 1 + 2**)
- NDEF message size
- List of all individual NDEF records
- Parses Smart Poster, URI, text, image and business card (vCard) records
- App is automatically launched for custom record with external RTD: "nokia.com:nfcinteractor"
- Stores parsed URI or text to the clipboard
- Automatically saves all collected NDEF messages to the phone's memory card for later reuse (if activated)

Tag writing:
- Dynamic UI to create a message consisting of multiple NDEF records, and to add information to each record
- Clone tag contents (tap on blue NDEF message contents after reading a tag -> clone)
- Save own composed tags to the file system on the phone
- Option to write empty message before real message, to better format factory-empty tags
- Open previously collected or saved NDEF message in compose mode for editing and writing again (tap on blue NDEF message contents after reading a tag -> edit, or load from compose view)
- Help for all supported message types
- NDEF message size calculation
- Specialized editors for each message type:
  - Social Network record: enter the username and choose the network to create a link to follow you. Creates a URI or Smart Poster, depending on if you add an optional title.
  - Business card record: select the data to include from a wealth of available fields, and create your own business card based on the universal vCard standard. Integrates with the Qt Mobility Contacts APIs. Note: usually large tag size requirements.
  - Geo record: enter coordinates and select the Geo Tag type. Supports Geo URI (RFC 5870), Nokia Maps links and a generic redirection script that enables Geo tags on Symbian and MeeGo, hosted at nfcinteractor.com*. Integrates with the Qt Mobility Location APIs for the coordinates. Creates a URI or Smart Poster, depending on if you add an optional title.
  - Image record: write an image from the file system of your mobile phone to the tag.
  - LaunchApp record: used on the Windows platform to directly launch an app through its ID.
  - Android Application Record (AAR): launch an app on Android through its package name.
  - App Store record: generate a download link to an app store. Creates a direct store link if one ID is specified, or a generic link using a platform detection script at nfcinteractor.com* if more app ids are specified. For Nokia, either specify a generic Nokia Store ID if it's the same for all platforms, or instead specify an own ID for Symbian, MeeGo Harmattan and if needed Series 40. Creates a URI or Smart Poster, depending on if you add an optional title.
  - SMS record: enter number and body to create a working SMS link, useful for example to request information services or for payment via SMS. Creates a URI or Smart Poster, depending on if you add an optional title.
  - Smart Posters: has to contain at least a URI, optionally an unlimited amount of titles in different languages, a recommended action and information about the linked content (size and MIME type).
  - URI records: automatically shortens URIs according to the specification to save tag space (e.g., won't save "http://www.", but encode that in a single byte).
  - Text records: set text and language.
  - Custom records: Choose the type name format and optionally add a type name and payload. Also useful for application autostart tags.

Peer to peer:
- SNEP support (Simple NDEF Exchange Protocol)
- Send and receive NDEF messages and raw data
- Connection-oriented and connection-less mode supported
- Define URI for connection-oriented mode or port for connection-less mode
- Connection-oriented: connect server and/or client socket, choose which socket to use for sending messages
  
Easily reusable code for developers including:
- NDEF record classes for: Smart Poster, Mime/vCard (Business Cards), Mime/Image, Geo coordinates, App Store links, Social Networks (Twitter, LinkedIn, Facebook, Xing, vKontakte, Foursquare), SMS, Smart URI (depending on content requirements uses Smart Poster or URI to create the smallest possible message)
- Tag-specifc access to read the tag size and lock status (for Tag Types 1 + 2**)
  
Ideas for future updates:
- Write a contact from the address book
- Write vCalendar records
- Skype call / add contact tag: http://www.skype.com/intl/en/tell-a-friend/wizard/
- Bluetooth pairing records
- Lock tags (type 1, 2**)
- Check composed record contents for errors according to specifications
- Use current location for geo tags (asynchronously updated UI)
- Birthday support for contacts (date picker UI)
- Geocoding (enter address, write coordinates to the tag - interactive UI)
- Splash screen for Symbian
- Turn NFC on from the app on Symbian: http://www.developer.nokia.com/Community/Wiki/How_to_put_NFC_ON_and_OFF_with_Qt
- Support for writing AAR records (Android Application Record): http://stackoverflow.com/questions/9016944/android-application-record-compatibility-with-pre-ics

* Disclaimer: The services hosted on nfcinteractor.com (e.g., the generic geo-tag redirection script) have no service or uptime guarantee. They are intended for testing purposes only and could be removed at any point. You should host the required server-side scripts on your own webserver for real-world deployment.
** MeeGo Harmattan currently does not allow tag type specific access through Qt APIs; therefore, it's not possible to access functionality not offered by Qt classes, such as getting tag specific information or locking tags.

SECURITY
--------------------------------------------------------------------------------

Symbian: The application can be self-signed.

Harmattan: No special aegis manifest is required.


KNOWN ISSUES
-------------------------------------------------------------------------------

General: the Tag Image Cache stores all images in memory. Images found in tags
are usually very small, making this no issue at all. For special usage 
scenarios, this class might have to be extended to cache the images
on the disc instead.

General: two records in a message, first has a payload, second no payload 
(length = 0, short record). Qt libraries keep the payload + size of the first 
record, instead of properly detecting an empty payload for the second record. 
This issue gets visible when reading a tag that is formatted as described
(which is a very rare case).
See: https://bugreports.qt-project.org/browse/QTMOBILITY-2077

Harmattan: In contrast to Symbian, Harmattan PR 1.0 does not provide
simultaneous tag read & write access. This has been fixed in PR 1.1, 
which is the firmware version required by this example.
For PR 1.0, the example would need to switch between those two modes.
See the earlier versions of Nfc Info or Nfc Corkboards for example code
if needed. This example has not been tested on PR 1.0.

Harmattan: if removing the tag while the app is interacting with it (reading
or writing), the Qt Mobility APIs might not emit the 'target lost' signal
on PR 1.1. This causes the app to keep the "writing..." animation active,
requiring the user to touch the tag again up to 2x. This behavior has
been fixed in PR 1.2.

Harmattan: No full SNEP support on the N9 - would require higher privileges 
to register for urn:nfc:sn:snep. Instead, using LibNDEFPush: 
http://harmattan-dev.nokia.com/downloads/QtSDK_additions/ for SNEP on the N9.
Only works on PR 1.2+, only for sending SNEP messages and not for receiving.

Symbian: Limited SNEP support: only possible to use service name 
urn:nfc:sn:snep, as using a connection-oriented port of 4 is not supported 
by the Qt APIs.

Symbian: auto-scrolling of settings page when opening virtual keyboard on 
Symbian not behaving correctly.


BUILD & INSTALLATION INSTRUCTIONS
-------------------------------------------------------------------------------

MeeGo Harmattan
~~~~~~~~~~~~~~~
Make sure you have the latest firmware version on your Nokia N9. The 
Nokia N950 doesn't support NFC.
To compile the source code, use the Harmattan target of the Qt SDK 1.2.

Install the LibNDEF Push for SNEP support:
- Download the package and the dev-package from http://harmattan-dev.nokia.com/downloads/QtSDK_additions/
- Open the MADDE terminal from the start menu
- Check installed MeeGo targets: 
  > mad list
- Set 1.2 target as default:
  > mad set harmattan_10.2011.34-1_rt1.2
- Go to directory where you saved the downloaded packages and install:
  > mad-admin xdpkg -i libndefpush-dev_0.23.2+0m6_armel.deb
  > mad-admin xdpkg -i libndefpush_0.23.2+0m6_armel.deb


Symbian
~~~~~~~
Compatible phones:
C7-00/Oro/Astound, 801T with Symbian Anna / Belle
Nokia 603, 700, 701, 808 PureView

Device Preparation
C7-00 with Symbian Anna: install using the Smart Installer.
For manual device preparation, additionally install Qt 4.7.4, Qt Mobility 1.2 and Qt Quick Components from the Qt SDK:
C:\QtSDK\Symbian\sis\Symbian_Anna\Qt\4.7.4\Qt-4.7.403-for-Anna.sis
C:\QtSDK\Symbian\sis\Symbian_Anna\Qt\4.7.4\QtWebKit-4.8.1-for-Anna.sis
C:\QtSDK\Symbian\sis\Symbian_Anna\QtMobility\1.2.1\QtMobility-1.2.1-for-Anna.sis
C:\QtSDK\Symbian\sis\Symbian_Anna\QtQuickComponents\1.1\QtQuickComponents-1.1-for-Anna-Belle.sis

Nokia Belle: install using the Smart Installer.
For manual device preparation, you only need to install Qt Quick Components, as Belle already includes Qt 4.7.4 and Qt Mobility 1.2:
C:\QtSDK\Symbian\sis\Symbian_Belle\QtQuickComponents\1.1\QtQuickComponents-1.1-for-Anna-Belle.sis 



Build & installation instructions using Qt SDK
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Open NfcInteractor.pro
   File > Open File or Project, select nfcinteractor.pro.

2. Symbian: Select the 'Qt 4.7.4 for Symbian Anna" target
   (compatible to phones with Symbian Belle).
   MeeGo Harmattan: Select the MeeGo 1.2 Harmattan (Qt SDK) release target.

3. Press the Run button to build the project and to install it on the device.

Note: if switching between Symbian and MeeGo Harmattan builds, make sure to 
clean the project inbetween. Otherwise, specific differences in the meta-objects
might not get rebuilt.


COMPATIBILITY
-------------------------------------------------------------------------------

- Qt SDK 1.2.x
- QtMobility 1.2
- Qt 4.7.4+
- Qt Quick Components 1.1

Tested on: 
- Nokia C7-00 with Symbian Anna and Qt 4.7.4 / QtM 1.2
- Nokia C7-00 and 701 with Nokia Belle
- Nokia 808 PureView with Nokia Belle FP1 / FP2
- Nokia N9 PR 1.0 / 1.1 / 1.2 / 1.3
  (no SNEP support on PR 1.0 / 1.1)


CHANGE HISTORY
--------------------------------------------------------------------------------

4.2 New Windows LaunchApp record to launch apps on the Windows 8 platform.
    New Android Application Record to launch apps on Android.
    Added Skype call link to Social Network class (using custom URI).
4.1 Edit collected records with external RTD.
    Improved handling of raw payload in custom tags.
4.0 Peer-to-peer support added, with SNEP support.
    Low level connection-oriented and connection-less LLCP sockets for sending
	raw NDEF messages.	
3.0 Cloning tags supported (long-press on read tag contents -> Clone).
    Editing tags supported (long-press on read tag contents -> Edit).
	Stores all collected tags to the data directory for later reuse.
	Extracts and saves images from tags (edit an existing tag, filename shown
	in the compose view).
	Settings screen to enable/disable storing all collected tags and to choose
	the data directory.
2.1 Added support for writing Foursquare tags.
    Print raw payload for external tag types.
	Updated In-App-Advertising library version.
2.0 Renamed from previous "Nfc Info" name to "Nfc Interactor" due to the bigger
	scope of the new version that also supports writing.
	Tag write support with dynamic tag editor UI.
	Switched to platform specific Qt Quick Components.
	Other fixes and code improvements.
1.0 First version


RELATED DOCUMENTATION
-------------------------------------------------------------------------------

Project page
http://www.nfcinteractor.com
https://projects.developer.nokia.com/nfcinteractor

Related example projects
https://projects.developer.nokia.com/nfccorkboards
https://projects.developer.nokia.com/nfcchat
https://projects.developer.nokia.com/nfccreator

NFC 
http://www.developer.nokia.com/NFC

Qt
http://www.developer.nokia.com/Qt

