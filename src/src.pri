
INCLUDEPATH += src \
	 src/ui \
	 src/ui/linechart \
	 src/ui/uas \
	 src/ui/map \
	 src/uas \
	 src/comm \
	 include/ui \
	 src/input \
	 src/ui/mavlink \
	 src/ui/map3D \
	 src/ui/mission \
	 src/ui/designer

FORMS += src/ui/MainWindow.ui \
	 src/ui/CommSettings.ui \
	 src/ui/SerialSettings.ui \
	 src/ui/UASControl.ui \
	 src/ui/UASList.ui \
	 src/ui/UASInfo.ui \
	 src/ui/Linechart.ui \
	 src/ui/UASView.ui \
	 src/ui/ParameterInterface.ui \
	 src/ui/WaypointList.ui \
#    src/ui/ObjectDetectionView.ui \
#    src/ui/JoystickWidget.ui \
	 src/ui/DebugConsole.ui \
	 src/ui/HDDisplay.ui \
	 src/ui/MAVLinkSettingsWidget.ui \
#    src/ui/AudioOutputWidget.ui \
	 src/ui/QGCSensorSettingsWidget.ui \
	 src/ui/QGCDataPlot2D.ui \
	 src/ui/QGCRemoteControlView.ui \
	 src/ui/QMap3D.ui \
	 src/ui/QGCWebView.ui \
	 src/ui/uas/QGCUnconnectedInfoWidget.ui \
	 src/ui/designer/QGCToolWidget.ui \
	 src/ui/designer/QGCParamSlider.ui \
	 src/ui/designer/QGCActionButton.ui \
	 src/ui/designer/QGCCommandButton.ui \
	 src/ui/QGCMAVLinkLogPlayer.ui \
	 src/ui/QGCWaypointListMulti.ui \
	 src/ui/mission/QGCCustomWaypointAction.ui \
	 src/ui/QGCUDPLinkConfiguration.ui \
	 src/ui/QGCSettingsWidget.ui \
	 src/ui/UASControlParameters.ui \
	 src/ui/mission/QGCMissionDoWidget.ui \
	 src/ui/mission/QGCMissionConditionWidget.ui \
	 src/ui/map/QGCMapTool.ui \
	 src/ui/map/QGCMapToolBar.ui \
	 src/ui/QGCMAVLinkInspector.ui \
	 src/ui/WaypointViewOnlyView.ui \
	 src/ui/WaypointEditableView.ui \
	 src/ui/UnconnectedUASInfoWidget.ui \
#	 src/ui/mavlink/QGCMAVLinkMessageSender.ui \
	 src/ui/QGCPluginHost.ui \
	 src/ui/mission/QGCMissionOther.ui \
	 src/ui/mission/QGCMissionNavWaypoint.ui \
	 src/ui/mission/QGCMissionDoJump.ui \
	 src/ui/mission/QGCMissionConditionDelay.ui \
	 src/ui/mission/QGCMissionNavLoiterUnlim.ui \
	 src/ui/mission/QGCMissionNavLoiterTurns.ui \
	 src/ui/mission/QGCMissionNavLoiterTime.ui \
	 src/ui/mission/QGCMissionNavReturnToLaunch.ui \
	 src/ui/mission/QGCMissionNavLand.ui \
	 src/ui/mission/QGCMissionNavTakeoff.ui \
	 src/ui/mission/QGCMissionNavSweep.ui \
	 src/ui/mission/QGCMissionDoStartSearch.ui \
	 src/ui/mission/QGCMissionDoFinishSearch.ui \
#    src/ui/QGCHilConfiguration.ui \
#    src/ui/QGCHilFlightGearConfiguration.ui \
#    src/ui/QGCHilXPlaneConfiguration.ui \
	 src/ui/qgcautoquad.ui \
	 src/ui/AQLinechart.ui \
#    src/ui/aq_LogExporter.ui \
	 src/ui/aq_telemetryView.ui \
	 src/ui/aq_pwmPortsConfig.ui \
	 src/ui/aq_LogViewer.ui \
	 src/ui/ESCtelemetryWidget.ui \
	 src/ui/map/GoToWaypointDialog.ui \
	 src/ui/WaypointDialog.ui \
	 src/ui/SelectAdjustableParamDialog.ui

HEADERS += src/MG.h \
	 src/QGCCore.h \
	 src/uas/UASInterface.h \
	 src/uas/UAS.h \
	 src/uas/UASManager.h \
	 src/comm/LinkManager.h \
	 src/comm/LinkInterface.h \
	 src/comm/SerialLinkInterface.h \
	 src/comm/SerialLink.h \
	 src/comm/ProtocolInterface.h \
	 src/comm/MAVLinkProtocol.h \
