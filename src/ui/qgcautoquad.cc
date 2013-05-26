#include "MainWindow.h"
#include "qgcautoquad.h"
#include "ui_qgcautoquad.h"
#include "aq_LogExporter.h"
#include "LinkManager.h"
#include "UASManager.h"
#include <SerialLinkInterface.h>
#include <SerialLink.h>
#include <qstringlist.h>
#include <configuration.h>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QTextBrowser>
#include <QMessageBox>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QSignalMapper>
#include <QSvgGenerator>
#include "GAudioOutput.h"

QGCAutoquad::QGCAutoquad(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCAutoquad),
    plot(new IncrementalPlot()),
    uas(NULL),
    paramaq(NULL),
    esc32(NULL)
{

    VisibleWidget = 0;

    model = NULL;
    picker = NULL;
    MarkerCut1  = NULL;
    MarkerCut2  = NULL;
    MarkerCut3  = NULL;
    MarkerCut4  = NULL;
    FwFileForEsc32 = "";

    aqFirmwareVersion = "";
    aqFirmwareRevision = 0;
    aqHardwareVersion = 6;
    aqHardwareRevision = 0;
    aqBuildNumber = 0;

    aqBinFolderPath = QCoreApplication::applicationDirPath() + "/aq/bin/";
    aqMotorMixesPath = QCoreApplication::applicationDirPath() + "/aq/mixes/";
#if defined(Q_OS_WIN)
    platformExeExt = ".exe";
#else
    platformExeExt = "";
#endif

    // these regexes are used for matching field names to AQ params
    fldnameRx.setPattern("^(COMM|CTRL|DOWNLINK|GMBL|GPS|IMU|L1|MOT|NAV|PPM|RADIO|SIG|SPVR|UKF|VN100)_[A-Z0-9_]+$"); // strict field name matching
    dupeFldnameRx.setPattern("___N[0-9]"); // for having duplicate field names, append ___N# after the field name (three underscores, "N", and a unique number)

    setHardwareInfo(aqHardwareVersion);  // populate hardware (AQ board) info with defaults

    /*
     * Start the UI
    */

    ui->setupUi(this);

    // load the Telemetry tab
    aqTelemetryView = new AQTelemetryView(this);
    ui->tabWidget->insertTab(5, aqTelemetryView, tr("Telemetry"));

    FlashEsc32Active = false;
    QHBoxLayout* layout = new QHBoxLayout(ui->plotFrame);
    layout->addWidget(plot);
    ui->plotFrame->setLayout(layout);

    // load the port config UI
    aqPwmPortConfig = new AQPWMPortsConfig(this);
    ui->tab_aq_settings->insertTab(2, aqPwmPortConfig, tr("Mixing && Output"));

#ifdef QT_NO_DEBUG
    ui->tabWidget->removeTab(6); // hide devel tab
#endif

    ui->lbl_version->setText(QGCAUTOQUAD::APP_NAME + " v. " + QGCAUTOQUAD::APP_VERSION_TXT);
    ui->lbl_version2->setText(QString("Based on %1 %2").arg(QGC_APPLICATION_NAME).arg(QGC_APPLICATION_VERSION));

    // populate field values

    ui->SPVR_FS_RAD_ST1->addItem("Position Hold", 0);
    ui->SPVR_FS_RAD_ST2->addItem("slow decent", 0);
    ui->SPVR_FS_RAD_ST2->addItem("RTH and slow decent", 1);

    ui->RADIO_TYPE->addItem("Spektrum 11Bit", 0);
    ui->RADIO_TYPE->addItem("Spektrum 10Bit", 1);
    ui->RADIO_TYPE->addItem("Futaba", 2);
    ui->RADIO_TYPE->addItem("PPM", 3);

    ui->comboBox_marker->clear();
    ui->comboBox_marker->addItem("Start & End 1s", 0);
    ui->comboBox_marker->addItem("Start & End 2s", 1);
    ui->comboBox_marker->addItem("Start & End 3s", 2);
    ui->comboBox_marker->addItem("Start & End 5s", 3);
    ui->comboBox_marker->addItem("Start & End 10s", 4);
    ui->comboBox_marker->addItem("Start & End 15s", 5);
    ui->comboBox_marker->addItem("manual", 6);

    ui->comboBox_mode->addItem("RPM",0);
    ui->comboBox_mode->addItem("Open Loop",1);
    ui->comboBox_mode->addItem("Thrust",2);

    ui->comboBox_in_mode->addItem("PWM",0);
    ui->comboBox_in_mode->addItem("UART",1);
    ui->comboBox_in_mode->addItem("I2C",2);
    ui->comboBox_in_mode->addItem("CAN",3);
    ui->comboBox_in_mode->addItem("OW", 4);

    ui->DoubleMaxCurrent->setValue(30.0);

    // populate COMM stream types
    QList<QButtonGroup *> commStreamTypBtns = this->findChildren<QButtonGroup *>(QRegExp("COMM_STREAM_TYP[\\d]$"));
    foreach (QButtonGroup* g, commStreamTypBtns) {
        foreach (QAbstractButton* abtn, g->buttons()) {
            QString ctyp = abtn->objectName().replace(QRegExp("[\\w]+_[\\w]+_"), "");
            if (ctyp == "multiplex")
                g->setId(abtn, COMM_TYPE_MULTIPLEX);
            else if (ctyp == "mavlink")
                g->setId(abtn, COMM_TYPE_MAVLINK);
            else if (ctyp == "telemetry")
                g->setId(abtn, COMM_TYPE_TELEMETRY);
            else if (ctyp == "gps")
                g->setId(abtn, COMM_TYPE_GPS);
            else if (ctyp == "file")
                g->setId(abtn, COMM_TYPE_FILEIO);
            else if (ctyp == "cli")
                g->setId(abtn, COMM_TYPE_CLI);
            else if (ctyp == "omapConsole")
                g->setId(abtn, COMM_TYPE_OMAP_CONSOLE);
            else if (ctyp == "omapPpp")
                g->setId(abtn, COMM_TYPE_OMAP_PPP);
            else
                g->setId(abtn, COMM_TYPE_NONE);
        }
    }

    // Final UI tweaks

    ui->label_radioChangeWarning->hide();
    ui->groupBox_ppmOptions->hide();
    ui->groupBox_ppmOptions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    ui->conatiner_radioGraphValues->setEnabled(false);
    ui->checkBox_raw_value->hide();

    adjustUiForHardware();

    ui->pushButton_start_calibration->setToolTip("WARNING: EXPERIMENTAL!!");

    delayedSendRCTimer.setInterval(800);  // timer for sending radio freq. update value

    // save this for easy iteration later
    allRadioChanCombos.append(ui->groupBox_channelMapping->findChildren<QComboBox *>(QRegExp("RADIO_.+")));
    allRadioChanProgressBars.append(ui->groupBox_Radio_Values->findChildren<QProgressBar *>(QRegExp("progressBar_chan_[0-9]")));
    allRadioChanValueLabels.append(ui->groupBox_Radio_Values->findChildren<QLabel *>(QRegExp("label_chanValue_[0-9]")));


    // Signal handlers

    connect(this, SIGNAL(hardwareInfoUpdated()), this, SLOT(adjustUiForHardware()));

    //GUI slots
    connect(ui->SelectFirmwareButton, SIGNAL(clicked()), this, SLOT(selectFWToFlash()));
    connect(ui->portName, SIGNAL(editTextChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->portName, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(flashFW()));
    //connect(LinkManager::instance(), SIGNAL(newLink(LinkInterface*)), this, SLOT(addLink(LinkInterface*)));

    connect(ui->comboBox_port_esc32, SIGNAL(editTextChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->comboBox_port_esc32, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->flashfwEsc32, SIGNAL(clicked()), this, SLOT(flashFWEsc32()));
    connect(ui->pushButton_connect_to_esc32, SIGNAL(clicked()), this, SLOT(btnConnectEsc32()));
    connect(ui->pushButton_read_config, SIGNAL(clicked()),this, SLOT(btnReadConfigEsc32()));
    connect(ui->pushButton_send_to_esc32, SIGNAL(clicked()),this,SLOT(btnSaveToEsc32()));
    connect(ui->pushButton_esc32_read_arm_disarm, SIGNAL(clicked()),this,SLOT(btnArmEsc32()));
    connect(ui->pushButton_esc32_read_start_stop, SIGNAL(clicked()),this,SLOT(btnStartStopEsc32()));
    connect(ui->pushButton_send_rpm, SIGNAL(clicked()),this,SLOT(btnSetRPM()));
    connect(ui->horizontalSlider_rpm, SIGNAL(valueChanged(int)),this,SLOT(Esc32RpmSlider(int)));
    connect(ui->pushButton_start_calibration, SIGNAL(clicked()),this,SLOT(Esc32StartCalibration()));
    connect(ui->pushButton_logging, SIGNAL(clicked()),this,SLOT(Esc32StartLogging()));
    connect(ui->pushButton_read_load_def, SIGNAL(clicked()),this,SLOT(Esc32ReadConf()));
    connect(ui->pushButton_reload_conf, SIGNAL(clicked()),this,SLOT(Esc32ReLoadConf()));

    connect(ui->pushButton_Add_Static, SIGNAL(clicked()),this,SLOT(addStatic()));
    connect(ui->pushButton_Remov_Static, SIGNAL(clicked()),this,SLOT(delStatic()));
    connect(ui->pushButton_Add_Dynamic, SIGNAL(clicked()),this,SLOT(addDynamic()));
    connect(ui->pushButton_Remove_Dynamic, SIGNAL(clicked()),this,SLOT(delDynamic()));
    connect(ui->pushButton_Sel_params_file_user, SIGNAL(clicked()),this,SLOT(setUsersParams()));
    connect(ui->pushButton_Create_params_file_user, SIGNAL(clicked()),this,SLOT(CreateUsersParams()));
    connect(ui->pushButton_save, SIGNAL(clicked()),this,SLOT(WriteUsersParams()));
    connect(ui->pushButton_Calculate, SIGNAL(clicked()),this,SLOT(CalculatDeclination()));

    connect(ui->pushButton_Export_Log, SIGNAL(clicked()),this,SLOT(openExportOptionsDlg()));
    connect(ui->pushButton_Open_Log_file, SIGNAL(clicked()),this,SLOT(OpenLogFile()));
    connect(ui->pushButton_set_marker, SIGNAL(clicked()),this,SLOT(startSetMarker()));
    connect(ui->pushButton_cut, SIGNAL(clicked()),this,SLOT(startCutting()));
    connect(ui->pushButton_remove_marker, SIGNAL(clicked()),this,SLOT(removeMarker()));
    connect(ui->pushButton_clearCurves, SIGNAL(clicked()),this,SLOT(deselectAllCurves()));
    connect(ui->pushButton_save_image_plot, SIGNAL(clicked()),this,SLOT(save_plot_image()));
    connect(ui->pushButtonshow_cahnnels, SIGNAL(clicked()),this,SLOT(showChannels()));
    connect(ui->comboBox_marker, SIGNAL(currentIndexChanged(int)),this,SLOT(CuttingItemChanged(int)));

    connect(ui->pushButton_save_to_aq_radio, SIGNAL(clicked()),this,SLOT(saveRadioSettings()));
    connect(ui->pushButton_save_to_aq_pid1, SIGNAL(clicked()),this,SLOT(saveAttitudePIDs()));
    connect(ui->pushButton_save_to_aq_pid2, SIGNAL(clicked()),this,SLOT(saveNavigationPIDs()));
    connect(ui->pushButton_save_to_aq_pid3, SIGNAL(clicked()),this,SLOT(saveSpecialSettings()));
    connect(ui->pushButton_save_to_aq_pid4, SIGNAL(clicked()),this,SLOT(saveGimbalSettings()));

    connect(&delayedSendRCTimer, SIGNAL(timeout()), this, SLOT(sendRcRefreshFreq()));
    connect(ui->RADIO_TYPE, SIGNAL(currentIndexChanged(int)), this, SLOT(radioType_changed(int)));
    connect(ui->checkBox_raw_value, SIGNAL(clicked()),this,SLOT(toggleRadioValuesUpdate()));
    connect(ui->pushButton_toggleRadioGraph, SIGNAL(clicked()),this,SLOT(toggleRadioValuesUpdate()));
    connect(ui->spinBox_rcGraphRefreshFreq, SIGNAL(valueChanged(int)), this, SLOT(delayedSendRcRefreshFreq(int)));
    foreach (QComboBox* cb, allRadioChanCombos)
        connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(validateRadioSettings(int)));

    connect(ui->pushButton_start_cal1, SIGNAL(clicked()),this,SLOT(startcal1()));
    connect(ui->pushButton_start_cal2, SIGNAL(clicked()),this,SLOT(startcal2()));
    connect(ui->pushButton_start_cal3, SIGNAL(clicked()),this,SLOT(startcal3()));
    connect(ui->pushButton_start_sim1, SIGNAL(clicked()),this,SLOT(startsim1()));
    connect(ui->pushButton_start_sim1_2, SIGNAL(clicked()),this,SLOT(startsim1b()));
    connect(ui->pushButton_start_sim2, SIGNAL(clicked()),this,SLOT(startsim2()));
    connect(ui->pushButton_start_sim3, SIGNAL(clicked()),this,SLOT(startsim3()));
    connect(ui->pushButton_abort_cal1, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_cal2, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_cal3, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_sim1, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_sim1_2, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_sim2, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->pushButton_abort_sim3, SIGNAL(clicked()),this,SLOT(abortcalc()));
    connect(ui->checkBox_sim3_4_var_1, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_4_stop_1, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_4_var_2, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_4_stop_2, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_5_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_5_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_6_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_6_stop, SIGNAL(clicked()),this,SLOT(check_stop()));

#ifndef QT_NO_DEBUG
    connect(ui->pushButton_dev1, SIGNAL(clicked()),this, SLOT(pushButton_dev1()));
#endif

    //Process Slots
    ps_master.setProcessChannelMode(QProcess::MergedChannels);
    connect(&ps_master, SIGNAL(finished(int)), this, SLOT(prtstexit(int)));
    connect(&ps_master, SIGNAL(readyReadStandardOutput()), this, SLOT(prtstdout()));
    connect(&ps_master, SIGNAL(readyReadStandardError()), this, SLOT(prtstderr()));


    // UAS slots
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(addUAS(UASInterface*)), Qt::UniqueConnection);
    QList<UASInterface*> mavs = UASManager::instance()->getUASList();
    foreach (UASInterface* currMav, mavs) {
        addUAS(currMav);
    }
    setActiveUAS(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(addUAS(UASInterface*)), Qt::UniqueConnection);
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)), Qt::UniqueConnection);

    setupPortList();
    loadSettings();

}

