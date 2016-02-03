#include "AQLogParser.h"

AQLogParser::AQLogParser()
{
    xValues.clear();
    yValues.clear();
    LoggerFrameSize = 0;
    if ( !xValues.contains("xvalue"))
        xValues.insert("xvalue", new QVector<double>());
}

AQLogParser::~AQLogParser()
{
}

void AQLogParser::ResetLog()
{
    xValues.clear();
    yValues.clear();
    LoggerFrameSize = 0;
}

int AQLogParser::ParseLogHeader(QString fileName)
{
    FILE *lf;
    fileName = QDir::toNativeSeparators(fileName);
    FileName = fileName;
    lf = fopen(fileName.toLocal8Bit().constData(),"rb");
    oldLog = false;
    xAxisCount = 0;
    if (lf) {
        LogChannelsStruct.clear();
        if (loggerReadHeader(lf) == 0 ) {
            fclose(lf);
            return 0;
        }
    }
    fclose(lf);
    return -1;
}

int AQLogParser::loggerReadHeader(FILE *fp)
{
    char ckA, ckB = 0;
    char ckA_Calculate, ckB_Calculate = 0;
    int i;
    int c = 0;
    uint8_t count_channels = 0;
    loggerFieldsAndActive_t fieldsInfo;
    LogChannelsStruct.clear();
    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fp)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fp)) != 'q')
            goto loggerTop;
        if ((c = fgetc(fp)) != 'H') {
            if (c != 'L')
                goto loggerTop;
            else {
                oldLog = true;
                createHeaderL();
                return 0;
            }
        }

        count_channels = fgetc(fp);
        logHeader = (loggerFields_t*) calloc(count_channels , sizeof(loggerFields_t));
        fread(logHeader, sizeof(loggerFields_t), count_channels, fp);
        ckA_Calculate = 0;
        ckB_Calculate = 0;
        LoggerFrameSize = 0;
        ckA = fgetc(fp);
        ckB = fgetc(fp);
        ckA_Calculate += count_channels;
        ckB_Calculate += ckA_Calculate;
        for (i = 0; i<count_channels; i++) {
            ckA_Calculate += logHeader[i].fieldId;
            ckB_Calculate += ckA_Calculate;
            ckA_Calculate += logHeader[i].fieldType;
            ckB_Calculate += ckA_Calculate;
            fieldsInfo.fieldActive = false;
            fieldsInfo.fieldId = logHeader[i].fieldId;
            fieldsInfo.fieldType = logHeader[i].fieldType;
            fieldsInfo.fieldName = GetChannelsName(fieldsInfo.fieldId);
            LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));
//            qDebug() << "FieldID=" << logHeader[i].fieldId << " Name" << fieldsInfo.fieldName << " Type" << logHeader[i].fieldType << " list" << LogChannelsStruct.count();

            switch (fieldsInfo.fieldType) {
                case LOG_TYPE_DOUBLE:
                    LoggerFrameSize += 8;
                    break;
                case LOG_TYPE_FLOAT:
                case LOG_TYPE_U32:
                case LOG_TYPE_S32:
                    LoggerFrameSize += 4;
                    break;
                case LOG_TYPE_U16:
                case LOG_TYPE_S16:
                    LoggerFrameSize += 2;
                    break;
                case LOG_TYPE_U8:
                case LOG_TYPE_S8:
                    LoggerFrameSize += 1;
                    break;
            }
        }

        if (ckA_Calculate == ckA && ckB_Calculate == ckB) {
            free(logHeader);
            return 0;
        }
        else {
            LogChannelsStruct.clear();
            free(logHeader);
            qDebug() << "logger: checksum error\n";
            goto loggerTop;
        }
    }

    if (logHeader)
        free(logHeader);
    return -1;
}

