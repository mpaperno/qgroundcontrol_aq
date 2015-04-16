#include "aq_telemetryView.h"
#include "ui_aq_telemetryView.h"
#include "UASManager.h"
#include <QLineEdit>

using namespace AUTOQUADMAV;

AQTelemetryView::AQTelemetryView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQTelemetryView),
    datasetFieldsSetup(-1),
    telemetryRunning(false),
    valGridRow(0),
    valGridCol(0),
    aqFwVerMaj(0),
    aqFwVerMin(0),
    aqFwVerBld(0),
    currentDataSet(TELEM_DATASET_DEFAULT),
    valuesGridLayout(NULL),
    btnsDataSets(NULL),
    AqTeleChart(NULL),
    uas(NULL)
{

    ui->setupUi(this);
    linLayoutPlot = new QGridLayout(ui->plotFrameTele);

    btnsDataSets = new QButtonGroup(ui->valuesGrid);
    btnsDataSets->setExclusive(false);

    ui->Frequenz_Telemetry->addItem("1 Hz", 1000000);
    ui->Frequenz_Telemetry->addItem("10 Hz", 100000);
    ui->Frequenz_Telemetry->addItem("25 Hz", 50000);
    ui->Frequenz_Telemetry->addItem("50 Hz", 20000);
//    ui->Frequenz_Telemetry->addItem("75 Hz", 13333);
//    ui->Frequenz_Telemetry->addItem("100 Hz", 10000);
//    ui->Frequenz_Telemetry->addItem("110 Hz", 9090);
//    ui->Frequenz_Telemetry->addItem("120 Hz", 8333);
//    ui->Frequenz_Telemetry->addItem("130 Hz", 7692);
//    ui->Frequenz_Telemetry->addItem("150 Hz", 6666);
//    ui->Frequenz_Telemetry->addItem("175 Hz", 5714);
//    ui->Frequenz_Telemetry->addItem("200 Hz", 5000);
    ui->Frequenz_Telemetry->setCurrentIndex(2);

    displaySets.append(DisplaySet(DSPSET_NONE, "None"));
    displaySets.append(DisplaySet(DSPSET_IMU, "IMU"));
    displaySets.append(DisplaySet(DSPSET_UKF, "UKF"));
    displaySets.append(DisplaySet(DSPSET_NAV, "NAV"));
    displaySets.append(DisplaySet(DSPSET_GPS, "GPS"));
    displaySets.append(DisplaySet(DSPSET_MOT, "MOT"));
    displaySets.append(DisplaySet(DSPSET_MOT_PWM, "MOT_PWM"));
    displaySets.append(DisplaySet(DSPSET_SUPERVISOR, "Supervisor"));
    displaySets.append(DisplaySet(DSPSET_GIMBAL, "Gimbal"));
    displaySets.append(DisplaySet(DSPSET_STACKS, "Stacks"));
    displaySets.append(DisplaySet(DSPSET_DEBUG, "Debug"));

    // define all data fields
    setupDisplaySetData();


    connect(ui->pushButton_start_tel_grid, SIGNAL(clicked()),this, SLOT(teleValuesToggle()));
    connect(ui->Frequenz_Telemetry, SIGNAL(activated(int)),this, SLOT(frequencyChanged(int)));
    connect(btnsDataSets, SIGNAL(buttonClicked(int)), this, SLOT(datasetButtonClicked(int)));

    initChart(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(initChart(UASInterface*)), Qt::UniqueConnection);
}

AQTelemetryView::~AQTelemetryView() {
    delete ui;
}

