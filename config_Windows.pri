#
# QGC Windows-only configuration directives
#

# Get rid of the console for release builds.
#ReleaseBuild:CONFIG -= console
CONFIG += console
CONFIG += embed_manifest_exe

RC_FILE = $$BASEDIR/qgroundcontrol.rc

DEFINES += USE_GOOGLE_EARTH_PLUGIN
# The EIGEN library needs this define
# to make the internal min/max functions work
DEFINES += NOMINMAX

win32-msvc* {
	# QWebkit is not needed on MS-Windows compilation environment
	CONFIG -= webkit webkitwidgets

	# Specify the inclusion of (U)INT*_(MAX/MIN) macros within Visual Studio
	DEFINES += __STDC_LIMIT_MACROS

	INCLUDEPATH += $$BASEDIR/libs/lib/sdl/msvc/include \
		  #$$BASEDIR/libs/lib/opal/include

	LIBS += -L$$BASEDIR/libs/lib/sdl/msvc/lib

	# Specify multi-process compilation within Visual Studio.
	# (drastically improves compilation times for multi-core computers)
	QMAKE_CXXFLAGS_DEBUG += -MP
	QMAKE_CXXFLAGS_RELEASE += -MP

	QMAKE_CXXFLAGS_WARN_ON += \
		  /wd4996 \   # silence warnings about deprecated strcpy and whatnot
		  /wd4005 \   # silence warnings about macro redefinition
		  /wd4290     # ignore exception specifications

	# QAxContainer support is needed for the Internet Control
	# element showing the Google Earth window
	greaterThan(QT_MAJOR_VERSION, 4) {
		QT += axcontainer
	} else {
		CONFIG += qaxcontainer
	}
} # end win32-msvc-*

win32-g++ {  # MinGW
	INCLUDEPATH += $$BASEDIR/libs/lib/sdl/include \
	LIBS += -L$$BASEDIR/libs/lib/sdl/win32 \
}


LIBS += -lSDLmain -lSDL -lsetupapi

contains(DEFINES, QGC_USE_VLC) {
	INCLUDEPATH += "libs/vlc/sdk/include"
	LIBS += -L$${BASEDIR}/libs/vlc/sdk/lib
	LIBS += -llibvlc
}

# xbee support
# libxbee only supported by linux and windows systems
#exists($$BASEDIR/libs/thirdParty/libxbee/lib) {
#	message("Building support for XBee")
#	DEPENDENCIES_PRESENT += xbee
#  DEFINES += XBEELINK
#  LIBS += -L$${BASEDIR}/libs/thirdParty/libxbee/lib \
#        -llibxbee
#}

#	exists($$BASEDIR/libs/lib/osg123) {
#		message("Building support for OSG")
#		DEPENDENCIES_PRESENT += osg

#		# Include OpenSceneGraph
#		INCLUDEPATH += $$BASEDIR/libs/lib/osgEarth/win32/include \
#			$$BASEDIR/libs/lib/osgEarth_3rdparty/win32/OpenSceneGraph-2.8.2/include
#		LIBS += -L$$BASEDIR/libs/lib/osgEarth_3rdparty/win32/OpenSceneGraph-2.8.2/lib \
#			-losg \
#			-losgViewer \
#			-losgGA \
#			-losgDB \
#			-losgText \
#			-lOpenThreads
#		DEFINES += QGC_OSG_ENABLED
#	}

# Include RT-LAB Library
#exists(src/lib/opalrt/OpalApi.h):exists(C:/OPAL-RT/RT-LAB7.2.4/Common/bin) {
#	message("Building support for Opal-RT")
#	DEPENDENCIES_PRESENT += rt-lab
#  DEFINES += OPAL_RT
#  LIBS += -LC:/OPAL-RT/RT-LAB7.2.4/Common/bin \
#        -lOpalApi
#}


# Support for Windows systems
# You have to install the official 3DxWare driver for Windows to use the 3D mouse support on Windows systems!
#exists($$BASEDIR/libs/thirdParty/3DMouse/win/) {
#  message("Including support for 3DxWare for Windows system.")
#	DEPENDENCIES_PRESENT += dxmouse
#  DEFINES += MOUSE_ENABLED_WIN
#}