QString AQLogParser::GetChannelsName(uint8_t fieldId)
{
    switch (fieldId) {
    case LOG_LASTUPDATE:
        return "LASTUPDATE";
        break;
    case LOG_VOLTAGE0:
        return "VOLTAGE0";
        break;
    case LOG_VOLTAGE1:
        return "VOLTAGE1";
        break;
    case LOG_VOLTAGE2:
        return "VOLTAGE2";
        break;
    case LOG_VOLTAGE3:
        return "VOLTAGE3";
        break;
    case LOG_VOLTAGE4:
        return "VOLTAGE4";
        break;
    case LOG_VOLTAGE5:
        return "VOLTAGE5";
        break;
    case LOG_VOLTAGE6:
        return "VOLTAGE6";
        break;
    case LOG_VOLTAGE7:
        return "VOLTAGE7";
        break;
    case LOG_VOLTAGE8:
        return "VOLTAGE8";
        break;
    case LOG_VOLTAGE9:
        return "VOLTAGE9";
        break;
    case LOG_VOLTAGE10:
        return "VOLTAGE10";
        break;
    case LOG_VOLTAGE11:
        return "VOLTAGE11";
        break;
    case LOG_VOLTAGE12:
        return "VOLTAGE12";
        break;
    case LOG_VOLTAGE13:
        return "VOLTAGE13";
        break;
    case LOG_VOLTAGE14:
        return "VOLTAGE14";
        break;
    case LOG_IMU_RATEX:
        return "IMU_RATEX";
        break;
    case LOG_IMU_RATEY:
        return "IMU_RATEY";
        break;
    case LOG_IMU_RATEZ:
        return "IMU_RATEZ";
        break;
    case LOG_IMU_ACCX:
        return "IMU_ACCX";
        break;
    case LOG_IMU_ACCY:
        return "IMU_ACCY";
        break;
    case LOG_IMU_ACCZ:
        return "IMU_ACCZ";
        break;
    case LOG_IMU_MAGX:
        return "IMU_MAGX";
        break;
    case LOG_IMU_MAGY:
        return "IMU_MAGY";
        break;
    case LOG_IMU_MAGZ:
        return "IMU_MAGZ";
        break;
    case LOG_GPS_PDOP:
        return "GPS_PDOP";
        break;
    case LOG_GPS_HDOP:
        return "GPS_HDOP";
        break;
    case LOG_GPS_VDOP:
        return "GPS_VDOP";
        break;
    case LOG_GPS_TDOP:
        return "GPS_TDOP";
        break;
    case LOG_GPS_NDOP:
        return "GPS_NDOP";
        break;
    case LOG_GPS_EDOP:
        return "GPS_EDOP";
        break;
    case LOG_GPS_ITOW:
        return "GPS_ITOW";
        break;
    case LOG_GPS_POS_UPDATE:
        return "GPS_POS_UPDATE";
        break;
    case LOG_GPS_LAT:
        return "GPS_LAT";
        break;
    case LOG_GPS_LON:
        return "GPS_LON";
        break;
    case LOG_GPS_HEIGHT:
        return "GPS_HEIGHT";
        break;
    case LOG_GPS_HACC:
        return "GPS_HACC";
        break;
    case LOG_GPS_VACC:
        return "GPS_VACC";
        break;
    case LOG_GPS_VEL_UPDATE:
        return "GPS_VEL_UPDATE";
        break;
    case LOG_GPS_VELN:
        return "GPS_VELN";
        break;
    case LOG_GPS_VELE:
        return "GPS_VELE";
        break;
    case LOG_GPS_VELD:
        return "GPS_VELD";
        break;
    case LOG_GPS_SACC:
        return "GPS_SACC";
        break;

    case LOG_ADC_PRESSURE1:
        return "ADC_PRESSURE1";
        break;
    case LOG_ADC_PRESSURE2:
        return "ADC_PRESSURE2";
        break;
    case LOG_ADC_TEMP0:
        return "ADC_TEMP0";
        break;

    case LOG_ADC_TEMP1:
        return "ADC_TEMP1";
        break;
    case LOG_ADC_TEMP2:
        return "ADC_TEMP2";
        break;

    case LOG_ADC_VIN:
        return "ADC_VIN";
        break;
    case LOG_ADC_MAG_SIGN:
        return "ADC_MAG_SIGN";
        break;
    case LOG_UKF_Q1:
        return "UKF_Q1";
        break;
    case LOG_UKF_Q2:
        return "UKF_Q2";
        break;
    case LOG_UKF_Q3:
        return "UKF_Q3";
        break;
    case LOG_UKF_Q4:
        return "UKF_Q4";
        break;
    case LOG_UKF_POSN:
        return "UKF_POSN";
        break;
    case LOG_UKF_POSE:
        return "UKF_POSE";
        break;
    case LOG_UKF_POSD:
        return "UKF_POSD";
        break;
    case LOG_UKF_PRES_ALT:
        return "UKF_PRES_ALT";
        break;
    case LOG_UKF_ALT:
        return "UKF_ALT";
        break;
    case LOG_UKF_VELN:
        return "UKF_VELN";
        break;
    case LOG_UKF_VELE:
        return "UKF_VELE";
        break;
    case LOG_UKF_VELD:
        return "UKF_VELD";
        break;
    case LOG_MOT_MOTOR0:
        return "MOT_MOTOR0";
        break;
    case LOG_MOT_MOTOR1:
        return "MOT_MOTOR1";
        break;
    case LOG_MOT_MOTOR2:
        return "MOT_MOTOR2";
        break;
    case LOG_MOT_MOTOR3:
        return "MOT_MOTOR3";
        break;
    case LOG_MOT_MOTOR4:
        return "MOT_MOTOR4";
        break;
    case LOG_MOT_MOTOR5:
        return "MOT_MOTOR5";
        break;
    case LOG_MOT_MOTOR6:
        return "MOT_MOTOR6";
        break;
    case LOG_MOT_MOTOR7:
        return "MOT_MOTOR7";
        break;
    case LOG_MOT_MOTOR8:
        return "MOT_MOTOR8";
        break;
    case LOG_MOT_MOTOR9:
        return "MOT_MOTOR9";
        break;
    case LOG_MOT_MOTOR10:
        return "MOT_MOTOR10";
        break;
    case LOG_MOT_MOTOR11:
        return "MOT_MOTOR11";
        break;
    case LOG_MOT_MOTOR12:
        return "MOT_MOTOR12";
        break;
    case LOG_MOT_MOTOR13:
        return "MOT_MOTOR13";
        break;

    case LOG_MOT_THROTTLE:
        return "MOT_THROTTLE";
        break;
    case LOG_MOT_PITCH:
        return "MOT_PITCH";
        break;
    case LOG_MOT_ROLL:
        return "MOT_ROLL";
        break;
    case LOG_MOT_YAW:
        return "MOT_YAW";
        break;
    case LOG_RADIO_QUALITY:
        return "RADIO_QUALITY";
        break;
    case LOG_RADIO_CHANNEL0:
        return "RADIO_CHANNEL0";
        break;
    case LOG_RADIO_CHANNEL1:
        return "RADIO_CHANNEL1";
        break;
    case LOG_RADIO_CHANNEL2:
        return "RADIO_CHANNEL2";
        break;
    case LOG_RADIO_CHANNEL3:
        return "RADIO_CHANNEL3";
        break;
    case LOG_RADIO_CHANNEL4:
        return "RADIO_CHANNEL4";
        break;
    case LOG_RADIO_CHANNEL5:
        return "RADIO_CHANNEL5";
        break;
    case LOG_RADIO_CHANNEL6:
        return "RADIO_CHANNEL6";
        break;
    case LOG_RADIO_CHANNEL7:
        return "RADIO_CHANNEL7";
        break;
    case LOG_RADIO_CHANNEL8:
        return "RADIO_CHANNEL8";
        break;
    case LOG_RADIO_CHANNEL9:
        return "RADIO_CHANNEL9";
        break;
    case LOG_RADIO_CHANNEL10:
        return "RADIO_CHANNEL10";
        break;
    case LOG_RADIO_CHANNEL11:
        return "RADIO_CHANNEL11";
        break;
    case LOG_RADIO_CHANNEL12:
        return "RADIO_CHANNEL12";
        break;
    case LOG_RADIO_CHANNEL13:
        return "RADIO_CHANNEL13";
        break;
    case LOG_RADIO_CHANNEL14:
        return "RADIO_CHANNEL14";
        break;
    case LOG_RADIO_CHANNEL15:
        return "RADIO_CHANNEL15";
        break;
    case LOG_RADIO_CHANNEL16:
        return "RADIO_CHANNEL16";
        break;
    case LOG_RADIO_CHANNEL17:
        return "RADIO_CHANNEL17";
        break;
    case LOG_RADIO_ERRORS:
        return "RADIO_ERRORS";
        break;
    case LOG_GMBL_TRIGGER:
        return "GMBL_TRIGGER";
        break;
    case LOG_ACC_BIAS_X:
        return "ACC_BIAS_X";
        break;
    case LOG_ACC_BIAS_Y:
        return "ACC_BIAS_Y";
        break;
    case LOG_ACC_BIAS_Z:
        return "ACC_BIAS_Z";
        break;
    case LOG_CURRENT_PDB:
        return "CURRENT_PDB";
        break;
    case LOG_CURRENT_EXT:
        return "CURRENT_EXT";
        break;
    case LOG_VIN_PDB:
        return "VIN_PDB";
        break;
    case LOG_UKF_ALT_VEL:
        return "UKF_ALT_VEL";
        break;
//    case LOG_NUM_IDS:
//        return "NUM_IDS";
//        break;
    default:
        return "Field_ID_" + QString::number(fieldId);
        break;

    }

}

