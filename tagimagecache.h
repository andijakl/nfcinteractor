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

#ifndef TAGIMAGECACHE_H
#define TAGIMAGECACHE_H

#include <QDeclarativeImageProvider>
#include <QImage>
#include <QDebug>

/*!
  \brief Caches any images found on tags in memory, for retrieval
  and display in the QML user interface.

  The class is derived from QDeclarativeImageProvider and uses the
  QImage operation mode.
  */
class TagImageCache : public QDeclarativeImageProvider
{
public:
    TagImageCache();
    ~TagImageCache();

    int addImage(QImage img);

    // From QDeclarativeImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    QList<QImage> m_imageCache;
};

#endif // TAGIMAGECACHE_H
