#ifndef QGCAUTOQUAD_H
#define QGCAUTOQUAD_H

#include "LinkManager.h"
#include "aq_comm.h"
#include <SerialLinkInterface.h>
#include <SerialLink.h>
#include "QGCUASParamManager.h"
#include "UASInterface.h"
#include "qgcaqparamwidget.h"
#include "Linecharts.h"
#include "IncrementalPlot.h"
#include "qwt_plot_marker.h"
#include <QWidget>
#include <QProcess>
#include <QMap>
#include <QStandardItemModel>

namespace Ui {
class QGCAutoquad;
}

class QGCAutoquad : public QWidget 
{
    Q_OBJECT
    
public:
    explicit QGCAutoquad(QWidget *parent = 0);
    ~QGCAutoquad();
    UASInterface* getUAS();

signals:
    void visibilityChanged(bool visible);

protected slots:
    void selectFWToFlash();
    void handleConnectButton();
    void changeEvent(QEvent *e);
    void addUAS(UASInterface* uas_ext);
    void setActiveUAS(UASInterface* uas_ext);
    void setChannelScaled(int channelId, float normalized);
    void setChannelRaw(int channelId, float normalized);
    void getGUIpara();

private slots:
        void prtstexit(int);
        void prtstdout();
        void prtstderr();
        void setPortName(QString port);
        void setPortNameEsc32(QString port);
        void flashFW();
        void btnConnectEsc32();
        void startcal1();
        void startcal2();
        void startcal3();
        void startsim1();
        void startsim1b();
        void startsim2();
        void startsim3();
        void abortcal1();
        void abortcal2();
        void abortcal3();
        void abortsim1();
        void abortsim1b();
        void abortsim2();
        void abortsim3();
        void raw_transmitter_view();
        void check_var();
        void check_stop();
        void addStatic();
        void delStatic();
        void addDynamic();
        void delDynamic();
        void setRadio();
        void setFrame();
        void setUsersParams();
        void CreateUsersParams();
        void WriteUsersParams();
		void LoadFrameFromFile();
		void SaveFrameToFile();
		void CalculatDeclination();
        void CurveItemChanged(QStandardItem *item);
        void OpenLogFile();
        void save_PID_toAQ1();
        void save_PID_toAQ2();
        void save_PID_toAQ3();
        void save_plot_image();
        void showChannels();
        void startSetMarker();
        void setPoint1(const QwtDoublePoint &pos);
        void startCutting();



        void showConfigEsc32(QString Config);
        void btnReadConfigEsc32();
        void btnSaveToEsc32();
        void btnArmEsc32();
        void btnStartStopEsc32();
        void btnSetRPM();

        void ParaWrittenEsc32(QString ParaName);
        void CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2 );
        void ESc32Disconnected();
        void Esc32Connected();
        void Esc32RpmSlider(int rpm);


private:
        Ui::QGCAutoquad *ui;
        QString output;
        QString output_cal1;
        QString output_cal2;
        QString output_cal3;
        QString output_sim1;
        QString output_sim1b;
        QString output_sim2;
        QString output_sim3;
        QString fileToFlash;
        QString portName;
        QString portNameEsc32;
        QProcess ps_master;
        LinkInterface* currLink;
        SerialLink* seriallink;
        void setupPortList();
        UASInterface* uas;
        QGCAQParamWidget* paramaq;
        int VisibleWidget;
        void loadSettings();
        void writeSettings();
        QStringList StaticFiles;
        QStringList DynamicFiles;
		quint32 active_cal_mode;
        QString UsersParamsFile;
        void ShowUsersParams(QString fileName);
        void DecodeLogFile(QString fileName);
        QString LogFile;
        void QuestionForROM();
        void exportPDF(QString fileName);
        void exportSVG(QString fileName);
        QString LastFilePath;
        AQEsc32 *esc32;
        QStandardItemModel *model;
        QMap<QString, QString> paramEsc32;
        QMap<QString, QString> paramEsc32Written;
        int WaitForParaWriten;
        QString ParaNameWritten;
        void saveEEpromEsc32();
        int StepCuttingPlot;
        QwtPlotPicker* picker;
        QwtPlotMarker *MarkerCut1;
        QwtPlotMarker *MarkerCut2;
        QwtPlotMarker *MarkerCut3;
        QwtPlotMarker *MarkerCut4;


protected:
        void showEvent(QShowEvent* event);
        void hideEvent(QHideEvent* event);
        void SetupListView();
        IncrementalPlot* plot;
        AQLogParser parser;

};

#endif // QGCAUTOQUAD_H