QGCAutoquad::~QGCAutoquad()
{
    if ( esc32)
        btnConnectEsc32();
    writeSettings();
    delete ui;
}

void QGCAutoquad::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QGCAutoquad::hideEvent(QHideEvent* event)
{
    if ( VisibleWidget <= 1)
        VisibleWidget = 0;
    QWidget::hideEvent(event);
    emit visibilityChanged(false);
}

void QGCAutoquad::showEvent(QShowEvent* event)
{
    // React only to internal (pre-display)
    // events
    if ( VisibleWidget <= 1)
        VisibleWidget = 1;

    if ( VisibleWidget == 1) {
        if ( uas != NULL)
        {
            setActiveUAS(uas);
            VisibleWidget = 2;
        }
    }
    QWidget::showEvent(event);
    emit visibilityChanged(true);
}

void QGCAutoquad::loadSettings()
{
    // Load defaults from settings
    // QSettings settings("Aq.ini", QSettings::IniFormat);

    settings.beginGroup("AUTOQUAD_SETTINGS");

    // if old style Aq.ini file exists, copy settings to QGC shared storage
    if (QFile("Aq.ini").exists()) {
        QSettings aq_settings("Aq.ini", QSettings::IniFormat);
        aq_settings.beginGroup("AUTOQUAD_SETTINGS");
        foreach (QString childKey, aq_settings.childKeys())
            settings.setValue(childKey, aq_settings.value(childKey));
        settings.sync();
        QFile("Aq.ini").rename("Aq.ini.bak");
        qDebug() << "Copied settings from Aq.ini to QGC shared config storage.";
    }

    if (settings.contains("STATIC_FILE_COUNT"))
    {
        qint32 FileStaticCount = settings.value("STATIC_FILE_COUNT").toInt();
        StaticFiles.clear();
        for ( int i =0; i<FileStaticCount; i++) {
            StaticFiles.append(settings.value("STATIC_FILE" + QString::number(i)).toString());
            ui->listWidgetStatic->addItem(settings.value("STATIC_FILE" + QString::number(i)).toString());
        }
    }
    if (settings.contains("DYNAMIC_FILE_COUNT"))
    {
        qint32 FileDynamicCount = settings.value("DYNAMIC_FILE_COUNT").toInt();
        DynamicFiles.clear();
        for ( int i =0; i<FileDynamicCount; i++) {
            DynamicFiles.append(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
            ui->listWidgetDynamic->addItem(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
        }
    }

    ui->lineEdit_insert_declination->setText(settings.value("DECLINATION_SOURCE").toString());
    ui->lineEdit_cal_declination->setText(settings.value("DECLINATION_CALC").toString());

    ui->lineEdit_insert_inclination->setText(settings.value("INCLINATION_SOURCE").toString());
    ui->lineEdit_cal_inclination->setText(settings.value("INCLINATION_CALC").toString());

    if (settings.contains("AUTOQUAD_FW_FILE")) {
        ui->fileLabel->setText(settings.value("AUTOQUAD_FW_FILE").toString());
        ui->fileLabel->setToolTip(settings.value("AUTOQUAD_FW_FILE").toString());
    }
    if (settings.contains("USERS_PARAMS_FILE")) {
        UsersParamsFile = settings.value("USERS_PARAMS_FILE").toString();
        if (QFile::exists(UsersParamsFile))
            ShowUsersParams(QDir::toNativeSeparators(UsersParamsFile));
    }

    ui->sim3_4_var_1->setText(settings.value("AUTOQUAD_VARIANCE1").toString());
    ui->sim3_4_var_2->setText(settings.value("AUTOQUAD_VARIANCE2").toString());
    ui->sim3_5_var->setText(settings.value("AUTOQUAD_VARIANCE3").toString());
    ui->sim3_6_var->setText(settings.value("AUTOQUAD_VARIANCE4").toString());

    ui->sim3_4_stop_1->setText(settings.value("AUTOQUAD_STOP1").toString());
    ui->sim3_4_stop_2->setText(settings.value("AUTOQUAD_STOP2").toString());
    ui->sim3_5_stop->setText(settings.value("AUTOQUAD_STOP3").toString());
    ui->sim3_6_stop->setText(settings.value("AUTOQUAD_STOP4").toString());

    LastFilePath = settings.value("AUTOQUAD_LAST_PATH").toString();

    ui->pushButton_toggleRadioGraph->setChecked(settings.value("RADIO_VALUES_UPDATE_BTN_STATE", true).toBool());

    settings.endGroup();
    settings.sync();
}

void QGCAutoquad::writeSettings()
{
    //QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

    settings.setValue("APP_VERSION", QGCAUTOQUAD::APP_VERSION);

    settings.setValue("STATIC_FILE_COUNT", QString::number(StaticFiles.count()));
    for ( int i = 0; i<StaticFiles.count(); i++) {
        settings.setValue("STATIC_FILE" + QString::number(i), StaticFiles.at(i));
    }
    settings.setValue("DYNAMIC_FILE_COUNT", QString::number(DynamicFiles.count()));
    for ( int i = 0; i<DynamicFiles.count(); i++) {
        settings.setValue("DYNAMIC_FILE" + QString::number(i), DynamicFiles.at(i));
    }

    settings.setValue("DECLINATION_SOURCE", ui->lineEdit_insert_declination->text());
    settings.setValue("DECLINATION_CALC", ui->lineEdit_cal_declination->text());
    settings.setValue("INCLINATION_SOURCE", ui->lineEdit_insert_inclination->text());
    settings.setValue("INCLINATION_CALC", ui->lineEdit_cal_inclination->text());

    settings.setValue("USERS_PARAMS_FILE", UsersParamsFile);

    settings.setValue("AUTOQUAD_FW_FILE", ui->fileLabel->text());

    settings.setValue("AUTOQUAD_VARIANCE1", ui->sim3_4_var_1->text());
    settings.setValue("AUTOQUAD_VARIANCE2", ui->sim3_4_var_2->text());
    settings.setValue("AUTOQUAD_VARIANCE3", ui->sim3_5_var->text());
    settings.setValue("AUTOQUAD_VARIANCE4", ui->sim3_6_var->text());

    settings.setValue("AUTOQUAD_STOP1", ui->sim3_4_stop_1->text());
    settings.setValue("AUTOQUAD_STOP2", ui->sim3_4_stop_2->text());
    settings.setValue("AUTOQUAD_STOP3", ui->sim3_5_stop->text());
    settings.setValue("AUTOQUAD_STOP4", ui->sim3_5_stop->text());

    settings.setValue("AUTOQUAD_LAST_PATH", LastFilePath);

    settings.setValue("RADIO_VALUES_UPDATE_BTN_STATE", ui->pushButton_toggleRadioGraph->isChecked());

    settings.sync();
    settings.endGroup();
}


//
// Calibration
//

void QGCAutoquad::addStatic()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Log (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        for ( int i=0; i<fileNames.size(); i++) {
            QString fileNameLocale = QDir::toNativeSeparators(fileNames.at(i));
            ui->listWidgetStatic->addItem(fileNameLocale);
            StaticFiles.append(fileNameLocale);
            LastFilePath = fileNameLocale;
        }
    }
}

void QGCAutoquad::delStatic()
{
    int currIndex = ui->listWidgetStatic->row(ui->listWidgetStatic->currentItem());
    if ( currIndex >= 0) {
        QString SelStaticFile =  ui->listWidgetStatic->item(currIndex)->text();
        StaticFiles.removeAt(StaticFiles.indexOf(SelStaticFile));
        ui->listWidgetStatic->takeItem(currIndex);
    }
}

void QGCAutoquad::addDynamic()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Log (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        for ( int i=0; i<fileNames.size(); i++) {
            QString fileNameLocale = QDir::toNativeSeparators(fileNames.at(i));
            ui->listWidgetDynamic->addItem(fileNameLocale);
            DynamicFiles.append(fileNameLocale);
            LastFilePath = fileNameLocale;
        }
    }

}

void QGCAutoquad::delDynamic()
{
    int currIndex = ui->listWidgetDynamic->row(ui->listWidgetDynamic->currentItem());
    if ( currIndex >= 0) {
        QString SelDynamicFile =  ui->listWidgetDynamic->item(currIndex)->text();
        DynamicFiles.removeAt(DynamicFiles.indexOf(SelDynamicFile));
        ui->listWidgetDynamic->takeItem(currIndex);
    }
}


void QGCAutoquad::setUsersParams() {
    QString dirPath = QDir::toNativeSeparators(UsersParamsFile);
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Parameters (*.params)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        ShowUsersParams(QDir::toNativeSeparators(fileNames.at(0)));
    }
}

void QGCAutoquad::ShowUsersParams(QString fileName) {
    QFile file(fileName);
    UsersParamsFile = file.fileName();
    ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    ui->textOutput_Users_Params->setText(file.readAll());
    file.close();
}

