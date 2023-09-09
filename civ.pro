lessThan(QT_MAJOR_VERSION, 5) {
	error("Required Qt 5.12.0 or later")
}
isEqual(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 12) {
	error("Required Qt 5.12.0 or later")
}

QT += core gui widgets printsupport

TARGET = civ
TEMPLATE = app
CONFIG += c++11

CONFIG(debug, debug|release) {
	message( "!!! DEBUG BUILD !!!" )
	QMAKE_CXXFLAGS += -g -rdynamic
	DEFINES += DEBUG_BUILD
}

CONFIG += optimize_full
DESTDIR = build
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp

exists (/usr/include/GraphicsMagick/Magick++.h) {
	message( "Has GraphicsMagick..." )
	INCLUDEPATH += /usr/include/GraphicsMagick
	LIBS += -lGraphicsMagick++
	DEFINES += HAS_GMAGICK
}

exists (/usr/include/vips/vips.h) {
	message( "Has libvips..." )
	INCLUDEPATH += /usr/include/vips/
	CONFIG += link_pkgconfig
	PKGCONFIG += vips-cpp
	LIBS += -lvips
	DEFINES += HAS_VIPS
}


DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ./src/

SOURCES += src/*.cpp \
	src/io/*.cpp \
	src/io/xt/*.cpp \
	src/widgets/*.cpp \
	src/dialogs/*.cpp \
	src/modules/*.cpp \
	src/effects/*.cpp 

HEADERS += src/*.h \
	src/io/*.h \
	src/io/xt/*.h \
	src/widgets/*.h \
	src/dialogs/*.h \
	src/modules/*.h \
	src/effects/*.h 

FORMS += src/*.ui \
	src/dialogs/*.ui

RESOURCES += resources/res.qrc

TRANSLATIONS = i18n/en.ts


# Automating Generation of QM Files
TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
	qmfile = $$shadowed($$tsfile)
	qmfile ~= s,.ts$,.qm,
	qmdir = $$dirname(qmfile)
	!exists($$qmdir) {
		mkpath($$qmdir)|error("Aborting.")
	}
	command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
	system($$command)|error("Failed to run: $$command")
	TRANSLATIONS_FILES += $$qmfile
}

# Install
CONFIG += nostrip

isEmpty(PREFIX) {
	packaging {
		PREFIX = /usr
	} else {
		PREFIX = ~/.local
	}
}

target.path = $${PREFIX}/bin/
icons.path = $${PREFIX}/share/icons/
icons.files = install/usr/share/icons/*
pixmaps.path = $${PREFIX}/share/pixmaps/
pixmaps.files = install/usr/share/icons/hicolor/128x128/apps/civ.png
qmfiles.path = $${PREFIX}/share/$${TARGET}/languages/
qmfiles.files = $${TRANSLATIONS_FILES}
desktop.path = $${PREFIX}/share/applications
desktop.files = install/usr/share/applications/classicimageviewer.desktop
INSTALLS += 	target \
		qmfiles \
		icons \
		pixmaps \
		desktop


