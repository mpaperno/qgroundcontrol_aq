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
#include <QSettings>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QSignalMapper>
#include <QSvgGenerator>
#include "GAudioOutput.h"


QGCAutoquad::QGCAutoquad(QWidget *parent) :
    QWidget(parent),
    currLink(NULL),
    plot(new IncrementalPlot()),
    uas(NULL),
    paramaq(NULL),
    esc32(NULL),
    ui(new Ui::QGCAutoquad)
{
    ui->setupUi(this);
    esc32 = NULL;
    model = NULL;
    picker = NULL;
    MarkerCut1  = NULL;
    MarkerCut2  = NULL;
    MarkerCut3  = NULL;
    MarkerCut4  = NULL;
    QHBoxLayout* layout = new QHBoxLayout(ui->plotFrame);
    layout->addWidget(plot);
    ui->plotFrame->setLayout(layout);


    EventComesFromMavlink = false;
    somethingChangedInMotorConfig = 0;

    ui->lbl_version->setText("Version 1.0.6");

	//GUI slots
	connect(ui->SelectFirmwareButton, SIGNAL(clicked()), this, SLOT(selectFWToFlash()));
	connect(ui->portName, SIGNAL(editTextChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->portName, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortName(QString)));

    connect(ui->comboBox_port_esc32, SIGNAL(editTextChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->comboBox_port_esc32, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->pushButton_connect_to_esc32, SIGNAL(clicked()), this, SLOT(btnConnectEsc32()));
    connect(ui->pushButton_read_config, SIGNAL(clicked()),this, SLOT(btnReadConfigEsc32()));
    connect(ui->pushButton_send_to_esc32, SIGNAL(clicked()),this,SLOT(btnSaveToEsc32()));
    connect(ui->pushButton_esc32_read_arm_disarm, SIGNAL(clicked()),this,SLOT(btnArmEsc32()));
    connect(ui->pushButton_esc32_read_start_stop, SIGNAL(clicked()),this,SLOT(btnStartStopEsc32()));
    connect(ui->pushButton_send_rpm, SIGNAL(clicked()),this,SLOT(btnSetRPM()));
    connect(ui->horizontalSlider_rpm, SIGNAL(valueChanged(int)),this,SLOT(Esc32RpmSlider(int)));

    connect(ui->pushButton_start_calibration, SIGNAL(clicked()),this,SLOT(Esc32StartCalibration()));
    connect(ui->pushButton_logging, SIGNAL(clicked()),this,SLOT(Esc32StartLogging()));
    //pushButton_logging
    connect(ui->pushButton_read_load_def, SIGNAL(clicked()),this,SLOT(Esc32ReadConf()));
    connect(ui->pushButton_reload_conf, SIGNAL(clicked()),this,SLOT(Esc32ReLoadConf()));


	connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(flashFW()));
    connect(ui->pushButton_Add_Static, SIGNAL(clicked()),this,SLOT(addStatic()));
    connect(ui->pushButton_Remov_Static, SIGNAL(clicked()),this,SLOT(delStatic()));
    connect(ui->pushButton_Add_Dynamic, SIGNAL(clicked()),this,SLOT(addDynamic()));
    connect(ui->pushButton_Remove_Dynamic, SIGNAL(clicked()),this,SLOT(delDynamic()));
    connect(ui->pushButton_save_to_aq_radio, SIGNAL(clicked()),this,SLOT(setRadio()));
    connect(ui->pushButton_save_to_aq_frame, SIGNAL(clicked()),this,SLOT(setFrame()));
    connect(ui->pushButton_Sel_params_file_user, SIGNAL(clicked()),this,SLOT(setUsersParams()));
    connect(ui->pushButton_Create_params_file_user, SIGNAL(clicked()),this,SLOT(CreateUsersParams()));
    connect(ui->pushButton_save, SIGNAL(clicked()),this,SLOT(WriteUsersParams()));
    connect(ui->pushButton_Load_frame_from_file, SIGNAL(clicked()),this,SLOT(LoadFrameFromFile()));
    connect(ui->pushButton_Save_farem_to_file, SIGNAL(clicked()),this,SLOT(SaveFrameToFile()));
    connect(ui->pushButton_Calculate, SIGNAL(clicked()),this,SLOT(CalculatDeclination()));
    connect(ui->pushButton_Export_Log, SIGNAL(clicked()),this,SLOT(openExportOptionsDlg()));
    connect(ui->pushButton_Open_Log_file, SIGNAL(clicked()),this,SLOT(OpenLogFile()));
    connect(ui->pushButton_set_marker, SIGNAL(clicked()),this,SLOT(startSetMarker()));
    connect(ui->pushButton_cut, SIGNAL(clicked()),this,SLOT(startCutting()));
    connect(ui->pushButton_remove_marker, SIGNAL(clicked()),this,SLOT(removeMarker()));
    //pushButton_remove_marker
    connect(ui->pushButton_save_to_aq_pid1, SIGNAL(clicked()),this,SLOT(save_PID_toAQ1()));
    connect(ui->pushButton_save_to_aq_pid2, SIGNAL(clicked()),this,SLOT(save_PID_toAQ2()));
    connect(ui->pushButton_save_to_aq_pid3, SIGNAL(clicked()),this,SLOT(save_PID_toAQ3()));
    connect(ui->pushButton_save_to_aq_pid4, SIGNAL(clicked()),this,SLOT(save_PID_toAQ4()));
    connect(ui->pushButton_save_image_plot, SIGNAL(clicked()),this,SLOT(save_plot_image()));
    connect(ui->pushButtonshow_cahnnels, SIGNAL(clicked()),this,SLOT(showChannels()));

    connect(ui->comboBox_marker, SIGNAL(currentIndexChanged(int)),this,SLOT(CuttingItemChanged(int)));

    port_nr_roll = 0;
    port_nr_pitch = 0;
    connect(ui->pushButton_start_cal1, SIGNAL(clicked()),this,SLOT(startcal1()));
    connect(ui->pushButton_start_cal2, SIGNAL(clicked()),this,SLOT(startcal2()));
    connect(ui->pushButton_start_cal3, SIGNAL(clicked()),this,SLOT(startcal3()));
    connect(ui->pushButton_start_sim1, SIGNAL(clicked()),this,SLOT(startsim1()));
    connect(ui->pushButton_start_sim1_2, SIGNAL(clicked()),this,SLOT(startsim1b()));
    connect(ui->pushButton_start_sim2, SIGNAL(clicked()),this,SLOT(startsim2()));
    connect(ui->pushButton_start_sim3, SIGNAL(clicked()),this,SLOT(startsim3()));
    connect(ui->pushButton_abort_cal1, SIGNAL(clicked()),this,SLOT(abortcal1()));
    connect(ui->pushButton_abort_cal2, SIGNAL(clicked()),this,SLOT(abortcal2()));
    connect(ui->pushButton_abort_cal3, SIGNAL(clicked()),this,SLOT(abortcal3()));
    connect(ui->pushButton_abort_sim1, SIGNAL(clicked()),this,SLOT(abortsim1()));
    connect(ui->pushButton_abort_sim1_2, SIGNAL(clicked()),this,SLOT(abortsim1b()));
    connect(ui->pushButton_abort_sim2, SIGNAL(clicked()),this,SLOT(abortsim2()));
    connect(ui->pushButton_abort_sim3, SIGNAL(clicked()),this,SLOT(abortsim3()));
    connect(ui->checkBox_sim3_4_var_1, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_4_stop_1, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_4_var_2, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_4_stop_2, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_5_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_5_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_6_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_6_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_raw_value, SIGNAL(clicked()),this,SLOT(raw_transmitter_view()));

    connect(ui->checkBox_isPitchM1, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P1(bool)));
    connect(ui->checkBox_isRollM1, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P1(bool)));

    connect(ui->checkBox_isPitchM2, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P2(bool)));
    connect(ui->checkBox_isRollM2, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P2(bool)));

    connect(ui->checkBox_isPitchM3, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P3(bool)));
    connect(ui->checkBox_isRollM3, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P3(bool)));

    connect(ui->checkBox_isPitchM4, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P4(bool)));
    connect(ui->checkBox_isRollM4, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P4(bool)));

    connect(ui->checkBox_isPitchM5, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P5(bool)));
    connect(ui->checkBox_isRollM5, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P5(bool)));

    connect(ui->checkBox_isPitchM6, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P6(bool)));
    connect(ui->checkBox_isRollM6, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P6(bool)));

    connect(ui->checkBox_isPitchM7, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P7(bool)));
    connect(ui->checkBox_isRollM7, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P7(bool)));

    connect(ui->checkBox_isPitchM8, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P8(bool)));
    connect(ui->checkBox_isRollM8, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P8(bool)));

    connect(ui->checkBox_isPitchM9, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P9(bool)));
    connect(ui->checkBox_isRollM9, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P9(bool)));

    connect(ui->checkBox_isPitchM10, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P10(bool)));
    connect(ui->checkBox_isRollM10, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P10(bool)));

    connect(ui->checkBox_isPitchM11, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P11(bool)));
    connect(ui->checkBox_isRollM11, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P11(bool)));

    connect(ui->checkBox_isPitchM12, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P12(bool)));
    connect(ui->checkBox_isRollM12, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P12(bool)));

    connect(ui->checkBox_isPitchM13, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P13(bool)));
    connect(ui->checkBox_isRollM13, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P13(bool)));

    connect(ui->checkBox_isPitchM14, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P14(bool)));
    connect(ui->checkBox_isRollM14, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P14(bool)));

    connect(ui->pushButton_start_tel_grid, SIGNAL(clicked()),this, SLOT(teleValuesStart()));
    connect(ui->pushButton_stop_tel_grid, SIGNAL(clicked()),this, SLOT(teleValuesStop()));

    ui->CMB_SPVR_FS_RAD_ST1->addItem("Position Hold", 0);

    ui->CMB_SPVR_FS_RAD_ST2->addItem("slow decent", 0);
    ui->CMB_SPVR_FS_RAD_ST2->addItem("RTH and slow decent", 1);

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

    VisibleWidget = 0;
    ui->comboBox_Radio_Type->addItem("Spektrum 11Bit", 0);
    ui->comboBox_Radio_Type->addItem("Spektrum 10Bit", 1);
    ui->comboBox_Radio_Type->addItem("Futaba", 2);
    ui->comboBox_Radio_Type->addItem("PPM", 3);

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

    setupPortList();
    loadSettings();

    ui->Frequenz_Telemetry->addItem("1 Hz", 1000000);
    ui->Frequenz_Telemetry->addItem("10 Hz", 100000);
    ui->Frequenz_Telemetry->addItem("25 Hz", 50000);
    ui->Frequenz_Telemetry->addItem("50 Hz", 20000);
    ui->Frequenz_Telemetry->setCurrentIndex(2);
}