void QGCAutoquad::CreateUsersParams() {
    QString dirPath = UsersParamsFile ;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Parameter-File (*.params)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        UsersParamsFile = fileNames.at(0);
    }
    if (!UsersParamsFile.endsWith(".params") )
        UsersParamsFile += ".params";

    UsersParamsFile = QDir::toNativeSeparators(UsersParamsFile);
    QFile file( UsersParamsFile );
    if ( file.exists())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("file already exists, Overwrite?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                file.close();
                QFile::remove(UsersParamsFile );
            }
            break;
            case QMessageBox::No:
            // ok was clicked
            break;
            default:
            // should never be reached
            break;
        }
    }

    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      return;
    }
    QDataStream stream( &file );
    stream << "";
    file.close();
    ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
}

void QGCAutoquad::WriteUsersParams() {
    QString message = ui->textOutput_Users_Params->toPlainText();
    QFile file(UsersParamsFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    QTextStream out(&file);
    out << message;
    file.close();
}

void QGCAutoquad::CalculatDeclination() {

    QString dec_source = ui->lineEdit_insert_declination->text();
    if ( !dec_source.contains(".")) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
    }
    /*
    if ( !dec_source.startsWith("-")) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
    }
    if ( dec_source.length() != 6 ) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
    }
    */
    QStringList HoursMinutes = dec_source.split(".");

    if ( HoursMinutes.count() != 2 ) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
    }
    qint32 secounds = HoursMinutes.at(1).toInt();
    float secounds_calc = (100.0f/60.0f) * secounds;
    // Set together
    QString recalculated;
    recalculated.append("#define");
    recalculated.append(' ');
    recalculated.append("IMU_MAG_INCL");
    recalculated.append(' ');
    recalculated.append(HoursMinutes.at(0));
    recalculated.append(".");
    recalculated.append( QString::number(secounds_calc,'f',0));

    ui->lineEdit_cal_declination->setText(recalculated);
    CalculatInclination();
}

double QGCAutoquad::Round(double Zahl, unsigned int Stellen)
{
    Zahl *= pow((double)10, (double)Stellen);
    if (Zahl >= 0)
        floor(Zahl + 0.5);
    else
        ceil(Zahl - 0.5);
    Zahl /= pow((double)10, (double)Stellen);
    return Zahl;
}

void QGCAutoquad::CalculatInclination() {

    QString inc_source = ui->lineEdit_insert_inclination->text();
    if ( !inc_source.contains(".")) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic inclination!",QMessageBox::Ok, 0 );
        return;
    }
    if ( inc_source.length() < 3 ) {
        QMessageBox::information(this, "Error", "Wrong format for magnetic inclination!",QMessageBox::Ok, 0 );
        return;
    }
    QStringList HoursMinutes = inc_source.split('.');

    qint32 secounds = HoursMinutes.at(1).toInt();
    float secounds_calc =  (100.0f/60.0f) * secounds;
    //secounds_calc = Round(secounds_calc, 0);
    // Set together
    QString recalculated;
    recalculated.append(HoursMinutes.at(0));
    recalculated.append(".");
    recalculated.append( QString::number(secounds_calc,'f',0));
    ui->lineEdit_cal_inclination->setText(recalculated);

}


void QGCAutoquad::check_var()
{
    if ( ui->checkBox_sim3_4_var_1->checkState()) {
        ui->sim3_4_var_1->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_var_1->checkState()) {
        ui->sim3_4_var_1->setEnabled(false);
    }

    if ( ui->checkBox_sim3_4_var_2->checkState()) {
        ui->sim3_4_var_2->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_var_2->checkState()) {
        ui->sim3_4_var_2->setEnabled(false);
    }

    if ( ui->checkBox_sim3_5_var->checkState()) {
        ui->sim3_5_var->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_5_var->checkState()) {
        ui->sim3_5_var->setEnabled(false);
    }

    if ( ui->checkBox_sim3_6_var->checkState()) {
        ui->sim3_6_var->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_6_var->checkState()) {
        ui->sim3_6_var->setEnabled(false);
    }

}

void QGCAutoquad::check_stop()
{
    if ( ui->checkBox_sim3_4_stop_1->checkState()) {
        ui->sim3_4_stop_1->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_stop_1->checkState()) {
        ui->sim3_4_stop_1->setEnabled(false);
    }

    if ( ui->checkBox_sim3_4_stop_2->checkState()) {
        ui->sim3_4_stop_2->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_stop_2->checkState()) {
        ui->sim3_4_stop_2->setEnabled(false);
    }

    if ( ui->checkBox_sim3_5_stop->checkState()) {
        ui->sim3_5_stop->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_5_stop->checkState()) {
        ui->sim3_5_stop->setEnabled(false);
    }

    if ( ui->checkBox_sim3_6_stop->checkState()) {
        ui->sim3_6_stop->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_6_stop->checkState()) {
        ui->sim3_6_stop->setEnabled(false);
    }


}

