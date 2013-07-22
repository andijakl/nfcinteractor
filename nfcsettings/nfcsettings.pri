
SOURCES += $$PWD/nfcsettings.cpp
HEADERS += $$PWD/nfcsettings.h
INCLUDEPATH += $$PWD

symbian {
    #TARGET.CAPABILITY = LocalServices
    HEADERS += $$PWD/nfcsettings_symbian_p.h
    SOURCES += $$PWD/nfcsettings_symbian_p.cpp
    LIBS += -lcentralrepository -lfeatdiscovery -lhal
    #QMAKE_CXXFLAGS.GCCE += "-fpermissive"
} else {
    HEADERS += $$PWD/nfcsettings_p.h
    SOURCES += $$PWD/nfcsettings_p.cpp
}

