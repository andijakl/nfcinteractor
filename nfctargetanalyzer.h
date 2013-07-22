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

#ifndef NFCTARGETANALYZER_H
#define NFCTARGETANALYZER_H

#include <QObject>
#include <QByteArray>
#include <QVariant>
#include <QDebug>
#include <QUrl>
#include <QNearFieldTarget>
#include <QNearFieldTagType1>
#include <QNearFieldTagType2>
#include "nearfieldtargetinfo.h"

#define TYPE1_STATIC_WRITABLE_SIZE 96
#define TYPE2_STATIC_MEMORY_SIZE 48
#define NDEF_MAGIC_NUMBER 0xE1

// Typical writable tag size
#define GUESS_TAG_WRITABLE_SIZE_TYPICAL_TLV_SIZE 6

// Memory availability is 96 bytes and expandable to 2 kbyte.
#define GUESS_TYPE1_SIZE (TYPE1_STATIC_WRITABLE_SIZE - GUESS_TAG_WRITABLE_SIZE_TYPICAL_TLV_SIZE)
// Memory availability is 48 bytes and expandable to 2 kbyte.
// Assume a dynamic tag with 144 bytes memory size, as 48 bytes is really small
#define GUESS_TYPE2_SIZE (144 - GUESS_TAG_WRITABLE_SIZE_TYPICAL_TLV_SIZE)
// Memory availability is variable, theoretical memory limit is 1MByte per service.
// FeliCa Lite provides 14 blocks of user memory (224 bytes)
// Standard FeliCa provides 154 blocks (2464 bytes)
#define GUESS_TYPE3_SIZE 224
// The memory availability is variable, up to 32 KBytes per service
#define GUESS_TYPE4_SIZE 2048
// The MIFARE Classic 1K offers 1024 bytes of data storage
#define GUESS_MIFARE_SIZE 1024

QTM_USE_NAMESPACE

/*!
  \brief Analyze the NFC target and return the gathered information
  in human-readable textual form.

  The output contains in all cases information about the tag type,
  UID and access methods.

  If the underlying Qt Mobility implementation allows tag-type specific
  access, it will further analyze the memory size and several other
  parts of the tag.

  For Type 1 & 2 targets, deeper analysis is performed using low-level
  commands. Note that tag-type specific access is currently only
  implemented for Symbian, and does not work on the N9.

  The NfcNdefParser has a similar task, but returns the tag contents
  in textual form.
  */
class NfcTargetAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit NfcTargetAnalyzer(QObject *parent = 0);
    QString analyzeTarget(QNearFieldTarget *target);
    QString analyzeType1Target(QNearFieldTagType1 *target);
    QString analyzeType2Target(QNearFieldTagType2 *target);

    QString convertTagTypeToString(const QNearFieldTarget::Type type);
    
private:
    
public:
    NearFieldTargetInfo m_tagInfo;
};

#endif // NFCTARGETANALYZER_H