void QGCAutoquad::startcal1(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "cal" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);

    QStringList Arguments;

    Arguments.append("--rate");
    for ( int i = 0; i<StaticFiles.count(); i++) {
        Arguments.append(StaticFiles.at(i));
    }
    Arguments.append(":");

    active_cal_mode = 1;
    ui->textOutput_cal1->clear();
    ui->pushButton_start_cal1->setEnabled(false);
    ui->pushButton_abort_cal1->setEnabled(true);
    ui->textOutput_cal1->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_cal1->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startcal2(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "cal" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);

    QStringList Arguments;

    Arguments.append("--acc");
    for ( int i = 0; i<StaticFiles.count(); i++) {
        Arguments.append(StaticFiles.at(i));
    }
    Arguments.append(":");

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 2;
    ui->textOutput_cal2->clear();
    ui->pushButton_start_cal2->setEnabled(false);
    ui->pushButton_abort_cal2->setEnabled(true);
    ui->textOutput_cal2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_cal2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startcal3(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "cal" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);

    QStringList Arguments;

    if ( !ui->checkBox_DIMU->isChecked()) {
        Arguments.append("--mag");

        for ( int i = 0; i<StaticFiles.count(); i++) {
            Arguments.append(StaticFiles.at(i));
        }
        Arguments.append(":");
    }
    else {
        Arguments.append("-b");
        Arguments.append("--mag");

        for ( int i = 0; i<StaticFiles.count(); i++) {
            Arguments.append(StaticFiles.at(i));
        }
        Arguments.append(":");
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    active_cal_mode = 3;
    ui->textOutput_cal3->clear();
    ui->pushButton_start_cal3->setEnabled(false);
    ui->pushButton_abort_cal3->setEnabled(true);
    ui->textOutput_cal3->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_cal3->append(Arguments.at(i));
    }

    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim1(){
    QString AppPath;
    QString Sim3ParaPath;
    if ( !ui->checkBox_DIMU->isChecked()) {
        AppPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3" + platformExeExt);
        ps_master.setWorkingDirectory(aqBinFolderPath);
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3.params");
    }
    else {
        AppPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3" + platformExeExt);
        ps_master.setWorkingDirectory(aqBinFolderPath);
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3_dimu.params");
    }

    QStringList Arguments;

    Arguments.append("--gyo");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_4_var_1->checkState() ) {
        Arguments.append("--var=" + ui->sim3_4_var_1->text());
    }
    if ( ui->checkBox_sim3_4_stop_1->checkState() ) {
        Arguments.append("--stop=" + ui->sim3_4_stop_1->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 4;
    ui->textOutput_sim1->clear();
    ui->pushButton_start_sim1->setEnabled(false);
    ui->pushButton_abort_sim1->setEnabled(true);
    ui->textOutput_sim1->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim1->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim1b(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);
    QString Sim3ParaPath;
    if ( !ui->checkBox_DIMU->isChecked())
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3.params");
    else
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3_dimu.params");

    QStringList Arguments;

    Arguments.append("--acc");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_4_var_2->checkState() ) {
        Arguments.append("--var=" + ui->sim3_4_var_2->text());
    }
    if ( ui->checkBox_sim3_4_stop_2->checkState() ) {
        Arguments.append("--stop=" + ui->sim3_4_var_2->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 41;
    ui->textOutput_sim1_2->clear();
    ui->pushButton_start_sim1_2->setEnabled(false);
    ui->pushButton_abort_sim1_2->setEnabled(true);
    ui->textOutput_sim1_2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim1_2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim2(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);
    QString Sim3ParaPath;
    if ( !ui->checkBox_DIMU->isChecked())
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3.params");
    else
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3_dimu.params");

    QStringList Arguments;

    Arguments.append("--acc");
    Arguments.append("--gyo");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append( QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_5_var->checkState() ) {
        Arguments.append("--var=" + ui->sim3_5_var->text());
    }
    if ( ui->checkBox_sim3_5_stop->checkState() ) {
        Arguments.append("--stop=" + ui->sim3_5_stop->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 5;
    ui->textOutput_sim2->clear();
    ui->pushButton_start_sim2->setEnabled(false);
    ui->pushButton_abort_sim2->setEnabled(true);
    ui->textOutput_sim2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim3(){
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3" + platformExeExt);
    ps_master.setWorkingDirectory(aqBinFolderPath);
    QString Sim3ParaPath;
    if ( !ui->checkBox_DIMU->isChecked())
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3.params");
    else
        Sim3ParaPath = QDir::toNativeSeparators(aqBinFolderPath + "sim3_dimu.params");

    QStringList Arguments;

    Arguments.append("--mag");
    if ( ui->checkBox_DIMU->isChecked())
        Arguments.append("-b");
    Arguments.append("--incl");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_6_var->checkState() ) {
        Arguments.append("--var=" + ui->sim3_6_var->text());
    }
    if ( ui->checkBox_sim3_6_stop->checkState() ) {
        Arguments.append("--stop=" + ui->sim3_6_var->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 6;
    ui->textOutput_sim3->clear();
    ui->pushButton_start_sim3->setEnabled(false);
    ui->pushButton_abort_sim3->setEnabled(true);
    ui->textOutput_sim3->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim3->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::abortcalc(){
    if ( ps_master.Running)
        ps_master.close();
}


//
// ESC32
//

void QGCAutoquad::setPortNameEsc32(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
    portNameEsc32 = port;
    ui->label_portName_esc32->setText(portNameEsc32);
}

void QGCAutoquad::flashFWEsc32() {

    QString msg = "";

    msg = QString("WARNING: Flashing firmware will reset all ESC32 settings back to default values. \
Make sure you have your custom settings saved.\n\n\
Make sure you are using the %1 port.\n\n\
There is a delay before the flashing process shows any progress. Please wait at least 20sec. before you retry!\n\n\
Do you wish to continue flashing?").arg(portNameEsc32);

    QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Firmware Flashing"), msg, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
    if (qrply == QMessageBox::Cancel)
        return;

    FlashEsc32Active = true;
    QString port = ui->label_portName_esc32->text();
    if ( ui->pushButton_connect_to_esc32->text() == "connect esc32") {
        esc32 = new AQEsc32();
        connect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        connect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        connect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        connect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        connect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        connect(esc32 , SIGNAL(getCommandBack(int)),this,SLOT(Esc32CaliGetCommand(int)));
        connect(esc32, SIGNAL(finishedCalibration(int)),this,SLOT(Esc32CalibrationFinished(int)));
        connect(esc32, SIGNAL(EnteredBootMode()),this,SLOT(Esc32BootModOk()));
        connect(esc32, SIGNAL(NoBootModeArmed()),this,SLOT(Esc32BootModFailure()));
        ui->pushButton_connect_to_esc32->setText("disconnect");
        esc32->Connect(port);
    }
}

void QGCAutoquad::Esc32BootModOk() {
    ui->tabWidget->setCurrentIndex(0);
    QString port = ui->label_portName_esc32->text();
    FlashEsc32Active = false;
    QString msg = "";
    if ( ui->pushButton_connect_to_esc32->text() != "connect esc32") {
        disconnect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        disconnect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        disconnect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        disconnect(esc32 , SIGNAL(getCommandBack(int)),this,SLOT(Esc32CaliGetCommand(int)));
        disconnect(esc32, SIGNAL(finishedCalibration(int)),this,SLOT(Esc32CalibrationFinished(int)));
        disconnect(esc32, SIGNAL(EnteredBootMode()),this,SLOT(Esc32BootModOk()));
        disconnect(esc32, SIGNAL(NoBootModeArmed()),this,SLOT(Esc32BootModFailure()));
        ui->pushButton_connect_to_esc32->setText("connect esc32");
        esc32->Disconnect(true);
        esc32 = NULL;
    }

    if (QFile::exists(FwFileForEsc32)) {
        msg = QString("Flashing the %1 file again?").arg(FwFileForEsc32);
        QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Firmware Flashing"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (qrply == QMessageBox::No)
            FwFileForEsc32 = "";
    }

    if ( FwFileForEsc32 == "") {
        QString dirPath;
        if ( LastFilePath == "")
            dirPath = QCoreApplication::applicationDirPath();
        else
            dirPath = LastFilePath;
        QFileInfo dir(dirPath);
        QFileDialog dialog;
        dialog.setDirectory(dir.absoluteDir());
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setFilter(tr("AQ hex (*.hex)"));
        dialog.setViewMode(QFileDialog::Detail);
        QStringList fileNames;
        if (dialog.exec())
        {
            fileNames = dialog.selectedFiles();
        }
        if (fileNames.size() > 0)
        {
            QString fileNameLocale = QDir::toNativeSeparators(fileNames.first());
            QFile file(fileNameLocale );
            ui->fileLabel->setText(fileNameLocale );
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QMessageBox msgBox;
                msgBox.setText("Could not read hex file. Permission denied");
                msgBox.exec();
            }
            FwFileForEsc32 = file.fileName();
            LastFilePath = FwFileForEsc32;
            file.close();
        }
    }

    ps_master.setWorkingDirectory(aqBinFolderPath);
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "stm32flash" + platformExeExt);
    QStringList Arguments;
    Arguments.append("-b 57600");
    Arguments.append("-w" );
    Arguments.append(QDir::toNativeSeparators(FwFileForEsc32));
    Arguments.append("-v");
    Arguments.append(port);
    active_cal_mode = 0;
    ui->textFlashOutput->clear();
    ps_master.start(AppPath , Arguments, QProcess::Unbuffered | QProcess::ReadWrite);
}

void QGCAutoquad::Esc32BootModFailure() {
    disconnect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
    disconnect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
    disconnect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
    disconnect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
    disconnect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
    disconnect(esc32 , SIGNAL(getCommandBack(int)),this,SLOT(Esc32CaliGetCommand(int)));
    disconnect(esc32, SIGNAL(finishedCalibration(int)),this,SLOT(Esc32CalibrationFinished(int)));
    ui->pushButton_connect_to_esc32->setText("connect esc32");
    esc32->Disconnect(false);
    esc32 = NULL;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Error");
    msgBox.setInformativeText("Your Esc is armed, please disarm it at first!");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void QGCAutoquad::btnConnectEsc32()
{
    QString msg = "";
    bool IsConnected = false;
    if ( uas != NULL ) {
        for ( int i=0; i<uas->getLinks()->count(); i++) {
            if ( uas->getLinks()->at(i)->isConnected() == true) {
                IsConnected = true;
            }
        }
    }

    if ( IsConnected ) {
        msg = QString("WARNING: You are already connected to AutoQuad! If you continue, you will be disconnected.\n\n\
Do you wish to continue connecting to ESC32?").arg(portName);
        QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Disconnect AutoQuad"), msg, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
        if (qrply == QMessageBox::Cancel)
            return;
        for ( int i=0; i<uas->getLinks()->count(); i++) {
            if ( uas->getLinks()->at(i)->isConnected() == true) {
                uas->getLinks()->at(i)->disconnect();
            }
        }
    }

    QString port = ui->label_portName_esc32->text();
    if ( ui->pushButton_connect_to_esc32->text() == "connect esc32") {
        esc32 = new AQEsc32();
        connect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        connect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        connect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        connect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        connect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        connect(esc32 , SIGNAL(getCommandBack(int)),this,SLOT(Esc32CaliGetCommand(int)));
        connect(esc32, SIGNAL(finishedCalibration(int)),this,SLOT(Esc32CalibrationFinished(int)));
        ui->pushButton_connect_to_esc32->setText("disconnect");
        esc32->Connect(port);
    }
    else {
        disconnect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        disconnect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        disconnect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        disconnect(esc32 , SIGNAL(getCommandBack(int)),this,SLOT(Esc32CaliGetCommand(int)));
        disconnect(esc32, SIGNAL(finishedCalibration(int)),this,SLOT(Esc32CalibrationFinished(int)));
        ui->pushButton_connect_to_esc32->setText("connect esc32");
        esc32->Disconnect(false);
        esc32 = NULL;
    }
}

void QGCAutoquad::showConfigEsc32(QString Config)
{
    paramEsc32.clear();
    QString ConfigStr = Config.remove("\n");
    QStringList RowList = ConfigStr.split("\r");
    for ( int j = 0; j< RowList.length(); j++) {
        QStringList ParaList = RowList.at(j).split(" ", QString::SkipEmptyParts);
        if ( ParaList.length() >= 3)
            paramEsc32.insert(ParaList.at(0),ParaList.at(2));
    }

    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
    for ( int i = 0; i<edtList.count(); i++) {
        //edtList.at(i)->setText("");
        QString ParaName = edtList.at(i)->objectName();
        if ( paramEsc32.contains(ParaName) )
        {
            QString value = paramEsc32.value(ParaName);
            edtList.at(i)->setText(value);
        }
    }
}

void QGCAutoquad::btnSaveToEsc32() {

    bool oneWritten = false;
    bool something_gos_wrong = false;
    int rettryToStore = 0;
    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tabWidget );
    for ( int i = 0; i<edtList.count(); i++) {
        QString ParaName = edtList.at(i)->objectName();
        QString valueText = edtList.at(i)->text();
        if ( paramEsc32.contains(ParaName) )
        {
            QString valueEsc32 = paramEsc32.value(ParaName);
            //QString valueText = edtList.at(i)->text();
            if ( valueEsc32 != valueText) {
                WaitForParaWriten = 1;
                ParaNameWritten = ParaName;
                esc32->SavePara(ParaName,valueText);
                oneWritten = true;
                int timeout = 0;
                while(WaitForParaWriten >0) {
                    if (paramEsc32Written.contains(ParaName)) {
                        paramEsc32Written.remove(ParaName);
                        break;
                    }
                    timeout++;
                    if ( timeout > 500000) {
                        something_gos_wrong = true;
                        break;
                    }
                    QCoreApplication::processEvents();
                }
            }
        }
        if ((rettryToStore >= 3) && (something_gos_wrong))
            break;
    }

    if (( oneWritten ) && (!something_gos_wrong))
        saveEEpromEsc32();
    else if (something_gos_wrong) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setInformativeText("Something went wrong trying to store the values. Please retry!");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

}

void QGCAutoquad::btnReadConfigEsc32() {
    esc32->sendCommand(BINARY_COMMAND_CONFIG,0.0f, 0.0f, 0, false);
    esc32->ReadConfigEsc32();
}

void QGCAutoquad::btnArmEsc32()
{
    if ( !esc32)
        return;
    if ( ui->pushButton_esc32_read_arm_disarm->text() == "arm")
        esc32->sendCommand(BINARY_COMMAND_ARM,0.0f, 0.0f, 0, false);
    else if ( ui->pushButton_esc32_read_arm_disarm->text() == "disarm")
        esc32->sendCommand(BINARY_COMMAND_DISARM,0.0f, 0.0f, 0, false);

}

void QGCAutoquad::btnStartStopEsc32()
{
    if ( !esc32)
        return;
    if ( ui->pushButton_esc32_read_start_stop->text() == "start")
        esc32->sendCommand(BINARY_COMMAND_START,0.0f, 0.0f, 0, false);
    else if ( ui->pushButton_esc32_read_start_stop->text() == "stop")
        esc32->sendCommand(BINARY_COMMAND_STOP,0.0f, 0.0f, 0, false);
}

void QGCAutoquad::ParaWrittenEsc32(QString ParaName) {
    if ( ParaNameWritten == ParaName) {
        WaitForParaWriten = 0;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaNamEedt = edtList.at(i)->objectName();
            if ( ParaNamEedt == ParaName )
            {
                paramEsc32.remove(ParaName);
                paramEsc32.insert(ParaName,edtList.at(i)->text());
                paramEsc32Written.insert(ParaName,edtList.at(i)->text());
                qDebug() << ParaName << " written";
                break;
            }
        }
    }
}

void QGCAutoquad::CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2) {
    if ( CommandName == BINARY_COMMAND_ARM) {
        ui->pushButton_esc32_read_arm_disarm->setText("disarm");
    }
    if ( CommandName == BINARY_COMMAND_DISARM) {
        ui->pushButton_esc32_read_arm_disarm->setText("arm");
    }
    if ( CommandName == BINARY_COMMAND_START) {
        ui->pushButton_esc32_read_start_stop->setText("stop");
    }
    if ( CommandName == BINARY_COMMAND_STOP) {
        ui->pushButton_esc32_read_start_stop->setText("start");
    }
    if ( CommandName == BINARY_COMMAND_RPM) {
        ui->label_rpm->setText(V1.toString());
    }
}

void QGCAutoquad::btnSetRPM()
{
    if (( ui->pushButton_esc32_read_start_stop->text() == "stop") &&( ui->pushButton_esc32_read_arm_disarm->text() == "disarm")) {
        float rpm = (float)ui->horizontalSlider_rpm->value();
        float ff1Term = ui->FF1TERM->text().toFloat();
        if ( ff1Term == 0.0f) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error");
            msgBox.setInformativeText("The Parameter FF1Term is 0.0, can't set the RPM!");
            msgBox.setWindowModality(Qt::ApplicationModal);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
        }
        else {
            ui->label_rpm->setText(QString::number(ui->horizontalSlider_rpm->value()));
            esc32->sendCommand(BINARY_COMMAND_RPM, rpm, 0.0f, 1, false);
        }
    }
}

void QGCAutoquad::Esc32RpmSlider(int rpm) {
    ui->label_rpm->setText(QString::number(rpm));
}

void QGCAutoquad::saveEEpromEsc32()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values have been transmitted to Esc32! Do you want to store the parameters into permanent memory (ROM)?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            esc32->sendCommand(BINARY_COMMAND_CONFIG,1.0f, 0.0f, 1, false);
        }
        break;
        case QMessageBox::No:
        break;
        default:
        // should never be reached
        break;
    }
}

void QGCAutoquad::Esc32Connected(){
    if ( !FlashEsc32Active )
        esc32->ReadConfigEsc32();
    else
        esc32->SetToBootMode();
}

void QGCAutoquad::ESc32Disconnected() {
}

void QGCAutoquad::Esc32StartLogging() {
    esc32->StartLogging();
}

