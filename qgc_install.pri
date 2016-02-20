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
# (c) 2013-2015 Maxim Paperno
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

greaterThan(QT_MAJOR_VERSION, 4) {
	QT_DLL_LIST = Qt5Core Qt5Gui Qt5Widgets Qt5Multimedia Qt5Network Qt5OpenGL Qt5Sql Qt5Svg Qt5Test Qt5WebKit Qt5WebKitWidgets Qt5Xml Qt5XmlPatterns
	QT_DLL_LIST += Qt5Concurrent Qt5MultimediaWidgets Qt5Positioning Qt5PrintSupport Qt5Qml Qt5Quick Qt5Sensors Qt5WebChannel
	QT_PLUGIN_LIST += imageformats iconengines sqldrivers audio mediaservice platforms bearer
} else {
	QT_DLL_LIST = QtCore4 QtGui4 QtMultimedia4 QtNetwork4 QtOpenGL4 QtSql4 QtSvg4 QtTest4 QtWebKit4 QtXml4 QtXmlPatterns4
	QT_PLUGIN_LIST = imageformats iconengines sqldrivers
}
QT_DLL_LIST += icu*

# copy AQ binary utils based on OS type
SOURCE_BINFILES_PATH = $$BASEDIR/aq/bin
WinBuild:SOURCE_BINFILES_PATH = $${SOURCE_BINFILES_PATH}/aq_win_all/*.*
MacBuild {
		  SOURCE_BINFILES_PATH = $${SOURCE_BINFILES_PATH}/aq_osx_all/*
		  binfiles.extra = chmod +x $$SOURCE_BINFILES_PATH
}
LinuxBuild {
		  SOURCE_BINFILES_PATH = $${SOURCE_BINFILES_PATH}/aq_unix_all/*
		  binfiles.extra = chmod +x $$SOURCE_BINFILES_PATH
}

WinBuild:ReleaseBuild {
	target.path = $${TARGETDIR}

	target.files += $$BASEDIR/libs/lib/sdl/win32/SDL.dll
	contains(DEFINES, QGC_USE_VLC) {
		# Copy VLC files
		#target.files += $$BASEDIR_WIN/libs/vl/plugins/*.dll
		target.files += $$BASEDIR/libs/vlc/libvlc*.dll
	}

	# Copy compiler-specific DLLs
	win32-msvc* {
		win32-msvc2010:MSVC_SHORT_VERSION = "100"
		win32-msvc2012:MSVC_SHORT_VERSION = "110"
		win32-msvc2013:MSVC_SHORT_VERSION = "120"
		#MSVC_ENV_VAR = "VS"$${MSVC_SHORT_VERSION}"COMNTOOLS"
		#MSVC_REDIST_DLL_PATH = $$getenv($${MSVC_ENV_VAR})  # qmake complains that $$getenv() is not valid even though it is documented and works
		win32-msvc2010:MSVC_REDIST_DLL_PATH = $(VS100COMNTOOLS)
		win32-msvc2012:MSVC_REDIST_DLL_PATH = $(VS110COMNTOOLS)
		win32-msvc2013:MSVC_REDIST_DLL_PATH = $(VS120COMNTOOLS)
		MSVC_REDIST_DLL_PATH ~= s,Common7.{1}Tools.{1},VC\redist\x86\Microsoft.VC$${MSVC_SHORT_VERSION}.CRT\,ig
		#isEmpty(MSVC_REDIST_DLL_PATH): error("MSVC directory not found in" $$MSVC_ENV_VAR "environment variable")
		target.files += $${MSVC_REDIST_DLL_PATH}msvc?1?0.dll
	}
	# MinGW DLLs can be found in QT bins install root
	win32-g++:QT_DLL_LIST += libgcc* libwin* libstd~1

	for(QT_DLL, QT_DLL_LIST) {
		target.files += $$[QT_INSTALL_BINS]/$${QT_DLL}.dll
	}

	INSTALLS += target

	for(QT_PLUGIN, QT_PLUGIN_LIST) {
		qtplugins_$${QT_PLUGIN}.path = $${TARGETDIR}/$${QT_PLUGIN}
		qtplugins_$${QT_PLUGIN}.files = $$[QT_INSTALL_PLUGINS]/$${QT_PLUGIN}/*.dll
		INSTALLS += qtplugins_$${QT_PLUGIN}
	}
	for(QT_PLUGIN, QT_PLUGIN_LIST) {
		qtpluginsclean_$${QT_PLUGIN}.path = $${TARGETDIR}/$${QT_PLUGIN}
		qtpluginsclean_$${QT_PLUGIN}.extra = del /Q $$replace(TARGETDIR, /, \\)\\$${QT_PLUGIN}\\*d.dll
		INSTALLS += qtpluginsclean_$${QT_PLUGIN}
	}

	# clean up stuff not needed by release versions
	cleanup.path = $${TARGETDIR}
	greaterThan(QT_MAJOR_VERSION, 4) {
		cleanup.extra = del /Q /F $$replace(TARGETDIR, /, \\)\\*.manifest && del /Q /F $$replace(TARGETDIR, /, \\)\\platforms\\qminimal* && del /Q /F $$replace(TARGETDIR, /, \\)\\platforms\\qoffscreen*
	} else {
		cleanup.extra = del /Q /F $$replace(TARGETDIR, /, \\)\\$${TARGET}.exp && del /Q /F $$replace(TARGETDIR, /, \\)\\$${TARGET}.lib
	}
	INSTALLS += cleanup
}

MacBuild {
	TARGETDIR = $${TARGETDIR}/$${TARGET}.app
	macdeploy.path = $$TARGETDIR
	macdeploy.extra = macdeployqt $$TARGETDIR

	frameworks.path = $$TARGETDIR/Contents/Frameworks
	frameworks.files = $$BASEDIR/libs/lib/Frameworks/*
	frameworks_nametool.path = $$TARGETDIR
	frameworks_nametool.extra = install_name_tool -change "@rpath/SDL.framework/Versions/A/SDL" "@executable_path/../Frameworks/SDL.framework/Versions/A/SDL" $$TARGETDIR/Contents/MacOS/$$TARGET
	INSTALLS += frameworks frameworks_nametool

	# extend target dir for subsequent file copy ops
	TARGETDIR = $${TARGETDIR}/Contents/MacOS
}

LinuxBuild:ReleaseBuild {
	QMAKE_LFLAGS_RPATH =
	QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN/qtlibs'"

	Build32Bits:exists(/usr/local):LIBS += -L/usr/local
	Build64Bits:exists(/usr/local/lib64):LIBS += -L/usr/local/lib64

	greaterThan(QT_MAJOR_VERSION, 4) {
		QT_DLL_LIST += Qt5DBus Qt5XcbQpa
		QT_PLUGIN_LIST += platformthemes
	}

	target.path = $${TARGETDIR}/qtlibs
	for(QT_DLL, QT_DLL_LIST) {
		target.files += $$[QT_INSTALL_LIBS]/lib$${QT_DLL}.so.5
	}
	target.files += $$[QT_INSTALL_LIBS]/libicu*.so.54

	for(QT_PLUGIN, QT_PLUGIN_LIST) {
		qtplugins_$${QT_PLUGIN}.path = $${TARGETDIR}/$${QT_PLUGIN}
		qtplugins_$${QT_PLUGIN}.files = $$[QT_INSTALL_PLUGINS]/$${QT_PLUGIN}/lib*.so
		INSTALLS += qtplugins_$${QT_PLUGIN}
	}

	iconfiles.path = $${TARGETDIR}
	iconfiles.files = $${ICONS}
	etcfiles.path = $${TARGETDIR}/files/etc
	etcfiles.files = $${BASEDIR}/files/etc/*

	post_target.path = $${TARGETDIR}
	post_target.extra = chrpath -r "'\$$ORIGIN/../qtlibs'" $${TARGETDIR}/platforms/libqxcb.so

	readmes.files += $${BASEDIR}/README-Linux.txt

	INSTALLS += target iconfiles etcfiles post_target
}

binfiles.path = $${TARGETDIR}/aq/bin
binfiles.files = $${SOURCE_BINFILES_PATH}
mixes.path = $${TARGETDIR}/aq/mixes
mixes.files = $${BASEDIR}/aq/mixes/*
extrafiles.path = $${TARGETDIR}/files
extrafiles.files = $${BASEDIR}/files/*.*
audiofiles.path = $${TARGETDIR}/files/audio
audiofiles.files = $${BASEDIR}/files/audio/*.*

INSTALLS += binfiles mixes extrafiles audiofiles

ReleaseBuild {
	langfiles.path = $${TARGETDIR}/files/lang
	# lupdate reports two errors in external library files (one from Win7 SDK and another from VLC lib) which makes Creator report errors during a build, which is annoying
	#langfiles.extra = $$[QT_INSTALL_BINS]/lupdate -silent -no-obsolete $${_PRO_FILE_} && $$[QT_INSTALL_BINS]/lrelease $${_PRO_FILE_}
	langfiles.extra = $$[QT_INSTALL_BINS]/lrelease $${_PRO_FILE_}
	langfiles.files = $${BASEDIR}/files/lang/*.qm
	flagfiles.path = $${TARGETDIR}/files/lang/flags
	flagfiles.files = $${BASEDIR}/files/lang/flags/*

	cssfiles.path = $${TARGETDIR}/files/styles
	cssfiles.files = $${BASEDIR}/files/styles/*.css

	readmes.path = $${TARGETDIR}
	readmes.files += $${BASEDIR}/CHANGES.txt

	INSTALLS += langfiles flagfiles cssfiles readmes

	MacBuild:DoMacDeploy:INSTALLS += macdeploy
}
