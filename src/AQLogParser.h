#ifndef AQLOGPARSER_H
#define AQLOGPARSER_H

#include <QFileDialog>
#include <cmath>
#include <QDebug>
#include <QPair>
#include <QCoreApplication>
#include <cstdint>

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
        LOG_ACC_BIAS_X,
        LOG_ACC_BIAS_Y,
        LOG_ACC_BIAS_Z,
        LOG_CURRENT_PDB,
        LOG_CURRENT_EXT,
        LOG_VIN_PDB,
        LOG_UKF_ALT_VEL,
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


#endif // AQLOGPARSER_H
