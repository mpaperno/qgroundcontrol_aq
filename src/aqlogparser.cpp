#include "aqlogparser.h"
#include <cmath>
#include <QDebug>
#include <QMap>


AQLogParser::AQLogParser()
{
    dumpNum=0;
    CurveName.clear();
    xValues.clear();
    yValues.clear();
    if ( !xValues.contains("xvalue"))
        xValues.insert("xvalue", new QVector<double>());
}

AQLogParser::~AQLogParser()
{

}

void AQLogParser::ResetLog()
{
    for (int i = 0; i < NUM_FIELDS; i++) {
        dumpMin[i] = +9999999.99;
        dumpMax[i] = -9999999.99;
    }
    xValues.clear();
    yValues.clear();
}

void AQLogParser::ParseLogFile(QString fileName)
{
FILE *lf;
int i;
int count;
fileName = QDir::toNativeSeparators(fileName);
#ifdef Q_OS_WIN
	lf = fopen(fileName.toLocal8Bit().constData(),"rb");
#else
    lf = fopen(fileName.toLocal8Bit().constData(),"rb");
#endif

    if (lf) {
        for (i = 0; i < NUM_FIELDS; i++) {
            dumpMin[i] = +9999999.99;
            dumpMax[i] = -9999999.99;
        }

        if (1 == 1) {
            count = 0;
            while (loggerReadEntry(lf, &logEntry) != EOF) {
                logDumpStats(&logEntry);
                count++;
            }

            logDumpPlotInit(count);

            for (i = 0; i < dumpNum; i++) {
                rewind(lf);
                count = 0;
                while (loggerReadEntry(lf, &logEntry) != EOF) {
                    //yVals[count++] = logDumpGetValue(&logEntry, dumpOrder[i]);
                    double va = logDumpGetValue(&logEntry, dumpOrder[i]);
                    yValues.value(CurveName.at(i))->append(va);
                }

                //plcol0(2+i);
                //plline(count, xVals, yVals);
            }

            //plend();
        }
        else {
            count = 0;
        }

        QString debugOut ;
        debugOut = QString("logDump: %d records X %lu bytes = %4.1f MB\n").arg(count, sizeof(logEntry), (float)count*sizeof(logEntry)/1024/1000);
        qDebug() << "PARAM WIDGET GOT PARAM:" << debugOut;
        debugOut = QString("logDump: %d mins %d seconds @ 200Hz\n").arg(count/200/60, count/200 % 60);
        qDebug() << "PARAM WIDGET GOT PARAM:" << debugOut;
    }
    else {
        qDebug() << "logDump: cannot open logfile\n";
    }
}

void AQLogParser::logDumpPlotInit(int n)
{
    float min = +9999999.99;
    float max = -9999999.99;
    int i;

    for (i = 0; i < dumpNum; i++) {
        if (dumpMin[i] < min)
            min = dumpMin[i];
        if (dumpMax[i] > max)
            max = dumpMax[i];
    }

	#ifdef Q_OS_WIN
		min = scaleMin;
		max = scaleMax;
	#else
    if (!isnan(scaleMin))
        min = scaleMin;
    if (!isnan(scaleMax))
        max = scaleMax;
	#endif
        //plinit();
        //plcol0(15);
        //plenv(0, n, min, max, 0, 0);

        //yValues.value(curveName)->append(y);

        for (i = 0; i < n; i++)
            xValues.value("xvalue")->append(i);
}

int AQLogParser::loggerReadEntry(FILE *fp, loggerRecord_t *r)
{
    char *buf = (char *)r;
    char ckA, ckB;
    int i;
    int c = 0;

    loggerTop:

    if (c != EOF) {
        if ((c = fgetc(fp)) != 'A')
            goto loggerTop;
        if ((c = fgetc(fp)) != 'q')
            goto loggerTop;
        if ((c = fgetc(fp)) != 'L')
            goto loggerTop;

        if (fread(buf, sizeof(loggerRecord_t), 1, fp) == 1) {
            // calc checksum
            ckA = ckB = 0;
            for (i = 0; i < sizeof(loggerRecord_t) - 2; i++) {
                ckA += buf[i];
                ckB += ckA;
            }

            if (r->ckA == ckA && r->ckB == ckB) {
                return 1;
            }
            else {
                fprintf(stderr, "logger: checksum error\n");
                goto loggerTop;
            }
        }
    }

    return EOF;
}