void AQLogParser::GenerateChannelsCurve() {

    if ( !xValues.contains("XVALUES") )
        xValues.insert("XVALUES", new QVector<double>());

    for ( int i=0; i<LogChannelsStruct.count(); i++ ) {
        QPair<QString,loggerFieldsAndActive_t> val_pair = LogChannelsStruct.at(i);
        loggerFieldsAndActive_t val  = val_pair.second;
        if ( val.fieldActive == 1 )
            yValues.insert(val_pair.first, new QVector<double>());
    }

}

void AQLogParser::ShowCurves() {
    int n = 0;
    int count = 0;
    FILE *lf;
    QString fileName = QDir::toNativeSeparators(FileName);
    xValues.clear();
    yValues.clear();
    xAxisCount = 0;
    lf = fopen(fileName.toLocal8Bit().constData(),"rb");

    if (lf) {

        if (!oldLog) {
            GenerateChannelsCurve();
            CRCErrorCnt = 0;
            PosOfCrcError = 0;
            while (ParseLogM(lf) != EOF) {
                PosOfCrcError++;
                n++;
            }
        } else {
            count = 0;
            GenerateChannelsCurve();
            bool appendXvalues = true;
            for ( int i=0; i<LogChannelsStruct.count(); i++ ) {
                QPair<QString,loggerFieldsAndActive_t> val_pair = LogChannelsStruct.at(i);
                if ( val_pair.second.fieldActive == 1 ) {
                    rewind(lf);
                    while (loggerReadEntry(lf, &logEntry) != EOF) {
                        if ( appendXvalues) {
                            count++;
                            xValues.value("XVALUES")->append(count);
                        }
                        double va = logDumpGetValue(&logEntry, val_pair.second.fieldId);
                        yValues.value(val_pair.first)->append(va);
                    }
                    appendXvalues = false;
                }
            }
        }
    }
    fclose(lf);
}

