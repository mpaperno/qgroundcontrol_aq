#include "aq_telemetryView.h"
#include "ui_aq_telemetryView.h"
#include <QLineEdit>

AQTelemetryView::AQTelemetryView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQTelemetryView),
    telemetryRunning(false),
    currentDataSet(TELEM_DATASET_DEFAULT),
    AqTeleChart(NULL)
{

    ui->setupUi(this);

    ui->Frequenz_Telemetry->addItem("1 Hz", 1000000);
    ui->Frequenz_Telemetry->addItem("10 Hz", 100000);
    ui->Frequenz_Telemetry->addItem("25 Hz", 50000);
    ui->Frequenz_Telemetry->addItem("50 Hz", 20000);
    ui->Frequenz_Telemetry->addItem("75 Hz", 13333);
    ui->Frequenz_Telemetry->addItem("100 Hz", 10000);
    ui->Frequenz_Telemetry->addItem("150 Hz", 6666);
    ui->Frequenz_Telemetry->addItem("200 Hz", 5000);
    ui->Frequenz_Telemetry->setCurrentIndex(2);

    // define all data fields

    //valueCallback callback = &AQTelemetryView::getTelemValue;
    QString unit = "float";
    telemDatasets dset = TELEM_DATASET_DEFAULT;
    int msgidx = 0;
    telemDataFields.append(telemFieldsMeta("AQ_ROLL", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("AQ_PITCH", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("AQ_YAW", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_RATEX", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_RATEY", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_RATEZ", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCX", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCY", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCZ", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("ACC Magnitude", unit, TELEM_VALDEF_ACC_MAGNITUDE, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGX", unit, 10, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGY", unit, 11, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGZ", unit, 12, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MAG Magnitude", unit, TELEM_VALDEF_MAG_MAGNITUDE, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("AQ_Pressure", unit, 14, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_TEMP", unit, 15, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("adcData.vIn", unit, 17, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ALTITUDE", unit, 16, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_POSN", unit, 18, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_POSE", unit, 19, msgidx, dset));
//    telemDataFields.append(telemFieldsMeta("Res", unit, 20, msgidx, dset));

    msgidx++; // new set of messages
    telemDataFields.append(telemFieldsMeta("UKF_VELN", unit, 12, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_VELE", unit, 13, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_VELD", unit, 14, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIASX", unit, 16, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIASY", unit, 17, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIASZ", unit, 18, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.lat", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.lon", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.hAcc", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.heading", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.height", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gpsData.pDOP", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("navData.HoldHeading", unit, 13, 0, dset)); // msgidx =0
    telemDataFields.append(telemFieldsMeta("navData.holdCourse", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("navData.holdDistance", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("navData.holdAlt", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("navData.holdTiltN", unit, 10, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("navData.holdTiltE", unit, 11, msgidx, dset));

    telemDataFields.append(telemFieldsMeta("super.flightTime", unit, 19, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_QUALITY", unit, 15, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_ERROR_COUNT", "int", 20, msgidx, dset));

    msgidx++; // new set of messages (INT msg types)
    unit = "int";
    telemDataFields.append(telemFieldsMeta("RADIO_THROT", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_RUDD", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_PITCH", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_ROLL", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_FLAPS", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("RADIO_AUX2", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR1", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR2", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR3", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR4", unit, 10, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR5", unit, 11, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR6", unit, 12, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR7", unit, 13, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR8", unit, 14, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR9", unit, 15, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR10", unit, 16, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR11", unit, 17, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR12", unit, 18, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR13", unit, 19, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MOTOR14", unit, 20, msgidx, dset));

    // save size of this data set
    totalDatasetFields[dset] = telemDataFields.size();

//    for (int i=0; i < telemDataFields.size(); i++)
//        qDebug() << "Label: " << telemDataFields[i].label;

    setupDataFields();

    connect(ui->pushButton_start_tel_grid, SIGNAL(clicked()),this, SLOT(teleValuesToggle()));
    connect(ui->Frequenz_Telemetry, SIGNAL(activated(int)),this, SLOT(frequencyChanged(int)));

}

AQTelemetryView::~AQTelemetryView()
{
    delete ui;
}

void AQTelemetryView::setupDataFields() {
    if (datasetFieldsSetup != currentDataSet) {

        const int totalFlds = totalDatasetFields[currentDataSet];
        const int rowsPerCol = (int) ceil((float)(totalFlds / 3.0f));
        int gridRow = 0;
        int curGrid = 0;
        int fldCnt = 0;
        int i;
        QGridLayout *grid;

        // clear data fields grid
        QLayoutItem* item;
        for (i=0; i < 3; i++) {
            grid = ui->valuesGrid->findChild<QGridLayout *>(QString("valsGrid%1").arg(i));
            while ( ( item = grid->layout()->takeAt(0) ) != NULL ) {
                if (item->widget())
                    delete item->widget();
                delete item;
            }
        }

        grid = ui->valsGrid0;
        for (i=0; i < telemDataFields.size(); i++) {
            if (telemDataFields[i].dataSet == currentDataSet) {
                if ( gridRow >= rowsPerCol && curGrid < 2 ) {
                    curGrid++;
                    grid = ui->valuesGrid->findChild<QGridLayout *>(QString("valsGrid%1").arg(curGrid));
                    gridRow = 0;
                }

                grid->addWidget(new QLabel(telemDataFields[i].label), gridRow, 0);
                grid->addWidget(new QLineEdit, gridRow, 1);

                gridRow = grid->rowCount();
                fldCnt++;
            } // if field is in correct data set
        } // loop over all fields

        datasetFieldsSetup = currentDataSet;
    }
}

void AQTelemetryView::setupCurves() {

    if (!uas) return;

    int uasId = uas->getUASID();

    // AqTeleChart->clearCurves();

    // populate curves
    for (int i=0; i < telemDataFields.size(); i++) {
        if (telemDataFields[i].dataSet == currentDataSet) {
            if (telemDataFields[i].unit == QString("float"))
                AqTeleChart->appendData(uasId, telemDataFields[i].label, "", 0.0f, 0);
            else
                AqTeleChart->appendData(uasId, telemDataFields[i].label, "", 0, 0);
        }
    }

}

void AQTelemetryView::initChart(UASInterface *uav) {
    if (!uav) return;
    uas = uav;
    if ( !AqTeleChart ) {
        AqTeleChart = new AQLinechartWidget(uas->getUASID(), ui->plotFrameTele);
        linLayoutPlot = new QGridLayout( ui->plotFrameTele);
        linLayoutPlot->addWidget(AqTeleChart, 0, Qt::AlignCenter);
    }
    setupCurves();
}

float AQTelemetryView::getTelemValue(const int idx) {
    float ret = 0.0f, x, y, z;
    switch(idx) {
    case 1 :
        ret = currentValuesF->value1;
        break;
    case 2 :
        ret = currentValuesF->value2;
        break;
    case 3 :
        ret = currentValuesF->value3;
        break;
    case 4 :
        ret = currentValuesF->value4;
        break;
    case 5 :
        ret = currentValuesF->value5;
        break;
    case 6 :
        ret = currentValuesF->value6;
        break;
    case 7 :
        ret = currentValuesF->value7;
        break;
    case 8 :
        ret = currentValuesF->value8;
        break;
    case 9 :
        ret = currentValuesF->value9;
        break;
    case 10 :
        ret = currentValuesF->value10;
        break;
    case 11 :
        ret = currentValuesF->value11;
        break;
    case 12 :
        ret = currentValuesF->value12;
        break;
    case 13 :
        ret = currentValuesF->value13;
        break;
    case 14 :
        ret = currentValuesF->value14;
        break;
    case 15 :
        ret = currentValuesF->value15;
        break;
    case 16 :
        ret = currentValuesF->value16;
        break;
    case 17 :
        ret = currentValuesF->value17;
        break;
    case 18 :
        ret = currentValuesF->value18;
        break;
    case 19 :
        ret = currentValuesF->value19;
        break;
    case 20 :
        ret = currentValuesF->value20;
        break;

    // values > 100 don't come from the mavlink messages (they're calculated, or whatever)

    case TELEM_VALDEF_ACC_MAGNITUDE : // ACC magnitude
        x = currentValuesF->value7;
        y = currentValuesF->value8;
        z = currentValuesF->value9;
        ret = sqrtf(x*x + y*y + z*z);
        break;
    case TELEM_VALDEF_MAG_MAGNITUDE : // MAG magnitude
        x = currentValuesF->value10;
        y = currentValuesF->value11;
        z = currentValuesF->value12;
        ret = sqrtf(x*x + y*y + z*z);
        break;
    }
    return ret;
}

void AQTelemetryView::teleValuesStart(){

    if (!uas) return;

    connect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));

    float freq = 40000;
    if ( ui->Frequenz_Telemetry->currentIndex() == 0)
        freq = 1000000;
    if ( ui->Frequenz_Telemetry->currentIndex() == 1)
        freq = 100000;
    if ( ui->Frequenz_Telemetry->currentIndex() == 2)
        freq = 40000;
    if ( ui->Frequenz_Telemetry->currentIndex() == 3)
        freq = 20000;

    uas->startStopTelemetry(true,freq);
}

void AQTelemetryView::teleValuesStop() {
    if (!uas)
        return;
    disconnect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));
    uas->startStopTelemetry(false,0.0f);
}

void AQTelemetryView::teleValuesToggle(){
    if (!telemetryRunning) {
        teleValuesStart();
        telemetryRunning = true;
        ui->pushButton_start_tel_grid->setText("Stop Telemetry");
    } else {
        teleValuesStop();
        telemetryRunning = false;
        ui->pushButton_start_tel_grid->setText("Start Telemetry");
    }
}

void AQTelemetryView::frequencyChanged(int freq) {
    Q_UNUSED(freq);
    if (telemetryRunning) {
        teleValuesStop();
        teleValuesStart();
    }
}

void AQTelemetryView::getNewTelemetry(int uasId, int valIdx){
    float val;
    qint32 ival;
    QString valStr;
    msec = 0;

    for (int i=0; i < telemDataFields.size(); i++) {
        if (valIdx == telemDataFields[i].msgValueIndex) {
            val = getTelemValue(telemDataFields[i].valueIndex);

            if (ui->tab_val_grid->isVisible()) {
                valStr = telemDataFields[i].unit == QString("int") ? QString::number((int)val) : QString::number(val);
                ui->valuesGrid->findChildren<QLineEdit *>(QString("")).at(i)->setText(valStr);
            }

            if (ui->tab_val_chart->isVisible()) // AqTeleChart->CurveIsActive[i]
                if (telemDataFields[i].unit == QString("float"))
                    AqTeleChart->appendData(uasId, telemDataFields[i].label, "", val, msec);
                else {
                    ival = static_cast<qint32>(val);
                    AqTeleChart->appendData(uasId, telemDataFields[i].label, "", ival, msec);
                }
        }
    }
}

void AQTelemetryView::getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values){
    currentValuesF = &values;
    currentValueType = TELEM_VALUETYPE_FLOAT;

    getNewTelemetry(uasId, values.Index);
}
