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

#ifndef RECORDTYPES_H
#define RECORDTYPES_H

#include <QObject>
#include "appsettings.h"

/*!
  \brief Important enums that are shared between QML and C++.

  QML doesn't have direct enum support, therefore, the enums are
  exported from C++ to be able to reuse them in the QML world.
  This is the only possible approach currently, but works fine.
  One of the limitations is that when passing data between
  C++ and QML, it's generally passed as Integer and not as a
  real enum, loosing some type safety due to the need to cast
  it back to an enum.
  */
class NfcTypes : public QObject
{
    Q_OBJECT
    Q_ENUMS(MessageType RecordContent NfcModes)
public:
    /*!
      \brief The message type of record items stored in the model.

      Some of the message types represent real NDEF record types,
      others are convenience classes provided by the Nfc Interactor
      (e.g., MsgGeo). Some other types defined here are just
      templates used to pre-populate the editing UI with various
      individual records, and do not actually appear in the model
      (e.g., MsgAnnotatedUrl, which is added as a MsgUri and a MsgText).

      Usually, several consecutive NfcRecordItem instances have the
      same MessageType, but different RecordContent - as these define
      different aspects of the message.
      */
    enum MessageType {
        MsgSmartPoster, // Note: do not change the order of the message types.
        MsgUri,         // The values are used to save & load stats and should
        MsgText,        // work after the app has been updated.
        MsgSms,
        MsgBusinessCard,
        MsgSocialNetwork,
        MsgGeo,
        MsgStore,
        MsgImage,
        MsgCustom,
        // Msg Types below this are not actually added to the model,
        // but just expand to a collection of the basic types listed above.
        // As such, no stats for these types are stored, the order can be changed.
        // If adding a new type that is included in stats, add it before here.
        MsgAnnotatedUrl,
        MsgCombination,
        MsgNfcAutostart,
        MsgLaunchApp,
        MsgAndroidAppRecord
    };

    /*!
      \brief Type of the content of an individual item in the model.

      The content defines what part of the whole message is defined by this
      record item. Usually, a complete NDEF record consists of multime
      NfcRecordItem instances, all of them having the same MessageType,
      but their individual content items defined by having a different
      RecordContent. For example, a Smart Poster MessageType always has a
      RecordHeader and a RecordUri. Optionally (= if added by the user),
      it can also have a RecordText, RecordTextLanguage, RecordSpAction, etc.

      Some of the RecordContent can occur in different MessageType s. A
      good example is the RecordText, which can be used as the title in all
      records that map to a Smart Poster, or also in an extra text NDEF record.
      Other RecordContent items are specific to a MessageType, for example the
      RecordSpAction is only used within a Smart Poster.

      The RecordContent tells the UI how to draw the item in the editing UI
      (e.g., as a text box, or a selection item). Additionally, it tells the
      NfcModelToNdef class what part of the NDEF record is defined by this
      NfcRecordItem.
      */
    enum RecordContent {
        RecordHeader,
        RecordUri,
        RecordText,
        RecordTextLanguage,
        RecordSmsBody,
        RecordSpAction,
        RecordSpType,
        RecordSpSize,
        RecordImageFilename,
        RecordTypeName,
        RecordTypeNameFormat,
        RecordId,
        RecordRawPayload,
        RecordSocialNetworkType,
        RecordSocialNetworkName,

        RecordNamePrefix,
        RecordFirstName,
        RecordMiddleName,
        RecordLastName,
        RecordNameSuffix,
        RecordNickname,

        RecordEmailAddress,
        RecordPhoneNumber,
        RecordContactUrl,

        RecordOrganizationName,
        RecordOrganizationDepartment,
        RecordOrganizationRole,
        RecordOrganizationTitle,

        RecordBirthday, // not implemented yet, would require a date editor field
        RecordNote,

        RecordCountry,
        RecordLocality,
        RecordPostOfficeBox,
        RecordPostcode,
        RecordRegion,
        RecordStreet,

        RecordGeoType,
        RecordGeoLatitude,
        RecordGeoLongitude,

        RecordStoreNokia,
        RecordStoreSymbian,
        RecordStoreMeeGoHarmattan,
        RecordStoreSeries40,
        RecordStoreWindowsPhone,
        RecordStoreAndroid,
        RecordStoreiOS,
        RecordStoreBlackberry,
        RecordStoreCustomName,

        RecordLaunchAppArguments,
        RecordLaunchAppWindows,
        RecordLaunchAppWindowsPhone,
        RecordLaunchAppPlatform,
        RecordLaunchAppId,

        RecordAndroidPackageName
    };

    /*!
      \brief Status information whether the NfcInfo class is currently in reading or writing mode.

      Used to send signals from C++ to QML, to let the UI react to status changes
      of the C++ engine classes.
      */
    enum NfcModes {
        nfcReading = 0,
        nfcWriting
    };
};

#endif // RECORDTYPES_H