void QGCAutoquad::Esc32StartCalibration() {
    if (!esc32)
        return;

    QString Esc32LoggingFile = "";
    QString Esc32ResultFile = "";

    if ( ui->pushButton_start_calibration->text() == "start calibration") {
        QMessageBox InfomsgBox;
        InfomsgBox.setText("<p style='color: red; font-weight: bold;'>WARNING!! EXPERIMENTAL FEATURE! BETTER TO USE Linux/OS-X COMMAND-LINE TOOLS!</p> \
<p>This is the calibration routine for ESC32!</p> \
<p>Please be careful with the calibration function! The motor will spin up to full throttle! Please stay clear of the motor & propeller!</p> \
<p><b style='color: red;'>Proceed at your own risk!</b>  You will have one more chance to cancel before the procedure starts.</p>");
        InfomsgBox.setWindowModality(Qt::ApplicationModal);
        InfomsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        InfomsgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = InfomsgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;

        ret = QMessageBox::question(this,"Question","Which calibration do you want to do?","RpmToVoltage","CurrentLimiter");
        if ( ret == 0) {
            Esc32CalibrationMode = 1;
            #ifdef Q_OS_WIN
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "RPMTOVOLTAGE.txt");
                Esc32ResultFile =  QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "RPMTOVOLTAGE_RESULT.txt");
            #else
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "RPMTOVOLTAGE_RESULT.TXT");
                Esc32ResultFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "RPMTOVOLTAGE_RESULT.TXT");
            #endif
        }
        else if ( ret == 1) {
            Esc32CalibrationMode = 2;
            #ifdef Q_OS_WIN
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "CURRENTLIMITER.TXT");
                Esc32ResultFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "CURRENTLIMITER_RESULT.TXT");
            #else
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "CURRENTLIMITER.TXT");
                Esc32ResultFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "CURRENTLIMITER_RESULT.TXT");
            #endif
        }
        else {
            QMessageBox InfomsgBox;
            InfomsgBox.setText("Failure in calibration routine!");
            InfomsgBox.exec();
            return;
        }

        if (QFile::exists(Esc32LoggingFile))
            QFile::remove(Esc32LoggingFile);
        if (QFile::exists(Esc32ResultFile))
            QFile::remove(Esc32ResultFile);

        QMessageBox msgBox;
        msgBox.setText("<p style='font-weight: bold;'>Again, be carful! You can abort using the Stop Calibration button, but the fastest stop is to pull the battery!</p> \
<p style='font-weight: bold;'>To start the calibration procedure, press Yes.</p><p style='color: red; font-weight: bold;'>This is your final warning!</p>");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        ret = msgBox.exec();
        if ( ret == QMessageBox::Cancel)
            return;

        if ( ret == QMessageBox::Yes) {
            float maxAmps = ui->DoubleMaxCurrent->text().toFloat();

            esc32->SetCalibrationMode(this->Esc32CalibrationMode);
            esc32->StartCalibration(maxAmps,Esc32LoggingFile,Esc32ResultFile);
            ui->pushButton_start_calibration->setText("stop calibration");
        }
    }
    else if ( ui->pushButton_start_calibration->text() == "stop calibration")
    {
        ui->pushButton_start_calibration->setText("start calibration");
        esc32->StopCalibration(true);
    }
}

void QGCAutoquad::Esc32CalibrationFinished(int mode) {
    //Emergency exit
    if ( mode == 99) {
        //No values from esc32
        QMessageBox InfomsgBox;
        InfomsgBox.setText("Something went wrong in data logging, Aborted!");
        InfomsgBox.exec();
        return;
    }
    if ( mode == 98) {
        //Abort
        return;
    }
    esc32->StopCalibration(false);
    if ( mode == 1) {
        ui->FF1TERM->setText(QString::number(esc32->getFF1Term()));
        ui->FF2TERM->setText(QString::number(esc32->getFF2Term()));
        ui->pushButton_start_calibration->setText("start calibration");
        //Esc32LoggingFile
        QMessageBox InfomsgBox;
        InfomsgBox.setText("Updated the fields with FF1Term and FF2Term!");
        InfomsgBox.exec();
        return;
    }
    if ( mode == 2) {
        ui->CL1TERM->setText(QString::number(esc32->getCL1()));
        ui->CL2TERM->setText(QString::number(esc32->getCL2()));
        ui->CL3TERM->setText(QString::number(esc32->getCL3()));
        ui->CL4TERM->setText(QString::number(esc32->getCL4()));
        ui->CL5TERM->setText(QString::number(esc32->getCL5()));
        ui->pushButton_start_calibration->setText("start calibration");
        QMessageBox InfomsgBox;
        InfomsgBox.setText("Updated the fields with Currentlimiter 1 to Currentlimiter 5!");
        InfomsgBox.exec();
        return;
    }
}

void QGCAutoquad::Esc32ReadConf() {
    esc32->sendCommand(BINARY_COMMAND_CONFIG,2.0f, 0.0f, 1, false);
    esc32->SwitchFromBinaryToAscii();
    esc32->ReadConfigEsc32();
}

void QGCAutoquad::Esc32ReLoadConf() {
    esc32->sendCommand(BINARY_COMMAND_CONFIG,0.0f, 0.0f, 1, false);
    esc32->SwitchFromBinaryToAscii();
    esc32->ReadConfigEsc32();
}

void QGCAutoquad::Esc32CaliGetCommand(int Command){
    esc32->SetCommandBack(Command);
}


//
// FW Flashing
//

void QGCAutoquad::setPortName(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
    portName = port;
    ui->ComPortLabel->setText(portName);
}

void QGCAutoquad::setupPortList()
{
    ui->portName->clear();
    ui->portName->clearEditText();

    ui->comboBox_port_esc32->clear();
    ui->comboBox_port_esc32->clearEditText();
    // Get the ports available on this system
    seriallink = new SerialLink();
    QVector<QString>* ports = seriallink->getCurrentPorts();

    // Add the ports in reverse order, because we prepend them to the list
    for (int i = ports->size() - 1; i >= 0; --i)
    {
        // Prepend newly found port to the list
        if (ui->portName->findText(ports->at(i)) == -1)
        {
            ui->portName->insertItem(0, ports->at(i));
        }
        if (ui->comboBox_port_esc32->findText(ports->at(i)) == -1)
        {
            ui->comboBox_port_esc32->insertItem(0, ports->at(i));
        }
    }
    ui->portName->setEditText(seriallink->getPortName());
    ui->comboBox_port_esc32->setEditText(seriallink->getPortName());
}

void QGCAutoquad::selectFWToFlash()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ hex (*.hex)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        QString fileNameLocale = QDir::toNativeSeparators(fileNames.first());
        QFile file(fileNameLocale);
        ui->fileLabel->setText(fileNameLocale);
        ui->fileLabel->setToolTip(fileNameLocale);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read hex file. Permission denied");
            msgBox.exec();
        }
        fileToFlash = file.fileName();
        LastFilePath = fileToFlash;
        file.close();
    }
}

void QGCAutoquad::flashFW()
{
    QString msg = "";
    bool IsConnected = false;
    connectedLink = NULL;
    if ( uas ) {
        for ( int i=0; i<uas->getLinks()->count(); i++) {
            if ( uas->getLinks()->at(i)->isConnected() == true) {
                IsConnected = true;
                connectedLink = uas->getLinks()->at(i);
                break;
            }
        }
    }

    if ( IsConnected )
        msg = QString("WARNING: You are already connected to AutoQuad. If you continue, you will be disconnected and then re-connected afterwards.\n\n");

    msg += QString("WARNING: Flashing firmware will reset all AutoQuad settings back to default values. \
Make sure you have your generated parameters and custom settings saved.\n\n\
Make sure AQ is connected to the %1 port.\n\n\
There is a delay before the flashing process shows any progress. Please wait at least 20sec. before you retry!\n\n\
Do you wish to continue flashing?").arg(portName);

    QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Firmware Flashing"), msg, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
    if (qrply == QMessageBox::Cancel)
        return;

    if ( IsConnected ) {
        for ( int i=0; i<uas->getLinks()->count(); i++) {
            if ( uas->getLinks()->at(i)->isConnected() == true) {
                uas->getLinks()->at(i)->disconnect();
            }
        }
    }

    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "stm32flash" + platformExeExt);

    QStringList Arguments;
    Arguments.append("-b 57600");
    Arguments.append("-w" );
    Arguments.append(QDir::toNativeSeparators(ui->fileLabel->text()));
    Arguments.append("-v");
    Arguments.append(portName);
    active_cal_mode = 0;
    ui->textFlashOutput->clear();
    ps_master.start(AppPath , Arguments, QProcess::Unbuffered | QProcess::ReadWrite);
}


//
// Radio view
//

void QGCAutoquad::radioType_changed(int idx) {
    emit hardwareInfoUpdated();

    if (ui->RADIO_TYPE->currentText() == "PPM") {
        ui->groupBox_ppmOptions->show();
        ui->groupBox_ppmOptions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    } else {
        ui->groupBox_ppmOptions->hide();
        ui->groupBox_ppmOptions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    }

    if (!paramaq)
        return;

    bool ok;
    int prevRadioValue = paramaq->getParaAQ("RADIO_TYPE").toInt(&ok);

    if (ok && idx != prevRadioValue)
        ui->label_radioChangeWarning->show();
    else
        ui->label_radioChangeWarning->hide();

}

bool QGCAutoquad::validateRadioSettings(int /*idx*/) {
    QList<int> portsUsed, conflictPorts;
    foreach (QComboBox* cb, allRadioChanCombos) {
        if (portsUsed.contains(cb->currentIndex()))
            conflictPorts.append(cb->currentIndex());
        portsUsed.append(cb->currentIndex());
    }
    foreach (QComboBox* cb, allRadioChanCombos) {
        if (conflictPorts.contains(cb->currentIndex()))
            cb->setStyleSheet("background-color: rgba(255, 0, 0, 200)");
        else
            cb->setStyleSheet("");
    }

    if (conflictPorts.size())
        return false;

    return true;
}

void QGCAutoquad::toggleRadioValuesUpdate() {
    if (!uas) {
        ui->pushButton_toggleRadioGraph->setChecked(false);
        return;
    }

    if (!ui->pushButton_toggleRadioGraph->isChecked()) {
        disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
        disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
        //ui->pushButton_toggleRadioGraph->setText("Start Updating");
        ui->conatiner_radioGraphValues->setEnabled(false);
        return;
    }

    int min, max, tmin, tmax;

    if ( ui->checkBox_raw_value->isChecked() ){
        tmax = 1500;
        tmin = -100;
        max = 1024;
        min = -1024;
        disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
        connect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
    } else {
        tmax = -500;
        tmin = 1500;
        max = -1500;
        min = 1500;
        disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
        connect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
    }

    foreach (QProgressBar* pb, allRadioChanProgressBars) {
        if (pb->objectName().contains("chan_0")) {
            pb->setMaximum(tmax);
            pb->setMinimum(tmin);
        } else {
            pb->setMaximum(max);
            pb->setMinimum(min);
        }
    }

    //ui->pushButton_toggleRadioGraph->setText("Stop Updating");
    ui->conatiner_radioGraphValues->setEnabled(true);

}

