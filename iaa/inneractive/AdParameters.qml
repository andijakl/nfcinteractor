/*
*
* AdParameters.qml
* © Copyrights 2012 inneractive LTD, Nokia. All rights reserved
*
* This file is part of inneractiveAdQML.	
*
* inneractiveAdQML is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* inneractiveAdQML is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with inneractiveAdQML. If not, see <http://www.gnu.org/licenses/>.
*/


import QtQuick 1.0
import QtMobility.systeminfo 1.1
import QtMobility.location 1.1
/*!
    \qmlclass AdParameters
    \ingroup com.inneractive
    \brief Parameters for ad requests
    This item is used to give request parameters for AdItem.
    The AdParameters component is part of the \l {inneractive QML Components} module.
  */
Item {
    id: adParamRoot
    //Required parameters, these must be set
    /*!
      Application ID used in ad request.
      Provided by inneractive administrator at the registration process.
      */
    property string applicationId
    /*!
      Distribution channel ID used in ad request.
      The default value is "551", for ovi store.
      */
    property string distributionId: "551"
    /*!
      Version used in ad request.
      This is version of inneractive API.
      */
    property string version: "1.5.3-QML-1.0.1"

    //Optional parameters
    /*!
      User’s age.
      */
    property string userAge
    /*!
      User’s gender.
	  Allowed values: M, m, F, f, Male, Female.
      */
    property string userGender
    /*!
      Keywords relevant to this user’s specific session (comma separated, w/o spaces).
      */
    property string keywords
    /*!
      Category of application.
      A single word description of the application.
      */
    property string category
    /*!
      User’s mobile number.
	  MSISDN format, with international prefix.
      */
    property string mobileNumber

    /*!
      Use GPS coordinates in ad request.
      The default value is true.
      */
    property bool usePositioning: true
    /*!
      Use location data of users position.
      Country,State,City format.
      The default value is false.
      \warning This functionality is not yet implemented
      */
    property bool useLocation: false


    /*!
      \internal
      Will be set after first request and next requests update
      */
    property string __clientId // Handled by request functions
    /*!
      \internal
      */
    property string __imei: deviceInfo.imei // Use mobility to get imei of device
    /*!
      \internal
      */
    property string __screenWidth: screen ? screen.displayWidth : ""
    /*!
      \internal
      */
    property string __screenHeight: screen ? screen.displayHeight : ""
    /*!
      \internal
      GPS position in format <latitude>,<longitude>
      */
    property string __gpsLoc: (usePositioning
                               && positionSource.position.latitudeValid
                               && positionSource.position.longitudeValid)
                              ? (positionSource.position.coordinate.latitude.toString()
                                 + ","
                                 + positionSource.position.coordinate.longitude.toString())
                              : ""
    /*!
      \internal
      Location in readable format country,state/province,city (US,NY,NY)
      */
    property string __location: useLocation ? ""
                                            : ""

    // Used to get IMEI of device
    DeviceInfo {
        id: deviceInfo
    }

    // Positioning stuff
    /*!
      \internal
      */
    property QtObject positionSource: null

    Component.onCompleted: {
        if (usePositioning)
            positionSource = posSourceComponent.createObject(adParamRoot);
    }

    onUsePositioningChanged: {
        if (usePositioning && positionSource == null) {
            positionSource = posSourceComponent.createObject(adParamRoot);
        } else if (!usePositioning && positionSource != null) {
            positionSource.destroy();
            positionSource = null;
        }
    }

    Component {
        id: posSourceComponent
        PositionSource {
            active: usePositioning
            updateInterval: 5000
        }
    }
}