void AQLogParser::logDumpStats(loggerRecord_t *l) {
    int i, j;
    double val;

    for (i = 0; i < dumpNum; i++) {
        val = logDumpGetValue(l, dumpOrder[i]);
        if (val > dumpMax[i])
            dumpMax[i] = val;
        if (val < dumpMin[i])
            dumpMin[i] = val;
    }
}

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
    case GPS_POS_MICROS:
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
    case GPS_VEL_MICROS:
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

void AQLogParser::RemoveChannels()
{
    for (int i = 0; i < NUM_FIELDS; i++) {
        dumpMin[i] = +9999999.99;
        dumpMax[i] = -9999999.99;
        dumpOrder[i] = 0;
    }
    dumpNum =0;
    CurveName.clear();
}

void AQLogParser::SetChannels(int section, int subsection)
{
    //static int longOpt;

    if ( !xValues.contains("xvalue"))
        xValues.insert("xvalue", new QVector<double>());

    switch (section) {

        case O_MICROS:
            if ( subsection == 1) {
                yValues.insert("curve0", new QVector<double>());
                CurveName.append("curve0");
                dumpOrder[dumpNum++] = MICROS;
            }
            break;
        case O_VOLTAGES:
            if ( subsection == 1) {
                yValues.insert("curve1", new QVector<double>());
                CurveName.append("curve1");
                dumpOrder[dumpNum++] = VOLTAGE1;
            }
            if ( subsection == 2) {
                yValues.insert("curve2", new QVector<double>());
                CurveName.append("curve2");
                dumpOrder[dumpNum++] = VOLTAGE2;
            }
            if ( subsection == 3) {
                yValues.insert("curve3", new QVector<double>());
                CurveName.append("curve3");
                dumpOrder[dumpNum++] = VOLTAGE3;
            }
            if ( subsection == 4) {
                yValues.insert("curve4", new QVector<double>());
                CurveName.append("curve4");
                dumpOrder[dumpNum++] = VOLTAGE4;
            }
            if ( subsection == 5) {
                yValues.insert("curve5", new QVector<double>());
                CurveName.append("curve5");
                dumpOrder[dumpNum++] = VOLTAGE5;
            }
            if ( subsection == 6) {
                yValues.insert("curve6", new QVector<double>());
                CurveName.append("curve6");
                dumpOrder[dumpNum++] = VOLTAGE6;
            }
            if ( subsection == 7) {
                yValues.insert("curve7", new QVector<double>());
                CurveName.append("curve7");
                dumpOrder[dumpNum++] = VOLTAGE7;
            }
            if ( subsection == 8) {
                yValues.insert("curve8", new QVector<double>());
                CurveName.append("curve8");
                dumpOrder[dumpNum++] = VOLTAGE8;
            }
            if ( subsection == 9) {
                yValues.insert("curve9", new QVector<double>());
                CurveName.append("curve9");
                dumpOrder[dumpNum++] = VOLTAGE9;
            }
            if ( subsection == 10) {
                yValues.insert("curve10", new QVector<double>());
                CurveName.append("curve10");
                dumpOrder[dumpNum++] = VOLTAGE10;
            }
            if ( subsection == 11) {
                yValues.insert("curve11", new QVector<double>());
                CurveName.append("curve11");
                dumpOrder[dumpNum++] = VOLTAGE11;
            }
            if ( subsection == 12) {
                yValues.insert("curve12", new QVector<double>());
                CurveName.append("curve12");
                dumpOrder[dumpNum++] = VOLTAGE12;
            }
            if ( subsection == 13) {
                yValues.insert("curve13", new QVector<double>());
                CurveName.append("curve13");
                dumpOrder[dumpNum++] = VOLTAGE13;
            }
            if ( subsection == 14) {
                yValues.insert("curve14", new QVector<double>());
                CurveName.append("curve14");
                dumpOrder[dumpNum++] = VOLTAGE14;
            }
            if ( subsection == 15) {
                yValues.insert("curve15", new QVector<double>());
                CurveName.append("curve15");
                dumpOrder[dumpNum++] = VOLTAGE15;
            }
            break;
        case O_RATES:
            if ( subsection == 1) {
                yValues.insert("curve16", new QVector<double>());
                CurveName.append("curve16");
                dumpOrder[dumpNum++] = RATEX;
            }
            if ( subsection == 2) {
                yValues.insert("curve17", new QVector<double>());
                CurveName.append("curve17");
                dumpOrder[dumpNum++] = RATEY;
            }
            if ( subsection == 3) {
                yValues.insert("curve18", new QVector<double>());
                CurveName.append("curve18");
                dumpOrder[dumpNum++] = RATEZ;
            }
            break;
        case O_ACCS:
            if ( subsection == 1) {
                yValues.insert("curve19", new QVector<double>());
                CurveName.append("curve19");
                dumpOrder[dumpNum++] = ACCX;
            }
            if ( subsection == 2) {
                yValues.insert("curve20", new QVector<double>());
                CurveName.append("curve20");
                dumpOrder[dumpNum++] = ACCY;
            }
            if ( subsection == 3) {
                yValues.insert("curve21", new QVector<double>());
                CurveName.append("curve21");
                dumpOrder[dumpNum++] = ACCZ;
            }
            break;
        case O_MAGS:
            if ( subsection == 1) {
                yValues.insert("curve22", new QVector<double>());
                CurveName.append("curve22");
                dumpOrder[dumpNum++] = MAGX;
            }
            if ( subsection == 2) {
                yValues.insert("curve23", new QVector<double>());
                CurveName.append("curve23");
                dumpOrder[dumpNum++] = MAGY;
            }
            if ( subsection == 3) {
                yValues.insert("curve24", new QVector<double>());
                CurveName.append("curve24");
                dumpOrder[dumpNum++] = MAGZ;
            }
            break;
        case O_PRESSURES:
            if ( subsection == 1) {
                yValues.insert("curve25", new QVector<double>());
                CurveName.append("curve25");
                dumpOrder[dumpNum++] = PRESSURE1;
            }
            if ( subsection == 2) {
                yValues.insert("curve26", new QVector<double>());
                CurveName.append("curve26");
                dumpOrder[dumpNum++] = PRESSURE2;
            }
            break;
        case O_TEMPS:
            if ( subsection == 1) {
                yValues.insert("curve27", new QVector<double>());
                CurveName.append("curve27");
                dumpOrder[dumpNum++] = TEMP1;
            }
            if ( subsection == 2) {
                yValues.insert("curve28", new QVector<double>());
                CurveName.append("curve28");
                dumpOrder[dumpNum++] = TEMP2;
            }
            if ( subsection == 3) {
                yValues.insert("curve29", new QVector<double>());
                CurveName.append("curve29");
                dumpOrder[dumpNum++] = TEMP3;
            }
            if ( subsection == 4) {
                yValues.insert("curve30", new QVector<double>());
                CurveName.append("curve30");
                dumpOrder[dumpNum++] = TEMP4;
            }
            break;
        case O_AUX_RATES:
            if ( subsection == 1) {
                yValues.insert("curve31", new QVector<double>());
                CurveName.append("curve31");
                dumpOrder[dumpNum++] = AUX_RATEX;
            }
            if ( subsection == 2) {
                yValues.insert("curve32", new QVector<double>());
                CurveName.append("curve32");
                dumpOrder[dumpNum++] = AUX_RATEY;
            }
            if ( subsection == 3) {
                yValues.insert("curve33", new QVector<double>());
                CurveName.append("curve33");
                dumpOrder[dumpNum++] = AUX_RATEZ;
            }
            break;
        case O_AUX_ACCS:
            if ( subsection == 1) {
                yValues.insert("curve34", new QVector<double>());
                CurveName.append("curve34");
                dumpOrder[dumpNum++] = AUX_ACCX;
            }
            if ( subsection == 2) {
                yValues.insert("curve35", new QVector<double>());
                CurveName.append("curve35");
                dumpOrder[dumpNum++] = AUX_ACCY;
            }
            if ( subsection == 3) {
                yValues.insert("curve36", new QVector<double>());
                CurveName.append("curve36");
                dumpOrder[dumpNum++] = AUX_ACCZ;
            }
            break;
        case O_AUX_MAGS:
            if ( subsection == 1) {
                yValues.insert("curve37", new QVector<double>());
                CurveName.append("curve37");
                dumpOrder[dumpNum++] = AUX_MAGX;
            }
            if ( subsection == 2) {
                yValues.insert("curve38", new QVector<double>());
                CurveName.append("curve38");
                dumpOrder[dumpNum++] = AUX_MAGY;
            }
            if ( subsection == 3) {
                yValues.insert("curve39", new QVector<double>());
                CurveName.append("curve39");
                dumpOrder[dumpNum++] = AUX_MAGZ;
            }
            break;
        case O_VIN:
            if ( subsection == 1) {
                yValues.insert("curve40", new QVector<double>());
                CurveName.append("curve40");
                dumpOrder[dumpNum++] = VIN;
            }
            break;
        case O_GPS_POS_MICROS:
            if ( subsection == 1) {
                yValues.insert("curve41", new QVector<double>());
                CurveName.append("curve41");
                dumpOrder[dumpNum++] = GPS_POS_MICROS;
            }
            break;
        case O_LAT:
            if ( subsection == 1) {
                yValues.insert("curve42", new QVector<double>());
                CurveName.append("curve42");
                dumpOrder[dumpNum++] = LAT;
            }
            break;
        case O_LON:
            if ( subsection == 1) {
                yValues.insert("curve43", new QVector<double>());
                CurveName.append("curve43");
                dumpOrder[dumpNum++] = LON;
            }
            break;
        case O_GPS_ALT:
            if ( subsection == 1) {
                yValues.insert("curve44", new QVector<double>());
                CurveName.append("curve44");
                dumpOrder[dumpNum++] = GPS_ALT;
            }
            break;
        case O_GPS_POS_ACC:
            if ( subsection == 1) {
                yValues.insert("curve45", new QVector<double>());
                CurveName.append("curve45");
                dumpOrder[dumpNum++] = GPS_POS_ACC;
            }
            break;
        case O_GPS_VEL_MICROS:
            if ( subsection == 1) {
                yValues.insert("curve46", new QVector<double>());
                CurveName.append("curve46");
                dumpOrder[dumpNum++] = GPS_VEL_MICROS;
            }
            break;
        case O_GPS_VELS:
            if ( subsection == 1) {
                yValues.insert("curve47", new QVector<double>());
                CurveName.append("curve47");
                dumpOrder[dumpNum++] = GPS_VELN;
            }
            if ( subsection == 2) {
                yValues.insert("curve48", new QVector<double>());
                CurveName.append("curve48");
                dumpOrder[dumpNum++] = GPS_VELE;
            }
            if ( subsection == 3) {
                yValues.insert("curve49", new QVector<double>());
                CurveName.append("curve49");
                dumpOrder[dumpNum++] = GPS_VELD;
            }
            break;
        case O_GPS_VEL_ACC:
            if ( subsection == 1) {
                yValues.insert("curve50", new QVector<double>());
                CurveName.append("curve50");
                dumpOrder[dumpNum++] = GPS_VEL_ACC;
            }
            break;
        case O_POSS:
            if ( subsection == 1) {
                yValues.insert("curve51", new QVector<double>());
                CurveName.append("curve51");
                dumpOrder[dumpNum++] = POSN;
            }
            if ( subsection == 2) {
                yValues.insert("curve52", new QVector<double>());
                CurveName.append("curve52");
                dumpOrder[dumpNum++] = POSE;
            }
            if ( subsection == 3) {
                yValues.insert("curve53", new QVector<double>());
                CurveName.append("curve53");
                dumpOrder[dumpNum++] = POSD;
            }
            break;
        case O_VELS:
            if ( subsection == 1) {
                yValues.insert("curve54", new QVector<double>());
                CurveName.append("curve54");
                dumpOrder[dumpNum++] = VELN;
            }
            if ( subsection == 2) {
                yValues.insert("curve55", new QVector<double>());
                CurveName.append("curve55");
                dumpOrder[dumpNum++] = VELE;
            }
            if ( subsection == 3) {
                yValues.insert("curve56", new QVector<double>());
                CurveName.append("curve56");
                dumpOrder[dumpNum++] = VELD;
            }
            break;
        case O_QUAT:
            if ( subsection == 1) {
                yValues.insert("curve54", new QVector<double>());
                CurveName.append("curve54");
                dumpOrder[dumpNum++] = QUAT0;
            }
            if ( subsection == 2) {
                yValues.insert("curve55", new QVector<double>());
                CurveName.append("curve55");
                dumpOrder[dumpNum++] = QUAT1;
            }
            if ( subsection == 3) {
                yValues.insert("curve56", new QVector<double>());
                CurveName.append("curve56");
                dumpOrder[dumpNum++] = QUAT2;
            }
            if ( subsection == 4) {
                yValues.insert("curve57", new QVector<double>());
                CurveName.append("curve57");
                dumpOrder[dumpNum++] = QUAT3;
            }
            break;
        case O_MOTORS:
            if ( subsection == 1) {
                yValues.insert("curve58", new QVector<double>());
                CurveName.append("curve58");
                dumpOrder[dumpNum++] = MOTOR1;
            }
            if ( subsection == 2) {
                yValues.insert("curve59", new QVector<double>());
                CurveName.append("curve59");
                dumpOrder[dumpNum++] = MOTOR2;
            }
            if ( subsection == 3) {
                yValues.insert("curve60", new QVector<double>());
                CurveName.append("curve60");
                dumpOrder[dumpNum++] = MOTOR3;
            }
            if ( subsection == 4) {
                yValues.insert("curve61", new QVector<double>());
                CurveName.append("curve61");
                dumpOrder[dumpNum++] = MOTOR4;
            }

            if ( subsection == 5) {
                yValues.insert("curve62", new QVector<double>());
                CurveName.append("curve62");
                dumpOrder[dumpNum++] = MOTOR5;
            }
            if ( subsection == 6) {
                yValues.insert("curve63", new QVector<double>());
                CurveName.append("curve63");
                dumpOrder[dumpNum++] = MOTOR6;
            }
            if ( subsection == 7) {
                yValues.insert("curve64", new QVector<double>());
                CurveName.append("curve64");
                dumpOrder[dumpNum++] = MOTOR7;
            }
            if ( subsection == 8) {
                yValues.insert("curve65", new QVector<double>());
                CurveName.append("curve65");
                dumpOrder[dumpNum++] = MOTOR8;
            }


            if ( subsection == 9) {
                yValues.insert("curve66", new QVector<double>());
                CurveName.append("curve66");
                dumpOrder[dumpNum++] = MOTOR9;
            }
            if ( subsection == 10) {
                yValues.insert("curve67", new QVector<double>());
                CurveName.append("curve67");
                dumpOrder[dumpNum++] = MOTOR10;
            }
            if ( subsection == 11) {
                yValues.insert("curve68", new QVector<double>());
                CurveName.append("curve68");
                dumpOrder[dumpNum++] = MOTOR11;
            }
            if ( subsection == 12) {
                yValues.insert("curve69", new QVector<double>());
                CurveName.append("curve69");
                dumpOrder[dumpNum++] = MOTOR12;
            }


            if ( subsection == 13) {
                yValues.insert("curve70", new QVector<double>());
                CurveName.append("curve70");
                dumpOrder[dumpNum++] = MOTOR13;
            }
            if ( subsection == 14) {
                yValues.insert("curve71", new QVector<double>());
                CurveName.append("curve71");
                dumpOrder[dumpNum++] = MOTOR14;
            }

            break;
        case O_THROTTLE:
            if ( subsection == 1) {
                yValues.insert("curve72", new QVector<double>());
                CurveName.append("curve72");
                dumpOrder[dumpNum++] = THROTTLE;
            }
            break;
        case O_EXTRAS:
            if ( subsection == 1) {
                yValues.insert("curve73", new QVector<double>());
                CurveName.append("curve73");
                dumpOrder[dumpNum++] = EXTRA1;
            }
            if ( subsection == 2) {
                yValues.insert("curve74", new QVector<double>());
                CurveName.append("curve74");
                dumpOrder[dumpNum++] = EXTRA2;
            }
            if ( subsection == 3) {
                yValues.insert("curve75", new QVector<double>());
                CurveName.append("curve75");
                dumpOrder[dumpNum++] = EXTRA3;
            }
            if ( subsection == 4) {
                yValues.insert("curve76", new QVector<double>());
                CurveName.append("curve76");
                dumpOrder[dumpNum++] = EXTRA4;
            }
            break;
    }
}

