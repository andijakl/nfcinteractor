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

#ifndef NEARFIELDTARGETINFO_H
#define NEARFIELDTARGETINFO_H

#include <QDebug>

/*!
  \brief Stores additional information about a QNearFieldTarget.
  */
class NearFieldTargetInfo
{
public:
    NearFieldTargetInfo();
    void resetInfo();

    enum NfcTagAccessStatus {
        NfcAccessUnknown,
        NfcAccessAllowed,
        NfcAccessForbidden
    };
    enum NfcTagMemoryType {
        NfcMemoryUnknown,
        NfcMemoryStatic,
        NfcMemoryDynamic
    };

    NearFieldTargetInfo::NfcTagAccessStatus combinedReadAccess() const;

    NearFieldTargetInfo::NfcTagAccessStatus combinedWriteAccess() const;

private:
    NearFieldTargetInfo::NfcTagAccessStatus combineAccess(const NfcTagAccessStatus accessCC, const NfcTagAccessStatus accessLockBits) const;

public:
    QString tagTypeName;
    int tagMajorVersion;
    int tagMinorVersion;
    int tagMemorySize;
    int tagWritableSize;
    NfcTagAccessStatus tagReadAccessCC;
    NfcTagAccessStatus tagWriteAccessCC;
    NfcTagAccessStatus tagReadAccessLockBits;
    NfcTagAccessStatus tagWriteAccessLockBits;
    NfcTagMemoryType tagMemoryType;

};


#endif // NEARFIELDTARGETINFO_H
