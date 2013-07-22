<?php
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
/*
   Nfc Geo Tags Redirection Script
   v1.2.0

   Summary
   ---------------------------------------------------------------------------
   Generate platform-independent NFC Geo tags.
   
   
   Use case
   ---------------------------------------------------------------------------
   Direct the user to a certain point on a map. The NFC tag
   contains location information (point of interest, POI).
   The user touches the tag with his NFC phone. The phone opens the maps
   application at the coordinates of the POI. This allows the user to
   navigate to the point or to get more information.
   
   
   Description
   ---------------------------------------------------------------------------
   Some phones support the "geo:" URI scheme and connect that to a maps
   app on the phone (e.g., the Nokia N9 or Android).
   The default maps client on Windows 8 uses a bingmaps: URI scheme.
   Other phones do not support URI scheme for opening maps, and should be 
   redirected to a web-based maps client instead (e.g., Symbian).
   On Symbian, this method also triggers the Nokia Maps application.
   
   To maximize compatibility of a tag to phones, you need to take the phone
   OS into account and use the supported URL scheme. However, it is still 
   desirable to have only a single NFC tag, regardless of the phone OS.
   
   The solution is to write a URL to this script to the tag. The script
   detects the phone OS through its user agent, and will redirect it to
   the appropriate URL. This allows having a single tag, which is compatible
   to multiple mobile operating systems.
   
   Depending on the phone, the redirection method also differs. Redirecting
   a MeeGo or Android phone to a geo: URI should be done by sending a HTTP 
   header with the new location. Other phone browsers don't handle the 
   geo URIs for this kind of redirect, and need to be redirected using
   JavaScript (or a manual link in case JavaScript is deactivated).
   
   
   Usage
   ---------------------------------------------------------------------------
   Upload m.php to PHP 4.x+ compatible web server.
   
   Call with location information as parameter:
   
       m.php?l=[latitude],[longitude]
   
   Example:
       http://www.nfcinteractor.com/m.php?l=60.17,24.829
   
   Alternatively, you can configure a custom place name in 
   the [CUSTOM PLACE NAMES] section. Then, you can store the 
   coordinates in the script and don't have to pass them via parameters:
   
       m.php?c=[custom place name]
   
   Example:
       http://www.nfcinteractor.com/m.php?c=nokia
   
   
   Advanced Usage
   ---------------------------------------------------------------------------
   
   If your Apache-based web server supports URL rewrites using the 
   mod_rewrite module, you can allow calling the script without adding 
   the ".php" to the URL, thus saving four bytes on the tag.
   
   Example:
       http://www.nfcinteractor.com/m?l=60.17,24.829
	   
   To enable this, add the following .htaccess file in the same directory where
   you place the m.php script:
   
       <IfModule mod_rewrite.c>
       RewriteEngine On
       RewriteBase /
       RewriteRule ^m$ m.php
       </IfModule>
   
 */

// Default location: Nokia House, Espoo, Finland
$location = "60.17,24.829";
// Check if any coordinates are set in the URL
if (isset($_GET['l']) || isset($_GET['c'])) {
	$lparam = isset($_GET['l']) ? $_GET['l'] : $_GET['c'];
	// Encode a + as %2B in the URL parameter if needed
	if (preg_match('/^[\-+]?\d+\.?\d+[,]{1}[\-+]?\d+\.?\d+$/', $lparam) == 1) {
		// Use provided coordinates
		$location = $lparam;
	} else {
		$locationString = strtolower($lparam);
		// [CUSTOM PLACE NAMES]
		// Provide custom place names here.
		// Allows calling the script by just using a textual place name
		// instead of coordinates, which allows for greater flexibility.
		// The script then replaces the place name with the coordinates.
		switch ($locationString) {
		case "nokia":
			$location = "60.17,24.829";
			break;
		}
	}
}

// Encoded platform names
// s = Symbian
// h = MeeGo Harmattan
// f = Series 40
// w8 = Windows 8
// wp8 = Windows Phone 8.x - will be added once SDK is available
// wp7 = Windows Phone 7.x
// a = Android
// i = iPhone
// b = RIM BlackBerry

// Stores encoded platform name
$ua_enc = '';
// Check user agent and assign encoded platform name
$user_agent = $_SERVER['HTTP_USER_AGENT'];
$locationUri = "http://m.nokia.me/?c=".$location."&z=15";
$redirectionMethod = 2;			// 1 = Header redirection, 2 = JavaScript
if (preg_match('/symbian/i',$user_agent)) {
	$ua_enc = 's';
	$locationUri = "http://m.ovi.me/?c=".$location."&z=15";
} elseif (preg_match('/meego/i',$user_agent)) {
	$ua_enc = 'h';
	$locationUri = "geo:".$location;
	$redirectionMethod = 1;
} elseif (preg_match('/Windows NT 6.2/i',$user_agent)) {
	$ua_enc = 'w8';
	$locationTilde = str_replace(',', '~', $location);
	$locationUri = "bingmaps://?cp=".$locationTilde;
} elseif (preg_match('/windows phone/i',$user_agent)) {
	$ua_enc = 'wp7';
	$locationSpace = str_replace(',', ' ', $location);
	$locationUri = "maps:".$locationSpace;
} elseif (preg_match('/nokia/i',$user_agent)) {
	$ua_enc = 'f';
} elseif (preg_match('/android/i',$user_agent)) {
	$ua_enc = 'a';
	$locationUri = "geo:".$location;
	$redirectionMethod = 1;
} elseif (preg_match('/iphone/i',$user_agent) || preg_match('/ipad/i',$user_agent)) {
	$ua_enc = 'i';
} elseif (preg_match('/blackberry/i',$user_agent)) {
	$ua_enc = 'b';
}


if ($redirectionMethod == 1) {
	// The phone supports direct redirection by sending a new header.
	header("Location: ".$locationUri);
	exit;
}

// For other phones, redirect the phone via a JavaScript redirect
// (e.g., using the header redirection on Symbian would not trigger opening the
// Nokia Maps client).
?>
<html>
<head>
<script type="text/javascript">
<!--
window.location = "<?php echo $locationUri; ?>";
//-->
</script>
</head>
<body>
<!--<noscript>-->
<font face="Tahoma,Arial"><a href="<?php echo $locationUri; ?>&z=15">Open Maps</a></font>
<!--</noscript>-->
</body>
</html>