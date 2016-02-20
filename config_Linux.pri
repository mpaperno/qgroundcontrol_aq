#
# Linux specific configuration directives
#

DEFINES += __STDC_LIMIT_MACROS

QMAKE_CXXFLAGS_WARN_ON += -Wno-switch -Wno-enum-compare

INCLUDEPATH += /usr/include \
	  /usr/local/include \

LIBS += \
	-lm \
	-lSDL \
	-lSDLmain

# speech support
!contains(DEFINES, NO_TEXT_TO_SPEECH) {
	 exists(/usr/include/festival) {
		  INCLUDEPATH += /usr/include/festival \
				/usr/include/speech_tools
		  LIBS += -lFestival # -L/usr/lib/speech_tools
	 } else {
		  message("Warning: Could not find 'festival-dev' components, disabling test-to-speech support.")
		  DEFINES += NO_TEXT_TO_SPEECH
	 }
}

exists(/usr/local/include/libfreenect/libfreenect.h) {
	message("Building support for libfreenect")
	DEPENDENCIES_PRESENT += libfreenect
	INCLUDEPATH += /usr/include/libusb-1.0
	# Include libfreenect libraries
	LIBS += -lfreenect
	DEFINES += QGC_LIBFREENECT_ENABLED
}

###################################################################
#### --- 3DConnexion 3d Mice support (e.g. spacenavigator) --- ####
###################################################################

# xdrvlib only supported by linux (theoretical all X11) systems
# You have to install the official 3DxWare driver for linux to use 3D mouse support on linux systems!
exists(/usr/local/lib/libxdrvlib.so){
	message("Including support for Magellan 3DxWare for linux system.")
	SOURCES  += src/input/Mouse6dofInput.cpp
	HEADERS  += src/input/Mouse6dofInput.h
	LIBS += -L/usr/local/lib/ -lxdrvlib
	INCLUDEPATH *= /usr/local/include
	DEFINES += MOUSE_ENABLED_LINUX \
				  ParameterCheck                      # Hack: Has to be defined for magellan usage
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

#	exists(/usr/include/osg) | exists(/usr/local/include/osg) {
#		message("Building support for OpenSceneGraph")
#		DEPENDENCIES_PRESENT += osg
#		# Include OpenSceneGraph libraries
#		LIBS += -losg \
#            -losgViewer \
#            -losgGA \
#            -losgDB \
#            -losgText \
#            -lOpenThreads
#
#		DEFINES += QGC_OSG_ENABLED
#	}

#	exists(/usr/include/osg/osgQt) | exists(/usr/include/osgQt) |
#	exists(/usr/local/include/osg/osgQt) | exists(/usr/local/include/osgQt) {
#		message("Building support for OpenSceneGraph Qt")
#		DEPENDENCIES_PRESENT += osgQt
#		LIBS += -losgQt
#		DEFINES += QGC_OSG_QT_ENABLED
#	}

#	exists(/usr/local/include/google/protobuf) {
#		message("Building support for Protocol Buffers")
#		DEPENDENCIES_PRESENT += protobuf
#		# Include Protocol Buffers libraries
#		LIBS += -lprotobuf \
#            -lprotobuf-lite \
#            -lprotoc
#
#		DEFINES += QGC_PROTOBUF_ENABLED
#	}