void QGCAutoquad::setRadioChannelDisplayValue(int channelId, float normalized)
{
    int val;
    bool raw = ui->checkBox_raw_value->isChecked();
    QString lblTxt;

    // three methods to find the right progress bar...
    // tested on a CoreDuo 3.3GHz at 1-10Hz mavlink refresh, seems to be no practical difference in CPU consumption (~40% ~10Hz)
    QProgressBar* bar = allRadioChanProgressBars.at(channelId);
    QLabel* lbl = allRadioChanValueLabels.at(channelId);
//    QProgressBar* bar = ui->groupBox_Radio_Values->findChild<QProgressBar *>(QString("progressBar_chan_%1").arg(channelId));
//    QLabel* lbl = ui->groupBox_Radio_Values->findChild<QLabel *>(QString("label_chanValue_%1").arg(channelId));
//    QProgressBar* bar = NULL;
//    QLabel* lbl = NULL;

//    switch (channelId) {
//    case 0:
//        bar = ui->progressBar_chan_0;
//        lbl = ui->label_chanValue_0;
//        break;
//    case 1:
//        bar = ui->progressBar_chan_1;
//        lbl = ui->label_chanValue_1;
//        break;
//    case 2:
//        bar = ui->progressBar_chan_2;
//        lbl = ui->label_chanValue_2;
//        break;
//    case 3:
//        bar = ui->progressBar_chan_3;
//        lbl = ui->label_chanValue_3;
//        break;
//    case 4:
//        bar = ui->progressBar_chan_4;
//        lbl = ui->label_chanValue_4;
//        break;
//    case 5:
//        bar = ui->progressBar_chan_5;
//        lbl = ui->label_chanValue_5;
//        break;
//    case 6:
//        bar = ui->progressBar_chan_6;
//        lbl = ui->label_chanValue_6;
//        break;
//    case 7:
//        bar = ui->progressBar_chan_7;
//        lbl = ui->label_chanValue_7;
//        break;
//    }

    if (raw)        // Raw values
        val = (int)(normalized-1024);
    else {    // Scaled values
        val = (int)((normalized*10000.0f)/13);
        if (channelId == 0)
            val += 750;
    }

    if (lbl) {
        lblTxt.sprintf("%+d", val);
        lbl->setText(lblTxt);
    }
    if (bar) {
        if (val > bar->maximum())
            val = bar->maximum();
        if (val < bar->minimum())
            val = bar->minimum();
        bar->setValue(val);
    }
}

void QGCAutoquad::setRssiDisplayValue(float normalized) {
    QProgressBar* bar = ui->progressBar_rssi;
    int val = (int)(normalized * 255.0f);

    if (bar && val <= bar->maximum() && val >= bar->minimum())
        bar->setValue(val);
}

void QGCAutoquad::delayedSendRcRefreshFreq(int rate)
{
    Q_UNUSED(rate);
    delayedSendRCTimer.start();
}

void QGCAutoquad::sendRcRefreshFreq()
{
    delayedSendRCTimer.stop();
    if (!uas)
        return;
    uas->enableRCChannelDataTransmission(ui->spinBox_rcGraphRefreshFreq->value());
}


//
// UAS Interfaces
//

void QGCAutoquad::addUAS(UASInterface* uas_ext)
{
    QString uasColor = uas_ext->getColor().name().remove(0, 1);

}

void QGCAutoquad::setActiveUAS(UASInterface* uas_ext)
{
    if (uas_ext)
    {
        if (uas) {
            disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
            disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
            disconnect(uas, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(globalPositionChangedAq(UASInterface*,double,double,double,quint64)) );
            disconnect(uas, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(handleStatusText(int, int, int, QString)));
            disconnect(uas, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(setRssiDisplayValue(float)));
        }
//        if (paramaq)
//            disconnect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(getGUIpara()));

        uas = uas_ext;
        connect(uas, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(globalPositionChangedAq(UASInterface*,double,double,double,quint64)) );
        connect(uas, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(handleStatusText(int, int, int, QString)));
        connect(uas, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(setRssiDisplayValue(float)));
        if ( !paramaq ) {
            paramaq = new QGCAQParamWidget(uas, this);
            ui->gridLayout_paramAQ->addWidget(paramaq);

            connect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(loadParametersToUI()));
            connect(paramaq, SIGNAL(paramRequestTimeout(int,int)), this, SLOT(paramRequestTimeoutNotify(int,int)));

            if ( LastFilePath == "")
                paramaq->setFilePath(QCoreApplication::applicationDirPath());
            else
                paramaq->setFilePath(LastFilePath);
        }
        paramaq->loadParaAQ();


        // get firmware version of this AQ
        aqFirmwareVersion = QString("");
        aqFirmwareRevision = 0;
        aqHardwareRevision = 0;
        aqBuildNumber = 0;
        ui->lbl_aq_fw_version->setText("AQ Firmware v. [unknown]");
        uas->sendCommmandToAq(MAV_CMD_AQ_REQUEST_VERSION, 1);

        VisibleWidget = 2;
        aqTelemetryView->initChart(uas);
        ui->checkBox_raw_value->setChecked(true);
        toggleRadioValuesUpdate();
    }
}

UASInterface* QGCAutoquad::getUAS()
{
    return uas;
}

QGCAQParamWidget* QGCAutoquad::getParamHandler()
{
    return paramaq;
}


//
// Parameter handling to/from AQ
//

void QGCAutoquad::adjustUiForHardware() {
    ui->groupBox_commSerial3->setVisible(aqHardwareVersion == 7);
    ui->groupBox_commSerial4->setVisible(aqHardwareVersion == 7);
}

void QGCAutoquad::getGUIpara(QWidget *parent) {
    if ( !paramaq || !parent)
        return;

    bool ok;
    int precision;
    QString paraName, valstr;
    QVariant val;

    // handle all input widgets
    QList<QWidget*> wdgtList = parent->findChildren<QWidget *>(fldnameRx);
    foreach (QWidget* w, wdgtList) {
        paraName = w->objectName().replace(dupeFldnameRx, "");
        if (!paramaq->paramExistsAQ(paraName)) {
            w->setEnabled(false);
            continue;
        }
        ok = true;
        precision = 6;
        if (paraName == "GMBL_SCAL_PITCH" || paraName == "GMBL_SCAL_ROLL" || paraName == "SIG_BEEP_PRT"){
            val = fabs(paramaq->getParaAQ(paraName).toFloat());
            precision = 8;
        }  else
            val = paramaq->getParaAQ(paraName);

        if (QLineEdit* le = qobject_cast<QLineEdit *>(w)){
            valstr.setNum(val.toFloat(), 'g', precision);
            le->setText(valstr);
        } else if (QComboBox* cb = qobject_cast<QComboBox *>(w))
            cb->setCurrentIndex(val.toInt(&ok));
        else if (QDoubleSpinBox* dsb = qobject_cast<QDoubleSpinBox *>(w)) {
            dsb->setValue(val.toDouble(&ok));
        }
        else if (QSpinBox* sb = qobject_cast<QSpinBox *>(w))
            sb->setValue(val.toInt(&ok));
        else
            continue;

        if (ok)
            w->setEnabled(true);
        else
            w->setEnabled(false);
            // TODO: notify the user, or something...
    }

    if (qobject_cast<QGCAutoquad *>(parent)) {
        // gimbal pitch/roll revese checkboxes
        float f_pitch_direction = paramaq->getParaAQ("GMBL_SCAL_PITCH").toFloat();
        float f_roll_direction = paramaq->getParaAQ("GMBL_SCAL_ROLL").toFloat();
        ui->reverse_gimbal_pitch->setChecked(f_pitch_direction < 0);
        ui->reverse_gimbal_roll->setChecked(f_roll_direction < 0);
    }


}

void QGCAutoquad::populateButtonGroups(QObject *parent) {
    QString paraName;
    QVariant val;

    // handle any button groups
    QList<QButtonGroup *> grpList = parent->findChildren<QButtonGroup *>(fldnameRx);
    foreach (QButtonGroup* g, grpList) {
        paraName = g->objectName().replace(dupeFldnameRx, "");
        val = paramaq->getParaAQ(paraName);

        foreach (QAbstractButton* abtn, g->buttons()) {
            if (paramaq->paramExistsAQ(paraName)) {
                abtn->setEnabled(true);
                if (g->exclusive()) { // individual values
                    abtn->setChecked(val.toInt() == g->id(abtn));
                } else { // bitmask
                    abtn->setChecked((val.toInt() & g->id(abtn)));
                }
            } else {
                abtn->setEnabled(false);
            }
        }
    }


}

void QGCAutoquad::loadParametersToUI() {
    getGUIpara(ui->RadioSettings);
    getGUIpara(ui->tab_aq_edit_para);
    populateButtonGroups(this);
    aqPwmPortConfig->loadOnboardConfig();
}

void QGCAutoquad::QuestionForROM()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values have been transmitted to AutoQuad and will persist until the next restart.\n\n\
Do you want to store all the current parameters into permanent on-board memory (ROM)?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            uas->writeParametersToStorageAQ();
        }
        break;
        case QMessageBox::No:
        break;
        default:
        // should never be reached
        break;
    }
}

bool QGCAutoquad::checkAqConnected(bool interactive) {

    if ( !paramaq || !uas || uas->getCommunicationStatus() != uas->COMM_CONNECTED ) {
        if (interactive)
            MainWindow::instance()->showCriticalMessage("Error", "No AutoQuad connected!");
        return false;
    } else
        return true;
}

bool QGCAutoquad::saveSettingsToAq(QWidget *parent, bool interactive)
{
    float val_uas, val_local;
    QString paraName;
    QStringList errors;
    bool changed=false, ok, chkstate;

    if ( !checkAqConnected(interactive) )
        return false;

    QList<QWidget*> wdgtList = parent->findChildren<QWidget *>(fldnameRx);
    foreach (QWidget* w, wdgtList) {
        paraName = w->objectName().replace(dupeFldnameRx, "");
        if (!paramaq->paramExistsAQ(paraName))
            continue;

        ok = true;
        val_uas = paramaq->getParaAQ(paraName).toFloat(&ok);

        if (QLineEdit* le = qobject_cast<QLineEdit *>(w))
            val_local = le->text().toFloat(&ok);
        else if (QComboBox* cb = qobject_cast<QComboBox *>(w))
            val_local = static_cast<float>(cb->currentIndex());
        else if (QAbstractSpinBox* sb = qobject_cast<QAbstractSpinBox *>(w))
            val_local = sb->text().replace(QRegExp("[^0-9,\\.]"), "").toFloat(&ok);
        else
            continue;

        if (!ok){
            errors.append(paraName);
            continue;
        }

        // special case for reversing gimbal servo direction
        if (paraName == "GMBL_SCAL_PITCH" || paraName == "GMBL_SCAL_ROLL" || paraName == "SIG_BEEP_PRT") {
            if (paraName == "GMBL_SCAL_PITCH")
                chkstate = parent->findChild<QCheckBox *>("reverse_gimbal_pitch")->checkState();
            else if (paraName == "GMBL_SCAL_ROLL")
                chkstate = parent->findChild<QCheckBox *>("reverse_gimbal_roll")->checkState();
            else if (paraName == "SIG_BEEP_PRT")
                chkstate = parent->findChild<QCheckBox *>("checkBox_useSpeaker")->checkState();

            if (chkstate)
                val_local = 0.0f - val_local;
        }

        if (val_uas != val_local){
            paramaq->setParaAQ(paraName, val_local);
            changed = true;
        }
    }

    if (errors.size()) {
        QString msg = "One or more parameter(s) could not be saved:\n\n";
        msg += errors.join("\n");
        MainWindow::instance()->showCriticalMessage("Error Saving Parameters!", msg);
    }

    if ( changed ) {
        if (interactive)
            QuestionForROM();
        return true;
    } else {
        if (interactive)
            MainWindow::instance()->showInfoMessage("Warning", "No changed parameters detected.  Nothing saved.");
        return false;
    }
}

void QGCAutoquad::saveRadioSettings() {
    if (!validateRadioSettings(0)) {
        MainWindow::instance()->showCriticalMessage("Error", "You have the same port assigned to multiple controls!");
        return;
    }
    saveSettingsToAq(ui->RadioSettings);
}

void QGCAutoquad::saveAttitudePIDs() {
    saveSettingsToAq(ui->TiltYawPID);
}

void QGCAutoquad::saveNavigationPIDs() {
    saveSettingsToAq(ui->NavigationPID);
}

void QGCAutoquad::saveSpecialSettings() {
    saveSettingsToAq(ui->SpecialSettings);
}

void QGCAutoquad::saveGimbalSettings() {
    saveSettingsToAq(ui->Gimbal);
}


//
// Log Viewer
//


