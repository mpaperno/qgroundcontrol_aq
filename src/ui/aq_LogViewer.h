#ifndef AQ_LOGVIEWER_H
#define AQ_LOGVIEWER_H

#include "aq_comm.h"
#include "IncrementalPlot.h"
#include "qwt_plot_marker.h"
#include <QWidget>
#include <QStandardItemModel>
#include <QSettings>
#include <QGridLayout>

namespace Ui {
class AQLogViewer;
}

class AQLogViewer : public QWidget
{
    Q_OBJECT
    
public:
    explicit AQLogViewer(QWidget *parent = 0);
    ~AQLogViewer();

private slots:
    // program settings
    void loadSettings();
    void writeSettings();

    // Log viewer
    void OpenLogFile(bool openFile=true);
    void SetupListView();
    void DecodeLogFile(QString fileName);
    void CurveItemClicked(QModelIndex index);
    void deselectAllCurves(void);
    void openExportOptionsDlg();
    void save_plot_image();
    void showChannels();
    void startSetMarker();
    void setPoint1(const QwtDoublePoint &pos);
    void startCutting();
    void removeMarker();
    void CuttingItemChanged(int itemIndex);
//    void exportPDF(QString fileName);
    void exportSVG(QString fileName);

protected:
    Ui::AQLogViewer *ui;
    IncrementalPlot* plot;
    AQLogParser parser;

private:
    QSettings settings;

    int StepCuttingPlot;
    QString LogFile;
    QString LastFilePath;
    QGridLayout* linLayoutPlot;
    QStandardItemModel *model;
    QwtPlotPicker* picker;
    QwtPlotMarker *MarkerCut1;
    QwtPlotMarker *MarkerCut2;
    QwtPlotMarker *MarkerCut3;
    QwtPlotMarker *MarkerCut4;
    QColor DefaultColorMeasureChannels;

};

#endif // AQ_LOGVIEWER_H
