#
# Mac OS X specific configuration directives
#

#CONFIG += cocoa

#QMAKE_INFO_PLIST = $$PWD/Info.plist
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
QMAKE_MAC_SDK = macosx10.9
ICON = $$BASEDIR/files/images/icons/macx.icns
#QT += quickwidgets

Build64Bits {
	MACBITS = "mac64"
	CONFIG -= x86
	CONFIG += x86_64
	DEFINES -= USE_GOOGLE_EARTH_PLUGIN
} else {
	MACBITS = "mac32"
	DEFINES += USE_GOOGLE_EARTH_PLUGIN
}

INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers

LIBS += -framework IOKit \
	-F$$BASEDIR/libs/lib/Frameworks \
	-framework SDL \
	-framework CoreFoundation \
	-framework ApplicationServices \
	-lm

QMAKE_CXXFLAGS_WARN_OFF += -Wno-switch

exists(/opt/local/include/libfreenect)|exists(/usr/local/include/libfreenect) {
	message("Building support for libfreenect")
	DEPENDENCIES_PRESENT += libfreenect
	LIBS += -lfreenect
	DEFINES += QGC_LIBFREENECT_ENABLED
}

# No check for GLUT.framework since it's a MAC default
#	message("Building support for OpenSceneGraph")
#	DEPENDENCIES_PRESENT += osg
#	DEFINES += QGC_OSG_ENABLED
#	# Include OpenSceneGraph libraries
#	INCLUDEPATH += -framework GLUT \
#        -framework Cocoa \
#		  $$BASEDIR/libs/lib/$${MACBITS}/include
#
#	LIBS += -framework GLUT \
#		  -framework Cocoa \
#		  -L$$BASEDIR/libs/lib/$${MACBITS}/lib \
#        -lOpenThreads \
#        -losg \
#        -losgViewer \
#        -losgGA \
#        -losgDB \
#        -losgText \
#        -losgWidget
