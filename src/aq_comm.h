#ifndef AQ_COMM_H
#define AQ_COMM_H

#include <QFileDialog>
#include <cmath>
#include <QDebug>
#include <QQueue>
#include <QPair>
#include <QCoreApplication>
#include <QLineEdit>
#include "Eigen/Eigen"
#include "UASInterface.h"
#include <SerialLinkInterface.h>
#include <SerialLink.h>
#include <QProcess>

class AQLogParser
{
public:
    explicit AQLogParser();
    ~AQLogParser();

    typedef struct {
        unsigned int lastUpdate;
        float voltages[15];
        float rate[3];
        float acc[3];
        float mag[3];
        float pressure[2];
        float temp[4];
        float vIn;
        float quat[4];
        float rateAux[3];
        float accAux[3];
        float magAux[3];
        unsigned int gpsPosUpdate;
        double lat, lon;
        float gpsAlt;
        float gpsPosAcc;
        unsigned int gpsVelUpdate;
        float gpsVel[3];
        float gpsVelAcc;
        float pos[3];
        float vel[3];
        short int motors[14];
        short int throttle;
        float extra[4];
        char ckA, ckB;
    } loggerRecord_t;

    enum fieldsT{
        LOG_TYPE_DOUBLE = 0,
        LOG_TYPE_FLOAT,
        LOG_TYPE_U32,
        LOG_TYPE_S32,
        LOG_TYPE_U16,
        LOG_TYPE_S16,
        LOG_TYPE_U8,
        LOG_TYPE_S8
    };

    enum fieldsN{
        LOG_LASTUPDATE = 0,
        LOG_VOLTAGE0,
        LOG_VOLTAGE1,
        LOG_VOLTAGE2,
        LOG_VOLTAGE3,
        LOG_VOLTAGE4,
        LOG_VOLTAGE5,
        LOG_VOLTAGE6,
        LOG_VOLTAGE7,
        LOG_VOLTAGE8,
        LOG_VOLTAGE9,
        LOG_VOLTAGE10,
        LOG_VOLTAGE11,
        LOG_VOLTAGE12,
        LOG_VOLTAGE13,
        LOG_VOLTAGE14,
        LOG_IMU_RATEX,
        LOG_IMU_RATEY,
        LOG_IMU_RATEZ,
        LOG_IMU_ACCX,
        LOG_IMU_ACCY,
        LOG_IMU_ACCZ,
        LOG_IMU_MAGX,
        LOG_IMU_MAGY,
        LOG_IMU_MAGZ,
        LOG_GPS_PDOP,
        LOG_GPS_HDOP,
        LOG_GPS_VDOP,
        LOG_GPS_TDOP,
        LOG_GPS_NDOP,
        LOG_GPS_EDOP,
        LOG_GPS_ITOW,
        LOG_GPS_POS_UPDATE,
        LOG_GPS_LAT,
        LOG_GPS_LON,
        LOG_GPS_HEIGHT,
        LOG_GPS_HACC,
        LOG_GPS_VACC,
        LOG_GPS_VEL_UPDATE,
        LOG_GPS_VELN,
        LOG_GPS_VELE,
        LOG_GPS_VELD,
        LOG_GPS_SACC,
        LOG_ADC_PRESSURE1,
        LOG_ADC_PRESSURE2,
        LOG_ADC_TEMP0,
        LOG_ADC_TEMP1,
        LOG_ADC_TEMP2,
        LOG_ADC_VIN,
        LOG_ADC_MAG_SIGN,
        LOG_UKF_Q1,
        LOG_UKF_Q2,
        LOG_UKF_Q3,
        LOG_UKF_Q4,
        LOG_UKF_POSN,
        LOG_UKF_POSE,
        LOG_UKF_POSD,
        LOG_UKF_PRES_ALT,
        LOG_UKF_ALT,
        LOG_UKF_VELN,
        LOG_UKF_VELE,
        LOG_UKF_VELD,
        LOG_MOT_MOTOR0,
        LOG_MOT_MOTOR1,
        LOG_MOT_MOTOR2,
        LOG_MOT_MOTOR3,
        LOG_MOT_MOTOR4,
        LOG_MOT_MOTOR5,
        LOG_MOT_MOTOR6,
        LOG_MOT_MOTOR7,
        LOG_MOT_MOTOR8,
        LOG_MOT_MOTOR9,
        LOG_MOT_MOTOR10,
        LOG_MOT_MOTOR11,
        LOG_MOT_MOTOR12,
        LOG_MOT_MOTOR13,
        LOG_MOT_THROTTLE,
        LOG_MOT_PITCH,
        LOG_MOT_ROLL,
        LOG_MOT_YAW,
        LOG_RADIO_QUALITY,
        LOG_RADIO_CHANNEL0,
        LOG_RADIO_CHANNEL1,
        LOG_RADIO_CHANNEL2,
        LOG_RADIO_CHANNEL3,
        LOG_RADIO_CHANNEL4,
        LOG_RADIO_CHANNEL5,
        LOG_RADIO_CHANNEL6,
        LOG_RADIO_CHANNEL7,
        LOG_RADIO_CHANNEL8,
        LOG_RADIO_CHANNEL9,
        LOG_RADIO_CHANNEL10,
        LOG_RADIO_CHANNEL11,
        LOG_RADIO_CHANNEL12,
        LOG_RADIO_CHANNEL13,
        LOG_RADIO_CHANNEL14,
        LOG_RADIO_CHANNEL15,
        LOG_RADIO_CHANNEL16,
        LOG_RADIO_CHANNEL17,
        LOG_RADIO_ERRORS,
        LOG_GMBL_TRIGGER,
        LOG_NUM_IDS
    };