QGCAutoquad::~QGCAutoquad()
{
    if ( esc32)
        btnConnectEsc32();
    writeSettings();
    delete ui;
}

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
//        DefaultColorMeasureChannels = item->background().color();
        model->appendRow(item);
    }
    ui->listView_Curves->setModel(model);
    connect(model, SIGNAL(itemChanged(QStandardItem*)), this,SLOT(CurveItemChanged(QStandardItem*)));
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

//    if (!LogFile.length()){
//        OpenLogFile(false);
//    }

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

void QGCAutoquad::CurveItemChanged(QStandardItem *item)
{
    if ( item->checkState() )
    {
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

void QGCAutoquad::DecodeLogFile(QString fileName)
{
    plot->removeData();
    plot->clear();
    plot->setStyleText("lines");
    if ( model)
        disconnect(model, SIGNAL(itemChanged(QStandardItem*)), this,SLOT(CurveItemChanged(QStandardItem*)));
    ui->listView_Curves->reset();
    if ( parser.ParseLogHeader(fileName) == 0)
        SetupListView();

}

void QGCAutoquad::showChannels() {

    parser.ShowCurves();
    plot->removeData();
    plot->clear();
    if (!QFile::exists(LogFile))
        return;

    for (int i = 0; i < parser.yValues.count(); i++) {
        plot->appendData(parser.yValues.keys().at(i), parser.xValues.values().at(0)->data(), parser.yValues.values().at(i)->data(), parser.xValues.values().at(0)->count());

    }

    plot->setStyleText("lines");
    plot->updateScale();
    for ( int i = 0; i < model->rowCount(); i++) {
        bool isChecked = model->item(i,0)->checkState();
        QStandardItem *item = model->item(i,0);
        if ( isChecked ) {
            if ( item) {
                //item->setForeground(plot->getColorForCurve(item->text()));
                item->setBackground(plot->getColorForCurve(item->text()));
            }
        }
        else {
            item->setBackground(DefaultColorMeasureChannels);
        }
    }

}

void QGCAutoquad::loadSettings()
{
    // Load defaults from settings
    QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

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

    if (settings.contains("AUTOQUAD_FW_FILE"))
        ui->fileLabel->setText(settings.value("AUTOQUAD_FW_FILE").toString());
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
    settings.endGroup();
    settings.sync();
}

void QGCAutoquad::writeSettings()
{
    QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

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

    settings.sync();
    settings.endGroup();
}

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

void QGCAutoquad::setPortName(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
	portName = port;
	ui->ComPortLabel->setText(portName);
}

void QGCAutoquad::setPortNameEsc32(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
    portNameEsc32 = port;
    ui->label_portName_esc32->setText(portNameEsc32);
}

void QGCAutoquad::btnConnectEsc32()
{
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

        /*
        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            if (!ParaName.contains("DoubleMaxCurrent"))
                //edtList.at(i)->setText("");
            else
                ParaName = ParaName;
        }
        */

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
        esc32->Disconnect();
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



    //comboBox_mode
    //comboBox_in_mode

    if (( oneWritten ) && (!something_gos_wrong))
        saveEEpromEsc32();
    else if (something_gos_wrong) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setInformativeText("Something goes wrong with storing the values. Please retry!");
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
        ui->label_rpm->setText(QString::number(ui->horizontalSlider_rpm->value()));
        esc32->sendCommand(BINARY_COMMAND_RPM,rpm, 0.0f, 1, false);
    }
}

void QGCAutoquad::Esc32RpmSlider(int rpm) {
    ui->label_rpm->setText(QString::number(rpm));
}

void QGCAutoquad::saveEEpromEsc32()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values are transmitted to Esc32! Do you want to store the para into ROM?");
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
    esc32->ReadConfigEsc32();
}

void QGCAutoquad::ESc32Disconnected() {
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

void QGCAutoquad::Esc32StartLogging() {
    esc32->StartLogging();
}

void QGCAutoquad::Esc32StartCalibration() {
    if (!esc32)
        return;

    QString Esc32LoggingFile = "";

    if ( ui->pushButton_start_calibration->text() == "start calibration") {
        QMessageBox InfomsgBox;
        InfomsgBox.setText("This is the calibration routine for esc32!\r\n Please be careful with the calibration function!\r\n The motor turn up to full throttle!\r\n Please fix the motor & prop!\r\n No guarantee about any damage or failures");
        InfomsgBox.exec();

        int ret = QMessageBox::question(this,"Question","Which calibration do you want to do?","RpmToVoltage","CurrentLimiter");
        if ( ret == 0) {
            Esc32CalibrationMode = 1;
            #ifdef Q_OS_WIN
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "RPMTOVOLTAGE.txt");
            #else
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "RPMTOVOLTAGE.txt");
            #endif
        }
        else if ( ret == 1) {
            Esc32CalibrationMode = 2;
            #ifdef Q_OS_WIN
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "CURRENTLIMITER.txt");
            #else
                Esc32LoggingFile = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "CURRENTLIMITER.txt");
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

        QMessageBox msgBox;
        msgBox.setWindowTitle("Information");
        msgBox.setInformativeText("Again be carful, you can abort with stop calibration!\r\n But the fastest stop is to pull the batery!\r\n If you want start the calibration procedure, press yes");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        ret = msgBox.exec();
        if ( ret == QMessageBox::Yes) {
            float maxAmps = ui->DoubleMaxCurrent->text().toFloat();

            esc32->SetCalibrationMode(this->Esc32CalibrationMode);
            esc32->StartCalibration(maxAmps,Esc32LoggingFile);
            ui->pushButton_start_calibration->setText("stop calibration");
        }
        else {
            return;
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
        QFile file(fileNameLocale );
        ui->fileLabel->setText(fileNameLocale );
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
	QString AppPath = "";
	#ifdef Q_OS_WIN
		AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "stm32flash.exe");
        #else
         AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "stm32flash");
	#endif


     QMessageBox msgBox;
     msgBox.setText("Flashing firmware, takes some seconds. Please wait 20sec. bevor you retry!");
     msgBox.exec();


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

void QGCAutoquad::prtstexit(int) {
    if ( active_cal_mode == 0 ) {
        ui->flashButton->setEnabled(true);
        active_cal_mode = 0;
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
            if ( output_sim2.contains("[H") ) {
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

void QGCAutoquad::handleConnectButton()
{
    if (currLink) {
        if (currLink->isConnected()) {
            currLink->disconnect();
        } else {
            currLink->connect();
        }
    }
}

UASInterface* QGCAutoquad::getUAS()
{
    return uas;
}

void QGCAutoquad::addUAS(UASInterface* uas_ext)
{
    QString uasColor = uas_ext->getColor().name().remove(0, 1);

}

void QGCAutoquad::setActiveUAS(UASInterface* uas_ext)
{
    if (uas_ext)
    {
        uas = uas_ext;
        disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
        disconnect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(getGUIpara()));
        if ( !paramaq ) {
            paramaq = new QGCAQParamWidget(uas, this);
            ui->gridLayout_paramAQ->addWidget(paramaq);
            connect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(getGUIpara()));
            if ( LastFilePath == "")
                paramaq->setFilePath(QCoreApplication::applicationDirPath());
            else
                paramaq->setFilePath(LastFilePath);
        }
        paramaq->loadParaAQ();
        //getGUIpara();

        VisibleWidget = 2;
        if ( !AqTeleChart ) {
            AqTeleChart = new AQLinechartWidget(uas->getUASID(), this->ui->plotFrameTele);
            linLayoutPlot = new QGridLayout( this->ui->plotFrameTele);
            linLayoutPlot->addWidget(AqTeleChart,0,Qt::AlignCenter);
        }
        ui->checkBox_raw_value->setChecked(true);
        raw_transmitter_view();
    }
}

void QGCAutoquad::raw_transmitter_view() {
    if ( ui->checkBox_raw_value->checkState() ){

         disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));

         ui->progressBar_Throttle->setMaximum(+1500);
         ui->progressBar_Throttle->setMinimum(-100);

         ui->progressBar_Roll->setMaximum(1024);
         ui->progressBar_Roll->setMinimum(-1024);

         ui->progressBar_Pitch->setMaximum(1024);
         ui->progressBar_Pitch->setMinimum(-1024);

         ui->progressBar_Rudd->setMaximum(1024);
         ui->progressBar_Rudd->setMinimum(-1024);

         ui->progressBar_Gear->setMaximum(1024);
         ui->progressBar_Gear->setMinimum(-1024);

         ui->progressBar_Flaps->setMaximum(1024);
         ui->progressBar_Flaps->setMinimum(-1024);

         ui->progressBar_Aux2->setMaximum(1024);
         ui->progressBar_Aux2->setMinimum(-1024);

         ui->progressBar_Aux3->setMaximum(1024);
         ui->progressBar_Aux3->setMinimum(-1024);

         connect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));

    }
    else
    {
        disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));

        ui->progressBar_Throttle->setMaximum(-500);
        ui->progressBar_Throttle->setMinimum(1500);

        ui->progressBar_Roll->setMaximum(-1500);
        ui->progressBar_Roll->setMinimum(1500);

        ui->progressBar_Pitch->setMaximum(-1500);
        ui->progressBar_Pitch->setMinimum(1500);

        ui->progressBar_Rudd->setMaximum(-1500);
        ui->progressBar_Rudd->setMinimum(1500);

        ui->progressBar_Gear->setMaximum(-1500);
        ui->progressBar_Gear->setMinimum(1500);

        ui->progressBar_Flaps->setMaximum(-1500);
        ui->progressBar_Flaps->setMinimum(1500);

        ui->progressBar_Aux2->setMaximum(-1500);
        ui->progressBar_Aux2->setMinimum(1500);

        ui->progressBar_Aux3->setMaximum(-1500);
        ui->progressBar_Aux3->setMinimum(1500);

        connect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
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
            paramaq = new QGCAQParamWidget(uas, this);
            ui->gridLayout_paramAQ->addWidget(paramaq);
            VisibleWidget = 2;
        }
    }
    QWidget::showEvent(event);
    emit visibilityChanged(true);
}