int AQLogParser::ParseLogM(FILE *fp) {
    int c = 0;
    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fp)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fp)) != 'q')
            goto loggerTop;

        c = fgetc(fp);
        if (c == 'M') {
            if (loggerReadEntryM(fp) == -1)
                goto loggerTop;
            else
                return 1;
        }
        else {
            goto loggerTop;
        }

    }

    return EOF;
}

int AQLogParser::loggerReadEntryM(FILE *fp) {
    char buffer[1024];
    char *buf = buffer;
    unsigned char ckA, ckB;
    int i;
    double tmp_double;
    float tmp_float;
    uint32_t tmp_uint32;
    int32_t tmp_int32;
    uint16_t tmp_uint16;
    int16_t tmp_int16;
    uint8_t tmp_uint8;
    int8_t tmp_int8;

    if (LoggerFrameSize > 0 && fread(buffer, LoggerFrameSize, 1, fp) == 1) {
        // calc checksum
        ckA = ckB = 0;
        for (i = 0; i < LoggerFrameSize; i++) {
            ckA += buf[i];
            ckB += ckA;
        }

        if (fgetc(fp) == ckA && fgetc(fp) == ckB) {
            xValues.value("XVALUES")->append(xAxisCount++);

            for ( int i=0; i<LogChannelsStruct.count(); i++ ) {
                QPair<QString,loggerFieldsAndActive_t> val_pair = LogChannelsStruct.at(i);
                loggerFieldsAndActive_t val  = val_pair.second;
                switch (val.fieldType) {
                case LOG_TYPE_DOUBLE:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_double = *(double *)buf;
                        yValues.value(val.fieldName)->append(tmp_double);
                    }
                    buf += 8;
                    break;
                case LOG_TYPE_FLOAT:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_float = *(float *)buf;
                        yValues.value(val.fieldName)->append(tmp_float);
                    }
                    buf += 4;
                    break;
                case LOG_TYPE_U32:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_uint32 = *(uint32_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_uint32);
                    }
                    buf += 4;
                    break;
                case LOG_TYPE_S32:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_int32 = *(int32_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_int32);
                    }
                    buf += 4;
                    break;
                case LOG_TYPE_U16:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_uint16 = *(uint16_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_uint16);
                    }
                    buf += 2;
                    break;
                case LOG_TYPE_S16:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_int16 = *(int16_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_int16);
                    }
                    buf += 2;
                    break;
                case LOG_TYPE_U8:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_uint8 = *(uint8_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_uint8);
                    }
                    buf += 1;
                    break;
                case LOG_TYPE_S8:
                    if ( val.fieldActive == 1 )
                    {
                        tmp_int8 = *(int8_t *)buf;
                        yValues.value(val.fieldName)->append(tmp_int8);
                    }
                    buf += 1;
                    break;
                }
            }
            return 1;
        }
        else {
            CRCErrorCnt++;
            qDebug() << "logger: checksum error " << CRCErrorCnt << " on frame " << PosOfCrcError;
        }

    }

    return -1;
}

