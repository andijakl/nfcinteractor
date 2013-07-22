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

#include "nfctargetanalyzer.h"

/*!
  \brief Create a new Nfc Target Analyzer instance.
  */
NfcTargetAnalyzer::NfcTargetAnalyzer(QObject *parent) :
    QObject(parent)
{
}

/*!
  \brief Create a string containing a textual description of the generic
  tag properties.

  \param target the NFC target to analyze. Works for all supported targets,
  not only for NDEF targets.
  */
QString NfcTargetAnalyzer::analyzeTarget(QNearFieldTarget* target)
{
    QString nfcInfo;
    m_tagInfo.resetInfo();

    // Tag type
    m_tagInfo.tagTypeName = convertTagTypeToString(target->type());
    nfcInfo.append("Target type: " + m_tagInfo.tagTypeName + "\n");
    // Tag UID
    QString uidString = QVariant(target->uid().toHex()).toString();
    nfcInfo.append("UID: " + uidString + "\n");
    // Tag URL (not to be confused with the URL of an NDEF record)
    if (!target->url().isEmpty()) { nfcInfo.append("Url: " + target->url().toString()) + "\n"; }

    // Test the access methods to this target
    QNearFieldTarget::AccessMethods accessMethods = target->accessMethods();
    nfcInfo.append("Access methods: ");
    if (accessMethods.testFlag(QNearFieldTarget::NdefAccess)) {
        // The target supports NDEF records.
        nfcInfo.append("Ndef access\n");
    }
    if (accessMethods.testFlag(QNearFieldTarget::TagTypeSpecificAccess)) {
        // The target supports sending tag type specific commands.
        nfcInfo.append("Tag type specific access\n");
    }
    if (accessMethods.testFlag(QNearFieldTarget::LlcpAccess)) {
        // The target supports peer-to-peer LLCP communication.
        nfcInfo.append("Llcp access\n");
    }

    // Read tag-type specific data
#ifdef Q_OS_SYMBIAN
    // On Symbian, the tag type specific access flag is never set,
    // so analyze the target in any case.
    // (doesn't work with QtM 1.2.1 on Symbian Anna, but works
    // with Symbian Belle).
    const bool alwaysAnalyzeTagSpecific = true;
#else
    // MeeGo doesn't support tag type specific access as of now (PR 1.2),
    // so don't even attempt to do it, as it will only make tag detection
    // slower.
    const bool alwaysAnalyzeTagSpecific = false;
#endif
    if (alwaysAnalyzeTagSpecific || accessMethods.testFlag(QNearFieldTarget::TagTypeSpecificAccess)) {

        if (target->type() == QNearFieldTarget::NfcTagType1)
        {
            // NFC Forum Tag Type 1
            QNearFieldTagType1* targetSpecific = qobject_cast<QNearFieldTagType1 *>(target);
            nfcInfo.append(analyzeType1Target(targetSpecific));
        }
        else if (target->type() == QNearFieldTarget::NfcTagType2)
        {
            // NFC Forum Tag Type 2
            QNearFieldTagType2* targetSpecific = qobject_cast<QNearFieldTagType2 *>(target);
            nfcInfo.append(analyzeType2Target(targetSpecific));
        }
    }

    return nfcInfo.trimmed();
}

/*!
  \brief Show more detailed information if the target is based on the NFC Forum Type 1 platform
  (such as the Innovision Topaz).
  */
