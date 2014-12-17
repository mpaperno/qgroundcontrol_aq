################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

QWTSRCDIR = libs/qwt
DEPENDPATH += $$QWTSRCDIR
INCLUDEPATH += $$QWTSRCDIR

QWT_CONFIG     = QwtPlot
#QWT_CONFIG     += QwtWidgets
QWT_CONFIG     += QwtSvg
QWT_CONFIG     += QwtOpenGL


HEADERS += \
	 $$QWTSRCDIR/qwt.h \
	 $$QWTSRCDIR/qwt_abstract_scale_draw.h \
	 $$QWTSRCDIR/qwt_clipper.h \
	 $$QWTSRCDIR/qwt_color_map.h \
	 $$QWTSRCDIR/qwt_compat.h \
	 $$QWTSRCDIR/qwt_column_symbol.h \
	 $$QWTSRCDIR/qwt_date.h \
	 $$QWTSRCDIR/qwt_date_scale_draw.h \
	 $$QWTSRCDIR/qwt_date_scale_engine.h \
	 $$QWTSRCDIR/qwt_dyngrid_layout.h \
	 $$QWTSRCDIR/qwt_global.h \
	 $$QWTSRCDIR/qwt_graphic.h \
	 $$QWTSRCDIR/qwt_interval.h \
	 $$QWTSRCDIR/qwt_interval_symbol.h \
	 $$QWTSRCDIR/qwt_math.h \
	 $$QWTSRCDIR/qwt_magnifier.h \
	 $$QWTSRCDIR/qwt_null_paintdevice.h \
	 $$QWTSRCDIR/qwt_painter.h \
	 $$QWTSRCDIR/qwt_painter_command.h \
	 $$QWTSRCDIR/qwt_panner.h \
	 $$QWTSRCDIR/qwt_picker.h \
	 $$QWTSRCDIR/qwt_picker_machine.h \
	 $$QWTSRCDIR/qwt_pixel_matrix.h \
	 $$QWTSRCDIR/qwt_point_3d.h \
	 $$QWTSRCDIR/qwt_point_polar.h \
	 $$QWTSRCDIR/qwt_round_scale_draw.h \
	 $$QWTSRCDIR/qwt_scale_div.h \
	 $$QWTSRCDIR/qwt_scale_draw.h \
	 $$QWTSRCDIR/qwt_scale_engine.h \
	 $$QWTSRCDIR/qwt_scale_map.h \
	 $$QWTSRCDIR/qwt_spline.h \
	 $$QWTSRCDIR/qwt_symbol.h \
	 $$QWTSRCDIR/qwt_system_clock.h \
	 $$QWTSRCDIR/qwt_text_engine.h \
	 $$QWTSRCDIR/qwt_text_label.h \
	 $$QWTSRCDIR/qwt_text.h \
	 $$QWTSRCDIR/qwt_transform.h \
	 $$QWTSRCDIR/qwt_widget_overlay.h

SOURCES += \
	 $$QWTSRCDIR/qwt_abstract_scale_draw.cpp \
	 $$QWTSRCDIR/qwt_clipper.cpp \
	 $$QWTSRCDIR/qwt_color_map.cpp \
	 $$QWTSRCDIR/qwt_column_symbol.cpp \
	 $$QWTSRCDIR/qwt_date.cpp \
	 $$QWTSRCDIR/qwt_date_scale_draw.cpp \
	 $$QWTSRCDIR/qwt_date_scale_engine.cpp \
	 $$QWTSRCDIR/qwt_dyngrid_layout.cpp \
	 $$QWTSRCDIR/qwt_event_pattern.cpp \
	 $$QWTSRCDIR/qwt_graphic.cpp \
	 $$QWTSRCDIR/qwt_interval.cpp \
	 $$QWTSRCDIR/qwt_interval_symbol.cpp \
	 $$QWTSRCDIR/qwt_math.cpp \
	 $$QWTSRCDIR/qwt_magnifier.cpp \
	 $$QWTSRCDIR/qwt_null_paintdevice.cpp \
	 $$QWTSRCDIR/qwt_painter.cpp \
	 $$QWTSRCDIR/qwt_painter_command.cpp \
	 $$QWTSRCDIR/qwt_panner.cpp \
	 $$QWTSRCDIR/qwt_picker.cpp \
	 $$QWTSRCDIR/qwt_picker_machine.cpp \
	 $$QWTSRCDIR/qwt_pixel_matrix.cpp \
	 $$QWTSRCDIR/qwt_point_3d.cpp \
	 $$QWTSRCDIR/qwt_point_polar.cpp \
	 $$QWTSRCDIR/qwt_round_scale_draw.cpp \
	 $$QWTSRCDIR/qwt_scale_div.cpp \
	 $$QWTSRCDIR/qwt_scale_draw.cpp \
	 $$QWTSRCDIR/qwt_scale_map.cpp \
	 $$QWTSRCDIR/qwt_spline.cpp \
	 $$QWTSRCDIR/qwt_scale_engine.cpp \
	 $$QWTSRCDIR/qwt_symbol.cpp \
	 $$QWTSRCDIR/qwt_system_clock.cpp \
	 $$QWTSRCDIR/qwt_text_engine.cpp \
	 $$QWTSRCDIR/qwt_text_label.cpp \
	 $$QWTSRCDIR/qwt_text.cpp \
	 $$QWTSRCDIR/qwt_transform.cpp \
	 $$QWTSRCDIR/qwt_widget_overlay.cpp


