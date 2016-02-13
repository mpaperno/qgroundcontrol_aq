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
class SelectAdjustableParamDialog;
#ifdef INCLUDE_ESC32V2_UI
class AQEsc32ConfigWidget;
#endif

class QTextEdit;
class QToolButton;
class QProgressBar;
class QLabel;
class QComboBox;
class QPushButton;
class QSpinBox;

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
    QString extProcessErrorToString(QProcess::ProcessError err);

public slots:
    void flashFwStart();

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void changeEvent(QEvent *e);

signals:
    void visibilityChanged(bool visible);
    void hardwareInfoUpdated(void);
    void firmwareInfoUpdated(void);
    void aqHasQuatosChanged(bool hasquatos);
    void remoteGuidanceEnabledChanged(bool on);

private slots:

    // program settings
    void loadSettings();
    void writeSettings();

    // UI handlers
    void splitterCollapseToggle();
    void splitterMoved();
    void adjustUiForHardware();
    void adjustUiForFirmware();
    void adjustUiForQuatos();
    void setupRadioTypes();
    void setupRadioPorts();
    bool radioHasPPM();
    void radioType_changed(int idx);
//    void on_tab_aq_settings_currentChanged(int idx);
    void on_SPVR_FS_RAD_ST2_currentIndexChanged(int index);
    void on_MOT_ESC_TYPE_currentIndexChanged(int index);
    void on_groupBox_tuningChannels_toggled(bool arg1);
    void on_groupBox_gimbal_toggled(bool arg1);
    void on_groupBox_autoTrigger_toggled(bool arg1);
    void onSwitchValueChanged(QSpinBox *origin = 0, QComboBox *target = 0);
    void onSwitchPositionChanged(QComboBox *origin = 0, QSpinBox *target = 0);
    void toggleRadioSwitchAdvancedSetup(bool on);
    void checkRadioSwitchHasAdvancedSetup();
    bool validateRadioSettings();
    bool checkTunableParamsChanged();
    bool checkLegacyChannelsChanged();
    bool hasAnyTunableParams();
    void on_radioButton_attitude_pid_clicked() { setAqHasQuatos(false); }
    void on_radioButton_attitude_quatos_clicked() { setAqHasQuatos(true); }

    // AQ FW flashing
    void setupPortList();
    void setPortName(QString str);
    void setFwType();
    void fwTypeChange();
    void selectFWToFlash();
    void flashFW();
    void flashFwDfu();

    // UAS setup
    void setActiveUAS(UASInterface* uas_ext);
    void uasDeleted(UASInterface *mav);
    void removeActiveUAS();
    void setUASstatus(bool timeout, unsigned int ms);
    void uasVersionChanged(int uasId, uint32_t fwVer, uint32_t hwVer, QString fwVerStr, QString hwVerStr);
    void dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off);

    // AQ Settings
    void onParametersLoaded(uint8_t component);
    void loadParametersToUI();
    void saveAQSettings();
    void saveDialogButtonClicked(QAbstractButton *btn);
    void saveDialogRestartOptionChecked(bool chk);
    void onTunableParamBtnClick();
    QString paramNameGuiToOnboard(QString paraName);
    int calcRadioSetting();
    void convertPidAttValsToFW68Scales();

    // Radio channels display
    void toggleRadioValuesUpdate(bool enable);
    void onToggleRadioValuesRefresh(const bool on);
    void delayedSendRcRefreshFreq();
    void sendRcRefreshFreq();
    void toggleRadioStream(const bool enable);
    void toggleConfigTelemetry(bool enable);
    void setRadioChannelDisplayValue(int channelId, float normalized);
    void setRssiDisplayValue(float normalized);
    void getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values);

    // Misc.
    bool checkProcRunning(bool warn = true);
    void prtstexit(int stat);
    void prtstdout();
    void extProcessError(QProcess::ProcessError err);
    void uasConnected(uint8_t component);
    void handleStatusText(int uasId, int compid, int severity, QString text);
    void setConnectedSystemInfoDefaults();
    void setHardwareInfo();
    void setFirmwareInfo();
    bool checkAqSerialConnection(QString port = "");
    void paramRequestTimeoutNotify(int readCount, int writeCount);
    void pushButton_dev1();

#if 0
    // Tracking
    void globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time);
    void pushButton_tracking();
    void pushButton_tracking_file();
    void prtstexitTR(int);
    void prtstdoutTR();
    void prtstderrTR();
    void sendTracking();
#endif

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
    bool useTunableParams;          // firmware supports CONFIG_ADJUST_Pn for live parameter adjustment
    bool remoteGuidanceEnabled;     // firmware supports remote guidance commands

    QString aqBinFolderPath;    // absolute path to AQ supporting utils
    QString aqMotorMixesPath;   // abs. path to pre-configured motor mix files
    const char *platformExeExt; // OS-specific executables suffix (.exe for Win)

protected:
    Ui::QGCAutoquad *ui;
    UASInterface* uas;
//    SerialLink* seriallink;
    QGCAQParamWidget* paramaq;

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

    // Radio
    QTimer delayedSendRCTimer;  // for setting radio channel refresh freq.
    QMap<uint8_t, QList<QPair<QWidget *, QWidget *> > > radioChannelIndicatorsMap;  // for quick lookup of incoming radio data telemetry by channel #

    // AQ settings
    QRegExp fldnameRx;          // these regexes are used for matching field names to AQ params
    QRegExp dupeFldnameRx;
    QRegExp paramsReqRestartRx;
    QRegExp paramsTunableControls;
    QList<QComboBox *> allRadioChanCombos;
    QList<QSpinBox *> allRadioSwitchValueBoxes;
    quint8 paramSaveType;
    int paramsLoadedForAqBuildNumber;
    bool restartAfterParamSave;

    // Misc
    int VisibleWidget;
    QString LastFilePath;
    int devCommand;
    QProcess ps_master;
    bool mtx_paramsAreLoading;
    bool m_initComplete;
    bool m_configTelemIsRunning;
    QString UsersParamsFile;
    QTextEdit* activeProcessStatusWdgt;
    SelectAdjustableParamDialog *m_selectAdjParamsDialog;

#ifdef INCLUDE_ESC32V2_UI
    // ESC32
    AQEsc32ConfigWidget *esc32Cfg;
#endif

#if 0
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
#endif

};

#endif // QGCAUTOQUAD_H
