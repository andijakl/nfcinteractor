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

#include "iapproduct.h"


IapProduct::IapProduct(const QString &productId, QObject *parent) :
    QObject(parent),
    m_productId(productId),
    m_price("Checking..."),
    m_requestId(-1)
{
}

QString IapProduct::productId() const
{
    return m_productId;
}

void IapProduct::setProductId(const QString &productId)
{
    m_productId = productId;
}

QString IapProduct::price() const
{
    return m_price;
}

void IapProduct::setPrice(const QString &price)
{
    m_price = price;
}

bool IapProduct::purchased() const
{
    return m_purchased;
}

void IapProduct::setPurchased(const bool activated)
{
    m_purchased = activated;
}

int IapProduct::requestId() const
{
    return m_requestId;
}

void IapProduct::setRequestId(const int requestId)
{
    m_requestId = requestId;
}

void IapProduct::setRequestFinished()
{
    m_requestId = -1;
}