#    src/comm/QGCFlightGearLink.h \
#    src/comm/QGCXPlaneLink.h \
	 src/ui/CommConfigurationWindow.h \
	 src/ui/SerialConfigurationWindow.h \
	 src/ui/MainWindow.h \
	 src/ui/uas/UASControlWidget.h \
	 src/ui/uas/UASListWidget.h \
	 src/ui/uas/UASInfoWidget.h \
	 src/ui/HUD.h \
	 src/ui/linechart/LinechartWidget.h \
	 src/ui/linechart/LinechartPlot.h \
	 src/ui/linechart/Scrollbar.h \
	 src/ui/linechart/ScrollZoomer.h \
	 src/configuration.h \
	 src/ui/uas/UASView.h \
#    src/ui/CameraView.h \
	 src/comm/MAVLinkSimulationLink.h \
	 src/comm/UDPLink.h \
	 src/ui/ParameterInterface.h \
	 src/ui/WaypointList.h \
	 src/Waypoint.h \
#    src/ui/ObjectDetectionView.h \
#    src/input/JoystickInput.h \
#    src/ui/JoystickWidget.h \
	 src/ui/DebugConsole.h \
	 src/ui/HDDisplay.h \
	 src/ui/MAVLinkSettingsWidget.h \
#    src/ui/AudioOutputWidget.h \
	 src/GAudioOutput.h \
	 src/LogCompressor.h \
#    src/ui/QGCParamWidget.h \
	 src/ui/QGCSensorSettingsWidget.h \
	 src/ui/linechart/Linecharts.h \
	 src/uas/UASWaypointManager.h \
	 src/ui/HSIDisplay.h \
	 src/QGC.h \
	 src/ui/QGCDataPlot2D.h \
	 src/ui/linechart/IncrementalPlot.h \
	 src/ui/QGCRemoteControlView.h \
	 src/comm/QGCMAVLink.h \
	 src/ui/QGCWebView.h \
	 src/ui/map3D/QGCWebPage.h \
#    src/comm/MAVLinkSwarmSimulationLink.h \
	 src/ui/uas/QGCUnconnectedInfoWidget.h \
	 src/ui/designer/QGCToolWidget.h \
	 src/ui/designer/QGCParamSlider.h \
	 src/ui/designer/QGCCommandButton.h \
	 src/ui/designer/QGCToolWidgetItem.h \
	 src/ui/QGCMAVLinkLogPlayer.h \
	 src/comm/MAVLinkSimulationWaypointPlanner.h \
	 src/comm/MAVLinkSimulationMAV.h \
	 src/uas/QGCMAVLinkUASFactory.h \
	 src/ui/QGCWaypointListMulti.h \
	 src/ui/QGCUDPLinkConfiguration.h \
	 src/ui/QGCSettingsWidget.h \
	 src/ui/uas/UASControlParameters.h \
	 src/ui/mission/QGCMissionDoWidget.h \
	 src/ui/mission/QGCMissionConditionWidget.h \
	 src/uas/QGCUASParamManager.h \
	 src/ui/map/QGCMapWidget.h \
	 src/ui/map/MAV2DIcon.h \
	 src/ui/map/Waypoint2DIcon.h \
	 src/ui/map/QGCMapTool.h \
	 src/ui/map/QGCMapToolBar.h \
	 src/QGCGeo.h \
	 src/ui/QGCToolBar.h \
	 src/ui/QGCMAVLinkInspector.h \
	 src/ui/MAVLinkDecoder.h \
	 src/ui/WaypointViewOnlyView.h \
	 src/ui/WaypointEditableView.h \
	 src/ui/UnconnectedUASInfoWidget.h \
	 src/ui/QGCRGBDView.h \
#	 src/ui/mavlink/QGCMAVLinkMessageSender.h \
	 src/ui/QGCPluginHost.h \
	 src/ui/mission/QGCMissionOther.h \
	 src/ui/mission/QGCMissionNavWaypoint.h \
	 src/ui/mission/QGCMissionDoJump.h \
	 src/ui/mission/QGCMissionConditionDelay.h \
	 src/ui/mission/QGCMissionNavLoiterUnlim.h \
	 src/ui/mission/QGCMissionNavLoiterTurns.h \
	 src/ui/mission/QGCMissionNavLoiterTime.h \
	 src/ui/mission/QGCMissionNavReturnToLaunch.h \
	 src/ui/mission/QGCMissionNavLand.h \
	 src/ui/mission/QGCMissionNavTakeoff.h \
	 src/ui/mission/QGCMissionNavSweep.h \
	 src/ui/mission/QGCMissionDoStartSearch.h \
	 src/ui/mission/QGCMissionDoFinishSearch.h \
