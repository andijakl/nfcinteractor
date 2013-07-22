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

#include "nearfieldtargetinfo.h"

/*!
  \brief New target info, using default values.
  */
NearFieldTargetInfo::NearFieldTargetInfo()
{
    resetInfo();
}

/*!
  \brief Reset all stored data to the defaults.
  */
void NearFieldTargetInfo::resetInfo()
{
    tagTypeName = "";
    tagMajorVersion = 0;
    tagMinorVersion = 0;
    tagMemorySize = -1;
    tagWritableSize = -1;
    tagReadAccessCC = NfcAccessUnknown;
    tagWriteAccessCC = NfcAccessUnknown;
    tagReadAccessLockBits = NfcAccessUnknown;
    tagWriteAccessLockBits = NfcAccessUnknown;
    tagMemoryType = NfcMemoryUnknown;
}

/*!
  \brief Check if both the lock bits of the tag as well as the
  capability container (CC) indicate that the tag is readable.
  */
NearFieldTargetInfo::NfcTagAccessStatus NearFieldTargetInfo::combinedReadAccess() const
{
    return combineAccess(tagReadAccessCC, tagReadAccessLockBits);
}

/*!
  \brief Check if both the lock bits of the tag as well as the
  capability container (CC) indicate that the tag is writable.
  */
NearFieldTargetInfo::NfcTagAccessStatus NearFieldTargetInfo::combinedWriteAccess() const
{
    return combineAccess(tagWriteAccessCC, tagWriteAccessLockBits);
}

/*!
  \brief Internal method used to combine the access status of CC and lock bits.

  If either CC or lock bits, or both indicate it's forbidden, it is overall forbidden.
  If either CC or lock bits, or both indicate that access is allowed, it is overall allowed.
  In all othercases, the access is unknown.
  */
NearFieldTargetInfo::NfcTagAccessStatus NearFieldTargetInfo::combineAccess(const NfcTagAccessStatus accessCC, const NfcTagAccessStatus accessLockBits) const
{
    if (accessCC == NfcAccessForbidden ||
        accessLockBits == NfcAccessForbidden) {
        // If one access states access is forbidden, best estimate is
        // that it's not allowed in general.
        return NfcAccessForbidden;
    } else if (accessCC == NfcAccessAllowed ||
        accessLockBits == NfcAccessAllowed) {
        // Both CC and lock bits indicate that the access is allowed
        return NfcAccessAllowed;
    } else
    // Mixed status or unknown in general
    return NfcAccessUnknown;
}