QString NfcTargetAnalyzer::analyzeType1Target(QNearFieldTagType1* target)
{
    QString nfcInfo;

    // Static or dynamic memory?
    QNearFieldTarget::RequestId id = target->readIdentification();
    if (!target->waitForRequestCompleted(id)) {
        qDebug() << "Error reading identification bytes of the NFC Forum tag type one target.";
    } else {
        QVariant response = target->requestResponse(id);
        if (response.type() == QVariant::ByteArray) {
            QByteArray tagIdentification = response.toByteArray();
            if (tagIdentification.size() > 2) {
                // Response contains at least HR0 and HR1
                // Byte 0: HR0
                // HR0 Upper nibble = 0001b SHALL determine that it as a Type 1, NDEF capable tag.
                const quint8 tagCheck = tagIdentification[0] >> 4;      // Most significant nibble
                if (tagCheck != 0x1) {
                    qDebug() << "No Type 1, NDEF capable tag";
                }
                // HR0 Lower nibble = 0001b SHALL determine static memory map,
                // != 0001b SHALL determine the dynamic memory map.
                const quint8 tagStaticMemory = tagIdentification[0] & 0x0F;    // Least significant nibble
                if (tagStaticMemory == 0x01) {
                    m_tagInfo.tagMemoryType = NearFieldTargetInfo::NfcMemoryStatic;
                } else {
                    m_tagInfo.tagMemoryType = NearFieldTargetInfo::NfcMemoryDynamic;
                }
                // Byte 1: HR1 = xxh is undefined and SHALL be ignored.
            }
        }
    }

    // Read capability container information
    // Tag version number (VNo)
    const quint8 tagVersion = target->version();
    const int tagMajorVersion = tagVersion >> 4;      // Most significant nibble
    const int tagMinorVersion = tagVersion & 0x0F;    // Least significant nibble
    if (tagVersion > 0) {
        nfcInfo.append("Version: " + QString::number(tagMajorVersion) + "." + QString::number(tagMinorVersion) + "\n");
    }
    m_tagInfo.tagMajorVersion = tagMajorVersion;
    m_tagInfo.tagMinorVersion = tagMinorVersion;

    // Read physical tag memory size (TMS)
    // This reads the tag size from the Capability Container (CC)
    // Therefore, only works when the tag is NDEF-formatted.
    // Also, the returned value can be the total tag size, and doesn't
    // usually mean the actual writable & usable tag memory size.
    int tagMemorySize = target->memorySize();
    if (tagMemorySize > 0) {
        nfcInfo.append("Memory size: " + QString::number(tagMemorySize) + " bytes");
    }
#ifdef MEEGO_EDITION_HARMATTAN
    if (tagMemorySize == 0) {
        // MeeGo returns 0 for the tag size because it can't determine it,
        // not because that's the real tag size.
        // -> set it to unknown
        tagMemorySize = -1;
    }
#endif
    if (m_tagInfo.tagMemoryType != NearFieldTargetInfo::NfcMemoryUnknown) {
        if (tagMemorySize > 0) {
            // Memory size is available - add type to the same line
            nfcInfo.append(" - ");
        } else {
            // No memory size available - add caption
            nfcInfo.append("Memory type: ");
        }
        if (m_tagInfo.tagMemoryType == NearFieldTargetInfo::NfcMemoryStatic) {
            nfcInfo.append("Static");
        } else if (m_tagInfo.tagMemoryType == NearFieldTargetInfo::NfcMemoryDynamic){
            nfcInfo.append("Dynamic");
        }
    }
    if (!(tagMemorySize == -1 && m_tagInfo.tagMemoryType == NearFieldTargetInfo::NfcMemoryUnknown)) {
        // If either the memory size or the memory type could be determined,
        // add a new line.
        nfcInfo.append("\n");
    }

    m_tagInfo.tagMemorySize = tagMemorySize;
    if (m_tagInfo.tagMemoryType == NearFieldTargetInfo::NfcMemoryStatic) {
        // Static memory:
        // The 12 blocks numbered as 1h to Ch contain 96 bytes
        // of general read/write memory.
        // Note that the real writable memory is very likely to be
        // smaller than that, as it also includes various TLVs
        // (for NDEF, reserved memory, termination, dynamic lock, etc.)
        m_tagInfo.tagWritableSize = TYPE1_STATIC_WRITABLE_SIZE;
    }
    if (m_tagInfo.tagWritableSize == -1) {
        m_tagInfo.tagWritableSize = m_tagInfo.tagMemorySize;
    }

    // Lock status of static memory tag
    if (m_tagInfo.tagMemoryType == NearFieldTargetInfo::NfcMemoryStatic) {
        // Read bytes 0, 1 of block 0xE
        // All twelve of the memory blocks 1h to Ch are separately lockable.
        // When a block‘s lock-bit is set to a 1, that block becomes irreversibly frozen as read-only.
        // The lock-bits are stored in the Bytes 0 & 1 of BLOCK-Eh.

        // readBlock() is a dynamic memory only method for type 1 tags
        // Use readByte() instead.
        id = target->readByte(0x0E*8);
        if (!target->waitForRequestCompleted(id)) {
            qDebug() << "Error reading block E, byte 0 of the NFC Forum tag type 1 target.";
        } else {
            quint8 lock0 = target->requestResponse(id).toUInt();

            id = target->readByte(0x0E*8 + 1);
            if (!target->waitForRequestCompleted(id)) {
                qDebug() << "Error reading block E, byte 1 of the NFC Forum tag type 1 target.";
            } else {
                quint8 lock1 = target->requestResponse(id).toUInt();
                qDebug() << "Lock bits: 0x" << QString::number(lock0, 16) << " 0x" << QString::number(lock1, 16);
                // Clear probably set bits that are not relevant for
                // the number of free data blocks
                // b0 of lock0 = UID block -> always locked
                lock0 &= 0xFE;
                // b5 / b6 of lock1 = lock area (block D/E) - irrelevant for data area
                // b7 of lock1 = not used
                lock1 &= 0x1F;
                // Count number of lock bits set
                unsigned int bitCount = 0;
                for (; lock0; bitCount++) {
                  lock0 &= lock0 - 1;   // Clear the least significant bit set
                }
                for (; lock1; bitCount++) {
                  lock1 &= lock1 - 1;   // Clear the least significant bit set
                }
                // Out of the 12 blocks of the static memory area,
                // count how many are still unlocked and then convert the blocks
                // to the number of bytes (8 bytes per block).
                const int unlockedBytes = (12 - bitCount) * 8;
                nfcInfo.append("Unlocked bytes in data area: " + QString::number(unlockedBytes) + "\n");
                // Check if this reduces the writable tag size
                if (unlockedBytes < m_tagInfo.tagWritableSize) {
                    m_tagInfo.tagWritableSize = unlockedBytes;
                }
            }
        }
    }
    if (m_tagInfo.tagWritableSize == 0) {
        m_tagInfo.tagWriteAccessLockBits = NearFieldTargetInfo::NfcAccessForbidden;
    } else if (m_tagInfo.tagWritableSize > 0) {
        m_tagInfo.tagWriteAccessLockBits = NearFieldTargetInfo::NfcAccessAllowed;
    }

    // Check if the Capability Container (CC) is present on the tag.
    // SHALL be the case when an NDEF message is present on the tag.
    // The CC SHALL be assigned to be in the first four bytes of memory block 1.
    // Byte 0 CC memory area starts with NDEF magic number (E1h)
    // Byte 1 SHALL carry the Version Number (VNo) of this document as supported by the Type 1 Tag.
    // Byte 2 SHALL indicate the physical tag memory size (TMS) of the Type 1 Tag as multipliers of (8 bytes) * (n+1).
    // Byte 3 SHALL indicate the read and write access (RWA) capability of the CC and data area of the Type 1 Tag.
    // --> Byte 0 of the CC block equals to target->readByte(8) -> block 1, byte 0 = 1 * 8 + 0
    id = target->readByte(8);   // Check the NDEF magic number
    if (!target->waitForRequestCompleted(id)) {
        qDebug() << "Error reading NDEF magic number of the NFC Forum Tag Type 1 target.";
    } else {
        const quint8 ndefMagicNumber = target->requestResponse(id).toUInt();
        if (ndefMagicNumber == NDEF_MAGIC_NUMBER) {
            qDebug() << "Ndef magic number correct";
            // Found the CC - now check read write access in byte 3 of block 1 -> byte 11 in total
            id = target->readByte(11);  // 11 = memory block 1, byte-3 (CC3)
            if (!target->waitForRequestCompleted(id)) {
                qDebug() << "Error reading RWA capability of the NFC Forum Tag Type 1 target.";
            } else {
                const quint8 rwa = target->requestResponse(id).toUInt();
                const int tagReadAccessCondition = rwa >> 4;      // Most significant nibble
                const int tagWriteAccessCondition = rwa & 0x0F;    // Least significant nibble
                // Read access 0 = read access without security
                const QString tagReadAccess = (tagReadAccessCondition == 0 ? "yes" : "unknown");
                if (tagReadAccessCondition == 0) {
                    m_tagInfo.tagReadAccessCC = NearFieldTargetInfo::NfcAccessAllowed;
                }
                QString tagWriteAccess;
                switch (tagWriteAccessCondition)
                {
                case 0x00:   // Write access without security
                    tagWriteAccess = "yes";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessAllowed;
                    break;
                case 0x0F:   // No write access
                    tagWriteAccess = "no";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessForbidden;
                    break;
                default:
                    tagWriteAccess = "unknown";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessUnknown;
                }
                nfcInfo.append("Access (CC): Read - " + tagReadAccess + ", Write - " + tagWriteAccess);
            }
        } else {
            qDebug() << "Wrong NDEF magic number";
        }
    }

    // Read all
//        if (!pendingWriteNdef)
//        {
//            cachedRequestId = targetSpecific->readAll();
//        }
    return nfcInfo;
}


