# QtSpeech -- a small cross-platform library to use TTS
# Copyright (C) 2010-2011 LynxLine.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#    INCLUDEPATH += "C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include"

SPEECHSRCDIR = libs/QtSpeech

#DEPENDPATH += $$PWD
INCLUDEPATH += $$SPEECHSRCDIR

HEADERS += \
	 $$SPEECHSRCDIR/QtSpeech \
	 $$SPEECHSRCDIR/QtSpeech.h \

macx {
	 SOURCES += $$SPEECHSRCDIR/QtSpeech_mac.cpp
	 #LIBS *= -framework AppKit
}

win32 {
	 SOURCES += $$SPEECHSRCDIR/QtSpeech_win.cpp

    INCLUDEPATH += "C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/atlmfc"
	 INCLUDEPATH += "C:/Programs/Authoring/Microsoft_SDKs/Windows/v7.1/Include"
# "C:/Program Files (x86)/Microsoft Speech SDK 5.1/Include"

	 LIBS += -L"C:/Programs/Authoring/Microsoft_SDKs/Windows/v7.1/Lib"
#"C:/Program Files (x86)/Microsoft Speech SDK 5.1/Lib/i386"
}

unix:!mac {
	 HEADERS += $$SPEECHSRCDIR/QtSpeech_unx.h
	 SOURCES += $$SPEECHSRCDIR/QtSpeech_unx.cpp

	 INCLUDEPATH += $$SPEECHSRCDIR/festival/speech_tools/include
	 INCLUDEPATH += $$SPEECHSRCDIR/festival/festival/src/include

    LIBS += -lncurses
	 LIBS += -L$$SPEECHSRCDIR/festival/festival/src/lib -lFestival
	 LIBS += -L$$SPEECHSRCDIR/festival/speech_tools/lib -lestools -lestbase -leststring

    # Linux: use asound 
    LIBS += -lasound
    
    # Mac: use system Frameworks
    #LIBS += -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework Carbon
}
