#include "ChartPlot.h"
#include "MainWindow.h"

#include <qwt_picker_machine.h>

const QColor ChartPlot::baseColors[numColors] = {
    QColor(70,80,242),
    QColor(232,33,47),
    QColor(116,251,110),
    QColor(81,183,244),
    QColor(234,38,107),
    QColor(92,247,217),
    QColor(151,59,239),
    QColor(231,72,28),
    QColor(236,48,221),
    QColor(75,133,243),
    QColor(203,254,121),
    QColor(104,64,240),
    QColor(200,54,238),
    QColor(104,250,138),
    QColor(235,43,165),
    QColor(98,248,176),
    QColor(161,252,116),
    QColor(87,231,246),
    QColor(230,126,23),
    QColor(242,255,128)
};

ChartPlot::ChartPlot(QWidget *parent):
    QwtPlot(parent),
    nextColorIndex(0),
    symbolWidth(4.0f),
    curveWidth(1.0f),
    gridWidth(0.8f),
    zoomerWidth(2.0f)
{
    // Initialize the list of curves.
    curves = QMap<QString, QwtPlotCurve*>();

    // Set the grid. The colorscheme was already set in generateColorScheme().
    grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->attach(this);

    // Enable zooming
    QwtPlotCanvas *c = static_cast<QwtPlotCanvas*>(canvas());
    zoomer = new ScrollZoomer(c);
    zoomer->setTrackerMode(QwtPicker::AlwaysOn);
    zoomer->setStateMachine(new QwtPickerDragRectMachine);

    colors = QList<QColor>();

    ///> Color map for plots, includes 20 colors
    ///> Map will start from beginning when the first 20 colors are exceeded
    for (int i = 0; i < numColors; ++i)
    {
        colors.append(baseColors[i]);
    }

    // Now that all objects have been initialized, color everything.
    styleChanged(MainWindow::instance()->getStyle());
    connect(MainWindow::instance(), SIGNAL(styleChanged(int)), this, SLOT(styleChanged(int)));
}

ChartPlot::~ChartPlot()
{

}

QColor ChartPlot::getNextColor()
{
    if(nextColorIndex >= colors.count())
    {
        nextColorIndex = 0;
    }
    return colors[nextColorIndex++];
}

QColor ChartPlot::getColorForCurve(const QString &id)
{
    return curves.value(id)->pen().color();
}

void ChartPlot::shuffleColors()
{
    foreach (QwtPlotCurve* curve, curves)
    {
        if (curve->isVisible()) {
            QPen pen(curve->pen());
            pen.setColor(getNextColor());
            curve->setPen(pen);
        }
    }
}

void ChartPlot::styleChanged(int style)
{
    QColor minPen(0x8C, 0x8C, 0x8C, 150);
    QColor majPen(0xB7, 0xB7, 0xB7, 150);
    //QColor minPen(0x80, 0x80, 0x80);
    QColor rbPen(0xB8, 0xD3, 0xE6);
    QColor trackPen(0x4A, 0xEB, 0xF7);
    QColor bgColor(0, 0, 0);

    // Generate a new color list for curves and recolor them.
    if (style != MainWindow::QGC_MAINWINDOW_STYLE_DARK) {
//        for (int i = 0; i < numColors; ++i)
//            colors[i] = baseColors[i].darker(120);
        minPen = minPen.darker(200);
        majPen = minPen.darker(200);
        rbPen = rbPen.darker(200);
        trackPen = trackPen.darker(200);
        bgColor = QColor(245, 245, 245);
    }

    resetColor();
    shuffleColors();

    grid->setMinorPen(QPen(minPen, gridWidth, Qt::DotLine));
    grid->setMajorPen(QPen(majPen, gridWidth, Qt::DotLine));
    zoomer->setRubberBandPen(QPen(rbPen, zoomerWidth, Qt::DotLine));
    zoomer->setTrackerPen(QPen(trackPen));
    setCanvasBackground(bgColor);
\
    // And finally refresh the widget to make sure all color changes are redrawn.
    replot();
}
