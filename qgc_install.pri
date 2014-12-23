# -------------------------------------------------
# QGroundControl AQ - AutoQuad Groundstation
# Based on QGroundControl - Micro Air Vehicle Groundstation
# Extensively Modifeid For AutoQuad flight controller
# Please see our website at <http://autoquad.org>
# Please see the original QGroundControl website
# at <http://qgroundcontrol.org>
#
# QGroundControl Creator:
# Lorenz Meier <lm@inf.ethz.ch>
# (c) 2009-2011 QGroundControl Developers
#
# AutoQuad Maintainer:
# Maxim Paperno <MPaperno@WorldDesign.com>
# (c) 2013-2014 Maxim Paperno
#
# Original Conversion for AutoQuad
# 2012-2013 by Peter Hafner
#
#
# This file is part of the open source AutoQuad project.
# QGroundControl AQ is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# QGroundControl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with QGroundControl AQ. If not, see <http://www.gnu.org/licenses/>.
# -------------------------------------------------

TARGETDIR = $$DESTDIR

# MAC OS X
MacBuild: {

	TARGETDIR = $${TARGETDIR}/$${TARGET}.app

	QMAKE_POST_LINK += $$quote(echo "Copying files")

	# Copy AQ files
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/bin/aq_osx_all/* $$TARGETDIR/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && chmod +x $$TARGETDIR/Contents/MacOS/aq/bin/*
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/Contents/MacOS/aq/mixes
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/mixes/* $$TARGETDIR/Contents/MacOS/aq/mixes

	# Copy google earth starter file
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/Contents/MacOS/files
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/*.* $$TARGETDIR/Contents/MacOS/files
	# Copy audio files
	QMAKE_POST_LINK += && cp -r $$BASEDIR/files/audio $$TARGETDIR/Contents/MacOS/files/.
	# Copy language files
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/Contents/MacOS/files/lang
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/lang/*.qm $$TARGETDIR/Contents/MacOS/files/lang
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/lang/flags $$TARGETDIR/Contents/MacOS/files/lang/.
	# Copy libraries
	#QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/libs
	#QMAKE_POST_LINK += && cp -rf $$BASEDIR/libs/lib/$${MACBITS}/lib/* $$TARGETDIR/qgroundcontrol.app/Contents/libs
	# Copy frameworks
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/qgroundcontrol.app/Contents/Frameworks
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/libs/lib/Frameworks/* $$TARGETDIR/Contents/Frameworks

	# SDL Framework
	QMAKE_POST_LINK += && install_name_tool -change "@rpath/SDL.framework/Versions/A/SDL" "@executable_path/../Frameworks/SDL.framework/Versions/A/SDL" $$TARGETDIR/Contents/MacOS/$$TARGET


	# Fix library paths inside executable
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosgViewer.dylib "@executable_path/../libs/libosgViewer.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol
#	QMAKE_POST_LINK += && install_name_tool -change libosgWidget.dylib "@executable_path/../libs/libosgWidget.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/MacOS/qgroundcontrol

	# Fix library paths within libraries (inter-library dependencies)

	# OSG GA LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgGA.dylib

	# OSG DB LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgDB.dylib

	# OSG TEXT LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgText.dylib

	# OSG UTIL LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgUtil.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgUtil.dylib


	# OSG VIEWER LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgViewer.dylib

	# OSG WIDGET LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libosgGA.dylib "@executable_path/../libs/libosgGA.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgText.dylib "@executable_path/../libs/libosgText.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgDB.dylib "@executable_path/../libs/libosgDB.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgUtil.dylib "@executable_path/../libs/libosgUtil.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosg.dylib "@executable_path/../libs/libosg.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib
#	QMAKE_POST_LINK += && install_name_tool -change libosgViewer.dylib "@executable_path/../libs/libosgViewer.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosgWidget.dylib

	# CORE OSG LIBRARY
#	QMAKE_POST_LINK += && install_name_tool -change libOpenThreads.dylib "@executable_path/../libs/libOpenThreads.dylib" $$TARGETDIR/qgroundcontrol.app/Contents/libs/libosg.dylib

	DoMacDeploy: QMAKE_POST_LINK += && $$dirname(QMAKE_QMAKE)/macdeployqt $$TARGETDIR

}


# GNU/Linux
LinuxBuild{


	QMAKE_POST_LINK += $$quote(echo "Copying files")

	# Validated copy commands
	!exists($$TARGETDIR){
		QMAKE_POST_LINK += && mkdir -p $$TARGETDIR
	}

	# Copy AQ and supporting files
	Build32Bits {
		exists(/usr/local):LIBS += -L/usr/local
	}
	Build64Bits {
		exists(/usr/local/lib64):LIBS += -L/usr/local/lib64
	}
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/aq/bin
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/bin/aq_unix_all/* $$TARGETDIR/aq/bin
	QMAKE_POST_LINK += && mkdir -p $$TARGETDIR/aq/mixes
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/aq/mixes/* $$TARGETDIR/aq/mixes
	QMAKE_POST_LINK += && chmod +x $$TARGETDIR/aq/bin/*
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/*.* $$TARGETDIR/files
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/audio $$TARGETDIR/files/audio
	QMAKE_POST_LINK += && cp -f $$BASEDIR/files/lang/*.qm $$TARGETDIR/files/lang
	QMAKE_POST_LINK += && cp -rf $$BASEDIR/files/lang/flags $$TARGETDIR/files/lang/flags

	# osg/osgEarth dynamic casts might fail without this compiler option.
	# see http://osgearth.org/wiki/FAQ for details.
	#QMAKE_CXXFLAGS += -Wl,-E
}

# Windows
WinBuild {

	# Copy dependencies
	BASEDIR_WIN = $$replace(BASEDIR,"/","\\")
	TARGETDIR_WIN = $$replace(TARGETDIR,"/","\\")

	greaterThan(QT_MAJOR_VERSION, 4) {
		QTLIBDLLPFX = "Qt5"
		QTLIBDBGDLLSFX = "d.dll"
	} else {
		QTLIBDLLPFX = "Qt"
		QTLIBDBGDLLSFX = "d4.dll"
	}

	QMAKE_POST_LINK += $$quote(echo "Copying files"$$escape_expand(\\n))

	# Copy AQ files
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\aq\\bin\\aq_win_all\\*" "$$TARGETDIR_WIN\\aq\\bin" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\aq\\mixes\\*" "$$TARGETDIR_WIN\\aq\\mixes" $$escape_expand(\\n))
	# Copy application resources
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /I "$$BASEDIR_WIN\\files\\*.*" "$$TARGETDIR_WIN\\files\\" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\files\\audio" "$$TARGETDIR_WIN\\files\\audio" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /I "$$BASEDIR_WIN\\files\\lang\\*.qm" "$$TARGETDIR_WIN\\files\\lang" $$escape_expand(\\n))
	QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$BASEDIR_WIN\\files\\lang" "$$TARGETDIR_WIN\\files\\lang" $$escape_expand(\\n))

	ReleaseBuild {

		COPY_DLL_LIST = \
			$$BASEDIR_WIN\\libs\\lib\\sdl\\win32\\SDL.dll \
			$$(QTDIR)\\bin\\icu*.dll \
#			$$BASEDIR_WIN\\libs\\thirdParty\\libxbee\\lib\\libxbee.dll \

		QT_DLL_LIST = Core Gui Multimedia Network OpenGL Sql Svg Test WebKit Xml XmlPatterns
		QT_PLUGIN_LIST = imageformats iconengines sqldrivers
		greaterThan(QT_MAJOR_VERSION, 4) {
			QT_DLL_LIST += Concurrent MultimediaWidgets Positioning PrintSupport Qml Quick Sensors WebChannel WebKitWidgets Widgets
			QT_PLUGIN_LIST += audio mediaservice platforms
		}
		for(QT_DLL, QT_DLL_LIST) {
			COPY_DLL_LIST += $$(QTDIR)\\bin\\$${QTLIBDLLPFX}$${QT_DLL}.dll
		}

		# Copy compiler-specific DLLs
		win32-msvc2010: COPY_DLL_LIST += "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 10.0\\VC\\redist\\x86\\Microsoft.VC100.CRT\\msvc?100.dll\""
		win32-msvc2012: COPY_DLL_LIST += "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 11.0\\VC\\redist\\x86\\Microsoft.VC110.CRT\\msvc?110.dll\""
		win32-msvc2012: COPY_DLL_LIST += "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 12.0\\VC\\redist\\x86\\Microsoft.VC120.CRT\\msvc?120.dll\""
		win32-g++ {
			# we need to know where MinGW lives so we can copy some DLLs from there.
			MINGW_PATH = $$(MINGW_PATH)
			isEmpty(MINGW_PATH): error("MINGW_PATH not found")
			COPY_DLL_LIST += $${MINGW_PATH}\\bin\\libwinpthread-1.dll
			COPY_DLL_LIST += $${MINGW_PATH}\\bin\\libstdc++-6.dll
		}

		# Copy VLC files
		contains(DEFINES, QGC_USE_VLC) {
			#QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$BASEDIR_WIN\\libs\\vlc\\plugins\\*"  "$$TARGETDIR_WIN\\plugins" /E /I $$escape_expand(\\n))
			COPY_DLL_LIST += $$BASEDIR_WIN\\libs\\vlc\\libvlccore.dll
			COPY_DLL_LIST += $$BASEDIR_WIN\\libs\\vlc\\libvlc.dll
		}

		# Copy all DLLs to base folder
		for(COPY_DLL, COPY_DLL_LIST) {
			QMAKE_POST_LINK += $$quote(xcopy /D /Y "$${COPY_DLL}" "$$TARGETDIR_WIN"$$escape_expand(\\n))
		}
		# Copy all QT plugin DLLs and delete the debug ones
		for(QT_PLUGIN, QT_PLUGIN_LIST) {
			QMAKE_POST_LINK += $$quote(xcopy /D /Y /E /I "$$(QTDIR)\\plugins\\$${QT_PLUGIN}\\*.dll" "$$TARGETDIR_WIN\\$${QT_PLUGIN}" $$escape_expand(\\n))
			QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\$${QT_PLUGIN}\\*$${QTLIBDBGDLLSFX}" $$escape_expand(\\n))
		}
		# clean up stuff not needed by release versions
		greaterThan(QT_MAJOR_VERSION, 4) {
			QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\*.manifest" $$escape_expand(\\n))
			QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\platforms\\qminimal*" $$escape_expand(\\n))
			QMAKE_POST_LINK += $$quote(del /Q "$$TARGETDIR_WIN\\platforms\\qoffscreen*" $$escape_expand(\\n))
		} else {
			QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\qgroundcontrol.exp" $$escape_expand(\\n))
			QMAKE_POST_LINK += $$quote(del /F "$$TARGETDIR_WIN\\qgroundcontrol.lib" $$escape_expand(\\n))
		}

	}  # end if release version
}
