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

#include "ndefnfcandroidapprecord.h"

/*!
  Name of the Android package.
  */
QString NdefNfcAndroidAppRecord::packageName() const
{
    const QByteArray p = payload();
    if (p.isEmpty())
        return QString();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    return codec->toUnicode(p);
}

/*!
  Sets the name of the Android package.
  */
void NdefNfcAndroidAppRecord::setPackageName(const QString &packageName)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QByteArray p = codec->fromUnicode(packageName);
    setPayload(p);
}