#    src/comm/QGCHilLink.h \
#    src/ui/QGCHilConfiguration.h \
#    src/ui/QGCHilFlightGearConfiguration.h \
#    src/ui/QGCHilXPlaneConfiguration.h \
	 src/ui/qgcautoquad.h \
	 src/ui/qgcaqparamwidget.h \
	 src/aq_comm.h \
	 src/ui/linechart/aqlinechartwidget.h \
#    src/ui/aq_LogExporter.h \
	 src/ui/aq_telemetryView.h \
	 src/ui/aq_pwmPortsConfig.h \
	 src/ui/PrimaryFlightDisplay.h \
	 src/ui/aq_LogViewer.h \
	 src/ui/QGCDataViewWidget.h \
	 src/uas/autoquadMAV.h \
	 src/ui/ESCtelemetryWidget.h \
	 src/ui/linechart/ChartPlot.h \
	 src/ui/WaypointDialog.h \
	 src/ui/SelectAdjustableParamDialog.h

SOURCES += src/main.cc \
	 src/QGCCore.cc \
	 src/uas/UASManager.cc \
	 src/uas/UAS.cc \
	 src/comm/LinkManager.cc \
	 src/comm/LinkInterface.cpp \
	 src/comm/SerialLink.cc \
	 src/comm/MAVLinkProtocol.cc \
#    src/comm/QGCFlightGearLink.cc \
#    src/comm/QGCXPlaneLink.cc \
	 src/ui/CommConfigurationWindow.cc \
	 src/ui/SerialConfigurationWindow.cc \
	 src/ui/MainWindow.cc \
	 src/ui/uas/UASControlWidget.cc \
	 src/ui/uas/UASListWidget.cc \
	 src/ui/uas/UASInfoWidget.cc \
	 src/ui/HUD.cc \
	 src/ui/linechart/LinechartWidget.cc \
	 src/ui/linechart/LinechartPlot.cc \
	 src/ui/linechart/Scrollbar.cc \
	 src/ui/linechart/ScrollZoomer.cc \
	 src/ui/uas/UASView.cc \
#    src/ui/CameraView.cc \
	 src/comm/MAVLinkSimulationLink.cc \
	 src/comm/UDPLink.cc \
	 src/ui/ParameterInterface.cc \
	 src/ui/WaypointList.cc \
	 src/Waypoint.cc \
#    src/ui/ObjectDetectionView.cc \
#    src/input/JoystickInput.cc \
#    src/ui/JoystickWidget.cc \
	 src/ui/DebugConsole.cc \
	 src/ui/HDDisplay.cc \
	 src/ui/MAVLinkSettingsWidget.cc \
#    src/ui/AudioOutputWidget.cc \
	 src/GAudioOutput.cc \
	 src/LogCompressor.cc \
#    src/ui/QGCParamWidget.cc \
	 src/ui/QGCSensorSettingsWidget.cc \
	 src/ui/linechart/Linecharts.cc \
	 src/uas/UASWaypointManager.cc \
	 src/ui/HSIDisplay.cc \
	 src/QGC.cc \
	 src/ui/QGCDataPlot2D.cc \
	 src/ui/linechart/IncrementalPlot.cc \
	 src/ui/QGCRemoteControlView.cc \
	 src/ui/QGCWebView.cc \
	 src/ui/map3D/QGCWebPage.cc \
	 src/ui/uas/QGCUnconnectedInfoWidget.cc \
	 src/ui/designer/QGCToolWidget.cc \
	 src/ui/designer/QGCParamSlider.cc \
	 src/ui/designer/QGCCommandButton.cc \
	 src/ui/designer/QGCToolWidgetItem.cc \
	 src/ui/QGCMAVLinkLogPlayer.cc \
	 src/comm/MAVLinkSimulationWaypointPlanner.cc \
	 src/comm/MAVLinkSimulationMAV.cc \
	 src/uas/QGCMAVLinkUASFactory.cc \
	 src/ui/QGCWaypointListMulti.cc \
	 src/ui/QGCUDPLinkConfiguration.cc \
	 src/ui/QGCSettingsWidget.cc \
	 src/ui/uas/UASControlParameters.cpp \
	 src/ui/mission/QGCMissionDoWidget.cc \
	 src/ui/mission/QGCMissionConditionWidget.cc \
	 src/uas/QGCUASParamManager.cc \
	 src/ui/map/QGCMapWidget.cc \
	 src/ui/map/MAV2DIcon.cc \
	 src/ui/map/Waypoint2DIcon.cc \
	 src/ui/map/QGCMapTool.cc \
	 src/ui/map/QGCMapToolBar.cc \
	 src/ui/QGCToolBar.cc \
	 src/ui/QGCMAVLinkInspector.cc \
	 src/ui/MAVLinkDecoder.cc \
	 src/ui/WaypointViewOnlyView.cc \
	 src/ui/WaypointEditableView.cc \
	 src/ui/UnconnectedUASInfoWidget.cc \
	 src/ui/QGCRGBDView.cc \