void QGCAutoquad::setChannelScaled(int channelId, float normalized)
{
    if (channelId == 0 )
    {
        qint32 val = (qint32)((normalized*10000.0f)/13)+750;
        ui->progressBar_Throttle->setValue(val);
        ui->label_chan_1_M->setText(QString::number(val));
    }
    if (channelId == 1 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Roll->setValue(val);
        ui->label_chan_2_M->setText(QString::number(val));
    }
    if (channelId == 2 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Pitch->setValue(val);
        ui->label_chan_3_M->setText(QString::number(val));
    }
    if (channelId == 3 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Rudd->setValue(val);
        ui->label_chan_4_M->setText(QString::number(val));
    }
    if (channelId == 4 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Gear->setValue(val);
        ui->label_chan_5_M->setText(QString::number(val));
    }
    if (channelId == 5 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Flaps->setValue(val);
        ui->label_chan_6_M->setText(QString::number(val) + " " + "Pos Hold");
    }
    if (channelId == 6 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux2->setValue(val);
        ui->label_chan_7_M->setText(QString::number(val));
    }
    if (channelId == 7 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux3->setValue(val);
        ui->label_chan_8_M->setText(QString::number(val));
    }

}

void QGCAutoquad::setChannelRaw(int channelId, float normalized)
{
    if (channelId == 0 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Throttle->maximum()) || ( val > ui->progressBar_Throttle->minimum()))
            ui->progressBar_Throttle->setValue(val);
        ui->label_chan_1_M->setText(QString::number(val));
    }
    if (channelId == 1 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Roll->maximum()) || ( val > ui->progressBar_Roll->minimum()))
            ui->progressBar_Roll->setValue(val);
        ui->label_chan_2_M->setText(QString::number(val));
    }
    if (channelId == 2 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Pitch->maximum()) || ( val > ui->progressBar_Pitch->minimum()))
            ui->progressBar_Pitch->setValue(val);
        ui->label_chan_3_M->setText(QString::number(val));
    }
    if (channelId == 3 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Rudd->maximum()) || ( val > ui->progressBar_Rudd->minimum()))
            ui->progressBar_Rudd->setValue(val);
        ui->label_chan_4_M->setText(QString::number(val));
    }
    if (channelId == 4 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Gear->maximum()) || ( val > ui->progressBar_Gear->minimum()))
            ui->progressBar_Gear->setValue(val);
        ui->label_chan_5_M->setText(QString::number(val));
    }
    if (channelId == 5 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Flaps->maximum()) || ( val > ui->progressBar_Flaps->minimum()))
            ui->progressBar_Flaps->setValue(val);
        ui->label_chan_6_M->setText(QString::number(val) + " " + "Pos Hold");
    }
    if (channelId == 6 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Aux2->maximum()) || ( val > ui->progressBar_Aux2->minimum()))
            ui->progressBar_Aux2->setValue(val);
        ui->label_chan_7_M->setText(QString::number(val));
    }
    if (channelId == 7 )
    {
        int val = (int)((normalized-1024));
        if (( val < ui->progressBar_Aux3->maximum()) || ( val > ui->progressBar_Aux3->minimum()))
            ui->progressBar_Aux3->setValue(val);
        ui->label_chan_8_M->setText(QString::number(val));
    }

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
    QString AppPath = "";
    #ifdef Q_OS_WIN
        AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
        ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    #else
        AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
        ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
    #endif

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
    QString AppPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
#endif

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
    QString AppPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
#endif

    QStringList Arguments;

    Arguments.append("--mag");

    for ( int i = 0; i<StaticFiles.count(); i++) {
        Arguments.append(StaticFiles.at(i));
    }
    Arguments.append(":");

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
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

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
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

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
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

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
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

    QStringList Arguments;

    Arguments.append("--mag");
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

