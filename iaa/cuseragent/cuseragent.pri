# ######################################################################## #
#																		   #
# cuseragent.pri														   #
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

LIBS += -lwebutils -lcone
INCLUDEPATH += $$PWD
OTHER_FILES += \
    $$PWD/webutils.dso \
    $$PWD/webutils{000a0000}.dso \
    $$PWD/webutils.lib \
    $$PWD/webutils{000a0000}.lib \
    $$PWD/cuseragent.h \
    $$PWD/sysversioninfo.h \
    $$PWD/sysversioninfo.inl

webutilsdso = $$PWD/webutils*.dso
webutilsdso = $$replace(webutilsdso, /, \\)
webutilslib = $$PWD/webutils*.lib
webutilslib = $$replace(webutilslib, /, \\)

system(xcopy /d /f "$$webutilslib" "%epocroot%epoc32\\release\\armv5\\lib\\")
system(xcopy /d /f "$$webutilsdso" "%epocroot%epoc32\\release\\armv5\\lib\\")