void AQLogParser::createHeaderL() {
    LogChannelsStruct.clear();
    SetChannelsStruct();
}

void AQLogParser::SetChannelsStruct()
{
    loggerFieldsAndActive_t fieldsInfo;

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MICROS;
    fieldsInfo.fieldType = LOG_TYPE_U32;
    fieldsInfo.fieldName = "MICROS";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE1;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE2;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE3;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE3";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE4;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE4";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE5;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE5";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE6;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE6";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE7;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE7";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE8;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE8";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE9;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE9";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE10;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE10";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE11;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE11";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE12;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE12";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE13;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE13";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE14;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE14";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VOLTAGE15;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VOLTAGE15";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = RATEX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "RATEX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = RATEY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "RATEY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = RATEZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "RATEZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = ACCX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "ACCX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = ACCY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "ACCY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = ACCZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "ACCZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MAGX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "MAGX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MAGY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "MAGY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MAGZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "MAGZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = PRESSURE1;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "PRESSURE1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = PRESSURE2;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "PRESSURE2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = TEMP1;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "TEMP1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = TEMP2;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "TEMP2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = TEMP3;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "TEMP3";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = TEMP4;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "TEMP4";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_RATEX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_RATEX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_RATEY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_RATEY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_RATEZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_RATEZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_ACCX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_ACCX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_ACCY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_ACCY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_ACCZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_ACCZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));



    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_MAGX;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_MAGX";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_MAGY;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_MAGY";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_MAGZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_MAGZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = AUX_MAGZ;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "AUX_MAGZ";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VIN;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VIN";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_POS_UPDATE;
    fieldsInfo.fieldType = LOG_TYPE_U32;
    fieldsInfo.fieldName = "GPS_POS_UPDATE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = LAT;
    fieldsInfo.fieldType = LOG_TYPE_DOUBLE;
    fieldsInfo.fieldName = "LAT";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = LON;
    fieldsInfo.fieldType = LOG_TYPE_DOUBLE;
    fieldsInfo.fieldName = "LON";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_ALT;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_ALT";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_POS_ACC;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_POS_ACC";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_VEL_UPDATE;
    fieldsInfo.fieldType = LOG_TYPE_U32;
    fieldsInfo.fieldName = "GPS_VEL_UPDATE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));



    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_VELN;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_VELN";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_VELE;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_VELE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_VELD;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_VELD";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = GPS_VEL_ACC;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "GPS_VEL_ACC";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));



    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = POSN;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "POSN";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = POSE;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "POSE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = POSD;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "POSD";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VELN;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VELN";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VELE;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VELE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = VELD;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "VELD";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = QUAT0;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "QUAT0";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = QUAT1;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "QUAT1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = QUAT2;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "QUAT2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = QUAT3;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "QUAT3";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));



    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR1;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR2;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR3;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR3";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR4;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR4";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR5;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR5";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR6;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR6";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR7;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR7";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR8;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR8";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR9;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR9";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR10;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR10";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR11;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR11";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR12;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR12";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR13;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR13";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = MOTOR14;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "MOTOR14";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = THROTTLE;
    fieldsInfo.fieldType = LOG_TYPE_U16;
    fieldsInfo.fieldName = "THROTTLE";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));


    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = EXTRA1;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "EXTRA1";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = EXTRA2;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "EXTRA2";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = EXTRA3;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "EXTRA3";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));

    fieldsInfo.fieldActive = false;
    fieldsInfo.fieldId = EXTRA4;
    fieldsInfo.fieldType = LOG_TYPE_FLOAT;
    fieldsInfo.fieldName = "EXTRA4";
    LogChannelsStruct.append(qMakePair(fieldsInfo.fieldName, fieldsInfo));
}