void QGCAutoquad::abortcal1(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortcal2(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortcal3(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim1(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim1b(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim2(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim3(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::getGUIpara() {
	if ( !paramaq)
		return;

	QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( this );
	for ( int i = 0; i<edtList.count(); i++) {
		QString ParaName = edtList.at(i)->objectName();
		if ( ParaName.startsWith("CTRL",Qt::CaseSensitive) || ParaName.startsWith("NAV",Qt::CaseSensitive) || ParaName.startsWith("GMBL",Qt::CaseSensitive) || ParaName.startsWith("SPVR",Qt::CaseSensitive) ) {
            if ( ParaName == "GMBL_SCAL_PITCH") {
                float dummy_value = fabs(paramaq->getParaAQ(ParaName).toFloat());
                edtList.at(i)->setText(QString::number(dummy_value));
            }
            else if ( ParaName == "GMBL_SCAL_ROLL"){
                float dummy_value = fabs(paramaq->getParaAQ(ParaName).toFloat());
                edtList.at(i)->setText(QString::number(dummy_value));
            }
            else {
                edtList.at(i)->setText(paramaq->getParaAQ(ParaName).toString());
            }
		}
		else if ( ParaName.startsWith("MOT_",Qt::CaseSensitive)) {
			edtList.at(i)->setText(paramaq->getParaAQ(ParaName).toString());
		}
	}

    int radio_type = paramaq->getParaAQ("RADIO_TYPE").toInt();
    ui->comboBox_Radio_Type->setCurrentIndex(radio_type);
    ui->IMU_ROT->setText(paramaq->getParaAQ("IMU_ROT").toString());
    ui->IMU_MAG_DECL->setText(paramaq->getParaAQ("IMU_MAG_DECL").toString());
    ui->IMU_MAG_INCL->setText(paramaq->getParaAQ("IMU_MAG_INCL").toString());
    ui->IMU_PRESS_SENSE->setText(paramaq->getParaAQ("IMU_PRESS_SENSE").toString());
    ui->DOWNLINK_BAUD->setText(paramaq->getParaAQ("DOWNLINK_BAUD").toString());


    EventComesFromMavlink = true;


    disconnect(ui->checkBox_isPitchM1, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P1(bool)));
    disconnect(ui->checkBox_isRollM1, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P1(bool)));
    disconnect(ui->checkBox_isPitchM2, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P2(bool)));
    disconnect(ui->checkBox_isRollM2, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P2(bool)));
    disconnect(ui->checkBox_isPitchM3, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P3(bool)));
    disconnect(ui->checkBox_isRollM3, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P3(bool)));
    disconnect(ui->checkBox_isPitchM4, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P4(bool)));
    disconnect(ui->checkBox_isRollM4, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P4(bool)));
    disconnect(ui->checkBox_isPitchM5, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P5(bool)));
    disconnect(ui->checkBox_isRollM5, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P5(bool)));
    disconnect(ui->checkBox_isPitchM6, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P6(bool)));
    disconnect(ui->checkBox_isRollM6, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P6(bool)));
    disconnect(ui->checkBox_isPitchM7, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P7(bool)));
    disconnect(ui->checkBox_isRollM7, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P7(bool)));
    disconnect(ui->checkBox_isPitchM8, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P8(bool)));
    disconnect(ui->checkBox_isRollM8, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P8(bool)));
    disconnect(ui->checkBox_isPitchM9, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P9(bool)));
    disconnect(ui->checkBox_isRollM9, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P9(bool)));
    disconnect(ui->checkBox_isPitchM10, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P10(bool)));
    disconnect(ui->checkBox_isRollM10, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P10(bool)));
    disconnect(ui->checkBox_isPitchM11, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P11(bool)));
    disconnect(ui->checkBox_isRollM11, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P11(bool)));
    disconnect(ui->checkBox_isPitchM12, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P12(bool)));
    disconnect(ui->checkBox_isRollM12, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P12(bool)));
    disconnect(ui->checkBox_isPitchM13, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P13(bool)));
    disconnect(ui->checkBox_isRollM13, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P13(bool)));
    disconnect(ui->checkBox_isPitchM14, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P14(bool)));
    disconnect(ui->checkBox_isRollM14, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P14(bool)));

    gmb_pitch_P1(false);
    gmb_pitch_P2(false);
    gmb_pitch_P3(false);
    gmb_pitch_P4(false);
    gmb_pitch_P5(false);
    gmb_pitch_P6(false);
    gmb_pitch_P7(false);
    gmb_pitch_P8(false);
    gmb_pitch_P9(false);
    gmb_pitch_P10(false);
    gmb_pitch_P11(false);
    gmb_pitch_P12(false);
    gmb_pitch_P13(false);
    gmb_pitch_P14(false);
    ui->checkBox_isPitchM1->setChecked(false);
    ui->checkBox_isPitchM2->setChecked(false);
    ui->checkBox_isPitchM3->setChecked(false);
    ui->checkBox_isPitchM4->setChecked(false);
    ui->checkBox_isPitchM5->setChecked(false);
    ui->checkBox_isPitchM6->setChecked(false);
    ui->checkBox_isPitchM7->setChecked(false);
    ui->checkBox_isPitchM8->setChecked(false);
    ui->checkBox_isPitchM9->setChecked(false);
    ui->checkBox_isPitchM10->setChecked(false);
    ui->checkBox_isPitchM11->setChecked(false);
    ui->checkBox_isPitchM12->setChecked(false);
    ui->checkBox_isPitchM13->setChecked(false);
    ui->checkBox_isPitchM14->setChecked(false);
    ui->checkBox_isRollM1->setChecked(false);
    ui->checkBox_isRollM2->setChecked(false);
    ui->checkBox_isRollM3->setChecked(false);
    ui->checkBox_isRollM4->setChecked(false);
    ui->checkBox_isRollM5->setChecked(false);
    ui->checkBox_isRollM6->setChecked(false);
    ui->checkBox_isRollM7->setChecked(false);
    ui->checkBox_isRollM8->setChecked(false);
    ui->checkBox_isRollM9->setChecked(false);
    ui->checkBox_isRollM10->setChecked(false);
    ui->checkBox_isRollM11->setChecked(false);
    ui->checkBox_isRollM12->setChecked(false);
    ui->checkBox_isRollM13->setChecked(false);
    ui->checkBox_isRollM14->setChecked(false);
    gmb_roll_P1(false);
    gmb_roll_P2(false);
    gmb_roll_P3(false);
    gmb_roll_P4(false);
    gmb_roll_P5(false);
    gmb_roll_P6(false);
    gmb_roll_P7(false);
    gmb_roll_P8(false);
    gmb_roll_P9(false);
    gmb_roll_P10(false);
    gmb_roll_P11(false);
    gmb_roll_P12(false);
    gmb_roll_P13(false);
    gmb_roll_P14(false);


    float f_pitch_direction = paramaq->getParaAQ("GMBL_SCAL_PITCH").toFloat();
    float f_roll_direction = paramaq->getParaAQ("GMBL_SCAL_ROLL").toFloat();

    port_nr_pitch = abs(paramaq->getParaAQ("GMBL_PITCH_PORT").toInt());
    port_nr_roll = abs(paramaq->getParaAQ("GMBL_ROLL_PORT").toInt());
    if ( f_pitch_direction < 0 ) {
        ui->reverse_gimbal_pitch->setChecked(true);
    }
    else {
        ui->reverse_gimbal_pitch->setChecked(false);
    }

    if ( f_roll_direction < 0 ) {
        ui->reverse_gimbal_roll->setChecked(true);
    }
    else {
        ui->reverse_gimbal_roll->setChecked(false);
    }

    if (port_nr_pitch  > 0 ) {
        switch (port_nr_pitch) {
            case 0:
                break;
            case 1:
                ui->checkBox_isPitchM1->setChecked(true);
                ui->checkBox_isRollM1->setChecked(false);
                break;
            case 2:
                ui->checkBox_isPitchM2->setChecked(true);
                ui->checkBox_isRollM2->setChecked(false);
                break;
            case 3:
                ui->checkBox_isPitchM3->setChecked(true);
                ui->checkBox_isRollM3->setChecked(false);
                break;
            case 4:
                ui->checkBox_isPitchM4->setChecked(true);
                ui->checkBox_isRollM4->setChecked(false);
                break;
            case 5:
                ui->checkBox_isPitchM5->setChecked(true);
                ui->checkBox_isRollM5->setChecked(false);
                break;
            case 6:
                ui->checkBox_isPitchM6->setChecked(true);
                ui->checkBox_isRollM6->setChecked(false);
                break;
            case 7:
                ui->checkBox_isPitchM7->setChecked(true);
                ui->checkBox_isRollM7->setChecked(false);
                break;
            case 8:
                ui->checkBox_isPitchM8->setChecked(true);
                ui->checkBox_isRollM8->setChecked(false);
                break;
            case 9:
                ui->checkBox_isPitchM9->setChecked(true);
                ui->checkBox_isRollM9->setChecked(false);
                break;
            case 10:
                ui->checkBox_isPitchM10->setChecked(true);
                ui->checkBox_isRollM10->setChecked(false);
                break;
            case 11:
                ui->checkBox_isPitchM11->setChecked(true);
                ui->checkBox_isRollM11->setChecked(false);
                break;
            case 12:
                ui->checkBox_isPitchM12->setChecked(true);
                ui->checkBox_isRollM12->setChecked(false);
                break;
            case 13:
                ui->checkBox_isPitchM13->setChecked(true);
                ui->checkBox_isRollM13->setChecked(false);
                break;
            case 14:
                ui->checkBox_isPitchM14->setChecked(true);
                ui->checkBox_isRollM14->setChecked(false);
                break;
        }
    }

    if (port_nr_roll > 0 ) {
        switch (port_nr_roll ) {
            case 0:
                break;
            case 1:
                ui->checkBox_isPitchM1->setChecked(false);
                ui->checkBox_isRollM1->setChecked(true);
                break;
            case 2:
                ui->checkBox_isPitchM2->setChecked(false);
                ui->checkBox_isRollM2->setChecked(true);
                break;
            case 3:
                ui->checkBox_isPitchM3->setChecked(false);
                ui->checkBox_isRollM3->setChecked(true);
                break;
            case 4:
                ui->checkBox_isPitchM4->setChecked(false);
                ui->checkBox_isRollM4->setChecked(true);
                break;
            case 5:
                ui->checkBox_isPitchM5->setChecked(false);
                ui->checkBox_isRollM5->setChecked(true);
                break;
            case 6:
                ui->checkBox_isPitchM6->setChecked(false);
                ui->checkBox_isRollM6->setChecked(true);
                break;
            case 7:
                ui->checkBox_isPitchM7->setChecked(false);
                ui->checkBox_isRollM7->setChecked(true);
                break;
            case 8:
                ui->checkBox_isPitchM8->setChecked(false);
                ui->checkBox_isRollM8->setChecked(true);
                break;
            case 9:
                ui->checkBox_isPitchM9->setChecked(false);
                ui->checkBox_isRollM9->setChecked(true);
                break;
            case 10:
                ui->checkBox_isPitchM10->setChecked(false);
                ui->checkBox_isRollM10->setChecked(true);
                break;
            case 11:
                ui->checkBox_isPitchM11->setChecked(false);
                ui->checkBox_isRollM11->setChecked(true);
                break;
            case 12:
                ui->checkBox_isPitchM12->setChecked(false);
                ui->checkBox_isRollM12->setChecked(true);
                break;
            case 13:
                ui->checkBox_isPitchM13->setChecked(false);
                ui->checkBox_isRollM13->setChecked(true);
                break;
            case 14:
                ui->checkBox_isPitchM14->setChecked(false);
                ui->checkBox_isRollM14->setChecked(true);
                break;
        }
    }
    connect(ui->checkBox_isPitchM1, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P1(bool)));
    connect(ui->checkBox_isRollM1, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P1(bool)));
    connect(ui->checkBox_isPitchM2, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P2(bool)));
    connect(ui->checkBox_isRollM2, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P2(bool)));
    connect(ui->checkBox_isPitchM3, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P3(bool)));
    connect(ui->checkBox_isRollM3, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P3(bool)));
    connect(ui->checkBox_isPitchM4, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P4(bool)));
    connect(ui->checkBox_isRollM4, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P4(bool)));
    connect(ui->checkBox_isPitchM5, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P5(bool)));
    connect(ui->checkBox_isRollM5, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P5(bool)));
    connect(ui->checkBox_isPitchM6, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P6(bool)));
    connect(ui->checkBox_isRollM6, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P6(bool)));
    connect(ui->checkBox_isPitchM7, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P7(bool)));
    connect(ui->checkBox_isRollM7, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P7(bool)));
    connect(ui->checkBox_isPitchM8, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P8(bool)));
    connect(ui->checkBox_isRollM8, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P8(bool)));
    connect(ui->checkBox_isPitchM9, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P9(bool)));
    connect(ui->checkBox_isRollM9, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P9(bool)));
    connect(ui->checkBox_isPitchM10, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P10(bool)));
    connect(ui->checkBox_isRollM10, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P10(bool)));
    connect(ui->checkBox_isPitchM11, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P11(bool)));
    connect(ui->checkBox_isRollM11, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P11(bool)));
    connect(ui->checkBox_isPitchM12, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P12(bool)));
    connect(ui->checkBox_isRollM12, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P12(bool)));
    connect(ui->checkBox_isPitchM13, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P13(bool)));
    connect(ui->checkBox_isRollM13, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P13(bool)));
    connect(ui->checkBox_isPitchM14, SIGNAL(clicked(bool)),this, SLOT(gmb_pitch_P14(bool)));
    connect(ui->checkBox_isRollM14, SIGNAL(clicked(bool)),this, SLOT(gmb_roll_P14(bool)));

    int failsaveStage1 = paramaq->getParaAQ("SPVR_FS_RAD_ST1").toInt();
    int failsaveStage2 = paramaq->getParaAQ("SPVR_FS_RAD_ST2").toInt();

    ui->CMB_SPVR_FS_RAD_ST1->setCurrentIndex(failsaveStage1);
    ui->CMB_SPVR_FS_RAD_ST2->setCurrentIndex(failsaveStage2);

    EventComesFromMavlink = false;
}