void QGCAutoquad::SetupListView()
{
    ui->listView_Curves->setAutoFillBackground(true);
    QPalette p =  ui->listView_Curves->palette();
    DefaultColorMeasureChannels = p.color(QPalette::Window);
    model = new QStandardItemModel(this); //listView_curves
    for ( int i=0; i<parser.LogChannelsStruct.count(); i++ ) {
        QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
        QStandardItem *item = new QStandardItem(val_pair.second.fieldName);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setData(false, Qt::UserRole);
        model->appendRow(item);
    }
    ui->listView_Curves->setModel(model);
    connect(ui->listView_Curves, SIGNAL(clicked(QModelIndex)), this, SLOT(CurveItemClicked(QModelIndex)));
}

void QGCAutoquad::OpenLogFile(bool openFile)
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ log file (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        QFile file(fileNames.first());
        LogFile = QDir::toNativeSeparators(file.fileName());
        LastFilePath = LogFile;
        if (openFile && !file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read Log file. Permission denied");
            msgBox.exec();
        } else if (openFile)
            DecodeLogFile(LogFile);
    }
}

void QGCAutoquad::openExportOptionsDlg() {
    static QWeakPointer<AQLogExporter> dlg_;

    if (!dlg_)
        dlg_ = new AQLogExporter(this);

    AQLogExporter *dlg = dlg_.data();

    if (LogFile.length())
        dlg->setLogFile(LogFile);

    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}


void QGCAutoquad::CurveItemClicked(QModelIndex index) {
    QStandardItem *item = model->itemFromIndex(index);

    if (item->data(Qt::UserRole).toBool() == false)
    {
        item->setCheckState(Qt::Checked);
        item->setData(true, Qt::UserRole);
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 1;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }
    else
    {
        item->setCheckState(Qt::Unchecked);
        item->setData(false, Qt::UserRole);
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 0;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }
}

void QGCAutoquad::deselectAllCurves(void) {
    if (model) {
        for (int i=0; i < model->rowCount(); ++i){
            if (model->item(i)->data(Qt::UserRole).toBool() == true)
                CurveItemClicked(model->item(i)->index());
        }
    }
}

void QGCAutoquad::DecodeLogFile(QString fileName)
{
    plot->removeData();
    plot->clear();
    plot->setStyleText("lines");

    disconnect(ui->listView_Curves, SIGNAL(clicked(QModelIndex)), this, SLOT(CurveItemClicked(QModelIndex)));
    ui->listView_Curves->reset();

    if ( parser.ParseLogHeader(fileName) == 0)
        SetupListView();

}

void QGCAutoquad::showChannels() {

    parser.ShowCurves();
    plot->removeData();
    plot->clear();
    plot->ResetColor();
    if (!QFile::exists(LogFile))
        return;

    for (int i = 0; i < parser.yValues.count(); i++) {
        plot->appendData(parser.yValues.keys().at(i), parser.xValues.values().at(0)->data(), parser.yValues.values().at(i)->data(), parser.xValues.values().at(0)->count());
    }

    plot->setStyleText("lines");
    plot->updateScale();
    for ( int i = 0; i < model->rowCount(); i++) {
        QStandardItem *item = model->item(i,0);
        if ( item->data(Qt::UserRole).toBool() )
            //item->setForeground(plot->getColorForCurve(item->text()));
            item->setBackground(plot->getColorForCurve(item->text()));
        else
            item->setBackground(DefaultColorMeasureChannels);
    }

}


void QGCAutoquad::save_plot_image(){
    QString fileName = "plot.svg";
    fileName = QFileDialog::getSaveFileName(this, "Export File Name", \
                                            QDesktopServices::storageLocation(QDesktopServices::DesktopLocation), \
                                            "SVG Images (*.svg)"); // ;;PDF Documents (*.pdf)

    if (!fileName.contains(".")) {
        // .svg is default extension
        fileName.append(".svg");
    }

    while(!(fileName.endsWith(".svg") || fileName.endsWith(".pdf"))) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Unsuitable file extension for PDF or SVG");
        msgBox.setInformativeText("Please choose .pdf or .svg as file extension. Click OK to change the file extension, cancel to not save the file.");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        // Abort if cancelled
        if(msgBox.exec() == QMessageBox::Cancel) return;
        fileName = QFileDialog::getSaveFileName(
                       this, "Export File Name", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
                       "PDF Documents (*.pdf);;SVG Images (*.svg)");
    }

    if (fileName.endsWith(".svg")) {
        exportSVG(fileName);
    } else if (fileName.endsWith(".pdf")) {
        exportPDF(fileName);
    }}

void QGCAutoquad::exportPDF(QString fileName)
{
    /*
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    //printer.setFullPage(true);
    printer.setPageMargins(10.0, 10.0, 10.0, 10.0, QPrinter::Millimeter);
    printer.setPageSize(QPrinter::A4);

    QString docName = plot->title().text();
    if ( !docName.isEmpty() ) {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("QGroundControl");
    printer.setOrientation(QPrinter::Landscape);

    plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
    //        plot->setCanvasBackground(Qt::white);
    //        QwtPlotPrintFilter filter;
    //        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);
    //        if ( printer.colorMode() == QPrinter::GrayScale )
    //        {
    //            int options = QwtPlotPrintFilter::PrintAll;
    //            options &= ~QwtPlotPrintFilter::PrintBackground;
    //            options |= QwtPlotPrintFilter::PrintFrameWithScales;
    //            filter.setOptions(options);
    //        }
    plot->print(printer);//, filter);
    plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
    //plot->setCanvasBackground(QColor(5, 5, 8));

    */
}

void QGCAutoquad::exportSVG(QString fileName)
{
    if ( !fileName.isEmpty() ) {
        plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
        //plot->setCanvasBackground(Qt::white);
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(QSize(800, 600));

        QwtPlotPrintFilter filter;
        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);

        plot->print(generator, filter);
        plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
    }
}

void QGCAutoquad::startSetMarker() {

    if ( parser.xValues.count() <= 0)
        return;
    if ( parser.yValues.count() <= 0)
        return;
    removeMarker();

    if ( picker == NULL ) {
        if ( ui->comboBox_marker->currentIndex() == 6) {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }
            ui->pushButton_cut->setEnabled(false);
            QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
            picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPicker::PointSelection,
                         QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOff,
                         plot->canvas());
            picker->setRubberBand(QwtPicker::CrossRubberBand);
            connect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
            StepCuttingPlot = 0;
        }
        else {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }
            ui->pushButton_cut->setEnabled(false);

            double x1,x2,y1,y2 = 0;
            int time_count = 0;
            //200 Hz
            if ( ui->comboBox_marker->currentIndex() == 0)
                time_count = 200 * 1;
            if ( ui->comboBox_marker->currentIndex() == 1)
                time_count = 200 * 2;
            if ( ui->comboBox_marker->currentIndex() == 2)
                time_count = 200 * 3;
            if ( ui->comboBox_marker->currentIndex() == 3)
                time_count = 200 * 5;
            if ( ui->comboBox_marker->currentIndex() == 4)
                time_count = 200 * 10;
            if ( ui->comboBox_marker->currentIndex() == 5)
                time_count = 200 * 15;

            MarkerCut1 = new QwtPlotMarker();
            MarkerCut1->setLabel(QString::fromLatin1("sp1"));
            MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
            MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
            x1 = parser.xValues.value("XVALUES")->value(0);
            y1 = parser.yValues.values().at(0)->value(0);
            MarkerCut1->setValue(x1,y1);
            MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut1->setVisible(true);
            MarkerCut1->attach(plot);

            MarkerCut2 = new QwtPlotMarker();
            MarkerCut2->setLabel(QString::fromLatin1("ep1"));
            MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
            MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x2 = parser.xValues.value("XVALUES")->value(time_count-1);
                y2 = parser.yValues.values().at(0)->value(time_count-1);
            }
            else {
                x2 = parser.xValues.value("XVALUES")->value(time_count);
                y2 = parser.yValues.values().at(0)->value(time_count);
            }
            MarkerCut2->setValue(x2,y2);
            MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut2->setVisible(true);
            MarkerCut2->attach(plot);

            MarkerCut3 = new QwtPlotMarker();
            MarkerCut3->setLabel(QString::fromLatin1("sp2"));
            MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
            MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x1 = (parser.xValues.values().at(0)->count()) - time_count;
                y1 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count())- time_count);
            }
            else {
                x1 = (parser.xValues.values().at(0)->count()-1) - time_count;
                y1 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()-1)- time_count);
            }
            MarkerCut3->setValue(x1,y1);
            MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut3->setVisible(true);
            MarkerCut3->attach(plot);

            MarkerCut4 = new QwtPlotMarker();
            MarkerCut4->setLabel(QString::fromLatin1("ep2"));
            MarkerCut4->setLabelAlignment(Qt::AlignLeft|Qt::AlignBottom);
            MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x2 = (parser.xValues.values().at(0)->count());
                y2 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()));
            }
            else {
                x2 = (parser.xValues.values().at(0)->count()-1);
                y2 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()-1));
            }
            MarkerCut4->setValue(x2,y2);
            MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut4->setVisible(true);
            MarkerCut4->attach(plot);
            plot->replot();
            ui->pushButton_cut->setEnabled(true);
        }
    }
    else {
        if ( picker){
            disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
            picker->setEnabled(false);
            picker = NULL;
        }
    }

}

void QGCAutoquad::setPoint1(const QwtDoublePoint &pos) {

    if ( StepCuttingPlot == 0) {
        ui->pushButton_cut->setEnabled(false);

        MarkerCut1 = new QwtPlotMarker();
        MarkerCut1->setLabel(QString::fromLatin1("sp1"));
        MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut1->setValue((int)pos.x(),pos.y());
        MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut1->setVisible(true);
        MarkerCut1->attach(plot);
        StepCuttingPlot = 1;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
    }
    else if ( StepCuttingPlot == 1 ) {
        MarkerCut2 = new QwtPlotMarker();
        MarkerCut2->setLabel(QString::fromLatin1("ep1"));
        MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut2->setValue((int)pos.x(),pos.y());
        MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut2->setVisible(true);
        MarkerCut2->attach(plot);
        StepCuttingPlot = 2;
        plot->replot();
        ui->pushButton_cut->setEnabled(true);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("Select one more cutting area?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                StepCuttingPlot = 3;
                QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
            }
            break;
            case QMessageBox::No:
            {
                if ( MarkerCut3 != NULL) {
                    MarkerCut3->setVisible(false);
                    MarkerCut3->detach();
                    MarkerCut3 = NULL;
                }
                if ( MarkerCut4 != NULL) {
                    MarkerCut4->setVisible(false);
                    MarkerCut4->detach();
                    MarkerCut4 = NULL;
                }
                disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
                picker->setEnabled(false);
                picker = NULL;
            }
            break;

            default:
            break;
        }

    }
    else if ( StepCuttingPlot == 3 ) {
        MarkerCut3 = new QwtPlotMarker();
        MarkerCut3->setLabel(QString::fromLatin1("sp2"));
        MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
        MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut3->setValue((int)pos.x(),pos.y());
        MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut3->setVisible(true);
        MarkerCut3->attach(plot);
        StepCuttingPlot = 4;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
    }
    else if ( StepCuttingPlot == 4 ) {
        MarkerCut4 = new QwtPlotMarker();
        MarkerCut4->setLabel(QString::fromLatin1("ep2"));
        MarkerCut4->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut4->setValue((int)pos.x(),pos.y());
        MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut4->setVisible(true);
        MarkerCut4->attach(plot);
        StepCuttingPlot = 5;
        plot->replot();
        ui->pushButton_cut->setEnabled(true);

        disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
        picker->setEnabled(false);
        picker = NULL;

    }
}

