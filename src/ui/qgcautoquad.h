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
#include "aq_pwmPortsConfig.h"

#include <QWidget>
#include <QProcess>
#include <QMap>
#include <QStandardItemModel>
#include <QSettings>

class QProgressBar;
class QLabel;

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
    void getGUIpara(QWidget* parent = NULL);
    bool saveSettingsToAq(QWidget *parent, bool interactive = true);
    void QuestionForROM();
    bool checkAqConnected(bool interactive = false);

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void changeEvent(QEvent *e);

signals:
    void visibilityChanged(bool visible);
    void hardwareInfoUpdated(void);

private slots:
    // program settings
    void loadSettings();
    void writeSettings();

    // AQ FW flashing
    void setupPortList();
    void setPortName(QString port);
    void selectFWToFlash();
    void flashFW();

    // Calculations
    void startcal1();
    void startcal2();
    void startcal3();
    void startsim1();
    void startsim1b();
    void startsim2();
    void startsim3();
    void abortcalc();
    void check_var();
    void check_stop();
    void addStatic();
    void delStatic();
    void addDynamic();
    void delDynamic();
    void setUsersParams();
    void CreateUsersParams();
    void WriteUsersParams();
    void ShowUsersParams(QString fileName);
    double Round(double Zahl, unsigned int Stellen);
    void CalculatDeclination();
    void CalculatInclination();

    // UAS setup
    void addUAS(UASInterface* uas_ext);
    void setActiveUAS(UASInterface* uas_ext);

    // AQ Settings
    void loadParametersToUI();
    void saveRadioSettings();
    void saveAttitudePIDs();
    void saveNavigationPIDs();
    void saveSpecialSettings();
    void saveGimbalSettings();

    // Radio setup
    void radioType_changed(int idx);
    bool validateRadioSettings(int);
    void toggleRadioValuesUpdate();
    void setRadioChannelDisplayValue(int channelId, float normalized);
    void setRssiDisplayValue(float normalized);
    void sendRcRefreshFreq();
    void delayedSendRcRefreshFreq(int rate);

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
    void exportPDF(QString fileName);
    void exportSVG(QString fileName);

    // ESC32
    void setPortNameEsc32(QString port);
    void btnConnectEsc32();
    void flashFWEsc32();
    void showConfigEsc32(QString Config);
    void btnReadConfigEsc32();
    void btnSaveToEsc32();
    void btnArmEsc32();
    void btnStartStopEsc32();
    void btnSetRPM();
    void saveEEpromEsc32();
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

    // Misc.
    void prtstexit(int);
    void prtstdout();
    void prtstderr();
    void globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time);
    void handleStatusText(int uasId, int compid, int severity, QString text);
    void setHardwareInfo(int boardVer);
    void paramRequestTimeoutNotify(int readCount, int writeCount);
    void pushButton_dev1();


/*
 * Variables
*/

public:
    int aqFirmwareRevision;
    int aqHardwareVersion;
    int aqHardwareRevision;
    int aqBuildNumber;
    QString aqFirmwareVersion;

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
    AQPWMPortsConfig* aqPwmPortConfig;

private:
    QSettings settings;

    // Calculations
    quint32 active_cal_mode;
    QString output;
    QString output_cal1;
    QString output_cal2;
    QString output_cal3;
    QString output_sim1;
    QString output_sim1b;
    QString output_sim2;
    QString output_sim3;
    QStringList StaticFiles;
    QStringList DynamicFiles;
    QString UsersParamsFile;

    // FW flashing
    QString fileToFlash;
    QString portName;
    LinkInterface* connectedLink;

    // ESC32
    int WaitForParaWriten;
    int Esc32CalibrationMode;
    bool FlashEsc32Active;
    QString portNameEsc32;
    QString FwFileForEsc32;
    QString ParaNameWritten;
    QMap<QString, QString> paramEsc32;
    QMap<QString, QString> paramEsc32Written;

    // Radio
    QTimer delayedSendRCTimer;  // for setting radio channel refresh freq.
    QList<QProgressBar *> allRadioChanProgressBars;
    QList<QLabel *> allRadioChanValueLabels;

    // AQ settings
    QRegExp fldnameRx;          // these regexes are used for matching field names to AQ params
    QRegExp dupeFldnameRx;
    QList<QComboBox *> allRadioChanCombos;

    // Log viewer
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

    // Misc
    int VisibleWidget;
    int devCommand;
    double lat,lon,alt;
    QProcess ps_master;

};

#endif // QGCAUTOQUAD_H