#	 src/ui/mavlink/QGCMAVLinkMessageSender.cc \
	 src/ui/QGCPluginHost.cc \
	 src/ui/mission/QGCMissionOther.cc \
	 src/ui/mission/QGCMissionNavWaypoint.cc \
	 src/ui/mission/QGCMissionDoJump.cc \
	 src/ui/mission/QGCMissionConditionDelay.cc \
	 src/ui/mission/QGCMissionNavLoiterUnlim.cc \
	 src/ui/mission/QGCMissionNavLoiterTurns.cc \
	 src/ui/mission/QGCMissionNavLoiterTime.cc \
	 src/ui/mission/QGCMissionNavReturnToLaunch.cc \
	 src/ui/mission/QGCMissionNavLand.cc \
	 src/ui/mission/QGCMissionNavTakeoff.cc \
	 src/ui/mission/QGCMissionNavSweep.cc \
	 src/ui/mission/QGCMissionDoStartSearch.cc \
	 src/ui/mission/QGCMissionDoFinishSearch.cc \
#    src/ui/QGCHilConfiguration.cc \
#    src/ui/QGCHilFlightGearConfiguration.cc \
#    src/ui/QGCHilXPlaneConfiguration.cc \
	 src/ui/qgcautoquad.cc \
	 src/ui/qgcaqparamwidget.cc \
	 src/aq_comm.cpp \
	 src/ui/linechart/aqlinechartwidget.cpp \
#    src/ui/aq_LogExporter.cpp \
	 src/ui/aq_telemetryView.cpp \
	 src/ui/aq_pwmPortsConfig.cc \
	 src/ui/PrimaryFlightDisplay.cpp \
	 src/ui/aq_LogViewer.cc \
	 src/ui/QGCDataViewWidget.cc \
	 src/ui/ESCtelemetryWidget.cpp \
	 src/ui/linechart/ChartPlot.cc \
	 src/ui/WaypointDialog.cpp \
	 src/ui/SelectAdjustableParamDialog.cpp

# Enable Google Earth only on Mac OS 32b and Windows
contains(DEFINES, USE_GOOGLE_EARTH_PLUGIN) {
	FORMS += src/ui/map3D/QGCGoogleEarthView.ui
	SOURCES += src/ui/map3D/QGCGoogleEarthView.cc
	HEADERS += src/ui/map3D/QGCGoogleEarthView.h
}

contains(DEPENDENCIES_PRESENT, libfreenect) {
	 message("Including sources for libfreenect")

	 # Enable only if libfreenect is available
	 HEADERS += src/input/Freenect.h
	 SOURCES += src/input/Freenect.cc
}


#contains(DEPENDENCIES_PRESENT, osg) {
#    message("Including headers for OpenSceneGraph")

