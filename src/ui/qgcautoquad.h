#ifndef QGCAUTOQUAD_H
#define QGCAUTOQUAD_H

#include "UASInterface.h"

#include <QWidget>
#include <QProcess>
#include <QMap>
#include <QSettings>
#include <QAbstractButton>

class QGCAQParamWidget;
class AQPWMPortsConfig;
class AQEsc32;
class QTextEdit;
class QToolButton;
class QProgressBar;
class QLabel;
class QComboBox;

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
    void populateButtonGroups(QObject *parent);
    bool saveSettingsToAq(QWidget *parent, bool interactive = true);
    bool checkAqConnected(bool interactive = false);
    void setAqHasQuatos(const bool yes);
    bool aqHasQuatos() { return usingQuatos; }
    void Esc32UpdateStatusText(QString text);
    QString extProcessErrorToString(QProcess::ProcessError err);

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void changeEvent(QEvent *e);

signals:
    void visibilityChanged(bool visible);
    void hardwareInfoUpdated(void);
    void firmwareInfoUpdated(void);
    void aqHasQuatosChanged(bool hasquatos);

private slots:

    // program settings
    void loadSettings();
    void writeSettings();

    // UI handlers
    void adjustUiForHardware();
    void adjustUiForFirmware();
    void adjustUiForQuatos();
    void setupRadioTypes();
    void setupRadioPorts();
    bool radioHasPPM();
    void radioType_changed(int idx);
    void on_tab_aq_settings_currentChanged(int idx);
    void on_groupBox_controlAdvancedSettings_toggled(bool arg1);
    void on_SPVR_FS_RAD_ST2_currentIndexChanged(int index);
    void splitterCollapseToggle();
    void splitterMoved();
    bool validateRadioSettings();
//  void on_groupBox_ppmOptions_toggled(bool arg1);
    void on_radioButton_attitude_pid_clicked() { setAqHasQuatos(false); }
    void on_radioButton_attitude_quatos_clicked() { setAqHasQuatos(true); }

    // AQ FW flashing
    void setupPortList();
    void setPortName(QString str);
    void setFwType();
    void fwTypeChange();
    void selectFWToFlash();
    void flashFW();
    void flashFwStart();
    void flashFwDfu();

    // UAS setup
//  void addUAS(UASInterface* uas_ext);
    void setActiveUAS(UASInterface* uas_ext);
    void uasDeleted(UASInterface *mav);
    void removeActiveUAS();
    void setUASstatus(bool timeout, unsigned int ms);
    void uasVersionChanged(int uasId, uint32_t fwVer, uint32_t hwVer, QString fwVerStr, QString hwVerStr);
    void dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off);

    // AQ Settings
    void loadParametersToUI();
    void saveAQSettings();
    void saveDialogButtonClicked(QAbstractButton *btn);
    void saveDialogRestartOptionChecked(bool chk);
    QString paramNameGuiToOnboard(QString paraName);
    int calcRadioSetting();
    void convertPidAttValsToFW68Scales();

    // Radio channels display
    void toggleRadioValuesUpdate(bool enable);
    void onToggleRadioValuesRefresh(const bool on);
    void delayedSendRcRefreshFreq();
    void sendRcRefreshFreq();
    void toggleRadioStream(const bool enable);
    void setRadioChannelDisplayValue(int channelId, float normalized);
    void setRssiDisplayValue(float normalized);
    void on_toolButton_toggleRadioValues_clicked(bool checked);

    // ESC32
    void setPortNameEsc32(QString port);
    void btnConnectEsc32();
    void flashFWEsc32();
    void Esc32LoadConfig(QString Config);
    void Esc32ShowConfig(QMap<QString, QString> paramPairs, bool disableMissing = 1);
    void btnSaveToEsc32();
    void Esc32SaveParamsToFile();
    void Esc32LoadParamsFromFile();
    void btnArmEsc32();
    void btnStartStopEsc32();
    void btnSetRPM();
    void saveEEpromEsc32();
    void ParaWrittenEsc32(QString ParaName);
    void CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2 );
    void ESc32Disconnected();
    void Esc32Connected();
    void Esc32StartCalibration();
    void btnReadConfigEsc32();
    void Esc32LoadDefaultConf();
    void Esc32ReLoadConf();
    void Esc32CaliGetCommand(int Command);
    void Esc32StartLogging();
    void Esc32CalibrationFinished(int mode);
    void Esc32BootModOk();
    void Esc32BootModFailure(QString err);
    void Esc32BootModeTimeout();
    void Esc32GotFirmwareVersion(QString ver);

    // Misc.
    bool checkProcRunning(bool warn = true);
    void prtstexit(int stat);
    void prtstdout();
    void extProcessError(QProcess::ProcessError err);
    void globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time);
    void uasConnected();
    void handleStatusText(int uasId, int compid, int severity, QString text);
    void setConnectedSystemInfoDefaults();
    void setHardwareInfo();
    void setFirmwareInfo();
    bool checkAqSerialConnection(QString port = "");
    void paramRequestTimeoutNotify(int readCount, int writeCount);
    void pushButton_dev1();

    // Tracking
    void pushButton_tracking();
    void pushButton_tracking_file();
    void prtstexitTR(int);
    void prtstdoutTR();
    void prtstderrTR();
    void sendTracking();

