/*
*
* inneractiveplugin.cpp
* © Copyrights 2012 inneractive LTD, Nokia. All rights reserved
*
* This file is part of inneractiveAdQML.	
*
* inneractiveAdQML is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* inneractiveAdQML is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with inneractiveAdQML. If not, see <http://www.gnu.org/licenses/>.
*/

#include "inneractiveplugin.h"
#include "adinterface.h"
#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDebug>

void inneractivePlugin::initializeEngine(QDeclarativeEngine *engine)
{
    if (!engine->rootContext()->contextProperty("adInterface").isValid()) {
        AdInterface *adI = new AdInterface(engine);
        engine->rootContext()->setContextProperty("adInterface", adI);
    } else {
        qDebug() << Q_FUNC_INFO << ":" << engine << "already initialized!";
    }
}