void QGCAutoquad::gmb_pitch_P1(bool value){
    if ( value )
        port_nr_pitch = 1;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM1->setEnabled(!value);
    CheckGimbal(1,value);
    DisableEnableAllPitchGimbal(1,value);
}
void QGCAutoquad::gmb_roll_P1(bool value){
    if ( value )
        port_nr_roll = 1;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM1->setEnabled(!value);
    CheckGimbal(1,value);
    DisableEnableAllRollGimbal(1,value);
}
void QGCAutoquad::gmb_pitch_P2(bool value){
    if ( value )
        port_nr_pitch = 2;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM2->setEnabled(!value);
    CheckGimbal(2,value);
    DisableEnableAllPitchGimbal(2,value);
}
void QGCAutoquad::gmb_roll_P2(bool value){
    if ( value )
        port_nr_roll = 2;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM2->setEnabled(!value);
    CheckGimbal(2,value);
    DisableEnableAllRollGimbal(2,value);
}
void QGCAutoquad::gmb_pitch_P3(bool value){
    if ( value )
        port_nr_pitch = 3;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM3->setEnabled(!value);
    CheckGimbal(3,value);
    DisableEnableAllPitchGimbal(3,value);
}
void QGCAutoquad::gmb_roll_P3(bool value){
    if ( value )
        port_nr_roll = 3;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM3->setEnabled(!value);
    CheckGimbal(3,value);
    DisableEnableAllRollGimbal(3,value);
}
void QGCAutoquad::gmb_pitch_P4(bool value){
    if ( value )
        port_nr_pitch = 4;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM4->setEnabled(!value);
    CheckGimbal(4,value);
    DisableEnableAllPitchGimbal(4,value);
}
void QGCAutoquad::gmb_roll_P4(bool value){
    if ( value )
        port_nr_roll = 4;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM4->setEnabled(!value);
    DisableEnableAllRollGimbal(4,value);
    CheckGimbal(4,value);
}
void QGCAutoquad::gmb_pitch_P5(bool value){
    if ( value )
        port_nr_pitch = 5;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM5->setEnabled(!value);
    CheckGimbal(5,value);
    DisableEnableAllPitchGimbal(5,value);
}
void QGCAutoquad::gmb_roll_P5(bool value){
    if ( value )
        port_nr_roll = 5;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM5->setEnabled(!value);
    CheckGimbal(5,value);
    DisableEnableAllRollGimbal(5,value);
}
void QGCAutoquad::gmb_pitch_P6(bool value){
    if ( value )
        port_nr_pitch = 6;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM6->setEnabled(!value);
    CheckGimbal(6,value);
    DisableEnableAllPitchGimbal(6,value);
}
void QGCAutoquad::gmb_roll_P6(bool value){
    if ( value )
        port_nr_roll = 6;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM6->setEnabled(!value);
    CheckGimbal(6,value);
    DisableEnableAllRollGimbal(6,value);
}
void QGCAutoquad::gmb_pitch_P7(bool value){
    if ( value )
        port_nr_pitch = 7;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM7->setEnabled(!value);
    CheckGimbal(7,value);
    DisableEnableAllPitchGimbal(7,value);
}
void QGCAutoquad::gmb_roll_P7(bool value){
    if ( value )
        port_nr_roll = 7;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM7->setEnabled(!value);
    CheckGimbal(7,value);
    DisableEnableAllRollGimbal(7,value);
}
void QGCAutoquad::gmb_pitch_P8(bool value){
    if ( value )
        port_nr_pitch = 8;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM8->setEnabled(!value);
    CheckGimbal(8,value);
    DisableEnableAllPitchGimbal(8,value);
}
void QGCAutoquad::gmb_roll_P8(bool value){
    if ( value )
        port_nr_roll = 8;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM8->setEnabled(!value);
    CheckGimbal(8,value);
    DisableEnableAllRollGimbal(8,value);
}
void QGCAutoquad::gmb_pitch_P9(bool value){
    if ( value )
        port_nr_pitch = 9;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM9->setEnabled(!value);
    CheckGimbal(9,value);
    DisableEnableAllPitchGimbal(9,value);
}
void QGCAutoquad::gmb_roll_P9(bool value){
    if ( value )
        port_nr_roll = 9;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM9->setEnabled(!value);
    CheckGimbal(9,value);
    DisableEnableAllRollGimbal(9,value);
}
void QGCAutoquad::gmb_pitch_P10(bool value){
    if ( value )
        port_nr_pitch = 10;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM10->setEnabled(!value);
    CheckGimbal(10,value);
    DisableEnableAllPitchGimbal(10,value);
}
void QGCAutoquad::gmb_roll_P10(bool value){
    if ( value )
        port_nr_roll = 10;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM10->setEnabled(!value);
    CheckGimbal(10,value);
    DisableEnableAllRollGimbal(10,value);
}
void QGCAutoquad::gmb_pitch_P11(bool value){
    if ( value )
        port_nr_pitch = 11;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM11->setEnabled(!value);
    CheckGimbal(11,value);
    DisableEnableAllPitchGimbal(11,value);
}
void QGCAutoquad::gmb_roll_P11(bool value){
    if ( value )
        port_nr_roll = 11;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM11->setEnabled(!value);
    CheckGimbal(11,value);
    DisableEnableAllRollGimbal(11,value);
}
void QGCAutoquad::gmb_pitch_P12(bool value){
    if ( value )
        port_nr_pitch = 12;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM12->setEnabled(!value);
    CheckGimbal(12,value);
    DisableEnableAllPitchGimbal(12,value);
}
void QGCAutoquad::gmb_roll_P12(bool value){
    if ( value )
        port_nr_roll = 12;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM12->setEnabled(!value);
    CheckGimbal(12,value);
    DisableEnableAllRollGimbal(12,value);
}
void QGCAutoquad::gmb_pitch_P13(bool value){
    if ( value )
        port_nr_pitch = 13;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM13->setEnabled(!value);
    CheckGimbal(13,value);
    DisableEnableAllPitchGimbal(13,value);
}
void QGCAutoquad::gmb_roll_P13(bool value){
    if ( value )
        port_nr_roll = 13;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM13->setEnabled(!value);
    CheckGimbal(13,value);
    DisableEnableAllRollGimbal(13,value);
}
void QGCAutoquad::gmb_pitch_P14(bool value){
    if ( value )
        port_nr_pitch = 14;
    else
        port_nr_pitch = 0;
    ui->checkBox_isRollM14->setEnabled(!value);
    CheckGimbal(14,value);
    DisableEnableAllPitchGimbal(14,value);
}
void QGCAutoquad::gmb_roll_P14(bool value){
    if ( value )
        port_nr_roll = 14;
    else
        port_nr_roll = 0;
    ui->checkBox_isPitchM14->setEnabled(!value);
    CheckGimbal(14,value);
    DisableEnableAllRollGimbal(14,value);
}

void QGCAutoquad::CheckGimbal(int port, bool value) {
    AlreadyShowMessage = false;
    somethingChangedInMotorConfig = 0;
    if (( port >= 1) && ( port <= 4)) {
        setMotorEnable(1,!value);
        setMotorEnable(2,!value);
        setMotorEnable(3,!value);
        setMotorEnable(4,!value);
    }
    else if (( port >= 5) && ( port <= 8)) {
        setMotorEnable(5,!value);
        setMotorEnable(6,!value);
        setMotorEnable(7,!value);
        setMotorEnable(8,!value);
    }
    else if (( port >= 9) && ( port <= 10)) {
        setMotorEnable(9,!value);
        setMotorEnable(10,!value);
    }
    else if (( port >= 11) && ( port <= 12)) {
        setMotorEnable(11,!value);
        setMotorEnable(12,!value);
    }
    else if (( port == 13)) {
        setMotorEnable(13,!value);
    }
    else if (( port == 14)) {
        setMotorEnable(14,!value);
    }
}

void QGCAutoquad::ShowMessageForChangingMotorConfig(int Motor) {
    if ( EventComesFromMavlink == false) {
        AlreadyShowMessage = true;
        QString MessageInfo = QString();
        MessageInfo.append("You have selected a Gimbal Port, that was already defined as a Motor Port," );
        MessageInfo.append("\r\n");
        MessageInfo.append("or shares a hardware timer with a motor port!");
        MessageInfo.append("\r\n");
        MessageInfo.append("Please check again your Motor configuration!");
        QMessageBox::information(this, "Information", MessageInfo ,QMessageBox::Ok, 0 );
    }
}

void QGCAutoquad::setMotorEnable(int MotorIndex, bool value){

    if ( MotorIndex == 1) {
        ui->MOT_PWRD_01_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_01_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_01_T->setText("0");
        }

        ui->MOT_PWRD_01_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_01_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_01_P->setText("0");
        }

        ui->MOT_PWRD_01_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_01_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_01_R->setText("0");
        }

        ui->MOT_PWRD_01_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_01_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_01_Y->setText("0");
        }

        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(1);
        }
        somethingChangedInMotorConfig = 0;
    }



    if ( MotorIndex == 2) {
        ui->MOT_PWRD_02_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_02_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_02_T->setText("0");
        }

        ui->MOT_PWRD_02_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_02_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_02_P->setText("0");
        }

        ui->MOT_PWRD_02_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_02_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_02_R->setText("0");
        }

        ui->MOT_PWRD_02_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_02_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_02_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(2);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 3) {
        ui->MOT_PWRD_03_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_03_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_03_T->setText("0");
        }

        ui->MOT_PWRD_03_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_03_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_03_P->setText("0");
        }

        ui->MOT_PWRD_03_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_03_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_03_R->setText("0");
        }

        ui->MOT_PWRD_03_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_03_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_03_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(3);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 4) {
        ui->MOT_PWRD_04_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_04_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_04_T->setText("0");
        }

        ui->MOT_PWRD_04_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_04_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_04_P->setText("0");
        }

        ui->MOT_PWRD_04_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_04_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_04_R->setText("0");
        }

        ui->MOT_PWRD_04_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_04_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_04_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(4);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 5) {
        ui->MOT_PWRD_05_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_05_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_05_T->setText("0");
        }

        ui->MOT_PWRD_05_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_05_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_05_P->setText("0");
        }

        ui->MOT_PWRD_05_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_05_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_05_R->setText("0");
        }

        ui->MOT_PWRD_05_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_05_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_05_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(5);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 6) {
        ui->MOT_PWRD_06_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_06_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_06_T->setText("0");
        }

        ui->MOT_PWRD_06_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_06_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            if (somethingChangedInMotorConfig > 0 ) {
                ShowMessageForChangingMotorConfig(6);
            }
            //ui->MOT_PWRD_06_P->setText("0");
        }

        ui->MOT_PWRD_06_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_06_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            if (somethingChangedInMotorConfig > 0 ) {
                ShowMessageForChangingMotorConfig(6);
            }
            //ui->MOT_PWRD_06_R->setText("0");
        }

        ui->MOT_PWRD_06_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_06_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            if (somethingChangedInMotorConfig > 0 ) {
                ShowMessageForChangingMotorConfig(6);
            }
            //ui->MOT_PWRD_06_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(6);
        }
        somethingChangedInMotorConfig  = 0;
    }

    if ( MotorIndex == 7) {
        ui->MOT_PWRD_07_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_07_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_07_T->setText("0");
        }

        ui->MOT_PWRD_07_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_07_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_07_P->setText("0");
        }

        ui->MOT_PWRD_07_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_07_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_07_R->setText("0");
        }

        ui->MOT_PWRD_07_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_07_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_07_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(7);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 8) {
        ui->MOT_PWRD_08_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_08_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_08_T->setText("0");
        }

        ui->MOT_PWRD_08_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_08_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_08_P->setText("0");
        }

        ui->MOT_PWRD_08_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_08_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_08_R->setText("0");
        }

        ui->MOT_PWRD_08_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_08_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_08_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(8);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 9) {
        ui->MOT_PWRD_09_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_09_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_09_T->setText("0");
        }

        ui->MOT_PWRD_09_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_09_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_09_P->setText("0");
        }

        ui->MOT_PWRD_09_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_09_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_09_R->setText("0");
        }

        ui->MOT_PWRD_09_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_09_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_09_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(9);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 10) {
        ui->MOT_PWRD_10_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_10_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_10_T->setText("0");
        }

        ui->MOT_PWRD_10_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_10_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_10_P->setText("0");
        }

        ui->MOT_PWRD_10_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_10_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_10_R->setText("0");
        }

        ui->MOT_PWRD_10_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_10_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_10_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(10);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 11) {
        ui->MOT_PWRD_11_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_11_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_11_T->setText("0");
        }

        ui->MOT_PWRD_11_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_11_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_11_P->setText("0");
        }

        ui->MOT_PWRD_11_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_11_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_11_R->setText("0");
        }

        ui->MOT_PWRD_11_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_11_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_11_Y->setText("0");
        }

        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(11);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 12) {
        ui->MOT_PWRD_12_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_12_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_12_T->setText("0");
        }

        ui->MOT_PWRD_12_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_12_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_12_P->setText("0");
        }

        ui->MOT_PWRD_12_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_12_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_12_R->setText("0");
        }

        ui->MOT_PWRD_12_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_12_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_12_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(12);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 13) {
        ui->MOT_PWRD_13_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_13_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_13_T->setText("0");
        }

        ui->MOT_PWRD_13_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_13_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_13_P->setText("0");
        }

        ui->MOT_PWRD_13_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_13_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_13_R->setText("0");
        }

        ui->MOT_PWRD_13_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_13_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_13_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(13);
        }
        somethingChangedInMotorConfig = 0;
    }

    if ( MotorIndex == 14) {
        ui->MOT_PWRD_14_T->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_14_T->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_14_T->setText("0");
        }

        ui->MOT_PWRD_14_P->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_14_P->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_14_P->setText("0");
        }

        ui->MOT_PWRD_14_R->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_14_R->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_14_R->setText("0");
        }

        ui->MOT_PWRD_14_Y->setEnabled(value);
        if ( value == false ) {
            if ( ui->MOT_PWRD_14_Y->text().toInt() != 0)
                somethingChangedInMotorConfig = 1;
            //ui->MOT_PWRD_14_Y->setText("0");
        }
        if ((somethingChangedInMotorConfig > 0 ) && ( !AlreadyShowMessage )) {
            ShowMessageForChangingMotorConfig(14);
        }
        somethingChangedInMotorConfig = 0;
    }
}

