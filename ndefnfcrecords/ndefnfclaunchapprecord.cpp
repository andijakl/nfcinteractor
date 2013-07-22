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

#include "ndefnfclaunchapprecord.h"

/*!
  \brief Create an empty LaunchApp record.

  Note that in order to write a LaunchApp to a tag,
  you have to add arguments (an empty arguments string
  will not be handled properly by Windows)
  and at least one platform + id.
  */
NdefNfcLaunchAppRecord::NdefNfcLaunchAppRecord()
    : QNdefRecord(QNdefRecord::Uri, "windows.com/LaunchApp")
{
    setPayload(QByteArray(0, char(0)));
}

/*!
  \brief Create a LaunchApp record based on the record passed
  through the argument.

  Internalizes and parses the payload of the original record.
  */
NdefNfcLaunchAppRecord::NdefNfcLaunchAppRecord(const QNdefRecord &other)
    : QNdefRecord(QNdefRecord::Uri, "windows.com/LaunchApp")
{
    setPayload(other.payload());
}

NdefNfcLaunchAppRecord::~NdefNfcLaunchAppRecord()
{
    m_platformAppIds.clear();
}

/*!
  \brief Sets the payload of the base class to the byte array
  from the parameter and parses its contents.
  */
void NdefNfcLaunchAppRecord::setPayload(const QByteArray &payload)
{
    setPayloadAndParse(payload, true);
}

/*!
  \brief Sets the payload of the base class to the byte array
  from the parameter and optionally parses its contents.

  The parsing needs to be done when the LaunchApp record is read from
  a tag. If a detail of an existing LaunchApp class is modified,
  it will just update its internal payload, but doesn't need to
  parse it again (as the details are already stored in the member
  variables).
  */
void NdefNfcLaunchAppRecord::setPayloadAndParse(const QByteArray &payload, const bool parseNewPayload)
{
    QNdefRecord::setPayload(payload);
    if (parseNewPayload)
    {
        parsePayloadToData();
    }
}

/*!
  \brief Arguments that will be passed to the launched application.

  The exact format is up to the app itself.
  */
QString NdefNfcLaunchAppRecord::arguments() const
{
    return m_arguments;
}

/*!
  \brief Set the argument string that will be passed to the launched application.

  The exact format is up to the app itself.
  */
void NdefNfcLaunchAppRecord::setArguments(const QString &arguments)
{
    m_arguments = arguments;
    assemblePayload();
}

/*!
  \brief Platform names and respective app IDs.

  The key is the platform name, the value the app ID for this specific platform.
  A valid LaunchApp tag needs to contain at least one platform / app ID
  tuple.
  The platform name needs to be unique. Each platform name + app ID has
  to be smaller or equal to 255 characters.
  */
void NdefNfcLaunchAppRecord::addPlatformAppId(const QString &platform, const QString &appId)
{
    m_platformAppIds.insert(platform, appId);
    assemblePayload();
}

/*!
  \brief Number of platform + app ID tuples stored.
  */
int NdefNfcLaunchAppRecord::platformAppIdsCount() const
{
    return m_platformAppIds.count();
}

/*!
  \brief The data structure containing the platforms + respective app IDs.
  */
QHash<QString, QString> NdefNfcLaunchAppRecord::platformAppIds() const
{
    return m_platformAppIds;
}

/*!
  \brief (Re)set the stored data of this launch app record.
  */
void NdefNfcLaunchAppRecord::initializeData()
{
    m_arguments = "";
    m_platformAppIds.clear();
}

/*!
  \brief Deletes any details currently stored in the LaunchApp class
  and re-initializes them by parsing the contents of the payload.
  */
void NdefNfcLaunchAppRecord::parsePayloadToData()
{
    initializeData();

    // Minimum legal length: 5 bytes for the lengths
    if (payload().count() < 5) {
        //qDebug() << "Empty payload";
        return;
    }

    // Create reader based on the payload

    QDataStream ds(payload());
    ds.setByteOrder(QDataStream::BigEndian);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");

    // Number of platforms stored in the record
    ushort platformIdsCount;
    ds >> platformIdsCount;

    // At least one platform found?
    for (int i = 0; i < platformIdsCount; i++)
    {
        // Length of the platform name
        quint8 platformLength;
        ds >> platformLength;
        // Platform name
        char* platformRaw = new char[platformLength];
        int platformRawLength = ds.readRawData(platformRaw, platformLength);
        QString platformName = codec->toUnicode(platformRaw, platformRawLength);
        delete[] platformRaw;

        // Length of the App Id
        quint8 appIdLength;
        ds >> appIdLength;
        // App Id
        char* appIdRaw = new char[appIdLength];
        int appIdRawLength = ds.readRawData(appIdRaw, appIdLength);
        QString appId = codec->toUnicode(appIdRaw, appIdRawLength);
        delete[] appIdRaw;

        // Add platform / app ID tuple to the dictionary
        m_platformAppIds.insert(platformName, appId);
    }

    // Length of the arguments string (big-endian)
    ushort argumentsLength;
    ds >> argumentsLength;

    // Arguments string
    char* argumentsRaw = new char[argumentsLength];
    int argumentsRawLength = ds.readRawData(argumentsRaw, argumentsLength);
    m_arguments = codec->toUnicode(argumentsRaw, argumentsRawLength);
    delete[] argumentsRaw;
}

/*!
  \brief Reverse function to parsePayloadToData() - this one takes
  the information stored in the individual record instances and assembles
  it into the payload of the base class.

  Note: at least one platform + app ID tuple has to be defined.
  */
bool NdefNfcLaunchAppRecord::assemblePayload()
{
    if (platformAppIdsCount() == 0)
    {
        //qDebug() << "Unable to assemble LaunchApp payload: at least one platform / AppID tuple is required.";
        return false;
    }

    QByteArray newPayload;
    QDataStream ds(&newPayload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");

    // First USHORT must contain the number of platform / AppID tuples in big-endian encoding
    ds << (ushort)platformAppIdsCount();

    // For each platform / AppID tuple
    QHashIterator<QString, QString> i(m_platformAppIds);
    while (i.hasNext()) {
        i.next();
        QByteArray rawPlatform = codec->fromUnicode(i.key());
        QByteArray rawAppId = codec->fromUnicode(i.value());

        if (rawPlatform.count() + rawAppId.length() > 255)
        {
            qDebug() << "Unable to assemble LaunchApp payload: length of platform / AppID tuple more than 255 characters";
            return false;
        }

        // Add a byte with the length of the platform string itself
        ds << (quint8)rawPlatform.count();
        // followed by the platform string itself
        ds.writeRawData(rawPlatform, rawPlatform.count());

        // followed by a byte with the length of the AppId string
        ds << (quint8)rawAppId.count();
        // followed by the AppID string itself
        ds.writeRawData(rawAppId, rawAppId.count());
    }

    // Argument string
    QByteArray rawArguments = codec->fromUnicode(m_arguments);

    // USHORT containing the length of the argument string
    ds << (ushort)rawArguments.count();

    // followed by the argument string itself
    ds.writeRawData(rawArguments, rawArguments.count());

    // Set payload
    setPayloadAndParse(newPayload, false);
    return true;
}


