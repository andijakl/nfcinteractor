/*
*
* inneractiveplugin.h
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

#ifndef INNERACTIVEPLUGIN_H
#define INNERACTIVEPLUGIN_H

class QDeclarativeEngine;
/*!
 * \brief The inneractivePlugin class
 */
class inneractivePlugin
{
public:
    /*!
     * \brief initializeEngine
     *Initializes QDeclarativeEngine to use inneractive QML components.
     *AdInterface object will be registered as context property with name "adInterface"
     * \param engine QDeclarativeEngine where AdInterface will be registered
     */
    static void initializeEngine(QDeclarativeEngine *engine);

};

#endif // INNERACTIVEPLUGIN_H