    enum fieldsO {
        MICROS = 0,
        VOLTAGE1,
        VOLTAGE2,
        VOLTAGE3,
        VOLTAGE4,
        VOLTAGE5,
        VOLTAGE6,
        VOLTAGE7,
        VOLTAGE8,
        VOLTAGE9,
        VOLTAGE10,
        VOLTAGE11,
        VOLTAGE12,
        VOLTAGE13,
        VOLTAGE14,
        VOLTAGE15,
        RATEX,
        RATEY,
        RATEZ,
        ACCX,
        ACCY,
        ACCZ,
        MAGX,
        MAGY,
        MAGZ,
        PRESSURE1,
        PRESSURE2,
        TEMP1,
        TEMP2,
        TEMP3,
        TEMP4,
        AUX_RATEX,
        AUX_RATEY,
        AUX_RATEZ,
        AUX_ACCX,
        AUX_ACCY,
        AUX_ACCZ,
        AUX_MAGX,
        AUX_MAGY,
        AUX_MAGZ,
        VIN,
        GPS_POS_UPDATE,
        LAT,
        LON,
        GPS_ALT,
        GPS_POS_ACC,
        GPS_VEL_UPDATE,
        GPS_VELN,
        GPS_VELE,
        GPS_VELD,
        GPS_VEL_ACC,
        POSN,
        POSE,
        POSD,
        VELN,
        VELE,
        VELD,
        QUAT0,
        QUAT1,
        QUAT2,
        QUAT3,
        MOTOR1,
        MOTOR2,
        MOTOR3,
        MOTOR4,
        MOTOR5,
        MOTOR6,
        MOTOR7,
        MOTOR8,
        MOTOR9,
        MOTOR10,
        MOTOR11,
        MOTOR12,
        MOTOR13,
        MOTOR14,
        THROTTLE,
        EXTRA1,
        EXTRA2,
        EXTRA3,
        EXTRA4,
        NUM_FIELDS
    };

    typedef struct {
        uint8_t fieldId;
        uint8_t fieldType;
    } loggerFields_t;

    typedef struct {
        uint8_t fieldId;
        uint8_t fieldType;
        uint8_t fieldActive;
        QVariant fieldValue;
        QString fieldName;
    } loggerFieldsAndActive_t;

    enum longOptions {
            O_MICROS,   //0
            O_VOLTAGES, //1
            O_RATES, //2
            O_ACCS, //3
            O_MAGS, //4
            O_PRESSURES, //5
            O_TEMPS, //6
            O_AUX_RATES, //7
            O_AUX_ACCS, //8
            O_AUX_MAGS, //9
            O_VIN, //10
            O_GPS_POS_MICROS, //11
            O_LAT, //12
            O_LON, //13
            O_GPS_ALT, //14
            O_GPS_POS_ACC, //15
            O_GPS_VEL_MICROS, //16
            O_GPS_VELS, //17
            O_GPS_VEL_ACC, //18
            O_POSS, //19
            O_VELS, //20
            O_QUAT, //21
            O_MOTORS, //22
            O_THROTTLE, //23
            O_EXTRAS //24
        };

    int ParseLogHeader(QString fileName);
    QList<QPair<QString,loggerFieldsAndActive_t> > LogChannelsStruct;
    void ShowCurves();
    void ResetLog();
    bool getOldLog();
    QMap<QString, QVector<double>*> xValues;
    QMap<QString, QVector<double>*> yValues;
    int GetFrameSize();
    void ReWriteFile(QString SourceFileName,QString DestinationFileName, int start1, int end1, int start2, int end2);

private:
    int loggerReadHeader(FILE *fp);
    int loggerWriteHeader(FILE *fs, FILE *fd);
    void loggerWriteDataM(FILE *fs, FILE *fd, int start1, int end1, int start2, int end2);
    void loggerWriteDataL(FILE *fs, FILE *fd, int start1, int end1, int start2, int end2);
    loggerFields_t *logHeader;
    QString GetChannelsName(uint8_t fieldId);
    int LoggerFrameSize;
    void GenerateChannelsCurve();
    int ParseLogM(FILE *fp);
    int loggerReadEntryM(FILE *fp);
    QString FileName;
    uint32_t xAxisCount;
    void createHeaderL();
    void SetChannelsStruct();
    bool oldLog;
    loggerRecord_t logEntry;
    int loggerReadEntry(FILE *fp, loggerRecord_t *r);
    double logDumpGetValue(loggerRecord_t *l, int field);
    long PosOfCrcError;
    int CRCErrorCnt;
};

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
