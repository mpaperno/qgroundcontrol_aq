# -------------------------------------------------
# QGroundControl - Micro Air Vehicle Groundstation
# Please see our website at <http://qgroundcontrol.org>
# Maintainer:
# Lorenz Meier <lm@inf.ethz.ch>
# (c) 2009-2011 QGroundControl Developers
# This file is part of the open groundstation project
# QGroundControl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# QGroundControl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with QGroundControl. If not, see <http://www.gnu.org/licenses/>.
# -------------------------------------------------

message(Qt version $$[QT_VERSION])

# Turn off serial port warnings
DEFINES += _TTY_NOWARN_

# MAC OS X
macx|macx-g++42|macx-g++|macx-llvm: {

	CONFIG += cocoa
#	CONFIG += x86_64 cocoa phonon
#	CONFIG -= x86

	# For release builds remove support for various Qt debugging macros.
	CONFIG(release, debug|release) {
		DEFINES += QT_NO_DEBUG
	}

	MACBITS = "mac32"
	DEFINES += USE_GOOGLE_EARTH_PLUGIN
	*64 {
		message("Building Mac64 version")
		MACBITS = "mac64"
		DEFINES -= USE_GOOGLE_EARTH_PLUGIN
	}

	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

	INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers

	LIBS += -framework IOKit \
#		-F$$BASEDIR/libs/lib/Frameworks \
		-framework SDL \
		-framework CoreFoundation \
		-framework ApplicationServices \
		-lm

	ICON = $$BASEDIR/files/images/icons/macx.icns

	QMAKE_POST_LINK += $$quote(echo "Copying files")

	# Copy AQ files
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/bin/aq_osx_all/* $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && chmod +x $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/aq/bin/*
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/aq/mixes
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/mixes/* $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/aq/mixes

	# Copy google earth starter file
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/*.* $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/files
	# Copy audio files
	QMAKE_POST_LINK += && cp -r $$BASEDIR/files/audio $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/files
	 # Copy language files
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/lang/*.qm $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/files/lang
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/lang/flags $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/files/lang
	# Copy libraries
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/libs
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/libs/lib/$${MACBITS}/lib/* $$TARGETDIR/qgroundcontrol.app/Contents/libs
	# Copy frameworks
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/Frameworks
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/libs/lib/Frameworks/* $$TARGETDIR/qgroundcontrol.app/Contents/Frameworks


	# Fix library paths inside executable
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosgViewer.dylib "@executable_path/../libs/libosgViewer.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
	QMAKE_POST_LINK += && install_name_tool -change libosgWidget.dylib "@executable_path/../libs/libosgWidget.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol

	# Fix library paths within libraries (inter-library dependencies)

	# OSG GA LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib

	# OSG DB LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib

	# OSG TEXT LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib

	# OSG UTIL LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgUtil.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgUtil.dylib


	# OSG VIEWER LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib

	# OSG WIDGET LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
	QMAKE_POST_LINK += && install_name_tool -change libosgViewer.dylib "@executable_path/../libs/libosgViewer.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib

	# CORE OSG LIBRARY
	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosg.dylib

	# SDL Framework
	QMAKE_POST_LINK += && install_name_tool -change "@rpath/SDL.framework/Versions/A/SDL" "@executable_path/../Frameworks/SDL.framework/Versions/A/SDL" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol

	# No check for GLUT.framework since it's a MAC default
	message("Building support for OpenSceneGraph")
	DEPENDENCIES_PRESENT += osg
	DEFINES += QGC_OSG_ENABLED
	# Include OpenSceneGraph libraries
	INCLUDEPATH += -framework GLUT \
        -framework Cocoa \
		  $$BASEDIR/libs/lib/$${MACBITS}/include

	LIBS += -framework GLUT \
		  -framework Cocoa \
		  -L$$BASEDIR/libs/lib/$${MACBITS}/lib \
        -lOpenThreads \
        -losg \
        -losgViewer \
        -losgGA \
        -losgDB \
        -losgText \
        -losgWidget

        #exists(/usr/local/include/google/protobuf) {
        #	message("Building support for Protocol Buffers")
        #	DEPENDENCIES_PRESENT += protobuf
        #	# Include Protocol Buffers libraries
        #	LIBS += -L/usr/local/lib \
        #    -lprotobuf \
        #    -lprotobuf-lite \
        #    -lprotoc
        #
        #	DEFINES += QGC_PROTOBUF_ENABLED
        #}

	exists(/opt/local/include/libfreenect)|exists(/usr/local/include/libfreenect) {
		message("Building support for libfreenect")
		DEPENDENCIES_PRESENT += libfreenect
		# Include libfreenect libraries
		LIBS += -lfreenect
		DEFINES += QGC_LIBFREENECT_ENABLED
	}
}

# GNU/Linux
linux-g++|linux-g++-64{

	CONFIG -= console
	DEFINES += __STDC_LIMIT_MACROS
	DESTDIR = $$TARGETDIR

	release {
		DEFINES += QT_NO_DEBUG
	}

	INCLUDEPATH += /usr/include \
		  /usr/local/include \

	LIBS += \
		-L/usr/lib \
		-L/usr/local/lib64 \
		-lm \
#		-lflite_cmu_us_kal \
#		-lflite_usenglish \
#		-lflite_cmulex \
#		-lflite \
		-lSDL \
		-lSDLmain

	exists(/usr/include/osg) | exists(/usr/local/include/osg) {
		message("Building support for OpenSceneGraph")
		DEPENDENCIES_PRESENT += osg
		# Include OpenSceneGraph libraries
		LIBS += -losg \
            -losgViewer \
            -losgGA \
            -losgDB \
            -losgText \
            -lOpenThreads

		DEFINES += QGC_OSG_ENABLED
	}

	exists(/usr/include/osg/osgQt) | exists(/usr/include/osgQt) |
	exists(/usr/local/include/osg/osgQt) | exists(/usr/local/include/osgQt) {
		message("Building support for OpenSceneGraph Qt")
		# Include OpenSceneGraph Qt libraries
		LIBS += -losgQt
		DEFINES += QGC_OSG_QT_ENABLED
	}

	exists(/usr/local/include/google/protobuf) {
		message("Building support for Protocol Buffers")
		DEPENDENCIES_PRESENT += protobuf
		# Include Protocol Buffers libraries
		LIBS += -lprotobuf \
            -lprotobuf-lite \
            -lprotoc

		DEFINES += QGC_PROTOBUF_ENABLED
	}

	exists(/usr/local/include/libfreenect/libfreenect.h) {
		message("Building support for libfreenect")
		DEPENDENCIES_PRESENT += libfreenect
		INCLUDEPATH += /usr/include/libusb-1.0
		# Include libfreenect libraries
		LIBS += -lfreenect
		DEFINES += QGC_LIBFREENECT_ENABLED
	}


	QMAKE_POST_LINK += $$quote(echo "Copying files")

	# Validated copy commands
	!exists($$TARGETDIR){
		QMAKE_POST_LINK += && mkdir -p $$TARGETDIR
	}

	# Copy AQ files
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/aq/bin
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/bin/aq_unix_all/* $$TARGETDIR/aq/bin
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/aq/mixes
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/mixes/* $$TARGETDIR/aq/mixes
	QMAKE_POST_LINK += && chmod +x $$TARGETDIR/aq/bin/*
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/*.* $$TARGETDIR/files
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/audio $$TARGETDIR/files/audio
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/lang/*.qm $$TARGETDIR/files/lang
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/lang/flags $$TARGETDIR/files/lang/flags
	#QMAKE_POST_LINK += && cp -rf $$BASEDIR/data $$TARGETDIR

	# osg/osgEarth dynamic casts might fail without this compiler option.
	# see http://osgearth.org/wiki/FAQ for details.
	QMAKE_CXXFLAGS += -Wl,-E
}

linux-g++ {
	message("Building for GNU/Linux 32bit/i386")
        QMAKE_POST_LINK += && mv -f $$TARGETDIR/aq/bin/cal_32 $$TARGETDIR/aq/bin/cal
        QMAKE_POST_LINK += && mv -f $$TARGETDIR/aq/bin/sim3_32 $$TARGETDIR/aq/bin/sim3
}
linux-g++-64 {
	message("Building for GNU/Linux 64bit/x64 (g++-64)")
	exists(/usr/local/lib64) {
		LIBS += -L/usr/local/lib64
	}
        QMAKE_POST_LINK += && mv -f $$TARGETDIR/aq/bin/cal_64 $$TARGETDIR/aq/bin/cal
        QMAKE_POST_LINK += && mv -f $$TARGETDIR/aq/bin/sim3_64 $$TARGETDIR/aq/bin/sim3
}

# Windows (32bit), Visual Studio
win32-msvc2010|win32-msvc2012|win32-g++ {

	win32-msvc2010 {
		message(Building for Windows Visual Studio 2010 (32bit))
	}
	win32-msvc2012 {
		message(Building for Windows Visual Studio 2012 (32bit))
	}
	win32-g++ {
		message(Building for Windows GCC (32bit))
	}

	# Specify multi-process compilation within Visual Studio.
	# (drastically improves compilation times for multi-core computers)
	QMAKE_CFLAGS_DEBUG += /MP
	QMAKE_CXXFLAGS_DEBUG += /MP
	QMAKE_CFLAGS_RELEASE += /MP
	QMAKE_CXXFLAGS_RELEASE += /MP

	# QAxContainer support is needed for the Internet Control
	# element showing the Google Earth window
	greaterThan(QT_MAJOR_VERSION, 4) {
		QT += axcontainer
	} else {
		CONFIG += qaxcontainer
	}

	DEFINES += USE_GOOGLE_EARTH_PLUGIN

	# The EIGEN library needs this define
	# to make the internal min/max functions work
	DEFINES += NOMINMAX

	# QWebkit is not needed on MS-Windows compilation environment
	CONFIG -= webkit webkitwidgets

	# Specify the inclusion of (U)INT*_(MAX/MIN) macros within Visual Studio
	DEFINES += __STDC_LIMIT_MACROS

	# For release builds remove support for various Qt debugging macros.
	CONFIG(release, debug|release) {
		DEFINES += QT_NO_DEBUG
	}

	# For debug releases we just want the debugging console.
	CONFIG(debug, debug|release) {
		CONFIG += console
	}

	INCLUDEPATH += $$BASEDIR/libs/lib/sdl/msvc/include \
        $$BASEDIR/libs/lib/opal/include \
        $$BASEDIR/libs/lib/msinttypes

	LIBS += -L$$BASEDIR/libs/lib/sdl/msvc/lib \
        -lSDLmain -lSDL \
        -lsetupapi

	exists($$BASEDIR/libs/lib/osg123) {
		message("Building support for OSG")
		DEPENDENCIES_PRESENT += osg

		# Include OpenSceneGraph
		INCLUDEPATH += $$BASEDIR/libs/lib/osgEarth/win32/include \
			$$BASEDIR/libs/lib/osgEarth_3rdparty/win32/OpenSceneGraph-2.8.2/include
		LIBS += -L$$BASEDIR/libs/lib/osgEarth_3rdparty/win32/OpenSceneGraph-2.8.2/lib \
			-losg \
			-losgViewer \
			-losgGA \
			-losgDB \
			-losgText \
			-lOpenThreads
		DEFINES += QGC_OSG_ENABLED
	}

	RC_FILE = $$BASEDIR/qgroundcontrol.rc

	# Copy dependencies
	BASEDIR_WIN = $$replace(BASEDIR,"/","\\")
	TARGETDIR_WIN = $$replace(TARGETDIR,"/","\\")\\release
	greaterThan(QT_MAJOR_VERSION, 4) {
		QTLIBDLLPFX = "Qt5"
		QTLIBDLLSFX = ".dll"
	} else {
		QTLIBDLLPFX = "Qt"
		QTLIBDLLSFX = "4.dll"
	}
	CONFIG(debug, debug|release) {
		TARGETDIR_WIN = $$replace(TARGETDIR,"/","\\")\\debug
		greaterThan(QT_MAJOR_VERSION, 4) {
			QTLIBDLLSFX = "d.dll"
		} else {
			QTLIBDLLSFX = "d4.dll"
		}
	}

	QMAKE_POST_LINK += $$quote(echo "Copying files"$$escape_expand(\\n))

	# Copy AQ files
	QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\aq\\bin\\aq_win_all\\*" "$$TARGETDIR_WIN\\aq\\bin" /E /I $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\aq\\mixes\\*" "$$TARGETDIR_WIN\\aq\\mixes" /E /I $$escape_expand(\\n))

	# Copy VLC files  --  just install VNC instaed!
#	contains(DEFINES, QGC_USE_VLC) {
#		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\vlc\\plugins\\*"  "$$TARGETDIR_WIN\\plugins" /E /I $$escape_expand(\\n))
#		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\vlc\\libvlccore.dll" "$$TARGETDIR_WIN"$$escape_expand(\\n))
#		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\vlc\\libvlc.dll" "$$TARGETDIR_WIN"$$escape_expand(\\n))
#	}

	# Copy application resources
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /I "$$BASEDIR_WIN\\files\\*.*" "$$TARGETDIR_WIN\\files\\" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\files\\audio" "$$TARGETDIR_WIN\\files\\audio" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /I "$$BASEDIR_WIN\\files\\lang\\*.qm" "$$TARGETDIR_WIN\\files\\lang" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\files\\lang" "$$TARGETDIR_WIN\\files\\lang" $$escape_expand(\\n))

	CONFIG(release, debug|release) {
		# Copy supporting library DLLs
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\lib\\sdl\\win32\\SDL.dll" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		#QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\thirdParty\\libxbee\\lib\\libxbee.dll" "$$TARGETDIR_WIN"$$escape_expand(\\n))

		# Copy Qt DLLs
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\plugins\\imageformats\\*$${QTLIBDLLSFX}" "$$TARGETDIR_WIN\\imageformats" /E /I $$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\plugins\\iconengines\\*$${QTLIBDLLSFX}" "$$TARGETDIR_WIN\\iconengines" /E /I $$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\plugins\\sqldrivers\\*$${QTLIBDLLSFX}" "$$TARGETDIR_WIN\\sqldrivers" /E /I $$escape_expand(\\n))
		CONFIG(release, debug|release) {
		}
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Core$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Gui$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Multimedia$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Network$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}OpenGL$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Sql$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Svg$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Test$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}WebKit$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Xml$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}XmlPatterns$$QTLIBDLLSFX" "$$TARGETDIR_WIN"$$escape_expand(\\n))

		# clean up stuff not needed by release versions
		QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\imageformats\\*d4.dll" $$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\iconengines\\*d4.dll" $$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\sqldrivers\\*d4.dll" $$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\qgroundcontrol.exp"$$escape_expand(\\n))
		QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\qgroundcontrol.lib"$$escape_expand(\\n))

		# Copy Visual Studio DLLs
		win32-msvc2010 {
			QMAKE_POST_LINK += $$quote(xcopy /D /Y "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 10.0\\VC\\redist\\x86\\Microsoft.VC100.CRT\\*.dll\""  "$$TARGETDIR_WIN\\"$$escape_expand(\\n))
		}
		win32-msvc2012 {
			 QMAKE_POST_LINK += $$quote(xcopy /D /Y "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 11.0\\VC\\redist\\x86\\Microsoft.VC110.CRT\\*.dll\""  "$${TARGETDIR}\\"$$escape_expand(\\n))
		}
		win32-g++ {
			# we need to know where MinGW lives so we can copy some DLLs from there.
			MINGW_PATH = $$(MINGW_PATH)
			isEmpty(MINGW_PATH): error("MINGW_PATH not found")
			QMAKE_POST_LINK  += $$quote(xcopy /D /Y "$${MINGW_PATH}\\bin\\libwinpthread-1.dll"  "$${TARGETDIR}"$$escape_expand(\\n\\t))
			QMAKE_POST_LINK  += $$quote(xcopy /D /Y "$${MINGW_PATH}\\bin\\libstdc++-6.dll"  "$${TARGETDIR}"$$escape_expand(\\n))
		}
	}
}