int AQLogParser::loggerReadLog(const char *fname, loggerRecord_t **l)
{
    loggerRecord_t buf;
    FILE *fp;
    int n = 0;
    int i;

    *l = NULL;

    fp = fopen(fname, "rb");
    if (fp == NULL) {
        fprintf(stderr, "logger: cannot open log file '%s'\n", fname);
    }
    else {
        while (loggerReadEntry(fp, &buf) != EOF)
            n++;

        *l = (loggerRecord_t *)calloc(n, sizeof(loggerRecord_t));

        rewind(fp);

        for (i = 0; i < n; i++)
            loggerReadEntry(fp, &(*l)[i]);

        fclose(fp);
    }

    return n;
}

void AQLogParser::loggerFree(loggerRecord_t *l)
{
    if (l) {
        free(l);
        l = NULL;
    }
}



AQEsc32::AQEsc32()
{
}

AQEsc32::~AQEsc32()
{

}

int AQEsc32::esc32SendCommand(unsigned char command, float param1, float param2, int n) {
    seqId++;
    QByteArray transmit;
    QByteArray TempByteArray;
    transmit.append('A');
    transmit.append('q');
    transmit.append(1+2+n*sizeof(float));
    transmit.append(command);
    memcpy(&seqId,TempByteArray.data(),sizeof(ushort));
    for ( int i = 0; i<TempByteArray.length(); i++)
        transmit.append(TempByteArray.at(i));

    TempByteArray.clear();
    if ( n > 0) {
        memcpy(&param1,TempByteArray.data(), sizeof(float));
        for ( int i = 0; i<TempByteArray.length(); i++)
            transmit.append(TempByteArray.at(i));
    }

    TempByteArray.clear();
    if ( n > 1) {
        memcpy(&param2,TempByteArray.data(), sizeof(float));
        for ( int i = 0; i<TempByteArray.length(); i++)
            transmit.append(TempByteArray.at(i));
    }

    short checkA = 0;
    short checkB = 0;
    for ( int i = 2; i<transmit.length(); i++) {
        checkA += transmit.at(i);
        checkB += checkA;
    }
    transmit.append(checkA);
    transmit.append(checkB);
    return 0;
}