void QGCAutoquad::DisableEnableAllPitchGimbal(int selectedIndex, bool value){
    int foundPitchBoxes;
    if ( value ) {
        foundPitchBoxes = 0;
        QList<QCheckBox*> checkList = qFindChildren<QCheckBox*> ( ui->tab_aq_settings );
        for ( int i = 0; i<checkList.count(); i++) {
            QString ParaName = checkList.at(i)->objectName();
            if ( ParaName.startsWith("checkBox_isPitch",Qt::CaseSensitive)) {
                foundPitchBoxes++;
                checkList.at(i)->setEnabled(false);
                if ( foundPitchBoxes == selectedIndex ) {
                    checkList.at(i)->setEnabled(true);
                }
            }
        }
    }
    else {
        QList<QCheckBox*> checkList = qFindChildren<QCheckBox*> ( ui->tab_aq_settings );
        foundPitchBoxes = 0;
        for ( int i = 0; i<checkList.count(); i++) {
            QString ParaName = checkList.at(i)->objectName();
            if ( ParaName.startsWith("checkBox_isPitch",Qt::CaseSensitive)) {
                checkList.at(i)->setEnabled(true);
                foundPitchBoxes++;
                if ( port_nr_roll > 0 ) {
                    if ( foundPitchBoxes == port_nr_roll ) {
                        checkList.at(i)->setEnabled(false);
                    }
                }
            }
        }
    }
}


void QGCAutoquad::DisableEnableAllRollGimbal(int selectedIndex, bool value) {
    int foundRollBoxes;
    if ( value ) {
        foundRollBoxes = 0;
        QList<QCheckBox*> checkList = qFindChildren<QCheckBox*> ( ui->tab_aq_settings );
        for ( int i = 0; i<checkList.count(); i++) {
            QString ParaName = checkList.at(i)->objectName();
            if ( ParaName.startsWith("checkBox_isRoll",Qt::CaseSensitive)) {
                checkList.at(i)->setEnabled(false);
                foundRollBoxes++;
                if ( foundRollBoxes == selectedIndex ) {
                    checkList.at(i)->setEnabled(true);
                }
            }
        }
    }
    else {
        QList<QCheckBox*> checkList = qFindChildren<QCheckBox*> ( ui->tab_aq_settings );
        foundRollBoxes = 0;
        for ( int i = 0; i<checkList.count(); i++) {
            QString ParaName = checkList.at(i)->objectName();
            if ( ParaName.startsWith("checkBox_isRoll",Qt::CaseSensitive)) {
                foundRollBoxes++;
                checkList.at(i)->setEnabled(true);
                if ( port_nr_pitch > 0 ) {
                    if ( foundRollBoxes == port_nr_pitch ) {
                        checkList.at(i)->setEnabled(false);
                    }
                }
            }
        }
    }
}


void QGCAutoquad::setRadio() {

    if ( !paramaq)
            return;
    paramaq->setParameter(190,"RADIO_TYPE",ui->comboBox_Radio_Type->currentIndex());
    QuestionForROM();

}

void QGCAutoquad::setFrame() {

    if ( !paramaq)
            return;

    paramaq->setParameter(190,"MOT_PWRD_01_T",ui->MOT_PWRD_01_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_T",ui->MOT_PWRD_02_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_T",ui->MOT_PWRD_03_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_T",ui->MOT_PWRD_04_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_T",ui->MOT_PWRD_05_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_T",ui->MOT_PWRD_06_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_T",ui->MOT_PWRD_07_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_T",ui->MOT_PWRD_08_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_T",ui->MOT_PWRD_09_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_T",ui->MOT_PWRD_10_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_T",ui->MOT_PWRD_11_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_T",ui->MOT_PWRD_12_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_T",ui->MOT_PWRD_13_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_T",ui->MOT_PWRD_14_T->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_P",ui->MOT_PWRD_01_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_P",ui->MOT_PWRD_02_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_P",ui->MOT_PWRD_03_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_P",ui->MOT_PWRD_04_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_P",ui->MOT_PWRD_05_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_P",ui->MOT_PWRD_06_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_P",ui->MOT_PWRD_07_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_P",ui->MOT_PWRD_08_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_P",ui->MOT_PWRD_09_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_P",ui->MOT_PWRD_10_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_P",ui->MOT_PWRD_11_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_P",ui->MOT_PWRD_12_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_P",ui->MOT_PWRD_13_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_P",ui->MOT_PWRD_14_P->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_R",ui->MOT_PWRD_01_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_R",ui->MOT_PWRD_02_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_R",ui->MOT_PWRD_03_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_R",ui->MOT_PWRD_04_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_R",ui->MOT_PWRD_05_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_R",ui->MOT_PWRD_06_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_R",ui->MOT_PWRD_07_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_R",ui->MOT_PWRD_08_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_R",ui->MOT_PWRD_09_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_R",ui->MOT_PWRD_10_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_R",ui->MOT_PWRD_11_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_R",ui->MOT_PWRD_12_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_R",ui->MOT_PWRD_13_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_R",ui->MOT_PWRD_14_R->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_Y",ui->MOT_PWRD_01_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_Y",ui->MOT_PWRD_02_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_Y",ui->MOT_PWRD_03_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_Y",ui->MOT_PWRD_04_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_Y",ui->MOT_PWRD_05_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_Y",ui->MOT_PWRD_06_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_Y",ui->MOT_PWRD_07_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_Y",ui->MOT_PWRD_08_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_Y",ui->MOT_PWRD_09_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_Y",ui->MOT_PWRD_10_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_Y",ui->MOT_PWRD_11_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_Y",ui->MOT_PWRD_12_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_Y",ui->MOT_PWRD_13_Y->text().toFloat());
	paramaq->setParameter(190,"MOT_PWRD_14_Y",ui->MOT_PWRD_14_Y->text().toFloat());

    paramaq->setParameter(190,"MOT_FRAME","0");
    paramaq->setParameter(190,"GMBL_ROLL_PORT",port_nr_roll);
    paramaq->setParameter(190,"GMBL_PITCH_PORT",port_nr_pitch);

    float abs_value;
    QVariant value_scal_pitch = 0.0f;
    paramaq->getParameterValue(190,"GMBL_SCAL_PITCH",value_scal_pitch);
    abs_value = fabs(value_scal_pitch.toFloat());
    if ( ui->reverse_gimbal_pitch->checkState()) {
        paramaq->setParameter(190,"GMBL_SCAL_PITCH",0-abs_value);
    }
    else {
        paramaq->setParameter(190,"GMBL_SCAL_PITCH",abs_value);
    }

    QVariant value_scal_roll = 0.0f;
    paramaq->getParameterValue(190,"GMBL_SCAL_ROLL",value_scal_roll);
    abs_value = fabs(value_scal_roll.toFloat());
    if ( ui->reverse_gimbal_roll->checkState()) {
        paramaq->setParameter(190,"GMBL_SCAL_ROLL",0-abs_value);
    }
    else {
        paramaq->setParameter(190,"GMBL_SCAL_ROLL",abs_value);
    }

    QuestionForROM();
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

void QGCAutoquad::LoadFrameFromFile() {

    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Mixing Table (*.mix)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }
	
    if (fileNames.size() > 0)
    {
        QString fileName = fileNames.first();
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read mixing file. Permission denied");
            msgBox.exec();
			return;
        }
		file.close();

		QSettings settings(fileName, QSettings::IniFormat);

		settings.beginGroup("Throttle");
		ui->MOT_PWRD_01_T->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_T->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_T->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_T->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_T->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_T->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_T->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_T->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_T->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_T->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_T->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_T->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_T->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_T->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Pitch");
		ui->MOT_PWRD_01_P->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_P->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_P->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_P->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_P->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_P->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_P->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_P->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_P->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_P->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_P->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_P->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_P->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_P->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Roll");
		ui->MOT_PWRD_01_R->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_R->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_R->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_R->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_R->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_R->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_R->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_R->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_R->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_R->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_R->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_R->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_R->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_R->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Yaw");
		ui->MOT_PWRD_01_Y->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_Y->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_Y->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_Y->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_Y->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_Y->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_Y->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_Y->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_Y->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_Y->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_Y->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_Y->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_Y->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_Y->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.sync();
	}

}

void QGCAutoquad::SaveFrameToFile() {

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "./motorMixing.mix", tr("AQ Mixing Table (*.mix)"));
	if ( !fileName.endsWith(".mix"))
		fileName += ".mix";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
	file.close();

	QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Throttle");
	settings.setValue("Motor1", ui->MOT_PWRD_01_T->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_T->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_T->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_T->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_T->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_T->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_T->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_T->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_T->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_T->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_T->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_T->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_T->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_T->text());
    settings.endGroup();
    settings.beginGroup("Pitch");
	settings.setValue("Motor1", ui->MOT_PWRD_01_P->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_P->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_P->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_P->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_P->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_P->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_P->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_P->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_P->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_P->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_P->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_P->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_P->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_P->text());
    settings.endGroup();
    settings.beginGroup("Roll");
	settings.setValue("Motor1", ui->MOT_PWRD_01_R->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_R->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_R->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_R->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_R->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_R->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_R->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_R->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_R->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_R->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_R->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_R->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_R->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_R->text());
    settings.endGroup();
    settings.beginGroup("Yaw");
	settings.setValue("Motor1", ui->MOT_PWRD_01_Y->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_Y->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_Y->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_Y->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_Y->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_Y->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_Y->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_Y->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_Y->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_Y->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_Y->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_Y->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_Y->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_Y->text());
    settings.endGroup();
    settings.sync();

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

void QGCAutoquad::QuestionForROM()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values are transmitted to AutoQuad! Do you want to store the para into ROM?");
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

