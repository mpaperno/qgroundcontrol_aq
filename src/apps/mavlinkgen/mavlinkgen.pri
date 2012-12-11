# Third-party includes.
# if you include this file with the commands below into
# your Qt project, you can enable your application
# to generate MAVLink code easily.

###### EXAMPLE BEGIN

## Include MAVLink generator
#DEPENDPATH += \
#    src/apps/mavlinkgen
#
#INCLUDEPATH += \
#    src/apps/mavlinkgen
#    src/apps/mavlinkgen/ui \
#    src/apps/mavlinkgen/generator
#
#include(src/apps/mavlinkgen/mavlinkgen.pri)

###### EXAMPLE END

MAVLINKGEN_SRCPATH = src/apps/mavlinkgen

INCLUDEPATH += $$MAVLINKGEN_SRCPATH/. \
    $$MAVLINKGEN_SRCPATH/ui \
    $$MAVLINKGEN_SRCPATH/generator

FORMS += $$MAVLINKGEN_SRCPATH/ui/XMLCommProtocolWidget.ui

HEADERS += \
    $$MAVLINKGEN_SRCPATH/ui/XMLCommProtocolWidget.h \
    $$MAVLINKGEN_SRCPATH/generator/MAVLinkXMLParser.h \
    $$MAVLINKGEN_SRCPATH/generator/MAVLinkXMLParserV10.h \
    $$MAVLINKGEN_SRCPATH/ui/DomItem.h \
    $$MAVLINKGEN_SRCPATH/ui/DomModel.h \
    $$MAVLINKGEN_SRCPATH/ui/QGCMAVLinkTextEdit.h
SOURCES += \
    $$MAVLINKGEN_SRCPATH/ui/XMLCommProtocolWidget.cc \
    $$MAVLINKGEN_SRCPATH/ui/DomItem.cc \
    $$MAVLINKGEN_SRCPATH/ui/DomModel.cc \
    $$MAVLINKGEN_SRCPATH/generator/MAVLinkXMLParser.cc \
    $$MAVLINKGEN_SRCPATH/generator/MAVLinkXMLParserV10.cc \
    $$MAVLINKGEN_SRCPATH/ui/QGCMAVLinkTextEdit.cc

RESOURCES += $$MAVLINKGEN_SRCPATH/mavlinkgen.qrc
