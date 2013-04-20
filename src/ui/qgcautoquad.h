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
#include "aq_telemetryView.h"
//#include "aq_pwmPortsConfig.h"

#include <QWidget>
#include <QProcess>
#include <QMap>
#include <QStandardItemModel>
#include <QSettings>

namespace Ui {
class QGCAutoquad;
}

class QGCAutoquad : public QWidget
{
    Q_OBJECT

public:
    explicit QGCAutoquad(QWidget *parent = 0);
    ~QGCAutoquad();
    QGCAQParamWidget* getParamHandler();
    UASInterface* getUAS();
    QStringList getAvailablePwmPorts(void);
    void QuestionForROM();

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void SetupListView();

private:
    double Round(double Zahl, unsigned int Stellen);
    void setupPortList();
    void loadSettings();
    void writeSettings();
    void ShowUsersParams(QString fileName);
    void DecodeLogFile(QString fileName);
    void exportPDF(QString fileName);
    void exportSVG(QString fileName);
    void saveEEpromEsc32();
    void setMotorPWMTimer(int pitch_port, int roll_port);
    void setMotorEnable(int MotorIndex, bool value);
    void CheckGimbal(int port, bool value);
    void ShowMessageForChangingMotorConfig(int Motor);
    void DisableEnableAllPitchGimbal(int selectedIndex, bool value);
    void DisableEnableAllRollGimbal(int selectedIndex, bool value);

signals:
    void visibilityChanged(bool visible);
    void hardwareInfoUpdated(void);

public slots:
    void OpenLogFile(bool openFile=true);

protected slots:
    void selectFWToFlash();
    void changeEvent(QEvent *e);
    void addUAS(UASInterface* uas_ext);
    void setActiveUAS(UASInterface* uas_ext);
    void setChannelScaled(int channelId, float normalized);
    void setChannelRaw(int channelId, float normalized);
    void getGUIpara();
    void handleStatusText(int uasId, int compid, int severity, QString text);

private slots:
        void prtstexit(int);
        void prtstdout();
        void prtstderr();
        void setPortName(QString port);
        void setPortNameEsc32(QString port);
        void flashFW();
        void btnConnectEsc32();
        void flashFWEsc32();
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
        void CalculatInclination();
        void CurveItemChanged(QStandardItem *item);
        void CurveItemClicked(QModelIndex index);
        void deselectAllCurves(void);
        void openExportOptionsDlg();
        void save_PID_toAQ1();
        void save_PID_toAQ2();
        void save_PID_toAQ3();
        void save_PID_toAQ4();
        void save_plot_image();
        void showChannels();
        void startSetMarker();
        void setPoint1(const QwtDoublePoint &pos);
        void startCutting();
        void removeMarker();
        void CuttingItemChanged(int itemIndex);

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
        void Esc32StartCalibration();
        void Esc32ReadConf();
        void Esc32ReLoadConf();
        void Esc32CaliGetCommand(int Command);
        void Esc32StartLogging();
        void Esc32CalibrationFinished(int mode);
        void Esc32BootModOk();
        void Esc32BootModFailure();
        void gmb_pitch_P1(bool value);
        void gmb_roll_P1(bool value);

        void gmb_pitch_P2(bool value);
        void gmb_roll_P2(bool value);

        void gmb_pitch_P3(bool value);
        void gmb_roll_P3(bool value);

        void gmb_pitch_P4(bool value);
        void gmb_roll_P4(bool value);

        void gmb_pitch_P5(bool value);
        void gmb_roll_P5(bool value);

        void gmb_pitch_P6(bool value);
        void gmb_roll_P6(bool value);

        void gmb_pitch_P7(bool value);
        void gmb_roll_P7(bool value);

        void gmb_pitch_P8(bool value);
        void gmb_roll_P8(bool value);

        void gmb_pitch_P9(bool value);
        void gmb_roll_P9(bool value);

        void gmb_pitch_P10(bool value);
        void gmb_roll_P10(bool value);

        void gmb_pitch_P11(bool value);
        void gmb_roll_P11(bool value);

        void gmb_pitch_P12(bool value);
        void gmb_roll_P12(bool value);

        void gmb_pitch_P13(bool value);
        void gmb_roll_P13(bool value);

        void gmb_pitch_P14(bool value);
        void gmb_roll_P14(bool value);

        void globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time);
        void setHardwareInfo(int boardRev);
        void paramRequestTimeoutNotify(int readCount, int writeCount);

        void pushButton_dev1();


/*
 * Variables
*/

public:
        QString LogFile;
        QString LastFilePath;

        QString aqFirmwareVersion;
        int aqFirmwareRevision;
        int aqHardwareRevision;
        int aqBuildNumber;
        uint8_t maxPwmPorts;        // total number of output ports on current hardware
        QList<uint8_t> pwmPortTimers; // table of timers corresponding to ports
        QString aqBinFolderPath;    // absolute path to AQ supporting utils
        QString aqMotorMixesPath;   // abs. path to pre-configured motor mix files
        const char *platformExeExt; // OS-specific executables suffix (.exe for Win)

protected:
        Ui::QGCAutoquad *ui;
        IncrementalPlot* plot;
        AQLogParser parser;
        UASInterface* uas;
        SerialLink* seriallink;
        QGCAQParamWidget* paramaq;
        AQEsc32 *esc32;
        AQTelemetryView* aqTelemetryView;
//        AQPWMPortsConfig* aqPwmPortConfig;

private:
        QSettings settings;
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
        int VisibleWidget;
        QStringList StaticFiles;
        QStringList DynamicFiles;
        quint32 active_cal_mode;
        QString UsersParamsFile;
        QStandardItemModel *model;
        QMap<QString, QString> paramEsc32;
        QMap<QString, QString> paramEsc32Written;
        int WaitForParaWriten;
        QString ParaNameWritten;
        int StepCuttingPlot;
        QwtPlotPicker* picker;
        QwtPlotMarker *MarkerCut1;
        QwtPlotMarker *MarkerCut2;
        QwtPlotMarker *MarkerCut3;
        QwtPlotMarker *MarkerCut4;
        int Esc32CalibrationMode;
        bool EventComesFromMavlink;
        int somethingChangedInMotorConfig;
        int port_nr_roll;
        int port_nr_pitch;
        QColor DefaultColorMeasureChannels;
        bool AlreadyShowMessage;
        QGridLayout* linLayoutPlot;
        int devCommand;
        double lat,lon,alt;
        QString FwFileForEsc32;
        bool FlashEsc32Active;


};

#endif // QGCAUTOQUAD_H