void QGCAutoquad::save_PID_toAQ1()
{
QVariant val_uas;
QVariant val_local;
bool changed;

    if ( !paramaq)
        return;

    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->TiltYawPID );
    for ( int i = 0; i<edtList.count(); i++) {
        QString ParaName = edtList.at(i)->objectName();
        // Hier alle NAV von PID Page 1
        if ( ParaName.startsWith("CTRL_",Qt::CaseSensitive)) {
            if ( paramaq->getParameterValue(190,ParaName,val_uas) )
            {
                val_local = edtList.at(i)->text().toFloat();
                if ( val_uas != val_local) {
                    paramaq->setParameter(190,ParaName,val_local);
                    changed = true;
                }
            }
        }
    }
    if ( changed )
        QuestionForROM();

}

void QGCAutoquad::save_PID_toAQ2()
{
    QVariant val_uas;
    QVariant val_local;
    bool changed;

        if ( !paramaq)
            return;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->NavigationPID );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            // Hier alle CTRL von PID Page 2
            if ( ParaName.startsWith("NAV_",Qt::CaseSensitive)) {
                if ( paramaq->getParameterValue(190,ParaName,val_uas) )
                {
                    val_local = edtList.at(i)->text().toFloat();
                    if ( val_uas != val_local) {
                        paramaq->setParameter(190,ParaName,val_local);
                        changed = true;
                    }
                }
            }
        }
        if ( changed )
            QuestionForROM();

}

void QGCAutoquad::save_PID_toAQ3()
{
    QVariant val_uas;
    QVariant val_local;
    bool changed;

        if ( !paramaq)
            return;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->SpecialSettings );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            // Hier alle CTRL von PID Page 3
            if ( paramaq->getParameterValue(190,ParaName,val_uas) )
            {
                val_local = edtList.at(i)->text().toFloat();
                if ( val_uas != val_local) {
                    paramaq->setParameter(190,ParaName,val_local);
                    changed = true;
                }
            }
        }

        paramaq->setParameter(190,"SPVR_FS_RAD_ST1",ui->CMB_SPVR_FS_RAD_ST1->currentIndex());
        paramaq->setParameter(190,"SPVR_FS_RAD_ST2",ui->CMB_SPVR_FS_RAD_ST2->currentIndex());

        if ( changed )
            QuestionForROM();
}


void QGCAutoquad::save_PID_toAQ4()
{
    QVariant val_uas;
    QVariant val_local;
    bool changed;

        if ( !paramaq)
            return;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->Gimbal );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            // Hier alle CTRL von PID Page 3
            if ( paramaq->getParameterValue(190,ParaName,val_uas) )
            {
                val_local = edtList.at(i)->text().toFloat();
                if ( val_uas != val_local) {
                    if (( ParaName != "GMBL_SCAL_PITCH") && ( ParaName != "GMBL_SCAL_ROLL"))
                        paramaq->setParameter(190,ParaName,val_local);
                    changed = true;
                }
            }
        }

        float abs_value;
        QVariant value_scal_pitch = 0.0f;
        paramaq->getParameterValue(190,"GMBL_SCAL_PITCH",value_scal_pitch);
        abs_value = fabs(value_scal_pitch.toFloat());
        if ( ui->reverse_gimbal_pitch->checkState()) {
            paramaq->setParameter(190,"GMBL_SCAL_PITCH",0-abs_value);
        }
        else {
            paramaq->setParameter(190,"GMBL_SCAL_PITCH",abs_value);
        }

        QVariant value_scal_roll = 0.0f;
        paramaq->getParameterValue(190,"GMBL_SCAL_ROLL",value_scal_roll);
        abs_value = fabs(value_scal_roll.toFloat());
        if ( ui->reverse_gimbal_roll->checkState()) {
            paramaq->setParameter(190,"GMBL_SCAL_ROLL",0-abs_value);
        }
        else {
            paramaq->setParameter(190,"GMBL_SCAL_ROLL",abs_value);
        }

        if ( changed )
            QuestionForROM();
}


void QGCAutoquad::save_plot_image(){
    QString fileName = "plot.svg";
    fileName = QFileDialog::getSaveFileName(
                   this, "Export File Name", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
                   "SVG Images (*.svg);;PDF Documents (*.pdf)");

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
            //ui->listView_Curves->reset();
            //QMessageBox::information(this, "Information", "Reload the !",QMessageBox::Ok, 0 );
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
    removeMarker();
}

void QGCAutoquad::teleValuesStart(){

    if (!uas)
        return;
    connect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));
    //connect(uas, SIGNAL(TelemetryChangedI(int,mavlink_aq_telemetry_i_t)), this, SLOT(getNewTelemetryI(int,mavlink_aq_telemetry_i_t)));

    int uasId = uas->getUASID();
    AqTeleChart->appendData(uasId,"AQ_ROLL","",0,0);
    AqTeleChart->appendData(uasId,"AQ_PITCH","",0,0);
    AqTeleChart->appendData(uasId,"AQ_YAW","",0,0);
    AqTeleChart->appendData(uasId,"IMU_RATEX","",0,0);
    AqTeleChart->appendData(uasId,"IMU_RATEY","",0,0);
    AqTeleChart->appendData(uasId,"IMU_RATEZ","",0,0);
    AqTeleChart->appendData(uasId,"IMU_ACCX","",0,0);
    AqTeleChart->appendData(uasId,"IMU_ACCY","",0,0);
    AqTeleChart->appendData(uasId,"IMU_ACCZ","",0,0);
    AqTeleChart->appendData(uasId,"IMU_MAGX","",0,0);
    AqTeleChart->appendData(uasId,"IMU_MAGY","",0,0);
    AqTeleChart->appendData(uasId,"IMU_MAGZ","",0,0);
    AqTeleChart->appendData(uasId,"navData.HoldHeading","",0,0);
    AqTeleChart->appendData(uasId,"AQ_Pressure","",0,0);
    AqTeleChart->appendData(uasId,"IMU_TEMP","",0,0);
    AqTeleChart->appendData(uasId,"UKF_ALTITUDE","",0,0);
    AqTeleChart->appendData(uasId,"adcData.vIn","",0,0);
    AqTeleChart->appendData(uasId,"UKF_POSN","",0,0);
    AqTeleChart->appendData(uasId,"UKF_POSE","",0,0);
    AqTeleChart->appendData(uasId,"Res1","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.lat","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.lon","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.hAcc","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.heading","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.height","",0,0);
    AqTeleChart->appendData(uasId,"gpsData.pDOP","",0,0);
    AqTeleChart->appendData(uasId,"navData.holdCourse","",0,0);
    AqTeleChart->appendData(uasId,"navData.holdDistance","",0,0);
    AqTeleChart->appendData(uasId,"navData.holdAlt","",0,0);
    AqTeleChart->appendData(uasId,"navData.holdTiltN","",0,0);
    AqTeleChart->appendData(uasId,"navData.holdTiltE","",0,0);
    AqTeleChart->appendData(uasId,"UKF_VELN","",0,0);
    AqTeleChart->appendData(uasId,"UKF_VELE","",0,0);
    AqTeleChart->appendData(uasId,"UKF_VELD","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_QUALITY","",0,0);
    AqTeleChart->appendData(uasId,"UKF_ACC_BIASX","",0,0);
    AqTeleChart->appendData(uasId,"UKF_ACC_BIASY","",0,0);
    AqTeleChart->appendData(uasId,"UKF_ACC_BIASZ","",0,0);
    AqTeleChart->appendData(uasId,"supervisor.flighttime","",0,0);
    AqTeleChart->appendData(uasId,"Res2","",0,0);

    AqTeleChart->appendData(uasId,"RADIO_THROT","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_RUDD","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_PITCH","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_ROLL","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_FLAPS","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_AUX2","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_AUX3","",0,0);
    AqTeleChart->appendData(uasId,"RADIO_AUX4","",0,0);

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

void QGCAutoquad::teleValuesStop() {
    if (!uas)
        return;
    disconnect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));
    //disconnect(uas, SIGNAL(TelemetryChangedI(int,mavlink_aq_telemetry_i_t)), this, SLOT(getNewTelemetryI(int,mavlink_aq_telemetry_i_t)));
    uas->startStopTelemetry(false,0.0f);
}

