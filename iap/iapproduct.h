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

#ifndef IAPPRODUCT_H
#define IAPPRODUCT_H

#include <QObject>

class IapProduct : public QObject
{
    Q_OBJECT
public:
    explicit IapProduct(const QString& productId, QObject *parent = 0);
    
public:
    QString productId() const;
    void setProductId(const QString& productId);
    QString price() const;
    void setPrice(const QString& price);
    bool purchased() const;
    void setPurchased(const bool purchased);
    int requestId() const;
    void setRequestId(const int requestId);
    void setRequestFinished();

signals:
    
public slots:
    
private:
    QString m_productId;
    QString m_price;
    bool m_purchased;
    int m_requestId;
};

#endif // IAPITEM_H