/*
 * Variables
*/
public:
    int aqFirmwareRevision;
    int aqHardwareVersion;
    int aqHardwareRevision;
    int aqBuildNumber;
    QString aqFirmwareVersion;

    uint8_t maxPwmPorts;            // total number of output ports on current hardware
    uint8_t maxMotorPorts;          // maximum possible motor outputs (PWM or CAN or ?)
    QList<uint8_t> pwmPortTimers;   // table of timers corresponding to ports
    bool motPortTypeCAN;            // is CAN bus available?
    bool motPortTypeCAN_H;          // are CAN ports 17-32 available?
    bool aqCanReboot;               // can system accept remote restart command?
    bool useRadioSetupParam;        // firmware uses newer RADIO_SETUP parameter
    bool usingQuatos;               // firmware reports quatos attitutde controller is used
    bool useNewControlsScheme;      // firmware supports _CTRL_ type parameters for control switch values
    bool useTunableParams;          // firmware supports CTRL_ADJ_PARAM_n for live parameter adjustment

    QString aqBinFolderPath;    // absolute path to AQ supporting utils
    QString aqMotorMixesPath;   // abs. path to pre-configured motor mix files
    const char *platformExeExt; // OS-specific executables suffix (.exe for Win)

protected:
    Ui::QGCAutoquad *ui;
    UASInterface* uas;
//    SerialLink* seriallink;
    QGCAQParamWidget* paramaq;
    AQEsc32 *esc32;
    AQPWMPortsConfig* aqPwmPortConfig;
    LinkInterface* connectedLink;

    enum commStreamTypes {
        COMM_TYPE_NONE          = 0,
        COMM_TYPE_MULTIPLEX	    = (1<<0),
        COMM_TYPE_MAVLINK	    = (1<<1),
        COMM_TYPE_TELEMETRY	    = (1<<2),
        COMM_TYPE_GPS           = (1<<3),
        COMM_TYPE_RX_TELEM	    = (1<<4),
        COMM_TYPE_CLI           = (1<<5),
        COMM_TYPE_OMAP_CONSOLE  = (1<<6),
        COMM_TYPE_OMAP_PPP	    = (1<<7)
    };

private:
    QSettings settings;
    QToolButton *splitterToggleBtn;

    // FW flashing
    QString fileToFlash;
    QString portName;
    bool fwFlashActive;

    // ESC32
    int WaitForParaWriten;
    int Esc32CalibrationMode;
    bool FlashEsc32Active;
    bool skipParamChangeCheck;
    bool esc32_connected;
    bool esc32_armed;
    bool esc32_running;
    bool esc32_calibrating;
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
    QRegExp paramsReqRestartRx;
    QRegExp paramsRadioControls;
    QRegExp paramsSwitchControls;
    QRegExp paramsTunableControls;
    QRegExp paramsTunableControlChannels;
    QRegExp paramsNonTunable;
    QList<QComboBox *> allRadioChanCombos;
    QList<QComboBox *> allTunableParamsCombos;
    quint8 paramSaveType;
    bool restartAfterParamSave;

    // Tracking
    double lat,lon,alt;
    QProcess ps_tracking;
    int TrackingIsrunning;
    QString FileNameForTracking;
    int TrackingResX;
    int TrackingResY;
    int TrackingMoveX;
    int TrackingMoveY;
    int OldTrackingMoveX;
    int OldTrackingMoveY;
    float focal_lenght;
    float camera_yaw_offset;
    float camera_pitch_offset;
    float pixel_size;
    int pixelFilterX;
    int pixelFilterY;
    float res1,res2;
    QStringList SplitRes;
    float currentPosN, currentPosE;

    // Misc
    int VisibleWidget;
    QString LastFilePath;
    int devCommand;
    QProcess ps_master;
    bool mtx_paramsAreLoading;
    QString UsersParamsFile;
    QTextEdit* activeProcessStatusWdgt;

};

#endif // QGCAUTOQUAD_H