contains(QWT_CONFIG, QwtPlot) {

	 HEADERS += \
		  $$QWTSRCDIR/qwt_curve_fitter.h \
		  $$QWTSRCDIR/qwt_event_pattern.h \
		  $$QWTSRCDIR/qwt_abstract_legend.h \
		  $$QWTSRCDIR/qwt_legend.h \
		  $$QWTSRCDIR/qwt_legend_data.h \
		  $$QWTSRCDIR/qwt_legend_label.h \
		  $$QWTSRCDIR/qwt_plot.h \
		  $$QWTSRCDIR/qwt_plot_renderer.h \
		  $$QWTSRCDIR/qwt_plot_curve.h \
		  $$QWTSRCDIR/qwt_plot_dict.h \
		  $$QWTSRCDIR/qwt_plot_directpainter.h \
		  $$QWTSRCDIR/qwt_plot_grid.h \
		  $$QWTSRCDIR/qwt_plot_histogram.h \
		  $$QWTSRCDIR/qwt_plot_item.h \
		  $$QWTSRCDIR/qwt_plot_abstract_barchart.h \
		  $$QWTSRCDIR/qwt_plot_barchart.h \
		  $$QWTSRCDIR/qwt_plot_multi_barchart.h \
		  $$QWTSRCDIR/qwt_plot_intervalcurve.h \
		  $$QWTSRCDIR/qwt_plot_tradingcurve.h \
		  $$QWTSRCDIR/qwt_plot_layout.h \
		  $$QWTSRCDIR/qwt_plot_marker.h \
		  $$QWTSRCDIR/qwt_plot_zoneitem.h \
		  $$QWTSRCDIR/qwt_plot_textlabel.h \
		  $$QWTSRCDIR/qwt_plot_rasteritem.h \
		  $$QWTSRCDIR/qwt_plot_spectrogram.h \
		  $$QWTSRCDIR/qwt_plot_spectrocurve.h \
		  $$QWTSRCDIR/qwt_plot_scaleitem.h \
		  $$QWTSRCDIR/qwt_plot_legenditem.h \
		  $$QWTSRCDIR/qwt_plot_seriesitem.h \
		  $$QWTSRCDIR/qwt_plot_shapeitem.h \
		  $$QWTSRCDIR/qwt_plot_canvas.h \
		  $$QWTSRCDIR/qwt_plot_panner.h \
		  $$QWTSRCDIR/qwt_plot_picker.h \
		  $$QWTSRCDIR/qwt_plot_zoomer.h \
		  $$QWTSRCDIR/qwt_plot_magnifier.h \
		  $$QWTSRCDIR/qwt_plot_rescaler.h \
		  $$QWTSRCDIR/qwt_point_mapper.h \
		  $$QWTSRCDIR/qwt_raster_data.h \
		  $$QWTSRCDIR/qwt_matrix_raster_data.h \
		  $$QWTSRCDIR/qwt_sampling_thread.h \
		  $$QWTSRCDIR/qwt_samples.h \
		  $$QWTSRCDIR/qwt_series_data.h \
		  $$QWTSRCDIR/qwt_series_store.h \
		  $$QWTSRCDIR/qwt_point_data.h \
		  $$QWTSRCDIR/qwt_scale_widget.h

	 SOURCES += \
		  $$QWTSRCDIR/qwt_curve_fitter.cpp \
		  $$QWTSRCDIR/qwt_abstract_legend.cpp \
		  $$QWTSRCDIR/qwt_legend.cpp \
		  $$QWTSRCDIR/qwt_legend_data.cpp \
		  $$QWTSRCDIR/qwt_legend_label.cpp \
		  $$QWTSRCDIR/qwt_plot.cpp \
		  $$QWTSRCDIR/qwt_plot_renderer.cpp \
		  $$QWTSRCDIR/qwt_plot_xml.cpp \
		  $$QWTSRCDIR/qwt_plot_axis.cpp \
		  $$QWTSRCDIR/qwt_plot_curve.cpp \
		  $$QWTSRCDIR/qwt_plot_dict.cpp \
		  $$QWTSRCDIR/qwt_plot_directpainter.cpp \
		  $$QWTSRCDIR/qwt_plot_grid.cpp \
		  $$QWTSRCDIR/qwt_plot_histogram.cpp \
		  $$QWTSRCDIR/qwt_plot_item.cpp \
		  $$QWTSRCDIR/qwt_plot_abstract_barchart.cpp \
		  $$QWTSRCDIR/qwt_plot_barchart.cpp \
		  $$QWTSRCDIR/qwt_plot_multi_barchart.cpp \
		  $$QWTSRCDIR/qwt_plot_intervalcurve.cpp \
		  $$QWTSRCDIR/qwt_plot_zoneitem.cpp \
		  $$QWTSRCDIR/qwt_plot_tradingcurve.cpp \
		  $$QWTSRCDIR/qwt_plot_spectrogram.cpp \
		  $$QWTSRCDIR/qwt_plot_spectrocurve.cpp \
		  $$QWTSRCDIR/qwt_plot_scaleitem.cpp \
		  $$QWTSRCDIR/qwt_plot_legenditem.cpp \
		  $$QWTSRCDIR/qwt_plot_seriesitem.cpp \
		  $$QWTSRCDIR/qwt_plot_shapeitem.cpp \
		  $$QWTSRCDIR/qwt_plot_marker.cpp \
		  $$QWTSRCDIR/qwt_plot_textlabel.cpp \
		  $$QWTSRCDIR/qwt_plot_layout.cpp \
		  $$QWTSRCDIR/qwt_plot_canvas.cpp \
		  $$QWTSRCDIR/qwt_plot_panner.cpp \
		  $$QWTSRCDIR/qwt_plot_rasteritem.cpp \
		  $$QWTSRCDIR/qwt_plot_picker.cpp \
		  $$QWTSRCDIR/qwt_plot_zoomer.cpp \
		  $$QWTSRCDIR/qwt_plot_magnifier.cpp \
		  $$QWTSRCDIR/qwt_plot_rescaler.cpp \
		  $$QWTSRCDIR/qwt_point_mapper.cpp \
		  $$QWTSRCDIR/qwt_raster_data.cpp \
		  $$QWTSRCDIR/qwt_matrix_raster_data.cpp \
		  $$QWTSRCDIR/qwt_sampling_thread.cpp \
		  $$QWTSRCDIR/qwt_series_data.cpp \
		  $$QWTSRCDIR/qwt_point_data.cpp \
		  $$QWTSRCDIR/qwt_scale_widget.cpp
}