bool AQLogParser::getOldLog(){
    return oldLog;
}

#if defined(_MSC_VER) && _MSC_VER >= 1700
#pragma optimize("", off)
#endif
int AQLogParser::loggerReadEntry(FILE *fp, loggerRecord_t *r)
{
    char *buf = (char *)r;
    char ckA, ckB;
    int c = 0;

    while (c != EOF) {
        if ((c = fgetc(fp)) != 'A')
            continue;
        if ((c = fgetc(fp)) != 'q')
            continue;
        if ((c = fgetc(fp)) != 'L')
            continue;

        if (fread(buf, sizeof(loggerRecord_t), 1, fp) == 1) {
            // calc checksum
            ckA = ckB = 0;
            for (unsigned i = 0; i < sizeof(loggerRecord_t) - 2; i++) {
                ckA += buf[i];
                ckB += ckA;
            }

            if (r->ckA == ckA && r->ckB == ckB) {
                return 1;
            }
            else {
                qDebug() << "logger: checksum error\n";
                continue;
            }
        }
    }

    return -1;
}
#if defined(_MSC_VER) && _MSC_VER >= 1700
#pragma optimize("", on)
#endif

double AQLogParser::logDumpGetValue(loggerRecord_t *l, int field)
{
    double val;

    switch (field) {
    case MICROS:
        val =  l->lastUpdate;
        break;
    case VOLTAGE1:
        val = l->voltages[0];
        break;
    case VOLTAGE2:
        val = l->voltages[1];
        break;
    case VOLTAGE3:
        val = l->voltages[2];
        break;
    case VOLTAGE4:
        val = l->voltages[3];
        break;
    case VOLTAGE5:
        val = l->voltages[4];
        break;
    case VOLTAGE6:
        val = l->voltages[5];
        break;
    case VOLTAGE7:
        val = l->voltages[6];
        break;
    case VOLTAGE8:
        val = l->voltages[7];
        break;
    case VOLTAGE9:
        val = l->voltages[8];
        break;
    case VOLTAGE10:
        val = l->voltages[9];
        break;
    case VOLTAGE11:
        val = l->voltages[10];
        break;
    case VOLTAGE12:
        val = l->voltages[11];
        break;
    case VOLTAGE13:
        val = l->voltages[12];
        break;
    case VOLTAGE14:
        val = l->voltages[13];
        break;
    case VOLTAGE15:
        val = l->voltages[14];
        break;
    case RATEX:
        val = l->rate[0];
        break;
    case RATEY:
        val = l->rate[1];
        break;
    case RATEZ:
        val = l->rate[2];
        break;
    case ACCX:
        val = l->acc[0];
        break;
    case ACCY:
        val = l->acc[1];
        break;
    case ACCZ:
        val = l->acc[2];
        break;
    case MAGX:
        val = l->mag[0];
        break;
    case MAGY:
        val = l->mag[1];
        break;
    case MAGZ:
        val = l->mag[2];
        break;
    case PRESSURE1:
        val = l->pressure[0];
        break;
    case PRESSURE2:
        val = l->pressure[1];
        break;
    case TEMP1:
        val = l->temp[0];
        break;
    case TEMP2:
        val = l->temp[1];
        break;
    case TEMP3:
        val = l->temp[2];
        break;
    case TEMP4:
        val = l->temp[3];
        break;
    case AUX_RATEX:
        val = l->rateAux[0];
        break;
    case AUX_RATEY:
        val = l->rateAux[1];
        break;
    case AUX_RATEZ:
        val = l->rateAux[2];
        break;
    case AUX_ACCX:
        val = l->accAux[0];
        break;
    case AUX_ACCY:
        val = l->accAux[1];
        break;
    case AUX_ACCZ:
        val = l->accAux[2];
        break;
    case AUX_MAGX:
        val = l->magAux[0];
        break;
    case AUX_MAGY:
        val = l->magAux[1];
        break;
    case AUX_MAGZ:
        val = l->magAux[2];
        break;
    case VIN:
        val = l->vIn;
        break;
    case GPS_POS_UPDATE:
        val = l->gpsPosUpdate;
        break;
    case LAT:
        val = l->lat;
        break;
    case LON:
        val = l->lon;
        break;
    case GPS_ALT:
        val = l->gpsAlt;
        break;
    case GPS_POS_ACC:
        val = l->gpsPosAcc;
        break;
    case GPS_VEL_UPDATE:
        val = l->gpsVelUpdate;
        break;
    case GPS_VELN:
        val = l->gpsVel[0];
        break;
    case GPS_VELE:
        val = l->gpsVel[1];
        break;
    case GPS_VELD:
        val = l->gpsVel[2];
        break;
    case GPS_VEL_ACC:
        val = l->gpsVelAcc;
        break;
    case POSN:
        val = l->pos[0];
        break;
    case POSE:
        val = l->pos[1];
        break;
    case POSD:
        val = l->pos[2];
        break;
    case VELN:
        val = l->vel[0];
        break;
    case VELE:
        val = l->vel[1];
        break;
    case QUAT0:
        val = l->quat[0];
        break;
    case QUAT1:
        val = l->quat[1];
        break;
    case QUAT2:
        val = l->quat[2];
        break;
    case QUAT3:
        val = l->quat[3];
        break;
    case VELD:
        val = l->vel[2];
        break;
    case MOTOR1:
        val = l->motors[0];
        break;
    case MOTOR2:
        val = l->motors[1];
        break;
    case MOTOR3:
        val = l->motors[2];
        break;
    case MOTOR4:
        val = l->motors[3];
        break;
    case MOTOR5:
        val = l->motors[4];
        break;
    case MOTOR6:
        val = l->motors[5];
        break;
    case MOTOR7:
        val = l->motors[6];
        break;
    case MOTOR8:
        val = l->motors[7];
        break;
    case MOTOR9:
        val = l->motors[8];
        break;
    case MOTOR11:
        val = l->motors[9];
        break;
    case MOTOR12:
        val = l->motors[10];
        break;
    case MOTOR13:
        val = l->motors[11];
        break;
    case MOTOR14:
        val = l->motors[12];
        break;
    case THROTTLE:
        val = l->throttle;
        break;
    case EXTRA1:
        val = l->extra[0];
        break;
    case EXTRA2:
        val = l->extra[1];
        break;
    case EXTRA3:
        val = l->extra[2];
        break;
    case EXTRA4:
        val = l->extra[3];
        break;
    }

    return val;
}

