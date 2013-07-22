# ######################################################################## #
#																		   #
# component.pri															   #
# © Copyrights 2012 inneractive LTD, Nokia. All rights reserved			   #
#																		   #
# This file is part of inneractiveAdQML.								   #
# 																		   #
# inneractiveAdQML is free software: you can redistribute it and/or modify #
# it under the terms of the GNU General Public License as published by	   #
# the Free Software Foundation, either version 3 of the License, or		   #
# (at your option) any later version.									   #
# 																		   #
# inneractiveAdQML is distributed in the hope that it will be useful,	   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of		   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			   #
# GNU General Public License for more details.							   #
# 																		   #
# You should have received a copy of the GNU General Public License		   #
# along with inneractiveAdQML. If not, see <http://www.gnu.org/licenses/>. #
# ######################################################################## #


QT += declarative network

symbian {
TARGET.CAPABILITY += NetworkServices Location SwEvent
LIBS += -lapgrfx -lcone
include(cuseragent/cuseragent.pri)
}

CONFIG += mobility
MOBILITY += location

SOURCES += \
    $$PWD/requestqueue.cpp \
    $$PWD/adinterface.cpp \
    $$PWD/inneractiveplugin.cpp

HEADERS += \
    $$PWD/requestqueue.h \
    $$PWD/adinterface.h \
    $$PWD/inneractiveplugin.h

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

componentFiles.files += \
    inneractive/adFunctions.js \
    inneractive/AdItem.qml \
    inneractive/AdParameters.qml