void AQTelemetryView::setupDisplaySetData() {
    QString unit;
    int dset;
    int msgidx;
    int valIdx;
    bool newDs = aqFwVerMaj == 0 || aqFwVerMaj > 7 || (aqFwVerMaj == 7 && aqFwVerMin >= 1);
    currentDataSet = TELEM_DATASET_DEFAULT;
    for (int i=1; i < DSPSET_ENUM_END; ++i)
        displaySets[i].datasets.clear();

    if (newDs) {
        displaySets[DSPSET_IMU].datasets << AQMAV_DATASET_IMU;
        displaySets[DSPSET_UKF].datasets << AQMAV_DATASET_UKF;
        displaySets[DSPSET_NAV].datasets << AQMAV_DATASET_NAV;
        displaySets[DSPSET_GPS].datasets << AQMAV_DATASET_GPS;
        displaySets[DSPSET_MOT].datasets << AQMAV_DATASET_MOTORS;
        displaySets[DSPSET_MOT_PWM].datasets << AQMAV_DATASET_MOTORS_PWM;
        displaySets[DSPSET_DEBUG].datasets << AQMAV_DATASET_DEBUG;
    } else {
        displaySets[DSPSET_IMU].datasets << AQMAV_DATASET_LEGACY1;
        displaySets[DSPSET_UKF].datasets << AQMAV_DATASET_LEGACY1 << AQMAV_DATASET_LEGACY2 << AQMAV_DATASET_UKF;
        displaySets[DSPSET_NAV].datasets << AQMAV_DATASET_LEGACY1;
        displaySets[DSPSET_GPS].datasets << AQMAV_DATASET_LEGACY2 << AQMAV_DATASET_GPS;
        displaySets[DSPSET_MOT].datasets << AQMAV_DATASET_LEGACY3;
    }
    displaySets[DSPSET_SUPERVISOR].datasets << AQMAV_DATASET_SUPERVISOR;
    displaySets[DSPSET_GIMBAL].datasets << AQMAV_DATASET_GIMBAL;
    displaySets[DSPSET_STACKS].datasets << AQMAV_DATASET_STACKSFREE;

    if (btnsDataSets)
        qDeleteAll(btnsDataSets->buttons());

    for (int i=1; i < DSPSET_ENUM_END; ++i) {
        if (!displaySets.at(i).datasets.size())
            continue;
        QCheckBox* cb = new QCheckBox(displaySets[i].name, this);
        btnsDataSets->addButton(cb, i);
        ui->hLayout_dataSets->addWidget(cb);
    }

    telemDataFields.clear();

    dset = DSPSET_IMU;
    unit = "float";
    msgidx = newDs ? AQMAV_DATASET_IMU : AQMAV_DATASET_LEGACY1;
    // att
    telemDataFields.append(telemFieldsMeta("AQ_ROLL", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("AQ_PITCH", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("AQ_YAW", unit, 3, msgidx, dset));
    // imu
    telemDataFields.append(telemFieldsMeta("IMU_RATEX", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_RATEY", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_RATEZ", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCX", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCY", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_ACCZ", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("ACC Magnitude", unit, TELEM_VALDEF_ACC_MAGNITUDE, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("ACC ROLL", unit, TELEM_VALDEF_ACC_ROLL, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("ACC PITCH", unit, TELEM_VALDEF_ACC_PITCH, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGX", unit, 10, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGY", unit, 11, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("IMU_MAGZ", unit, 12, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("MAG Magnitude", unit, TELEM_VALDEF_MAG_MAGNITUDE, msgidx, dset));
    valIdx = newDs ? 13 : 15;
    telemDataFields.append(telemFieldsMeta("IMU_TEMP", unit, valIdx, msgidx, dset));
    // environment
    valIdx = newDs ? 20 : 14;
    telemDataFields.append(telemFieldsMeta("AQ_Pressure", unit, valIdx, msgidx, dset));
    if (!newDs)
        telemDataFields.append(telemFieldsMeta("VOLTS IN", unit, 17, msgidx, dset));

    // ukf
    dset = DSPSET_UKF;
    msgidx = newDs ? AQMAV_DATASET_UKF : AQMAV_DATASET_LEGACY1;
    valIdx = newDs ? 11 : 16;
    telemDataFields.append(telemFieldsMeta("UKF_ALTITUDE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_POSN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_POSE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_POSD", unit, valIdx++, msgidx, dset));
    if (!newDs) {
        msgidx = AQMAV_DATASET_LEGACY2;
        valIdx = 12;
    }
    telemDataFields.append(telemFieldsMeta("UKF_VELN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_VELE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_VELD", unit, valIdx++, msgidx, dset));
    msgidx = AQMAV_DATASET_UKF;
    telemDataFields.append(telemFieldsMeta("UKF_GYO_BIAS_X", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_GYO_BIAS_Y", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_GYO_BIAS_Z", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIAS_X", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIAS_Y", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_ACC_BIAS_Z", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_Q1", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_Q2", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_Q3", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("UKF_Q4", unit, 10, msgidx, dset));

    // nav
    dset = DSPSET_NAV;
    msgidx = newDs ? AQMAV_DATASET_NAV : AQMAV_DATASET_LEGACY2;
    valIdx = newDs ? 1 : 6;
    telemDataFields.append(telemFieldsMeta("nav.holdHeading", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("nav.holdCourse", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("nav.holdDistance", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("nav.holdAlt", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("nav.holdTiltN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("nav.holdTiltE", unit, valIdx++, msgidx, dset));
    if (newDs) {
        telemDataFields.append(telemFieldsMeta("nav.holdSpeedN", unit, valIdx++, msgidx, dset));
        telemDataFields.append(telemFieldsMeta("nav.holdSpeedE", unit, valIdx++, msgidx, dset));
        telemDataFields.append(telemFieldsMeta("nav.holdSpeedAlt", unit, valIdx++, msgidx, dset));
        telemDataFields.append(telemFieldsMeta("nav.targetHoldSpeedAlt", unit, valIdx++, msgidx, dset));
        telemDataFields.append(telemFieldsMeta("nav.lastUpdate DLTA", "int", 19, msgidx, dset));
        telemDataFields.append(telemFieldsMeta("nav.fixType", "int", 20, msgidx, dset));
    }

    // gps
    dset = DSPSET_GPS;
    msgidx = newDs ? AQMAV_DATASET_GPS : AQMAV_DATASET_LEGACY2;
    valIdx = newDs ? 11 : 1;
    telemDataFields.append(telemFieldsMeta("gps.Lat", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.Lon", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.hAcc", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.vAcc", unit, 10, AQMAV_DATASET_GPS, dset));
    telemDataFields.append(telemFieldsMeta("gps.course", unit, valIdx++, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.height", unit, valIdx++, msgidx, dset));
    msgidx = AQMAV_DATASET_GPS;
    telemDataFields.append(telemFieldsMeta("gps.pDOP", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.tDOP", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.sAcc", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.cAcc", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.velN", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.velE", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.velD", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.lastPosUpdt", "int", 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gps.lastMessage", "int", 9, msgidx, dset));

    // supervisor
    dset = DSPSET_SUPERVISOR;
    msgidx = AQMAV_DATASET_SUPERVISOR;
    telemDataFields.append(telemFieldsMeta("spvr.state", "int", 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.flightTime", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.flightTimeRemaining", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.flightSecAvg", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.vIn_LPF", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.StateOfCahrge", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.lastGoodRadio_usec", "int", 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("spvr.idlePercent", unit, 8, msgidx, dset));

    unit = "int";

    // motors
    dset = DSPSET_MOT;
    msgidx = newDs ? AQMAV_DATASET_MOTORS : AQMAV_DATASET_LEGACY3;
    valIdx = newDs ? 1 : 7;
    for (int i=1; i <= (newDs ? 16 : 14); ++i) {
        telemDataFields.append(telemFieldsMeta("MOTOR_" + QString::number(i), unit, valIdx++, msgidx, dset));
    }

    // motors PWM
    if (newDs) {
        dset = DSPSET_MOT_PWM;
        msgidx = AQMAV_DATASET_MOTORS_PWM;
        for (int i=1; i <= 14; ++i) {
            telemDataFields.append(telemFieldsMeta("MOT_PWM_" + QString::number(i), unit, i, msgidx, dset));
        }
    }

    dset = DSPSET_GIMBAL;
    msgidx = AQMAV_DATASET_GIMBAL;
    telemDataFields.append(telemFieldsMeta("gmbl.Pitch PWM", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.Tilt PWM", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.Roll PWM", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.Trigger PWM", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.P-thru PWM", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TiltDeg", "float", 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TriggerActive", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TrigLstTime", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TrigLstLat", "float", 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TrigLstLon", "float", 10, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("gmbl.TrigCount", unit, 11, msgidx, dset));

    dset = DSPSET_STACKS;
    msgidx = AQMAV_DATASET_STACKSFREE;
    telemDataFields.append(telemFieldsMeta("stack.INIT", unit, 1, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.FILER", unit, 2, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.SUPERVISOR", unit, 3, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.ADC", unit, 4, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.RADIO", unit, 5, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.CONTROL", unit, 6, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.GPS", unit, 7, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.RUN", unit, 8, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.COMM", unit, 9, msgidx, dset));
    telemDataFields.append(telemFieldsMeta("stack.DIMU", unit, 10, msgidx, dset));

    if (newDs) {
        dset = DSPSET_DEBUG;
        msgidx = AQMAV_DATASET_DEBUG;
        unit = "int";
        for (int i=1; i <= 20; ++i) {
            if (i == 11)
                unit = "float";
            telemDataFields.append(telemFieldsMeta("debug.v" + QString::number(i) + ":" + unit, unit, i, msgidx, dset));
        }
    }

    // save size of this data set
    totalDatasetFields[dset] = telemDataFields.size();

//    for (int i=0; i < telemDataFields.size(); i++)
//        qDebug() << "Label: " << telemDataFields[i].label;

}

void AQTelemetryView::clearValuesGrid() {
    qDeleteAll(ui->valuesGrid->findChildren<QLabel*>());
    qDeleteAll(ui->valuesGrid->findChildren<QLineEdit*>());
    qDeleteAll(ui->valuesGrid->findChildren<QGridLayout*>());

    valGridRow = 0;
    valGridCol = 0;

    valuesGridLayout = new QGridLayout(ui->valuesGrid);
    valuesGridLayout->setVerticalSpacing(4);
    valuesGridLayout->setHorizontalSpacing(4);
}

void AQTelemetryView::setupDataFields(int dspSet) {
    const int maxRowsPerColumn = 20;
    int i;
    QLineEdit *le;

    for (i=0; i < telemDataFields.size(); i++) {
        if (telemDataFields[i].dataSet != currentDataSet || telemDataFields[i].dspSetId != dspSet)
            continue;

        if (valGridRow >= maxRowsPerColumn) {
            valGridCol += 3;
            valuesGridLayout->setColumnMinimumWidth(valGridCol-1, 20);
            valGridRow = 0;
        }

        le = new QLineEdit();
        le->setProperty("msgValueIndex", telemDataFields[i].msgValueIndex);
        le->setProperty("valueIndex", telemDataFields[i].valueIndex);
        le->setProperty("fieldIndex", i);
        valuesGridLayout->addWidget(new QLabel(telemDataFields[i].label + ": "), valGridRow, valGridCol, Qt::AlignRight);
        valuesGridLayout->addWidget(le, valGridRow, valGridCol + 1);

        valGridRow++;

    } // loop over all fields
}

void AQTelemetryView::setupCurves(int dspSet) {

    if (!uas) return;

    int uasId = uas->getUASID();

    // populate curves
    for (int i=0; i < telemDataFields.size(); i++) {
        if (telemDataFields[i].dataSet == currentDataSet && telemDataFields[i].dspSetId == dspSet) {
            QVariant var = QVariant::fromValue(0.0f);
            AqTeleChart->appendData(uasId, telemDataFields[i].label, "", var, 0);
        }
    }

}

void AQTelemetryView::initChart(UASInterface *uav) {
    if (!uav) return;
    if (uas)
        disconnect(uas, 0, this, 0);
    uas = uav;
    //setupDisplaySetData();
    if ( !AqTeleChart ) {
        AqTeleChart = new AQLinechartWidget(uas->getUASID(), ui->plotFrameTele);
        linLayoutPlot->addWidget(AqTeleChart, 0, Qt::AlignCenter);
    }
    connect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));
    connect(uas, SIGNAL(systemVersionChanged(int,uint32_t,uint32_t,QString,QString)), this, SLOT(uasVersionChanged(int,uint32_t,uint32_t,QString,QString)));
}

void AQTelemetryView::uasVersionChanged(int uasId, uint32_t fwVer, uint32_t hwVer, QString fwVerStr, QString hwVerStr)
{
    Q_UNUSED(hwVer)
    Q_UNUSED(fwVerStr)
    Q_UNUSED(hwVerStr)
    if (!uas || uasId != uas->getUASID())
        return;

    aqFwVerMaj = (fwVer >> 24) & 0xFF;
    aqFwVerMin = (fwVer >> 16) & 0xFF;
    aqFwVerBld = fwVer & 0xFFFF;
    setupDisplaySetData();
}

bool AQTelemetryView::teleValuesStart() {
    if (!uas)
        return false;

    bool started = false;
    clearValuesGrid();
    AqTeleChart->clearCurves();

    int id;
    float freq = ui->Frequenz_Telemetry->itemData(ui->Frequenz_Telemetry->currentIndex()).toFloat();
    foreach (QAbstractButton* abtn, btnsDataSets->buttons()) {
        if (abtn->isChecked()) {
            id = btnsDataSets->id(abtn);
            if (id < 0 || id >= DSPSET_ENUM_END)
                continue;

            //valuesGridLayout->addWidget(new QLabel(displaySets[id].name), valGridRow++, valGridCol, 1, 2);
            setupDataFields(id);
            setupCurves(id);

            foreach (int ds, displaySets[id].datasets) {
                uas->startStopTelemetry(true, freq, ds);
                qDebug() << "Requesting dataset:" << ds;
            }
            started = (bool)displaySets[id].datasets.size();
        }
    }
    return started;
}

void AQTelemetryView::teleValuesStop() {
    if (!uas)
        return;
    for (int i=0; i < AQMAV_DATASET_ENUM_END; ++i)
        uas->startStopTelemetry(false, 0, i);
}

void AQTelemetryView::teleValuesToggle(){
    if (!telemetryRunning) {
        if (teleValuesStart()) {
            telemetryRunning = true;
            ui->pushButton_start_tel_grid->setText("Stop Telemetry");
        }
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

void AQTelemetryView::datasetButtonClicked(int btnid) {
    Q_UNUSED(btnid);
    if (telemetryRunning) {
        teleValuesStop();
        teleValuesStart();
    }
}

void AQTelemetryView::on_checkBox_allDatasets_clicked(bool checked) {
    foreach (QAbstractButton* abtn, btnsDataSets->buttons())
        abtn->setChecked(checked);
}

float AQTelemetryView::getTelemValue(const int idx) {
    float x, y, z;
    switch(idx) {
        case 1 :
            return currentValuesF->value1;
        case 2 :
            return currentValuesF->value2;
        case 3 :
            return currentValuesF->value3;
        case 4 :
            return currentValuesF->value4;
        case 5 :
            return currentValuesF->value5;
        case 6 :
            return currentValuesF->value6;
        case 7 :
            return currentValuesF->value7;
        case 8 :
            return currentValuesF->value8;
        case 9 :
            return currentValuesF->value9;
        case 10 :
            return currentValuesF->value10;
        case 11 :
            return currentValuesF->value11;
        case 12 :
            return currentValuesF->value12;
        case 13 :
            return currentValuesF->value13;
        case 14 :
            return currentValuesF->value14;
        case 15 :
            return currentValuesF->value15;
        case 16 :
            return currentValuesF->value16;
        case 17 :
            return currentValuesF->value17;
        case 18 :
            return currentValuesF->value18;
        case 19 :
            return currentValuesF->value19;
        case 20 :
            return currentValuesF->value20;

        // values > 100 don't come from the mavlink messages (they're calculated, or whatever)

        case TELEM_VALDEF_ACC_MAGNITUDE : // ACC magnitude
            x = currentValuesF->value7;
            y = currentValuesF->value8;
            z = currentValuesF->value9;
            return sqrtf(x*x + y*y + z*z);

        case TELEM_VALDEF_MAG_MAGNITUDE : // MAG magnitude
            x = currentValuesF->value10;
            y = currentValuesF->value11;
            z = currentValuesF->value12;
            return sqrtf(x*x + y*y + z*z);

        case TELEM_VALDEF_ACC_PITCH : // pure ACC-derived pitch
            x = currentValuesF->value7;
            z = currentValuesF->value9;
            return MG::UNITS::radiansToDegrees(atan2f(x, -z));

        case TELEM_VALDEF_ACC_ROLL : // pure ACC-derived roll
            y = currentValuesF->value8;
            z = currentValuesF->value9;
            return MG::UNITS::radiansToDegrees(atan2f(-y, -z));
    }

    return 0.0f;
}

void AQTelemetryView::getNewTelemetry(int uasId, int valIdx){
    float val;
    int i;
    QString valStr;
    QVariant var;
    bool ok;

    foreach (QLineEdit* le, ui->valuesGrid->findChildren<QLineEdit *>()) {
        i = le->property("msgValueIndex").toInt(&ok);
        if (!ok || i != valIdx)
            continue;

        i = le->property("fieldIndex").toInt(&ok);
        if (!ok || i < 0 || i >= telemDataFields.size())
            continue;

        val = getTelemValue(telemDataFields[i].valueIndex);
        valStr = telemDataFields[i].unit == QString("int") ? QString::number((int)val) : QString::number(val);
        le->setText(valStr);
        var = QVariant::fromValue(val);
        AqTeleChart->appendData(uasId, telemDataFields[i].label, "", var, 0);
    }
}

void AQTelemetryView::getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values){
    currentValuesF = &values;
    currentValueType = TELEM_VALUETYPE_FLOAT;

    getNewTelemetry(uasId, values.Index);
}