int AQLogParser::GetFrameSize() {
    return LoggerFrameSize;
}

void AQLogParser::ReWriteFile(QString SourceFileName,QString DestinationFileName, int start1, int end1, int start2, int end2) {
    FILE *sf;
    QString fileNameSource;
    fileNameSource = QDir::toNativeSeparators(SourceFileName);
    sf = fopen(fileNameSource.toLocal8Bit().constData(),"rb");

    FILE *df;
    QString fileNameDestination;
    fileNameDestination = QDir::toNativeSeparators(DestinationFileName);
    df = fopen(fileNameDestination.toLocal8Bit().constData(),"wb");

    loggerWriteHeader(sf,df);

    if ( oldLog ) {
        loggerWriteDataL(sf,df, start1,end1,start2,end2);
    }
    else {
        loggerWriteDataM(sf,df, start1,end1,start2,end2);
    }


    fclose(sf);
    fclose(df);
}

int AQLogParser::loggerWriteHeader(FILE *fs, FILE *fd)
{
    char ckA, ckB = 0;
    char ckA_Calculate, ckB_Calculate = 0;
    int i;
    int c = 0;
    uint8_t count_channels = 0;
    loggerFieldsAndActive_t fieldsInfo;
    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fs)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fs)) != 'q')
            goto loggerTop;
        if ((c = fgetc(fs)) != 'H') {
            if (c == 'L' ) {
                oldLog = true;
                return 0;
            }
            goto loggerTop;
        }
        oldLog = false;

        count_channels = fgetc(fs);
        logHeader = (loggerFields_t*) calloc(count_channels , sizeof(loggerFields_t));
        fread(logHeader, sizeof(loggerFields_t), count_channels, fs);
        ckA_Calculate = 0;
        ckB_Calculate = 0;
        LoggerFrameSize = 0;
        ckA = fgetc(fs);
        ckB = fgetc(fs);
        ckA_Calculate += count_channels;
        ckB_Calculate += ckA_Calculate;
        for (i = 0; i<count_channels; i++) {
            ckA_Calculate += logHeader[i].fieldId;
            ckB_Calculate += ckA_Calculate;
            ckA_Calculate += logHeader[i].fieldType;
            ckB_Calculate += ckA_Calculate;
            switch (logHeader[i].fieldType) {
                case LOG_TYPE_DOUBLE:
                    LoggerFrameSize += 8;
                    break;
                case LOG_TYPE_FLOAT:
                case LOG_TYPE_U32:
                case LOG_TYPE_S32:
                    LoggerFrameSize += 4;
                    break;
                case LOG_TYPE_U16:
                case LOG_TYPE_S16:
                    LoggerFrameSize += 2;
                    break;
                case LOG_TYPE_U8:
                case LOG_TYPE_S8:
                    LoggerFrameSize += 1;
                    break;
            }
        }

        if (ckA_Calculate == ckA && ckB_Calculate == ckB) {
            fputc('A',fd);
            fputc('q',fd);
            fputc('H',fd);
            fputc(count_channels,fd);
            fwrite(logHeader,sizeof(loggerFields_t),count_channels, fd);
            fputc(ckA,fd);
            fputc(ckB,fd);
            free(logHeader);
            return 0;
        }
        else {
            free(logHeader);
            qDebug() << "logger: checksum error\n";
            goto loggerTop;
        }
    }

    if (logHeader)
        free(logHeader);
    return -1;
}