#    # Enable only if OpenSceneGraph is available
#    HEADERS += src/ui/map3D/gpl.h \
#        src/ui/map3D/CameraParams.h \
#        src/ui/map3D/ViewParamWidget.h \
#        src/ui/map3D/SystemContainer.h \
#        src/ui/map3D/SystemViewParams.h \
#        src/ui/map3D/GlobalViewParams.h \
#        src/ui/map3D/SystemGroupNode.h \
#        src/ui/map3D/Q3DWidget.h \
#        src/ui/map3D/GCManipulator.h \
#        src/ui/map3D/ImageWindowGeode.h \
#        src/ui/map3D/PixhawkCheetahNode.h \
#        src/ui/map3D/Pixhawk3DWidget.h \
#        src/ui/map3D/Q3DWidgetFactory.h \
#        src/ui/map3D/WebImageCache.h \
#        src/ui/map3D/WebImage.h \
#        src/ui/map3D/TextureCache.h \
#        src/ui/map3D/Texture.h \
#        src/ui/map3D/Imagery.h \
#        src/ui/map3D/HUDScaleGeode.h \
#        src/ui/map3D/WaypointGroupNode.h \
#        src/ui/map3D/TerrainParamDialog.h \
#        src/ui/map3D/ImageryParamDialog.h
#
#    SOURCES += src/ui/map3D/gpl.cc \
#        src/ui/map3D/CameraParams.cc \
#        src/ui/map3D/ViewParamWidget.cc \
#        src/ui/map3D/SystemContainer.cc \
#        src/ui/map3D/SystemViewParams.cc \
#        src/ui/map3D/GlobalViewParams.cc \
#        src/ui/map3D/SystemGroupNode.cc \
#        src/ui/map3D/Q3DWidget.cc \
#        src/ui/map3D/ImageWindowGeode.cc \
#        src/ui/map3D/GCManipulator.cc \
#        src/ui/map3D/PixhawkCheetahNode.cc \
#        src/ui/map3D/Pixhawk3DWidget.cc \
#        src/ui/map3D/Q3DWidgetFactory.cc \
#        src/ui/map3D/WebImageCache.cc \
#        src/ui/map3D/WebImage.cc \
#        src/ui/map3D/TextureCache.cc \
#        src/ui/map3D/Texture.cc \
#        src/ui/map3D/Imagery.cc \
#        src/ui/map3D/HUDScaleGeode.cc \
#        src/ui/map3D/WaypointGroupNode.cc \
#        src/ui/map3D/TerrainParamDialog.cc \
#        src/ui/map3D/ImageryParamDialog.cc

#    contains(DEPENDENCIES_PRESENT, osgearth) {
#        message("Including sources for osgEarth")

#        # Enable only if OpenSceneGraph is available
#        SOURCES +=
#    }
#}

#contains(DEPENDENCIES_PRESENT, protobuf):contains(MAVLINK_CONF, pixhawk) {
#    message("Including sources for Protocol Buffers")

#    # Enable only if protobuf is available
#    HEADERS += libs/mavlink/include/mavlink/v1.0/pixhawk/pixhawk.pb.h \
#        src/ui/map3D/ObstacleGroupNode.h \
#        src/ui/map3D/GLOverlayGeode.h

#    SOURCES += libs/mavlink/share/mavlink/src/v1.0/pixhawk/pixhawk.pb.cc \
#        src/ui/map3D/ObstacleGroupNode.cc \
#        src/ui/map3D/GLOverlayGeode.cc
#}

#contains(DEPENDENCIES_PRESENT, rt-lab) {
#WinBuild:
#    INCLUDEPATH += src/lib/opalrt
#    HEADERS += src/comm/OpalRT.h \
#        src/comm/OpalLink.h \
#        src/comm/Parameter.h \
#        src/comm/QGCParamID.h \
#        src/comm/ParameterList.h \
#        src/ui/OpalLinkConfigurationWindow.h
#    SOURCES += src/comm/OpalRT.cc \
#        src/comm/OpalLink.cc \
#        src/comm/Parameter.cc \
#        src/comm/QGCParamID.cc \
#        src/comm/ParameterList.cc \
#        src/ui/OpalLinkConfigurationWindow.cc
#    FORMS += src/ui/OpalLinkSettings.ui
#}

#contains(DEPENDENCIES_PRESENT, xbee) {
#    HEADERS += src/comm/XbeeLinkInterface.h \
#        src/comm/XbeeLink.h \
#        src/comm/HexSpinBox.h \
#        src/ui/XbeeConfigurationWindow.h \
#        src/comm/CallConv.h
#    SOURCES += src/comm/XbeeLink.cpp \
#        src/comm/HexSpinBox.cpp \
#        src/ui/XbeeConfigurationWindow.cpp
#    INCLUDEPATH += libs/thirdParty/libxbee
#}

#contains(DEPENDENCIES_PRESENT, dxmouse) {
#    SOURCES  += libs/thirdParty/3DMouse/win/MouseParameters.cpp \
#                libs/thirdParty/3DMouse/win/Mouse3DInput.cpp \
#                src/input/Mouse6dofInput.cpp
#    HEADERS  += libs/thirdParty/3DMouse/win/I3dMouseParams.h \
#                libs/thirdParty/3DMouse/win/MouseParameters.h \
#                libs/thirdParty/3DMouse/win/Mouse3DInput.h \
#                src/input/Mouse6dofInput.h
#    INCLUDEPATH += libs/thirdParty/3DMouse/win
#}
