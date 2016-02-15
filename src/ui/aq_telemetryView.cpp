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
    valuesGridLayout(NULL),
	 currentDataSet(TELEM_DATASET_DEFAULT),
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
    ui->Frequenz_Telemetry->addItem("25 Hz", 40000);
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
    displaySets.append(DisplaySet(DSPSET_RC, "RC"));
    displaySets.append(DisplaySet(DSPSET_CONFIG, "Config"));
    displaySets.append(DisplaySet(DSPSET_STACKS, "MCU"));
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
    telemValueTypes unit;
    displaySetTypes dset;
    int msgidx;
    int valIdx;
    bool newDs = mavUsesNewDatasets();
    currentDataSet = TELEM_DATASET_DEFAULT;

    foreach (DisplaySet dsp, displaySets)
        dsp.datasets.clear();

    if (newDs) {
        displaySets[DSPSET_IMU].datasets << AQMAV_DATASET_IMU;
        displaySets[DSPSET_UKF].datasets << AQMAV_DATASET_UKF;
        displaySets[DSPSET_NAV].datasets << AQMAV_DATASET_NAV;
        displaySets[DSPSET_GPS].datasets << AQMAV_DATASET_GPS;
        displaySets[DSPSET_MOT].datasets << AQMAV_DATASET_MOTORS;
        displaySets[DSPSET_MOT_PWM].datasets << AQMAV_DATASET_MOTORS_PWM;
        displaySets[DSPSET_DEBUG].datasets << AQMAV_DATASET_DEBUG;
        displaySets[DSPSET_RC].datasets << AQMAV_DATASET_RC;
        displaySets[DSPSET_CONFIG].datasets << AQMAV_DATASET_CONFIG;
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

    foreach (DisplaySet dsp, displaySets) {
        if (!dsp.datasets.size())
            continue;
        QCheckBox* cb = new QCheckBox(dsp.name, this);
        cb->setProperty("dspset_id", dsp.id);
        btnsDataSets->addButton(cb);
        ui->hLayout_dataSets->addWidget(cb);
    }

    telemDataFields.clear();

    dset = DSPSET_IMU;
    unit = TELEM_VALUETYPE_FLOAT;
    msgidx = newDs ? AQMAV_DATASET_IMU : AQMAV_DATASET_LEGACY1;
    // att
    telemDataFields.append(TelemFieldsMeta("AQ_ROLL", unit, 1, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("AQ_PITCH", unit, 2, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("AQ_YAW", unit, 3, msgidx, dset));
    // imu
    telemDataFields.append(TelemFieldsMeta("IMU_RATEX", unit, 4, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_RATEY", unit, 5, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_RATEZ", unit, 6, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_ACCX", unit, 7, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_ACCY", unit, 8, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_ACCZ", unit, 9, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("ACC Magnitude", unit, TELEM_VALDEF_ACC_MAGNITUDE, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("ACC ROLL", unit, TELEM_VALDEF_ACC_ROLL, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("ACC PITCH", unit, TELEM_VALDEF_ACC_PITCH, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_MAGX", unit, 10, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_MAGY", unit, 11, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("IMU_MAGZ", unit, 12, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("MAG Magnitude", unit, TELEM_VALDEF_MAG_MAGNITUDE, msgidx, dset));
    valIdx = newDs ? 13 : 15;
    telemDataFields.append(TelemFieldsMeta("IMU_TEMP", unit, valIdx, msgidx, dset));
    // environment
    valIdx = newDs ? 20 : 14;
    telemDataFields.append(TelemFieldsMeta("AQ_Pressure", unit, valIdx, msgidx, dset));
    if (newDs)
        telemDataFields.append(TelemFieldsMeta("lastUpdate DLTA", TELEM_VALUETYPE_INT, 14, msgidx, dset));
    else
        telemDataFields.append(TelemFieldsMeta("VOLTS IN", unit, 17, msgidx, dset));

    // ukf
    dset = DSPSET_UKF;
    msgidx = newDs ? AQMAV_DATASET_UKF : AQMAV_DATASET_LEGACY1;
    valIdx = newDs ? 11 : 16;
    telemDataFields.append(TelemFieldsMeta("UKF_ALTITUDE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_POSN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_POSE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_POSD", unit, valIdx++, msgidx, dset));
    if (!newDs) {
        msgidx = AQMAV_DATASET_LEGACY2;
        valIdx = 12;
    }
    telemDataFields.append(TelemFieldsMeta("UKF_VELN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_VELE", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_VELD", unit, valIdx++, msgidx, dset));
    msgidx = AQMAV_DATASET_UKF;
    telemDataFields.append(TelemFieldsMeta("UKF_PRES_ALT", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("ALTITUDE", unit, valIdx++, msgidx, dset));
    valIdx=1;
    telemDataFields.append(TelemFieldsMeta("UKF_GYO_BIAS_X", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_GYO_BIAS_Y", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_GYO_BIAS_Z", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_ACC_BIAS_X", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_ACC_BIAS_Y", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_ACC_BIAS_Z", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_Q1", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_Q2", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_Q3", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("UKF_Q4", unit, valIdx++, msgidx, dset));

    // nav
    dset = DSPSET_NAV;
    msgidx = newDs ? AQMAV_DATASET_NAV : AQMAV_DATASET_LEGACY2;
    valIdx = newDs ? 1 : 6;
    telemDataFields.append(TelemFieldsMeta("holdHeading", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("holdCourse", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("holdDistance", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("holdAlt", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("holdTiltN", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("holdTiltE", unit, valIdx++, msgidx, dset));
    if (newDs) {
        telemDataFields.append(TelemFieldsMeta("holdSpeedN", unit, valIdx++, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("holdSpeedE", unit, valIdx++, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("holdSpeedAlt", unit, valIdx++, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("targetHoldSpeedAlt", unit, valIdx++, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("presAltOffset", unit, valIdx++, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("lastUpdate DLTA", TELEM_VALUETYPE_INT, 19, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("fixType", TELEM_VALUETYPE_INT, 20, msgidx, dset));
    }

    // gps
    dset = DSPSET_GPS;
    msgidx = newDs ? AQMAV_DATASET_GPS : AQMAV_DATASET_LEGACY2;
    valIdx = newDs ? 11 : 1;
    telemDataFields.append(TelemFieldsMeta("Lat", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Lon", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("hAcc", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("vAcc", unit, 10, AQMAV_DATASET_GPS, dset));
    telemDataFields.append(TelemFieldsMeta("course", unit, valIdx++, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("height", unit, valIdx++, msgidx, dset));
    msgidx = AQMAV_DATASET_GPS;
    telemDataFields.append(TelemFieldsMeta("pDOP", unit, 1, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("tDOP", unit, 2, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("sAcc", unit, 3, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("cAcc", unit, 4, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("velN", unit, 5, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("velE", unit, 6, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("velD", unit, 7, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("lastPosUpdt", TELEM_VALUETYPE_INT, 8, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("lastMessage", TELEM_VALUETYPE_INT, 9, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("timeOfWeek", TELEM_VALUETYPE_INT, 13, msgidx, dset));

    // supervisor
    dset = DSPSET_SUPERVISOR;
    msgidx = AQMAV_DATASET_SUPERVISOR;
    telemDataFields.append(TelemFieldsMeta("state", TELEM_VALUETYPE_INT, 1, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("flightTime", unit, 2, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("vIn", unit, 18, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("vIn_LPF", unit, 5, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("idlePercent", unit, 8, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("comm.txBufStarved", TELEM_VALUETYPE_INT, 17, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("lastGoodRadio_usec", TELEM_VALUETYPE_INT, 7, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("RADIO_QUALITY", TELEM_VALUETYPE_INT, 19, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("RADIO_ERRORS", TELEM_VALUETYPE_INT, 20, msgidx, dset));

    unit = TELEM_VALUETYPE_INT;

    // motors
    dset = DSPSET_MOT;
    msgidx = AQMAV_DATASET_LEGACY3;
    valIdx = 7;
    if (newDs) {
        msgidx = AQMAV_DATASET_MOTORS;
        valIdx = 1;
        telemDataFields.append(TelemFieldsMeta("Throt", TELEM_VALUETYPE_FLOAT, 17, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Pitch", TELEM_VALUETYPE_FLOAT, 18, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Roll", TELEM_VALUETYPE_FLOAT, 19, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Yaw", TELEM_VALUETYPE_FLOAT, 20, msgidx, dset));
    }
    for (int i=1; i <= (newDs ? 16 : 14); ++i) {
        telemDataFields.append(TelemFieldsMeta("MOTOR_" + QString::number(i), unit, valIdx++, msgidx, dset));
    }

    dset = DSPSET_GIMBAL;
    msgidx = AQMAV_DATASET_GIMBAL;
    telemDataFields.append(TelemFieldsMeta("Pitch PWM", unit, 1, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Tilt PWM", unit, 2, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Roll PWM", unit, 3, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Trigger PWM", unit, 4, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("P-thru PWM", unit, 5, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TiltDeg", TELEM_VALUETYPE_FLOAT, 6, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TriggerActive", unit, 7, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TrigLstTime", unit, 8, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TrigLstLat", TELEM_VALUETYPE_FLOAT, 9, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TrigLstLon", TELEM_VALUETYPE_FLOAT, 10, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("TrigCount", unit, 11, msgidx, dset));

    dset = DSPSET_STACKS;
    msgidx = AQMAV_DATASET_STACKSFREE;
    telemDataFields.append(TelemFieldsMeta("Stack INIT", unit, 1, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack FILER", unit, 2, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack SUPERVISOR", unit, 3, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack ADC", unit, 4, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack RADIO", unit, 5, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack CONTROL", unit, 6, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack GPS", unit, 7, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack RUN", unit, 8, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack COMM", unit, 9, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("Stack DIMU", unit, 10, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("RAM heapUsed", unit, 18, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("RAM heapHighWater", unit, 19, msgidx, dset));
    telemDataFields.append(TelemFieldsMeta("RAM dataSramUsed", unit, 20, msgidx, dset));

    if (newDs) {

        // motors PWM
        dset = DSPSET_MOT_PWM;
        msgidx = AQMAV_DATASET_MOTORS_PWM;
        for (int i=1; i <= 14; ++i) {
            telemDataFields.append(TelemFieldsMeta("MOT_PWM_" + QString::number(i), unit, i, msgidx, dset));
        }

        // proportional and switch control values
        dset = DSPSET_RC;
        msgidx = AQMAV_DATASET_RC;
        valIdx = 0;
        telemDataFields.append(TelemFieldsMeta("Throttle", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Rudder", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Pitch", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Roll", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("ALT HOLD", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("POS HOLD", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("MISSION", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("HOME SET", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("HOME GO", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("HDFREE SET", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("HDFREE LOCK", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("WAYPNT REC", unit, ++valIdx, msgidx, dset));
        valIdx = 17;
        telemDataFields.append(TelemFieldsMeta("CAM TRIG", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Gimbal Tilt", unit, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Passthr 1", unit, ++valIdx, msgidx, dset));

        // adjustable parameter values
        dset = DSPSET_CONFIG;
        msgidx = AQMAV_DATASET_CONFIG;
        valIdx = 0;
        telemDataFields.append(TelemFieldsMeta("Adj. Param 1", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP1 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Adj. Param 2", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP2 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Adj. Param 3", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP3 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Adj. Param 4", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP4 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Adj. Param 5", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP5 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));
        telemDataFields.append(TelemFieldsMeta("Adj. Param 6", TELEM_VALUETYPE_INT, ++valIdx, msgidx, dset, &AQTelemetryView::getParamName));
        telemDataFields.append(TelemFieldsMeta("AP6 Value", TELEM_VALUETYPE_FLOAT, ++valIdx, msgidx, dset));

        dset = DSPSET_DEBUG;
        msgidx = AQMAV_DATASET_DEBUG;
        unit = TELEM_VALUETYPE_INT;
        for (int i=1; i <= 20; ++i) {
            if (i == 11)
                unit = TELEM_VALUETYPE_FLOAT;
            telemDataFields.append(TelemFieldsMeta("v" + QString::number(i), unit, i, msgidx, dset));
        }
    }

    // save size of this data set
    totalDatasetFields[currentDataSet] = telemDataFields.size();

//    for (int i=0; i < telemDataFields.size(); i++)
//        qDebug() << "Label: " << telemDataFields[i].label;

}

void AQTelemetryView::clearValuesGrid() {
    qDeleteAll(ui->valuesGrid->findChildren<QLabel*>());
    qDeleteAll(ui->valuesGrid->findChildren<QLineEdit*>());
//    qDeleteAll(ui->valuesGrid->findChildren<QLayoutItem*>());
    qDeleteAll(ui->valuesGrid->findChildren<QGridLayout*>());

    valGridRow = 0;
    valGridCol = 0;

    valuesGridLayout = new QGridLayout(ui->valuesGrid);
    valuesGridLayout->setVerticalSpacing(4);
    valuesGridLayout->setHorizontalSpacing(4);
}

void AQTelemetryView::setupDataFields(displaySetTypes dspSet) {
    int mt, mb;
    ui->tab_val_grid->layout()->getContentsMargins(NULL, &mt, NULL, &mb);
    const int maxRowsPerColumn = ceilf((float)((ui->tab_val_grid->height() - mt - mb) / 25.0f)); //25;
    int i;
    QLineEdit *le;

    if (valGridRow >= maxRowsPerColumn - 3) {
        valGridCol += 3;
        valGridRow = 0;
    }

    valuesGridLayout->addWidget(new QLabel(displaySets.at(dspSet).name), valGridRow, valGridCol, 1, 2);
    if (!valGridRow)
        valuesGridLayout->addItem(new QSpacerItem(20, 1, QSizePolicy::Fixed), valGridRow, valGridCol + 2);
    ++valGridRow;

    for (i=0; i < telemDataFields.size(); i++) {
        if (telemDataFields.at(i).dataSet != currentDataSet || telemDataFields.at(i).dspSetId != dspSet)
            continue;

        if (valGridRow >= maxRowsPerColumn) {
            valGridCol += 3;
            valGridRow = 0;
            valuesGridLayout->addWidget(new QLabel(displaySets.at(dspSet).name + " (cont'd)"), valGridRow, valGridCol, 1, 2);
            valuesGridLayout->addItem(new QSpacerItem(20, 1, QSizePolicy::Fixed), valGridRow++, valGridCol + 2);
        }

        le = new QLineEdit();
        le->setProperty("msgValueIndex", telemDataFields.at(i).msgValueIndex);
        le->setProperty("valueIndex", telemDataFields.at(i).valueIndex);
        le->setProperty("fieldIndex", i);
        valuesGridLayout->addWidget(new QLabel(telemDataFields.at(i).label + ": "), valGridRow, valGridCol, Qt::AlignRight);
        valuesGridLayout->addWidget(le, valGridRow++, valGridCol + 1);

    } // loop over all fields
}

void AQTelemetryView::setupCurves(displaySetTypes dspSet) {

    if (!uas) return;

    int uasId = uas->getUASID();

    // populate curves
    for (int i=0; i < telemDataFields.size(); i++) {
        if (telemDataFields.at(i).dataSet != currentDataSet || telemDataFields.at(i).dspSetId != dspSet)
            continue;

        QVariant var = QVariant::fromValue(0.0f);
        AqTeleChart->appendData(uasId, displaySets.at(dspSet).name % "." % telemDataFields[i].label, "", var, 0);
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

    uint8_t maj = (fwVer >> 24) & 0xFF,
            min= (fwVer >> 16) & 0xFF;
    uint16_t bld = fwVer & 0xFFFF;
    if (maj != aqFwVerMaj || min != aqFwVerMin || bld != aqFwVerBld) {
        aqFwVerMaj = maj;
        aqFwVerMin = min;
        aqFwVerBld = bld;
        setupDisplaySetData();
    }
}

bool AQTelemetryView::teleValuesStart() {
    if (!uas)
        return false;

    bool started = false;
    clearValuesGrid();
    AqTeleChart->clearCurves();

    displaySetTypes id;
    float freq = ui->Frequenz_Telemetry->itemData(ui->Frequenz_Telemetry->currentIndex()).toFloat();
    foreach (QAbstractButton* abtn, btnsDataSets->buttons()) {
        if (abtn->isChecked()) {
            id = (displaySetTypes)abtn->property("dspset_id").toInt();
            if (id <= DSPSET_NONE || id >= DSPSET_ENUM_END)
                continue;

            setupDataFields(id);
            setupCurves(id);

            foreach (int ds, displaySets.at(id).datasets) {
                uas->startStopTelemetry(true, freq, ds);
                //qDebug() << "Requesting dataset:" << ds;
            }
            started = (bool)displaySets.at(id).datasets.size();
        }
    }
    return started;
}

void AQTelemetryView::teleValuesStop() {
    if (!uas)
        return;
    uas->startStopTelemetry(false, 0, AQMAV_DATASET_ALL);
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

QVariant AQTelemetryView::getParamName(const float id)
{
    if (!id)
        return "NONE";
    if (!uas)
        return "UNKNOWN";

    return uas->getParamManager()->getParameterNameById(190, (int)id);
}

bool AQTelemetryView::mavUsesNewDatasets()
{
    return aqFwVerMaj == 0 || aqFwVerMaj > 7 || (aqFwVerMaj == 7 && aqFwVerMin > 1) || aqFwVerBld >= 1859;
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

        val = getTelemValue(telemDataFields.at(i).valueIndex);
        if (telemDataFields.at(i).cb) {
            var = (this->*telemDataFields.at(i).cb)(val);
            valStr = var.toString();
        } else {
            var = QVariant::fromValue(val);
            valStr = telemDataFields.at(i).unit == TELEM_VALUETYPE_INT ? QString::number((int)val) : QString::number(val);
        }
        le->setText(valStr);
        AqTeleChart->appendData(uasId, displaySets.at(telemDataFields.at(i).dspSetId).name % "." % telemDataFields.at(i).label, "", var, 0);
    }
}

void AQTelemetryView::getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values){
    currentValuesF = &values;
    currentValueType = TELEM_VALUETYPE_FLOAT;

    getNewTelemetry(uasId, values.Index);
}
