
#################################################################
# EXTERNAL LIBRARY CONFIGURATION

## Required libs

INCLUDEPATH += $$MAVLINKPATH
isEmpty(MAVLINK_CONF) {
	 INCLUDEPATH += $$MAVLINKPATH/common
} else {
	 message("Adding support for additional MAVLink messages for: " $$MAVLINK_CONF)
	 INCLUDEPATH += $$MAVLINKPATH/$$MAVLINK_CONF
	 DEFINES += $$sprintf('QGC_USE_%1_MESSAGES', $$upper($$MAVLINK_CONF))
}

# EIGEN matrix library (header-only)
INCLUDEPATH += libs libs/eigen

# OPMapControl library (from OpenPilot)
include(libs/opmapcontrol/opmapcontrol_external.pri)

# Include QWT plotting library
include(libs/qwt/qwt.pri)

# Include serial port library
include(libs/thirdParty/qextserialport/src/qextserialport.pri)


## Optional libs

!contains(DEFINES, NO_TEXT_TO_SPEECH) {
	include(libs/QtSpeech/QtSpeech.pri)
} else {
	message("Skipping Text-to-Speech support.")
}

#!unix:!macx:!symbian: LIBS += -losg
