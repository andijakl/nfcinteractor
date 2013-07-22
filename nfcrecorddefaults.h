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

#ifndef NFCRECORDDEFAULTS_H
#define NFCRECORDDEFAULTS_H

#include <QObject>
#include "nfctypes.h"

class NfcRecordDefaults : public QObject
{
    Q_OBJECT
public:
    explicit NfcRecordDefaults(QObject *parent = 0);

public:
    QString itemHeaderTextDefault(const NfcTypes::MessageType messageType);
    void itemContentDefault(const NfcTypes::MessageType msgType, const NfcTypes::RecordContent contentType, QString &defaultTitle, QString &defaultContents);
    QVariantList itemSelectionDefault(const NfcTypes::RecordContent contentType, int &defaultSelectedItem);
};

#endif // NFCRECORDDEFAULTS_H