void QGCAutoquad::getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values){
    msec = 0;
    if ( values.Index == 0) {
        //QTime time = QTime::currentTime();

        ui->Tele_Value1->setText(QString::number(values.value1));
        if ( AqTeleChart->CurveIsActive[0])
            AqTeleChart->appendData(uasId,"AQ_ROLL","",values.value1,msec);

        ui->Tele_Value2->setText(QString::number(values.value2));
        if ( AqTeleChart->CurveIsActive[1])
            AqTeleChart->appendData(uasId,"AQ_PITCH","",values.value2,msec);

        ui->Tele_Value3->setText(QString::number(values.value3));
        if ( AqTeleChart->CurveIsActive[2])
            AqTeleChart->appendData(uasId,"AQ_YAW","",values.value3,msec);

        ui->Tele_Value4->setText(QString::number(values.value4));
        if ( AqTeleChart->CurveIsActive[3])
            AqTeleChart->appendData(uasId,"IMU_RATEX","",values.value4,msec);

        ui->Tele_Value5->setText(QString::number(values.value5));
        if ( AqTeleChart->CurveIsActive[4])
            AqTeleChart->appendData(uasId,"IMU_RATEY","",values.value5,msec);

        ui->Tele_Value6->setText(QString::number(values.value6));
        if ( AqTeleChart->CurveIsActive[5])
            AqTeleChart->appendData(uasId,"IMU_RATEZ","",values.value6,msec);

        ui->Tele_Value7->setText(QString::number(values.value7));
        if ( AqTeleChart->CurveIsActive[6])
            AqTeleChart->appendData(uasId,"IMU_ACCX","",values.value7,msec);

        ui->Tele_Value8->setText(QString::number(values.value8));
        if ( AqTeleChart->CurveIsActive[7])
            AqTeleChart->appendData(uasId,"IMU_ACCY","",values.value8,msec);

        ui->Tele_Value9->setText(QString::number(values.value9));
        if ( AqTeleChart->CurveIsActive[8])
            AqTeleChart->appendData(uasId,"IMU_ACCZ","",values.value9,msec);

        ui->Tele_Value10->setText(QString::number(values.value10));
        if ( AqTeleChart->CurveIsActive[9])
            AqTeleChart->appendData(uasId,"IMU_MAGX","",values.value10,msec);

        ui->Tele_Value11->setText(QString::number(values.value11));
        if ( AqTeleChart->CurveIsActive[10])
            AqTeleChart->appendData(uasId,"IMU_MAGY","",values.value11,msec);

        ui->Tele_Value12->setText(QString::number(values.value12));
        if ( AqTeleChart->CurveIsActive[11])
            AqTeleChart->appendData(uasId,"IMU_MAGZ","",values.value12,msec);

        ui->Tele_Value13->setText(QString::number(values.value13));
        if ( AqTeleChart->CurveIsActive[12])
            AqTeleChart->appendData(uasId,"navData.HoldHeading","",values.value13,msec);

        ui->Tele_Value14->setText(QString::number(values.value14));
        if ( AqTeleChart->CurveIsActive[13])
            AqTeleChart->appendData(uasId,"AQ_Pressure","",values.value14,msec);

        ui->Tele_Value15->setText(QString::number(values.value15));
        if ( AqTeleChart->CurveIsActive[14])
            AqTeleChart->appendData(uasId,"IMU_TEMP","",values.value15,msec);

        ui->Tele_Value16->setText(QString::number(values.value16));
        if ( AqTeleChart->CurveIsActive[15])
            AqTeleChart->appendData(uasId,"UKF_ALTITUDE","",values.value16,msec);

        ui->Tele_Value17->setText(QString::number(values.value17));
        if ( AqTeleChart->CurveIsActive[16])
            AqTeleChart->appendData(uasId,"adcData.vIn","",values.value17,msec);

        ui->Tele_Value18->setText(QString::number(values.value18));
        if ( AqTeleChart->CurveIsActive[17])
            AqTeleChart->appendData(uasId,"UKF_POSN","",values.value18,msec);

        ui->Tele_Value19->setText(QString::number(values.value19));
        if ( AqTeleChart->CurveIsActive[18])
            AqTeleChart->appendData(uasId,"UKF_POSE","",values.value19,msec);

        ui->Tele_Value20->setText(QString::number(values.value20));
        if ( AqTeleChart->CurveIsActive[19])
            AqTeleChart->appendData(uasId,"Res1","",values.value20,msec);
    }
    else if ( values.Index == 1) {
        ui->Tele_Value21->setText(QString::number(values.value1));
        if ( AqTeleChart->CurveIsActive[20])
            AqTeleChart->appendData(uasId,"gpsData.lat","",values.value1,msec);

        ui->Tele_Value22->setText(QString::number(values.value2));
        if ( AqTeleChart->CurveIsActive[21])
            AqTeleChart->appendData(uasId,"gpsData.lon","",values.value2,msec);

        ui->Tele_Value23->setText(QString::number(values.value3));
        if ( AqTeleChart->CurveIsActive[22])
            AqTeleChart->appendData(uasId,"gpsData.hAcc","",values.value3,msec);

        ui->Tele_Value24->setText(QString::number(values.value4));
        if ( AqTeleChart->CurveIsActive[23])
            AqTeleChart->appendData(uasId,"gpsData.heading","",values.value4,msec);

        ui->Tele_Value25->setText(QString::number(values.value5));
        if ( AqTeleChart->CurveIsActive[24])
            AqTeleChart->appendData(uasId,"gpsData.height","",values.value5,msec);

        ui->Tele_Value26->setText(QString::number(values.value6));
        if ( AqTeleChart->CurveIsActive[25])
            AqTeleChart->appendData(uasId,"gpsData.pDOP","",values.value6,msec);

        ui->Tele_Value27->setText(QString::number(values.value7));
        if ( AqTeleChart->CurveIsActive[26])
            AqTeleChart->appendData(uasId,"navData.holdCourse","",values.value7,msec);

        ui->Tele_Value28->setText(QString::number(values.value8));
        if ( AqTeleChart->CurveIsActive[27])
            AqTeleChart->appendData(uasId,"navData.holdDistance","",values.value8,msec);

        ui->Tele_Value29->setText(QString::number(values.value9));
        if ( AqTeleChart->CurveIsActive[28])
            AqTeleChart->appendData(uasId,"navData.holdAlt","",values.value9,msec);

        ui->Tele_Value30->setText(QString::number(values.value10));
        if ( AqTeleChart->CurveIsActive[29])
            AqTeleChart->appendData(uasId,"navData.holdTiltN","",values.value10,msec);

        ui->Tele_Value31->setText(QString::number(values.value11));
        if ( AqTeleChart->CurveIsActive[30])
            AqTeleChart->appendData(uasId,"navData.holdTiltE","",values.value11,msec);

        ui->Tele_Value32->setText(QString::number(values.value12));
        if ( AqTeleChart->CurveIsActive[31])
            AqTeleChart->appendData(uasId,"UKF_VELN","",values.value12,msec);

        ui->Tele_Value33->setText(QString::number(values.value13));
        if ( AqTeleChart->CurveIsActive[32])
            AqTeleChart->appendData(uasId,"UKF_VELE","",values.value13,msec);

        ui->Tele_Value34->setText(QString::number(values.value14));
        if ( AqTeleChart->CurveIsActive[33])
            AqTeleChart->appendData(uasId,"UKF_VELD","",values.value14,msec);

        ui->Tele_Value35->setText(QString::number(values.value15));
        if ( AqTeleChart->CurveIsActive[34])
            AqTeleChart->appendData(uasId,"RADIO_QUALITY","",values.value15,msec);

        ui->Tele_Value36->setText(QString::number(values.value16));
        if ( AqTeleChart->CurveIsActive[35])
            AqTeleChart->appendData(uasId,"UKF_ACC_BIASX","",values.value16,msec);

        ui->Tele_Value37->setText(QString::number(values.value17));
        if ( AqTeleChart->CurveIsActive[36])
            AqTeleChart->appendData(uasId,"UKF_ACC_BIASY","",values.value17,msec);

        ui->Tele_Value38->setText(QString::number(values.value18));
        if ( AqTeleChart->CurveIsActive[37])
            AqTeleChart->appendData(uasId,"UKF_ACC_BIASZ","",values.value18,msec);

        ui->Tele_Value39->setText(QString::number(values.value19));
        if ( AqTeleChart->CurveIsActive[38])
            AqTeleChart->appendData(uasId,"supervisor.flighttime","",values.value19,msec);

        ui->Tele_Value40->setText(QString::number(values.value20));
        if ( AqTeleChart->CurveIsActive[39])
            AqTeleChart->appendData(uasId,"Res2","",values.value20,msec);
    }
    else if ( values.Index == 2) {
        ui->Tele_Value40->setText(QString::number(values.value1));
        if ( AqTeleChart->CurveIsActive[40])
            AqTeleChart->appendData(uasId,"RADIO_THROT","",values.value1,msec);

        ui->Tele_Value41->setText(QString::number(values.value2));
        if ( AqTeleChart->CurveIsActive[41])
            AqTeleChart->appendData(uasId,"RADIO_RUDD","",values.value2,msec);

        ui->Tele_Value42->setText(QString::number(values.value3));
        if ( AqTeleChart->CurveIsActive[42])
            AqTeleChart->appendData(uasId,"RADIO_PITCH","",values.value3,msec);

        ui->Tele_Value43->setText(QString::number(values.value4));
        if ( AqTeleChart->CurveIsActive[43])
            AqTeleChart->appendData(uasId,"RADIO_ROLL","",values.value4,msec);

        ui->Tele_Value44->setText(QString::number(values.value5));
        if ( AqTeleChart->CurveIsActive[44])
            AqTeleChart->appendData(uasId,"RADIO_FLAPS","",values.value5,msec);

        ui->Tele_Value45->setText(QString::number(values.value6));
        if ( AqTeleChart->CurveIsActive[45])
            AqTeleChart->appendData(uasId,"RADIO_AUX2","",values.value6,msec);

        ui->Tele_Value45->setText(QString::number(values.value7));
        if ( AqTeleChart->CurveIsActive[46])
            AqTeleChart->appendData(uasId,"RADIO_AUX3","",values.value7,msec);

        ui->Tele_Value45->setText(QString::number(values.value8));
        if ( AqTeleChart->CurveIsActive[47])
            AqTeleChart->appendData(uasId,"RADIO_AUX4","",values.value8,msec);

    }
}

void QGCAutoquad::getNewTelemetryI(int uasId, mavlink_aq_telemetry_i_t values){

        ui->Tele_Value40->setText(QString::number(values.value1));
        if ( AqTeleChart->CurveIsActive[40])
            AqTeleChart->appendData(uasId,"RADIO_THROT","",values.value1,msec);

        ui->Tele_Value41->setText(QString::number(values.value2));
        if ( AqTeleChart->CurveIsActive[41])
            AqTeleChart->appendData(uasId,"RADIO_RUDD","",values.value2,msec);

        ui->Tele_Value42->setText(QString::number(values.value3));
        if ( AqTeleChart->CurveIsActive[42])
            AqTeleChart->appendData(uasId,"RADIO_PITCH","",values.value3,msec);

        ui->Tele_Value43->setText(QString::number(values.value4));
        if ( AqTeleChart->CurveIsActive[43])
            AqTeleChart->appendData(uasId,"RADIO_ROLL","",values.value4,msec);

        ui->Tele_Value44->setText(QString::number(values.value5));
        if ( AqTeleChart->CurveIsActive[44])
            AqTeleChart->appendData(uasId,"RADIO_FLAPS","",values.value5,msec);

        ui->Tele_Value45->setText(QString::number(values.value6));
        if ( AqTeleChart->CurveIsActive[45])
            AqTeleChart->appendData(uasId,"RADIO_AUX2","",values.value6,msec);

        ui->Tele_Value45->setText(QString::number(values.value7));
        if ( AqTeleChart->CurveIsActive[46])
            AqTeleChart->appendData(uasId,"RADIO_AUX3","",values.value7,msec);

        ui->Tele_Value45->setText(QString::number(values.value8));
        if ( AqTeleChart->CurveIsActive[47])
            AqTeleChart->appendData(uasId,"RADIO_AUX4","",values.value8,msec);

        /*
        ui->Tele_Value46->setText(QString::number(values.value7));
        ui->Tele_Value47->setText(QString::number(values.value8));
        ui->Tele_Value48->setText(QString::number(values.value9));
        ui->Tele_Value49->setText(QString::number(values.value10));
        ui->Tele_Value50->setText(QString::number(values.value11));
        ui->Tele_Value51->setText(QString::number(values.value12));
        ui->Tele_Value52->setText(QString::number(values.value13));
        ui->Tele_Value53->setText(QString::number(values.value14));
        ui->Tele_Value54->setText(QString::number(values.value15));
        */
}
