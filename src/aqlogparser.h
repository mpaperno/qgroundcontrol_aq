#ifndef AQLOGPARSER_H
#define AQLOGPARSER_H

#include <QFileDialog>
#include <cmath>
#include <QDebug>
#ifdef Q_OS_WIN

#pragma pack(push)
#pragma pack(1)
typedef struct {
    unsigned int lastUpdate; //0
    float voltages[15]; //1
    float rate[3]; //2
    float acc[3]; //3
    float mag[3];  //4
    float pressure[2];  //5
    float temp[4];  //6
    float vIn;  //7
    float quat[4];  //8
    float rateAux[3];  //9
    float accAux[3];  //10
    float magAux[3];  //11
    unsigned int gpsPosUpdate;  //12
    double lat, lon; //13
    float gpsAlt;  //14
    float gpsPosAcc; //15
    unsigned int gpsVelUpdate; //16
    float gpsVel[3]; //17
    float gpsVelAcc;  //18
    float pos[3];  //19
    float vel[3];  //20
    short int motors[14];  //21
    short int throttle;  //22
    float extra[4];  //23
    char ckA, ckB;  //24
} loggerRecord_t;
#pragma pack(pop)

#else
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
} __attribute__((packed)) loggerRecord_t;
#endif

enum fields {
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
    GPS_POS_MICROS,
    LAT,
    LON,
    GPS_ALT,
    GPS_POS_ACC,
    GPS_VEL_MICROS,
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


class AQLogParser
{
public:
    explicit AQLogParser();
    ~AQLogParser();
    void ParseLogFile(QString fileName);
    void SetChannels(int section, int subsection);
    void RemoveChannels();
    void ResetLog();
    QMap<QString, QVector<double>*> xValues;
    QMap<QString, QVector<double>*> yValues;

private:
    int count;
    void logDumpPlotInit(int n);
    int loggerReadEntry(FILE *fp, loggerRecord_t *r);
    double logDumpGetValue(loggerRecord_t *l, int field);
    void logDumpStats(loggerRecord_t *l);
    int loggerReadLog(const char *fname, loggerRecord_t **l);
    void loggerFree(loggerRecord_t *l);
    loggerRecord_t logEntry;
    int dumpOrder[NUM_FIELDS];
    float dumpMin[NUM_FIELDS];
    float dumpMax[NUM_FIELDS];
    int dumpNum;
    QStringList CurveName;
    float scaleMin, scaleMax;


signals:
    void finishedParse();

};

#endif // AQLOGPARSER_H