/*!
  \brief Show more detailed information if the target is based on the NFC Forum Type 2 platform.
  */
QString NfcTargetAnalyzer::analyzeType2Target(QNearFieldTagType2* target)
{
    QString nfcInfo;
    // Read capability container information
    // Tag version number (VNo)
    const quint8 tagVersion = target->version();
    const int tagMajorVersion = tagVersion >> 4;      // Most significant nibble
    const int tagMinorVersion = tagVersion & 0x0F;    // Least significant nibble
    if (tagVersion > 0) {
        nfcInfo.append("Version: " + QString::number(tagMajorVersion) + "." + QString::number(tagMinorVersion) + "\n");
    }
    m_tagInfo.tagMajorVersion = tagMajorVersion;
    m_tagInfo.tagMinorVersion = tagMinorVersion;

    // Read physical tag memory size (TMS)
    // TODO: check if the returned number is correct
    int tagMemorySize = target->memorySize();
    if (tagMemorySize > 0) {
        nfcInfo.append("Memory size: " + QString::number(tagMemorySize) + " bytes - ");
    }
#ifdef MEEGO_EDITION_HARMATTAN
    if (tagMemorySize == 0) {
        // MeeGo returns 0 for the tag size because it can't determine it,
        // not because that's the real tag size.
        // -> set it to unknown
        tagMemorySize = -1;
    }
#endif

    m_tagInfo.tagMemorySize = tagMemorySize;
    // TODO: search for TLV areas in dynamic memory tags
    m_tagInfo.tagWritableSize = m_tagInfo.tagMemorySize;


    // Static memory: 48 bytes (TYPE2_STATIC_MEMORY_SIZE)
    if (m_tagInfo.tagMemorySize > 0) {
        if (m_tagInfo.tagMemorySize > TYPE2_STATIC_MEMORY_SIZE) {
            m_tagInfo.tagMemoryType = NearFieldTargetInfo::NfcMemoryStatic;
            nfcInfo.append("Static\n");
        } else {
            m_tagInfo.tagMemoryType = NearFieldTargetInfo::NfcMemoryDynamic;
            nfcInfo.append("Dynamic\n");
        }
    }

    // Read static lock bytes of the tag
    // Each block on a NFC Forum Type 2 tag is 4 bytes (0 - 4).
    // The bits of byte 2 and 3 of block 2 represent the field-programmable read-only locking
    // mechanism called static lock bytes. Depending on the value of the bits of the static lock bytes two
    // configurations are possible:
    // * All bits are set to 0b, the CC area and the data area of the tag can be read and written.
    // * All bits are set to 1b, the CC area and the data area of the tag can be only read.

    // Note: selecting sector 0 doesn't work really, but not important
    // as previous calls to get the memory size through Qt Mobility already
    // selected the sector. The variable in the tag-specific class where it stores
    // the current sector unfortunately isn't public, to switch the sector
    // only on demand.

    // Static memory lock
    QNearFieldTarget::RequestId id = target->readBlock(2);
    if (!target->waitForRequestCompleted(id)) {
        qDebug() << "Error reading static lock bytes of the NFC Forum tag type two target.";
    } else {
        QVariant response = target->requestResponse(id);
        if (response.isValid() && response.type() == QVariant::ByteArray) {
            QByteArray p = response.toByteArray();
            // Response: 16 bytes + 2 bytes checksum
            if (p.size() > 4) {
                if (p.at(2) == char(0x00) && p.at(3) == char(0x00)) {
                    nfcInfo.append("Static lock: Read - yes, Write - yes\n");
                    m_tagInfo.tagReadAccessLockBits = NearFieldTargetInfo::NfcAccessAllowed;
                    m_tagInfo.tagWriteAccessLockBits = NearFieldTargetInfo::NfcAccessAllowed;
                } else if (p.at(2) == char(0xFF) && p.at(3) == char(0xFF)) {
                    nfcInfo.append("Static lock: Read - yes, Write - no\n");
                    m_tagInfo.tagReadAccessLockBits = NearFieldTargetInfo::NfcAccessAllowed;
                    m_tagInfo.tagWriteAccessLockBits = NearFieldTargetInfo::NfcAccessForbidden;
                } else {
                    nfcInfo.append("Static lock: not set according to specifications\n");
                    m_tagInfo.tagReadAccessLockBits = NearFieldTargetInfo::NfcAccessUnknown;
                    m_tagInfo.tagWriteAccessLockBits = NearFieldTargetInfo::NfcAccessUnknown;
                }
            } else if (p.at(0) == char(0x05) || p.at(0) == char(0x01)){
                // Received NACK
                nfcInfo.append("Static lock: not successful (NACK response from tag)\n");
            } else {
                nfcInfo.append("Static lock: unexpected response (size: " + QString::number(p.size()) + ")\n");
            }
        }
    }

    // CC lock
    // The CC is stored in the block 3 of the static or dynamic memory structure
    // Byte 0 is equal to E1h (magic number) to indicate that NFC Forum defined data is stored in the data area
    // Byte 3 indicates the read and write access capability of the data area and CC area of the Type 2 Tag Platform
    id = target->readBlock(3);   // Check the NDEF magic number
    if (!target->waitForRequestCompleted(id)) {
        qDebug() << "Error reading NDEF magic number of the NFC Forum Tag Type 2 target.";
    } else {
        QVariant response = target->requestResponse(id);
        if (response.isValid() && response.type() == QVariant::ByteArray) {
            QByteArray p = response.toByteArray();
            const quint8 ndefMagicNumber = p.at(0);
            if (ndefMagicNumber == NDEF_MAGIC_NUMBER) {
                // Found the CC - now check read write access in byte 3
                // The most significant nibble (the 4 most significant bits) indicates the read access condition:
                // - The value 0h indicates read access granted without any security.
                // - The values from 1h to 7h and Fh are reserved for future use.
                // - The values from 8h to Eh are proprietary.
                // The least significant nibble (the 4 least significant bits) indicates the write access condition:
                // - The value 0h indicates write access granted without any security.
                // - The values from 1h to 7h are reserved for future use.
                // - The values from 8h to Eh are proprietary.
                // - The value Fh indicates no write access granted at all.
                const quint8 rwa = p.at(3); // RWA = byte 3 of CC
                const int tagReadAccessCondition = rwa >> 4;      // Most significant nibble
                const int tagWriteAccessCondition = rwa & 0x0F;    // Least significant nibble

                QString tagReadAccess;
                if (tagReadAccessCondition == 0x0) {
                    tagReadAccess = "yes";
                    m_tagInfo.tagReadAccessCC = NearFieldTargetInfo::NfcAccessAllowed;
                } else if (tagReadAccessCondition >= 0x8) {
                    tagReadAccess = "proprietary";
                    m_tagInfo.tagReadAccessCC = NearFieldTargetInfo::NfcAccessUnknown;
                } else {
                    tagReadAccess = "unknown";
                    m_tagInfo.tagReadAccessCC = NearFieldTargetInfo::NfcAccessUnknown;
                }

                QString tagWriteAccess;
                if (tagWriteAccessCondition == 0x0) {
                    tagWriteAccess = "yes";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessAllowed;
                } else if (tagReadAccessCondition >= 0x8) {
                    tagWriteAccess = "proprietary";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessUnknown;
                } else {
                    tagWriteAccess = "unknown";
                    m_tagInfo.tagWriteAccessCC = NearFieldTargetInfo::NfcAccessUnknown;
                }
                nfcInfo.append("Access (CC): Read - " + tagReadAccess + ", Write - " + tagWriteAccess);

            } else {
                qDebug() << "Wrong NDEF magic number";
            }
        }
    }

    return nfcInfo;
}


/*!
  \brief Return a textual description of the NFC target \a type.
  */
QString NfcTargetAnalyzer::convertTagTypeToString(const QNearFieldTarget::Type type)
{
    QString tagType = "Unknown";
    switch (type)
    {
    case QNearFieldTarget::AnyTarget:
        tagType = "This value is only used when registering handlers to indicate that any compatible target can be used.";
        break;
    case QNearFieldTarget::ProprietaryTag:
        tagType = "Unidentified proprietary tag";
        break;
    case QNearFieldTarget::NfcTagType1:
        tagType = "NFC tag type 1";
        break;
    case QNearFieldTarget::NfcTagType2:
        tagType = "NFC tag type 2";
        break;
    case QNearFieldTarget::NfcTagType3:
        tagType = "NFC tag type 3";
        break;
    case QNearFieldTarget::NfcTagType4:
        tagType = "NFC tag type 4";
        break;
    case QNearFieldTarget::MifareTag:
        tagType = "Mifare";
        break;
    case QNearFieldTarget::NfcForumDevice:
        tagType = "NFC Forum device";
        break;
    }
    return tagType;
}
