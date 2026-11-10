TARGET = harbour-saildice

CONFIG += sailfishapp c++11 link_pkgconfig
QT += quick

PKGCONFIG += sailfishapp

SOURCES += \
    src/main.cpp \
    src/dicetray.cpp \
    src/dicegeometry.cpp \
    src/diceatlas.cpp \
    src/selftest.cpp

HEADERS += \
    src/dicetray.h \
    src/dicegeometry.h \
    src/diceatlas.h \
    src/selftest.h

DISTFILES += \
    qml/harbour-saildice.qml \
    qml/pages/MainPage.qml \
    qml/pages/DiceConfigPage.qml \
    qml/components/DiceConfigDelegate.qml \
    qml/components/HistoryDelegate.qml \
    qml/cover/CoverPage.qml \
    harbour-saildice.desktop \
    rpm/harbour-saildice.spec \
    README.md \
    LICENSE \
    CHANGELOG.md \
    PRIVACY.md \
    SECURITY.md \
    CONTRIBUTING.md \
    docs/harbour-listing.md \
    docs/RELEASING.md \
    tools/generate_icons.py

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172 256x256
