#ifndef AQ_COMM_H
#define AQ_COMM_H

#include <QFileDialog>
#include <cmath>
#include <QDebug>
#include <QQueue>
#include <QPair>
#include <QCoreApplication>
#include <SerialLinkInterface.h>
#include <SerialLink.h>
#include <QProcess>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wunused-local-typedefs"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-assign"
#endif
//#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/LU>
#if defined(__clang__)
#pragma clang diagnostic pop
//#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define EIGEN_DONT_PARALLELIZE

#include <QThread>
#define MAX_TELEM_STORAGE	200000

class AQEsc32Logger;

class AQEsc32 : public QObject {
    Q_OBJECT

public:
    AQEsc32();
    ~AQEsc32();

    enum binaryCommands {
        BINARY_COMMAND_NOP = 0,
        BINARY_COMMAND_ARM,
        BINARY_COMMAND_CLI,
        BINARY_COMMAND_CONFIG,
        BINARY_COMMAND_DISARM,
        BINARY_COMMAND_DUTY,
        BINARY_COMMAND_PWM,
        BINARY_COMMAND_RPM,
        BINARY_COMMAND_SET,
        BINARY_COMMAND_START,
        BINARY_COMMAND_STATUS,
        BINARY_COMMAND_STOP,
        BINARY_COMMAND_TELEM_RATE,
        BINARY_COMMAND_VERSION,
        BINARY_COMMAND_TELEM_VALUE,
        BINARY_COMMAND_ACK = 250,
        BINARY_COMMAND_NACK
    };

    enum binaryValues {
        BINARY_VALUE_NONE = 0,
        BINARY_VALUE_AMPS,
        BINARY_VALUE_VOLTS_BAT,
        BINARY_VALUE_VOLTS_MOTOR,
        BINARY_VALUE_RPM,
        BINARY_VALUE_DUTY,
        BINARY_VALUE_COMM_PERIOD,
        BINARY_VALUE_BAD_DETECTS,
        BINARY_VALUE_ADC_WINDOW,
        BINARY_VALUE_IDLE_PERCENT,
        BINARY_VALUE_STATE,
        BINARY_VALUE_AVGA,
        BINARY_VALUE_AVGB,
        BINARY_VALUE_AVGC,
        BINARY_VALUE_AVGCOMP,
        BINARY_VALUE_FETSTEP,
        BINARY_VALUE_NUM
    };

    enum configParameters {
        CONFIG_VERSION = 0,
        STARTUP_MODE,
        BAUD_RATE,
        PTERM,
        ITERM,
        FF1TERM,
        FF2TERM,
        CL1TERM,
        CL2TERM,
        CL3TERM,
        CL4TERM,
        CL5TERM,
        SHUNT_RESISTANCE,
        MIN_PERIOD,
        MAX_PERIOD,
        BLANKING_MICROS,
        ADVANCE,
        START_VOLTAGE,
        GOOD_DETECTS_START,
        BAD_DETECTS_DISARM,
        MAX_CURRENT,
        SWITCH_FREQ,
        MOTOR_POLES,
        PWM_MIN_PERIOD,
        PWM_MAX_PERIOD,
        PWM_MIN_VALUE,
        PWM_LO_VALUE,
        PWM_HI_VALUE,
        PWM_MAX_VALUE,
        PWM_MIN_START,
        PWM_RPM_SCALE,
        FET_BRAKING,
        PNFAC,
        INFAC,
        THR1TERM,
        THR2TERM,
        START_ALIGN_TIME,
        START_ALIGN_VOLTAGE,
        START_STEPS_NUM,
        START_STEPS_PERIOD,
        START_STEPS_ACCEL,
        PWM_LOWPASS,
        RPM_MEAS_LP,
        SERVO_DUTY,
        SERVO_P,
        SERVO_D,
        SERVO_MAX_RATE,
        SERVO_SCALE,
        ESC_ID,
        DIRECTION,
        CONFIG_NUM_PARAMS
    };

    bool currentError;
    int ExitCalibration;
    float TelemetryFrequenzy;
    QString firmwareVersion;

