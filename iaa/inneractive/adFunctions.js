/*
*
* adFunctions.qml
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

// Helper functions
.pragma library
var __baseUrl = "http://m2m1.inner-active.mobi/simpleM2M/clientRequestAd"

function createQuery(adItem)
{
    var parameters = adItem.parameters;
    function buildRequestUrl()
    {
        var requestsUrl = __baseUrl;
        function addQuery(key, value)
        {
            if (value) { // no need to add if no string to add
                if (requestsUrl !== __baseUrl)
                    requestsUrl +="&";
                else
                    requestsUrl +="?";
                requestsUrl += key + "=" + value;
            }
        }

        // Required parameters
        addQuery("aid", parameters.applicationId);
        addQuery("v", parameters.version);
        addQuery("po", parameters.distributionId);
        addQuery("cid", parameters.__clientId);
        addQuery("hid", parameters.__imei);
        addQuery("w", parameters.__screenWidth);
        addQuery("h", parameters.__screenHeight);

        // Optional parameters
        addQuery("a", parameters.userAge);
        addQuery("c", parameters.category);
        addQuery("g", parameters.userGender);
        addQuery("k", parameters.keywords);
        addQuery("mn", parameters.mobileNumber);

        if (parameters.useLocation) {
            addQuery("l", parameters.__location);
        }
        if (parameters.usePositioning) {
            addQuery("lg", parameters.__gpsLoc);
        }
        return requestsUrl;
    }

    if (!parameters) {
        console.log("Ad parameters item is Null");
        Qt.quit();
        return;
    }
    else if (!parameters.applicationId) {
        console.log("ApplicationID is Null");
        Qt.quit();
        return;
    }
    else if (!parameters.distributionId) {
        console.log("DistributionID is Null");
        Qt.quit();
        return;
    }

    adItem.__query = buildRequestUrl();

}
