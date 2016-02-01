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

message(Qt version $$[QT_VERSION])

TEMPLATE = app
TARGET = qgroundcontrol_aq

## Set up some config vars for ease of use
# platform
linux-g++* {
	message(Linux build)
	CONFIG += LinuxBuild
} else : win* {
	message(Windows build)
	CONFIG += WinBuild
} else : macx* {
	message(Mac build)
	CONFIG += MacBuild
	CONFIG += DoMacDeploy
} else {
	error(Unsupported build type)
}
# debug/release
CONFIG(debug, debug|release) {
	message(Debug build)
	CONFIG += DebugBuild
	DEFINES += QT_DEBUG
} else:CONFIG(release, debug|release) {
	message(Release build)
	CONFIG += ReleaseBuild
	DEFINES += QT_NO_DEBUG
} else {
	error(Unsupported build type)
}
# bitness
*64 {
	message(64-bit build)
	CONFIG += Build64Bits
} else {
	message(32-bit build)
	CONFIG += Build32Bits
}

# Qt configuration
QT += network \
	 opengl \
	 svg \
	 xml \
	 webkit \
	 sql

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets webkitwidgets multimedia printsupport concurrent
} else {
	CONFIG += qt thread
}

# build directories
DESTDIR = $${OUT_PWD}/bin
BUILDDIR = $${OUT_PWD}/build
OBJECTS_DIR = $${BUILDDIR}/obj
MOC_DIR = $${BUILDDIR}/moc
UI_DIR = $${BUILDDIR}/ui
RCC_DIR = $${BUILDDIR}/rcc
# root of project files, used later in install step
BASEDIR = $$_PRO_FILE_PWD_

## Misc definitions & settings

# Make sure custom qDebug() handler has context available
DEFINES *= QT_MESSAGELOGCONTEXT
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES += MAVLINK_NO_DATA
# set to build w/out QtSpeech
#DEFINES += NO_TEXT_TO_SPEECH
# set to build with VLC support # Windows only #
#DEFINES += QGC_USE_VLC
# Turn off serial port warnings
#DEFINES += _TTY_NOWARN_

# if the variable MAVLINK_CONF contains the name of an
# additional project, QGroundControl includes the support
# of custom MAVLink messages of this project. It will also
# create a QGC_USE_{AUTOPILOT_NAME}_MESSAGES macro for use within the actual code.
MAVLINK_CONF = "autoquad"
MAVLINKPATH = $$BASEDIR/libs/mavlink/include

LANGUAGE = C++
CODECFORTR = UTF-8
CODECFORSRC = UTF-8

# If a user config file exists, it will be included.
exists(user_config.pri) {
	 include(user_config.pri)
	 message("----- USING CUSTOM USER QGROUNDCONTROL CONFIG FROM user_config.pri -----")
}

# OS-specific external libs and settings
MacBuild: include(config_OSX.pri)
WinBuild: include(config_Windows.pri)
LinuxBuild: include(config_Linux.pri)

# common external libs
include(config_external_libs.pri)

# post-build steps
include(qgc_install.pri)

# Input
include(src/src.pri)

RESOURCES += qgroundcontrol.qrc

TRANSLATIONS += files/lang/de.ts \
	 files/lang/en.ts \
	 files/lang/pl.ts

OTHER_FILES += files/styles/*.css