void QGCAutoquad::startCutting() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("Delete the selected frames from the file?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            //LogFile
            QString newFileName = LogFile+".orig";
            if (!QFile::exists(newFileName)) {
                QFile::copy(LogFile,newFileName);
            }
            else
            {
                if (QFile::exists(newFileName))
                    QFile::remove(newFileName);
                QFile::copy(LogFile,newFileName);
            }

            if (QFile::exists(LogFile))
                QFile::remove(LogFile);

            QFile file(LogFile);
            file.write("");
            file.close();


            if ( ui->comboBox_marker->currentIndex() == 6) {
                if ( !MarkerCut3 )
                    parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),-1,-1);
                else
                    parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
            } else {
                parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
            }
            removeMarker();
            plot->removeData();
            plot->clear();
            plot->updateScale();
            DecodeLogFile(LogFile);
            ui->pushButton_cut->setEnabled(false);
        }
        break;
        case QMessageBox::No:
            removeMarker();
        break;

        default:
        break;
    }
}

void QGCAutoquad::removeMarker() {
    bool needRedraw = false;
    if ( MarkerCut1 != NULL) {
        MarkerCut1->setVisible(false);
        MarkerCut1->detach();
        MarkerCut1 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut2 != NULL) {
        MarkerCut2->setVisible(false);
        MarkerCut2->detach();
        MarkerCut2 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut3 != NULL) {
        MarkerCut3->setVisible(false);
        MarkerCut3->detach();
        MarkerCut3 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut4 != NULL) {
        MarkerCut4->setVisible(false);
        MarkerCut4->detach();
        MarkerCut4 = NULL;
        needRedraw = true;
    }
    if ( needRedraw)
        plot->replot();

    ui->pushButton_cut->setEnabled(false);
}

void QGCAutoquad::CuttingItemChanged(int itemIndex) {
    Q_UNUSED(itemIndex);
    removeMarker();
}


//
// Miscellaneous
//


void QGCAutoquad::prtstexit(int) {
    if ( active_cal_mode == 0 ) {
        ui->flashButton->setEnabled(true);
        if (connectedLink) {
            connectedLink->connect();
        }
    }
    if ( active_cal_mode == 1 ) {
        ui->pushButton_start_cal1->setEnabled(true);
        ui->pushButton_abort_cal1->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 2 ) {
        ui->pushButton_start_cal2->setEnabled(true);
        ui->pushButton_abort_cal2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 3 ) {
        ui->pushButton_start_cal3->setEnabled(true);
        ui->pushButton_abort_cal3->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 4 ) {
        ui->pushButton_start_sim1->setEnabled(true);
        ui->pushButton_abort_sim1->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 41 ) {
        ui->pushButton_start_sim1_2->setEnabled(true);
        ui->pushButton_abort_sim1_2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 5 ) {
        ui->pushButton_start_sim2->setEnabled(true);
        ui->pushButton_abort_sim2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 6 ) {
        ui->pushButton_start_sim3->setEnabled(true);
        ui->pushButton_abort_sim3->setEnabled(false);
        active_cal_mode = 0;
    }
}

void QGCAutoquad::prtstdout() {
        if ( active_cal_mode == 0 ) {
            output = ps_master.readAllStandardOutput();
            if ( output.contains("[uWrote")) {
                output = output.right(output.length()-3);
                ui->textFlashOutput->clear();
            }
            ui->textFlashOutput->append(output);
        }
        if ( active_cal_mode == 1 ) {
            output_cal1 = ps_master.readAllStandardOutput();
            if ( output_cal1.contains("[H") ) {
                ui->textOutput_cal1->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_cal1->append(output_cal1);
        }
        if ( active_cal_mode == 2 ) {
            output_cal2 = ps_master.readAllStandardOutput();
            if ( output_cal2.contains("[H") ) {
                ui->textOutput_cal2->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_cal2->append(output_cal2);
        }
        if ( active_cal_mode == 3 ) {
            output_cal3 = ps_master.readAllStandardOutput();
            if ( output_cal3.contains("[H") ) {
                ui->textOutput_cal3->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_cal3->append(output_cal3);
        }
        if ( active_cal_mode == 4 ) {
            output_sim1 = ps_master.readAllStandardOutput();
            if ( output_sim1.contains("[H") ) {
                ui->textOutput_sim1->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_sim1->append(output_sim1);
        }
        if ( active_cal_mode == 41 ) {
            output_sim1b = ps_master.readAllStandardOutput();
            if ( output_sim1b.contains("[H") ) {
                ui->textOutput_sim1_2->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_sim1_2->append(output_sim1b);
        }
        if ( active_cal_mode == 5 ) {
            output_sim2 = ps_master.readAllStandardOutput();
            if ( output_sim2.contains("[H") ) {
                ui->textOutput_sim2->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_sim2->append(output_sim2);
        }
        if ( active_cal_mode == 6 ) {
            output_sim3 = ps_master.readAllStandardOutput();
            if ( output_sim3.contains("[H") ) {
                ui->textOutput_sim3->clear();
                output = output.right(output.length()-2);
            }

            ui->textOutput_sim3->append(output_sim3);
        }
}

void QGCAutoquad::prtstderr() {
    if ( active_cal_mode == 0 ) {
        output = ps_master.readAllStandardError();
    ui->textFlashOutput->append(output);
    }
    if ( active_cal_mode == 1 ) {
            output_cal1 = ps_master.readAllStandardError();
            if ( output_cal1.contains("[") )
                    ui->textOutput_cal1->clear();
            ui->textOutput_cal1->append(output_cal1);
    }
    if ( active_cal_mode == 2 ) {
            output_cal2 = ps_master.readAllStandardError();
            if ( output_cal2.contains("[") )
                    ui->textOutput_cal2->clear();
            ui->textOutput_cal2->append(output_cal2);
    }
    if ( active_cal_mode == 3 ) {
            output_cal3 = ps_master.readAllStandardError();
            if ( output_cal3.contains("[") )
                    ui->textOutput_cal3->clear();
            ui->textOutput_cal3->append(output_cal3);
    }
    if ( active_cal_mode == 4 ) {
            output_sim1 = ps_master.readAllStandardError();
            if ( output_sim1.contains("[") )
                    ui->textOutput_sim1->clear();
            ui->textOutput_sim1->append(output_sim1);
    }
    if ( active_cal_mode == 41 ) {
            output_sim1b = ps_master.readAllStandardError();
            if ( output_sim1b.contains("[") )
                    ui->textOutput_sim1_2->clear();
            ui->textOutput_sim1_2->append(output_sim1b);
    }
    if ( active_cal_mode == 5 ) {
            output_sim2 = ps_master.readAllStandardError();
            if ( output_sim2.contains("[") )
                    ui->textOutput_sim2->clear();
            ui->textOutput_sim2->append(output_sim2);
    }
    if ( active_cal_mode == 6 ) {
            output_sim3 = ps_master.readAllStandardError();
            if ( output_sim3.contains("[") )
                    ui->textOutput_sim3->clear();
            ui->textOutput_sim3->append(output_sim3);
    }
}


void QGCAutoquad::globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time){
    Q_UNUSED(time);
    if ( !uas)
        return;
    this->lat = lat;
    this->lon = lon;
    this->alt = alt;
}

void QGCAutoquad::setHardwareInfo(int boardVer) {
    switch (boardVer) {
    case 7:
        maxPwmPorts = 9;
        pwmPortTimers.empty();
        pwmPortTimers << 1 << 1 << 1 << 1 << 4 << 4 << 9 << 9 << 11;
        break;
    case 6:
    default:
        maxPwmPorts = 14;
        pwmPortTimers.empty();
        pwmPortTimers << 1 << 1 << 1 << 1 << 4 << 4 << 4 << 4 << 9 << 9 << 2 << 2 << 10 << 11;
        break;
    }
    emit hardwareInfoUpdated();
}

QStringList QGCAutoquad::getAvailablePwmPorts(void) {
    QStringList portsList;
    unsigned short maxport = maxPwmPorts;

    if (ui->RADIO_TYPE->currentIndex() == 3)
        maxport--;

    for (int i=1; i <= maxport; i++)
        portsList.append(QString::number(i));

    return portsList;
}

void QGCAutoquad::handleStatusText(int uasId, int compid, int severity, QString text) {
    Q_UNUSED(severity);
    Q_UNUSED(compid);
    QRegExp versionRe("^(?:A.*Q.*: )?(\\d+\\.\\d+(?:\\.\\d+)?)([\\s\\-A-Z]*)(?:r(?:ev)?(\\d{1,5}))?(?: b(\\d+))?,?(?: (?:HW ver: (\\d) )?(?:hw)?rev(\\d))?\n?$");
    QString aqFirmwareVersionQualifier;
    bool ok;

    // parse version number
    if (uasId == uas->getUASID() && text.contains(versionRe)) {
        QStringList vlist = versionRe.capturedTexts();
//        qDebug() << vlist.at(1) << vlist.at(2) << vlist.at(3) << vlist.at(4) << vlist.at(5);
        aqFirmwareVersion = vlist.at(1);
        aqFirmwareVersionQualifier = vlist.at(2);
        aqFirmwareVersionQualifier.replace(QString(" "), QString(""));
        if (vlist.at(3).length()) {
            aqFirmwareRevision = vlist.at(3).toInt(&ok);
            if (!ok) aqFirmwareRevision = 0;
        }
        if (vlist.at(4).length()) {
            aqBuildNumber = vlist.at(4).toInt(&ok);
            if (!ok) aqBuildNumber = 0;
        }
        if (vlist.at(5).length()) {
            aqHardwareVersion = vlist.at(5).toInt(&ok);
            if (!ok) aqHardwareRevision = 0;
            else
                setHardwareInfo(aqHardwareVersion);
        }
        if (vlist.at(6).length()) {
            aqHardwareRevision = vlist.at(6).toInt(&ok);
            if (!ok) aqHardwareRevision = -1;
        }

        if (aqFirmwareVersion.length()) {
            QString verStr = QString("AQ FW v. %1%2").arg(aqFirmwareVersion).arg(aqFirmwareVersionQualifier);
            if (aqFirmwareRevision > 0)
                verStr += QString(" r%1").arg(QString::number(aqFirmwareRevision));
            if (aqBuildNumber > 0)
                verStr += QString(" b%1").arg(QString::number(aqBuildNumber));
            verStr += QString(" HW v. %1").arg(QString::number(aqHardwareVersion));
            if (aqHardwareRevision > -1)
                verStr += QString(" r%1").arg(QString::number(aqHardwareRevision));

            ui->lbl_aq_fw_version->setText(verStr);
        } else
            ui->lbl_aq_fw_version->setText("AQ Firmware v. [unknown]");
    }
}

void QGCAutoquad::paramRequestTimeoutNotify(int readCount, int writeCount) {
    MainWindow::instance()->showStatusMessage(tr("PARAMETER READ/WRITE TIMEOUT! Missing: %1 read, %2 write.").arg(readCount).arg(writeCount));
}

void QGCAutoquad::pushButton_dev1(){
//    QString audiostring = QString("Hello, welcome to AutoQuad");
//    GAudioOutput::instance()->say(audiostring.toLower());
    float headingInDegree = ui->lineEdit_13->text().toFloat();
    uas->sendCommmandToAq(7, 1, headingInDegree,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
//    QEventLoop waiter;
//    connect(uas, SIGNAL(textMessageReceived()), &waiter, SLOT(quit()));
//    QTimer::singleShot(5000, &waiter, SLOT(quit()));
//    ui->lineEdit_13->setText("");
//    ui->lineEdit_14->setText("");
//    ui->lineEdit_13->setText(QString::number(aqFirmwareVersion));
//    ui->lineEdit_14->setText(QString::number(aqFirmwareRevision));
//    ui->lineEdit_15->setText(QString::number(aqHardwareRevision));
//    waiter.exec();
}