    void Connect(QString port, QString baud);
    void Disconnect();
    void SleepThread(int msec);
    void SavePara(QString ParaName, QVariant ParaValue);
    int SwitchFromBinaryToAscii();
    int SwitchFromAsciiToBinary();
    void sendCommand(int command, float Value1, float Value2, int num, bool withOutCheck);
    void ReadConfigEsc32();
    void SetToBootMode();
    void CheckVersion();
    int GetEsc32State();
    SerialLink* getSerialLink();
    void StartCalibration(float MaxCurrent, QString LogFile, QString ResFile);
    void StopCalibration(bool withEmergencyExit);
    void StartLogging();
    void SetCommandBack(int Command);
    void SetCalibrationMode(int mode);
    float getFF1Term();
    float getFF2Term();
    float getCL1();
    float getCL2();
    float getCL3();
    float getCL4();
    float getCL5();

protected:
    AQEsc32Logger* esc32dataLogger;

private:
    int esc32state;
    int TimerState;
    unsigned char checkInA, checkInB;
    unsigned short commandSeqId;
    unsigned char commandSeqIdBack;
    unsigned char commandBack;
    unsigned char commandLengthBack;
    unsigned char command_ACK_NACK;
    int StepMessageFromEsc32;
    QString LIST_MessageFromEsc32;
    QString ParaNameLastSend;
    QString BootloaderMessage;
    int ParaLastSend;
    QVariant LastParaValueSend1;
    QVariant LastParaValueSend2;
    QString ParaWriten_MessageFromEsc32;
    QByteArray ResponseFromEsc32;
    int TimeOutWaiting;
    int esc32BinaryMode;
    int esc32DoCommand;
    unsigned char checkOutA, checkOutB;
    int indexOfAqC;
    QTimer *checkEsc32State;
    int CommandBack;
    float FF1Term;
    float FF2Term;
    float CurrentLimiter1;
    float CurrentLimiter2;
    float CurrentLimiter3;
    float CurrentLimiter4;
    float CurrentLimiter5;
    SerialLink* seriallinkEsc32;
    int calibrationMode;
    float maximum_Current;
    QString LoggingFile;
    QString ResultFile;
    FILE *calResultFile;
    bool fastSend;
    bool bootloaderInitReturned;
    QTimer* bootModeTimer;

    int esc32SendCommand(unsigned char command, float param1, float param2, int n);
    int getEnumByName(QString Name);
    void esc32SendChar(unsigned char c);
    void esc32SendShort(unsigned short i);
    void esc32SendFloat(float f);
    void esc32OutChecksum(unsigned char c);
    void esc32InChecksum(unsigned char c);
    bool RpmToVoltage(float maxAmps);
    bool CurrentLimiter(float maxAmps);
    void stepUp(float start, float end);

private slots:
    void connectedEsc32();
    void disconnectedEsc32();
    void destroyedEsc32();
    void BytesRceivedEsc32(LinkInterface* link, QByteArray bytes);
    void checkEsc32StateTimeOut();
    void communicationErrorEsc32(QString err1, QString err2);
    void emitBootModeTimeout();


signals:
    void ShowConfig(QString Config);
    void Esc32Connected();
    void ESc32Disconnected();
    void Esc32ParaWritten(QString ParaName);
    void Esc32CommandWritten(int CommandName, QVariant V1, QVariant V2 );
    void getCommandBack(int Command);
    void finishedCalibration(int CalibrationMode);
    void EnteredBootMode();
    void NoBootModeArmed(QString err);
    void BootModeTimeout();
    void GotFirmwareVersion(QString ver);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class AQEsc32Logger : public QThread
{
    Q_OBJECT
public:
    AQEsc32Logger();
    ~AQEsc32Logger();
    void run();
    bool isFinished();
    float getTelemValueAvgs(int index);
    void setTelemValueMaxs(int index, float value);
    float getTelemValueMaxs(int index);
    float getTelemStorage(int index);
    int getTelemStorageNum();
    void StartStopDecoding(bool Start);
    void startLoggingTelemetry(SerialLink *link, QString FileName);
    void stopLoggingTelemetry();

private:
    void esc32InChecksum(unsigned char c);
    volatile float TelemData[256][AQEsc32::BINARY_VALUE_NUM];
    volatile float TelemValueAvgs[AQEsc32::BINARY_VALUE_NUM];
    volatile float TelemValueMaxs[AQEsc32::BINARY_VALUE_NUM];
    volatile qint64 TelemStorageNum;
    float *TelemStorage;
    unsigned char checkInA, checkInB;
    unsigned short esc32GetShort(QByteArray data, int startIndex);
    float esc32GetFloat(QByteArray data, int startIndex);
    unsigned char rows;
    unsigned char cols;
    qint64 bitsReceivedTotal;
    qint64 connectionStartTime;
    FILE *TelemOutFile;
    QMutex dataMutex;
    bool StartStop;
public slots:
    void teleDataReceived(QByteArray data, int rows, int cols);

protected:
    SerialLink* seriallinkEsc32;

};


#endif // AQ_COMM_H