void AQLogParser::loggerWriteDataM(FILE *fs, FILE *fd, int start1, int end1, int start2, int end2) {
    int c = 0;
    char buffer[1024];
    char *buf = buffer;
    unsigned char ckA, ckB;
    int i;
    int DataSetCount = 0;
    bool WriteToDestination = true;
    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fs)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fs)) != 'q')
            goto loggerTop;

        c = fgetc(fs);
        if (c == 'M') {

            if (LoggerFrameSize > 0 && fread(buffer, LoggerFrameSize, 1, fs) == 1) {
                // calc checksum
                ckA = ckB = 0;
                for (i = 0; i < LoggerFrameSize; i++) {
                    ckA += buf[i];
                    ckB += ckA;
                }

                if (fgetc(fs) == ckA && fgetc(fs) == ckB) {
                    if ( DataSetCount == start1)
                        WriteToDestination = false;
                    if ( DataSetCount == end1)
                        WriteToDestination = true;

                    if ( DataSetCount == start2)
                        WriteToDestination = false;
                    if ( DataSetCount == end2)
                        WriteToDestination = true;

                    if ( WriteToDestination ) {
                        fputc('A',fd);
                        fputc('q',fd);
                        fputc('M',fd);
                        fwrite(buffer,LoggerFrameSize,1, fd);
                        fputc(ckA,fd);
                        fputc(ckB,fd);
                    }
                    DataSetCount++;
                }
                goto loggerTop;
            }
            else
                goto loggerTop;
        }
        else {
            goto loggerTop;
        }
    }
}

void AQLogParser::loggerWriteDataL(FILE *fs, FILE *fd, int start1, int end1, int start2, int end2) {
    int DataSetCount = 0;
    bool WriteToDestination = true;
    //read the full source file
    char buf[1024];
    //char *buf = buffer;

    char ckA_calc, ckB_calc;
    char ckA, ckB;
    uint i;
    int c = 0;

    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fs)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fs)) != 'q')
            goto loggerTop;
        if ((c = fgetc(fs)) != 'L')
            goto loggerTop;

        if (fread(buf, sizeof(loggerRecord_t), 1, fs) == 1) {
            // calc checksum
            ckA_calc = 0;
            ckB_calc = 0;
            for (i = 0; i < sizeof(loggerRecord_t) - 2; i++) {
                ckA_calc += buf[i];
                ckB_calc += ckA_calc;
            }
            int ind = sizeof(loggerRecord_t);
            ckA = buf[ind-2];
            ckB = buf[ind-1];
            if (ckA_calc == ckA && ckB_calc == ckB) {
                if ( DataSetCount == start1)
                    WriteToDestination = false;
                if ( DataSetCount == end1)
                    WriteToDestination = true;

                if ( DataSetCount == start2)
                    WriteToDestination = false;
                if ( DataSetCount == end2)
                    WriteToDestination = true;

                if ( WriteToDestination ) {
                    fputc('A',fd);
                    fputc('q',fd);
                    fputc('L',fd);
                    fwrite(buf,sizeof(loggerRecord_t), 1, fd);
                    fputc(ckA,fd);
                    fputc(ckB,fd);
                }
                DataSetCount++;
                goto loggerTop;
            }
            else {
                fprintf(stderr, "logger: checksum error\n");
                goto loggerTop;
            }
        }
    }
}