greaterThan(QT_MAJOR_VERSION, 4) {

	 QT += printsupport
	 QT += concurrent
}

contains(QWT_CONFIG, QwtSvg) {

	 QT += svg

	 HEADERS += $$QWTSRCDIR/qwt_plot_svgitem.h
	 SOURCES += $$QWTSRCDIR/qwt_plot_svgitem.cpp
}
else {

	 DEFINES += QWT_NO_SVG
}

contains(QWT_CONFIG, QwtOpenGL) {

	 QT += opengl

	 HEADERS += $$QWTSRCDIR/qwt_plot_glcanvas.h
	 SOURCES += $$QWTSRCDIR/qwt_plot_glcanvas.cpp
}
else {

	 DEFINES += QWT_NO_OPENGL
}

contains(QWT_CONFIG, QwtWidgets) {

	 HEADERS += \
		  $$QWTSRCDIR/qwt_abstract_slider.h \
		  $$QWTSRCDIR/qwt_abstract_scale.h \
		  $$QWTSRCDIR/qwt_arrow_button.h \
		  $$QWTSRCDIR/qwt_analog_clock.h \
		  $$QWTSRCDIR/qwt_compass.h \
		  $$QWTSRCDIR/qwt_compass_rose.h \
		  $$QWTSRCDIR/qwt_counter.h \
		  $$QWTSRCDIR/qwt_dial.h \
		  $$QWTSRCDIR/qwt_dial_needle.h \
		  $$QWTSRCDIR/qwt_knob.h \
		  $$QWTSRCDIR/qwt_slider.h \
		  $$QWTSRCDIR/qwt_thermo.h \
		  $$QWTSRCDIR/qwt_wheel.h

	 SOURCES += \
		  $$QWTSRCDIR/qwt_abstract_slider.cpp \
		  $$QWTSRCDIR/qwt_abstract_scale.cpp \
		  $$QWTSRCDIR/qwt_arrow_button.cpp \
		  $$QWTSRCDIR/qwt_analog_clock.cpp \
		  $$QWTSRCDIR/qwt_compass.cpp \
		  $$QWTSRCDIR/qwt_compass_rose.cpp \
		  $$QWTSRCDIR/qwt_counter.cpp \
		  $$QWTSRCDIR/qwt_dial.cpp \
		  $$QWTSRCDIR/qwt_dial_needle.cpp \
		  $$QWTSRCDIR/qwt_knob.cpp \
		  $$QWTSRCDIR/qwt_slider.cpp \
		  $$QWTSRCDIR/qwt_thermo.cpp \
		  $$QWTSRCDIR/qwt_wheel.cpp
}
