#include "qgcautoquad.h"
#include "ui_qgcautoquad.h"
#include "../configuration.h"
#include "MG.h"
#include "QGCCore.h"
#include "MainWindow.h"
#include "autoquadMAV.h"
#include "qgcaqparamwidget.h"
#include "aq_pwmPortsConfig.h"
#include "LinkManager.h"
#include "UASManager.h"
#include "SerialLinkInterface.h"
#include "SerialLink.h"
#include "GAudioOutput.h"
#include "SelectAdjustableParamDialog.h"
#ifdef INCLUDE_ESC32V2_UI
#include "AQEsc32ConfigWidget.h"
#endif
#ifdef INCLUDE_DEVEL_WIDGET
#include "AQDevelWidget.h"
#endif

#include <QWidget>
#include <QFileDialog>
#include <QTextBrowser>
#include <QComboBox>
#include <QMessageBox>
#include <QSignalMapper>
#include <QStringList>
#include <QDialogButtonBox>
#include <qextserialenumerator.h>
#include <float.h>

QGCAutoquad::QGCAutoquad(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCAutoquad),
    uas(NULL),
    paramaq(NULL),
    connectedLink(NULL),
    mtx_paramsAreLoading(false),
    m_initComplete(false),
    m_configTelemIsRunning(false),
    m_selectAdjParamsDialog(NULL)
{

    VisibleWidget = 0;
    fwFlashActive = false;
	 maxMotorPorts = 16;
	 maxPwmPorts = 14;

    aqBinFolderPath = QCoreApplication::applicationDirPath() + "/aq/bin/";
    aqMotorMixesPath = QCoreApplication::applicationDirPath() + "/aq/mixes/";
#if defined(Q_OS_WIN)
    platformExeExt = ".exe";
#else
    platformExeExt = "";
#endif

    // these regexes are used for matching field names to AQ params
    fldnameRx.setPattern("^(CONFIG|COMM|CTRL|DOWNLINK|GMBL|GPS|IMU|L1|MOT|NAV|PPM|RADIO|SIG|SPVR|TELEMETRY|UKF|VN100|QUATOS|LIC)_[A-Z0-9_]+$"); // strict field name matching
    dupeFldnameRx.setPattern("___N[0-9]"); // for having duplicate field names, append ___N# after the field name (three underscores, "N", and a unique number)
    paramsReqRestartRx.setPattern("^(COMM_.+|RADIO_(TYPE|SETUP)|MOT_(PWRD|CAN|ESC).+|GMBL_.+_PORT|SIG_.+_PRT|SPVR_VIN_SOURCE|CONFIG_ADJUST_P[0-9]+)$");
    //paramsRadioControls.setPattern("^(RADIO|NAV|GMBL)_.+_(CH|CHAN)$");
    //paramsSwitchControls.setPattern("^(NAV|GMBL|SPVR)_CTRL_[A-Z0-9_]+$");
    paramsTunableControls.setPattern("^CONFIG_ADJUST_P[0-9]+$");
    //paramsTunableControlChannels.setPattern("^(QUATOS_.+_KNOB|CONFIG_ADJUST_P[0-9]+_chan)$");
    //paramsNonTunable.setPattern("^((CONFIG|COMM|IMU|LIC|RADIO|SPVR|SIG|TELEMETRY)_.+|(CTRL_PID_TYPE|GMBL_.+_(PORT|CHAN|TILT)|NAV_CTRL_.+|MOT_(CAN|FRAM|ESC_|PWRD_).+|QUATOS_(MM_.+|.+_KNOB)))$");

    /*
     * Start the UI
    */

    ui->setupUi(this);

    // load the port config UI
    aqPwmPortConfig = new AQPWMPortsConfig(this);
    ui->tabLayout_aqMixingOutput->addWidget(aqPwmPortConfig);

    // set up the splitter expand/collapse button
    QSplitterHandle *shandle = ui->splitter_aqWidgetSidebar->handle(1);
    ui->splitter_aqWidgetSidebar->setProperty("styleType", "withButton");
    shandle->setProperty("styleType", "withButton");
    shandle->setContentsMargins(0, 25, 0, 0);
    shandle->setToolTip(tr("<html><body><p>Click the arrow button to collapse/expand the left sidebar. Click and drag anywhere to resize.</p></body></html>"));
    QVBoxLayout *hlayout = new QVBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);
    splitterToggleBtn = new QToolButton(shandle);
    splitterToggleBtn->setObjectName("toolButton_splitterToggleBtn");
    splitterToggleBtn->setArrowType(Qt::LeftArrow);
    splitterToggleBtn->setCursor(QCursor(Qt::ArrowCursor));
    splitterToggleBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    hlayout->addWidget(splitterToggleBtn);
    hlayout->setAlignment(splitterToggleBtn, Qt::AlignTop);
    hlayout->addStretch(3);
    shandle->setLayout(hlayout);

    // populate field values

    // multiple-radio mode selector
    ui->comboBox_multiRadioMode->addItem(tr("Diversity"), 0);
    ui->comboBox_multiRadioMode->addItem(tr("Split"), 1);

    ui->SPVR_FS_RAD_ST1->addItem(tr("Position Hold"), 0);

    // button groups, button ID = param value/bit
    ui->QUATOS_ENABLE->setId(ui->radioButton_attitude_pid, 0);
    ui->QUATOS_ENABLE->setId(ui->radioButton_attitude_quatos, 1);

    ui->CONFIG_FLAGS->setId(ui->checkBox_saveAdjustedParams, 0x01);

    // baud rates
    QList<int> availableBaudRates = MG::SERIAL::getBaudRates();
    QStringList baudRates;
    for (int i=0; i < availableBaudRates.length(); ++i)
        baudRates.append(QString::number(availableBaudRates[i]));

    ui->DOWNLINK_BAUD->addItems(baudRates);
    ui->COMM_BAUD1->addItems(baudRates);
    ui->COMM_BAUD2->addItems(baudRates);
    ui->COMM_BAUD3->addItems(baudRates);
    ui->COMM_BAUD4->addItems(baudRates);

    // firmware types
    ui->comboBox_fwType->addItem(tr("AutoQuad Serial"), "aq");
    ui->comboBox_fwType->addItem(tr("AutoQuad M4 USB"), "dfu");
    ui->comboBox_fwType->setCurrentIndex(0);

    // firmware serial flash baud rates
    QStringList flashBaudRates;
    flashBaudRates << "38400" <<  "57600" << "115200";
    ui->comboBox_fwPortSpeed->addItems(flashBaudRates);


    // populate COMM stream types
    QList<QComboBox *> commStreamTypCb = this->findChildren<QComboBox *>(QRegExp("COMM_STREAM_TYP[\\d]$"));
    foreach (QComboBox* cb, commStreamTypCb) {
        cb->addItem(tr("None"), COMM_TYPE_NONE);
        cb->addItem(tr("Multiplex (*)"), COMM_TYPE_MULTIPLEX);
        cb->addItem(tr("MAVLink"), COMM_TYPE_MAVLINK);
        cb->addItem(tr("AQ Serial"), COMM_TYPE_TELEMETRY);
        cb->addItem(tr("GPS Passthrough"), COMM_TYPE_GPS);
        cb->addItem(tr("Custom Telemetry"), COMM_TYPE_RX_TELEM);
        cb->addItem(tr("CLI (*)"), COMM_TYPE_CLI);
        cb->addItem(tr("OMAP Console (*)"), COMM_TYPE_OMAP_CONSOLE);
        cb->addItem(tr("OMAP PPP (*)"), COMM_TYPE_OMAP_PPP);
    }

    // Final UI tweaks

    ui->label_legacyChannelSwitchWarning->hide();
    ui->label_adjustParamsChannelSwitchWarning->hide();
    ui->spinBox_rcGraphRefreshFreq->hide();
    ui->groupBox_ppmOptions->hide();

	 // hide some controls which may get shown later based on AQ fw version
    ui->comboBox_multiRadioMode->hide();
    ui->label_multiRadioMode->hide();
    ui->RADIO_AUX3_CH->hide();
    ui->label_RADIO_AUX3_CH->hide();
    ui->ind_RADIO_AUX3_CH->hide();
    ui->DOWNLINK_BAUD->hide();
    ui->label_DOWNLINK_BAUD->hide();
    ui->MOT_MIN->hide();
    ui->label_MOT_MIN->hide();
    ui->CTRL_FACT_RUDD->hide();
    //ui->label_CTRL_FACT_RUDD->hide();
    ui->cmdBtn_ConvertTov68AttPIDs->hide();
    ui->container_RADIO_FLAP_CH->hide();


    // save this for easy iteration later
    allRadioChanCombos.append(ui->groupBox_channelMapping->findChildren<QComboBox *>(QRegExp("^(RADIO|NAV|GMBL)_.+_(CH|CHAN)$")));
    allRadioChanCombos.append(ui->groupBox_channelMapping->findChildren<QComboBox *>(QRegExp("^(NAV|GMBL|SPVR)_CTRL_[A-Z0-9_]+$")));
    allRadioChanCombos.append(ui->groupBox_tuningChannels->findChildren<QComboBox *>(QRegExp("^(QUATOS_.+_KNOB|CONFIG_ADJUST_P[0-9]+_chan)$")));

    // populate a bunch of meta data for every radio channel combo box to make validation and channel value displays much quicker
    {
        QString paramName;
        QSpinBox *val_sb = NULL;
		  QComboBox *pos_cb = NULL;
        QDoubleSpinBox *scale_sb = NULL;
        QPushButton *param_pb = NULL;
        QWidget *indicator;

        foreach (QComboBox* cb, allRadioChanCombos) {
            paramName = cb->objectName();
            paramName.replace("_chan", "");
            indicator = cb->parent()->findChild<QWidget *>("ind_" % paramName);
            val_sb = cb->parent()->findChild<QSpinBox *>(paramName % "_val");
            param_pb = cb->parent()->findChild<QPushButton *>(paramName);
            cb->setProperty("paramName", paramName);
            cb->setProperty("existsOnboard", cb->isVisible());
            cb->setProperty("indicator_ptr", QVariant::fromValue<void *>(indicator));
            cb->setProperty("btn_ptr", QVariant::fromValue<void *>(param_pb));
            cb->setProperty("value_ptr", QVariant::fromValue<void *>(val_sb));
            if (param_pb) {
                scale_sb = cb->parent()->findChild<QDoubleSpinBox *>(paramName % "_scale");
                param_pb->setProperty("channel_ptr", QVariant::fromValue<void *>(cb));
                param_pb->setProperty("scale_ptr", QVariant::fromValue<void *>(scale_sb));
                param_pb->setProperty("paramName", paramName);
                param_pb->setProperty("paramValue", 0);
                param_pb->setEnabled(false);
                connect(param_pb, SIGNAL(clicked(bool)), this, SLOT(onTunableParamBtnClick()));
                connect(scale_sb, SIGNAL(editingFinished()), this, SLOT(checkTunableParamsChanged()));
                connect(cb, SIGNAL(activated(int)), this, SLOT(checkTunableParamsChanged()));
            }
            if (val_sb) {
					 pos_cb = cb->parent()->findChild<QComboBox *>(paramName % "_pos");
					 if (pos_cb) {
						 pos_cb->insertItems(0, QStringList() << tr("High") << tr("Mid") << tr("Low"));
						 cb->setProperty("swpos_ptr", QVariant::fromValue<void *>(pos_cb));
						 pos_cb->setProperty("paramName", paramName);
						 pos_cb->setProperty("existsOnboard", true);
						 pos_cb->setProperty("value_ptr", QVariant::fromValue<void *>(val_sb));
						 connect(pos_cb, SIGNAL(activated(int)), this, SLOT(onSwitchPositionChanged()));
						 allRadioSwitchValueBoxes.append(val_sb);
					 }
					 val_sb->setProperty("paramName", paramName);
					 val_sb->setProperty("existsOnboard", true);
					 val_sb->setProperty("swpos_ptr", QVariant::fromValue<void *>(pos_cb));
					 connect(val_sb, SIGNAL(editingFinished()), this, SLOT(onSwitchValueChanged()));
            }
            if (indicator) {
                indicator->setProperty("paramName", paramName);
                indicator->setProperty("status", 0);
                indicator->setProperty("value", 0);
                indicator->setEnabled(false);
                if (!cb->property("default_port").isValid())
                    indicator->hide();
                if (qobject_cast<QProgressBar *>(indicator))
                    indicator->setProperty("ind_type", "bar");
                else if (indicator->property("styleType").toString() == "indicator")
                    indicator->setProperty("ind_type", "fill");
                else if (indicator->property("styleType").toString() == "tunableValue")
                    indicator->setProperty("ind_type", "value_lbl");
            }
            connect(cb, SIGNAL(activated(int)), this, SLOT(validateRadioSettings()));
        }
    }

    setConnectedSystemInfoDefaults();
    setHardwareInfo();  // populate hardware (AQ board) info with defaults
    setFirmwareInfo();  // set defaults based on fw version
    adjustUiForHardware();
    adjustUiForFirmware();
    setupRadioPorts();
    showStatusMessage();
    ui->widget_buttonBox->setEnabled(false);

#ifdef INCLUDE_ESC32V2_UI
    esc32Cfg = new AQEsc32ConfigWidget(this);
    esc32Cfg->setBaudRates(baudRates);
    esc32Cfg->setOutputBrowser(ui->textFlashOutput);
    esc32Cfg->setAqBinFolderPath(aqBinFolderPath);
    esc32Cfg->setLastFilePath(LastFilePath);
    connect(esc32Cfg, SIGNAL(flashFwStart()), this, SLOT(flashFwStart()));

    ui->comboBox_fwType->addItem(tr("ESC32 Serial"), "esc32");
    ui->tab_aq_settings->addTab(esc32Cfg, tr("ESC32v2 Settings")); // add esc tab
#endif

#ifdef INCLUDE_DEVEL_WIDGET
    develWdgt = new AQDevelWidget(this);
    ui->tab_aq_settings->addTab(develWdgt, tr("Devel")); // add devel tab
#endif

    // done setting up UI //


    delayedSendRCTimer.setInterval(800);  // timer for sending radio freq. update value

    // Signal handlers

    connect(this, SIGNAL(hardwareInfoUpdated()), this, SLOT(adjustUiForHardware()));
    connect(this, SIGNAL(firmwareInfoUpdated()), this, SLOT(adjustUiForFirmware()));

    //
    // GUI slots

    // splitter
    connect(splitterToggleBtn, SIGNAL(clicked()), this, SLOT(splitterCollapseToggle()));
    connect(ui->splitter_aqWidgetSidebar, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));

    connect(ui->RADIO_TYPE, SIGNAL(currentIndexChanged(int)), this, SLOT(radioType_changed(int)));
    connect(ui->RADIO_SETUP, SIGNAL(currentIndexChanged(int)), this, SLOT(radioType_changed(int)));
    connect(ui->comboBox_radioSetup2, SIGNAL(currentIndexChanged(int)), this, SLOT(radioType_changed(int)));
    connect(ui->comboBox_multiRadioMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setupRadioPorts()));
    connect(ui->SelectFirmwareButton, SIGNAL(clicked()), this, SLOT(selectFWToFlash()));
    connect(ui->portName, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->comboBox_fwPortSpeed, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(flashFW()));
    connect(ui->comboBox_fwType, SIGNAL(currentIndexChanged(int)), this, SLOT(fwTypeChange()));
    connect(ui->toolButton_fwReloadPorts, SIGNAL(clicked()), this, SLOT(setupPortList()));
    connect(ui->toolButton_toggleRadioGraph, SIGNAL(clicked(bool)),this,SLOT(onToggleRadioValuesRefresh(bool)));
    connect(ui->checkBox_showAdvRadioCfg, SIGNAL(clicked(bool)), this, SLOT(toggleRadioSwitchAdvancedSetup(bool)));
    // collapsible group boxes
    connect(ui->groupBox_gimbal, SIGNAL(toggled(bool)), this, SLOT(toggleGroupBox(bool)));
    connect(ui->groupBox_autoTrigger, SIGNAL(toggled(bool)), this, SLOT(toggleGroupBox(bool)));
    connect(ui->groupBox_addlRadioControls, SIGNAL(toggled(bool)), this, SLOT(toggleGroupBox(bool)));
    connect(ui->groupBox_tuningChannels, SIGNAL(toggled(bool)), this, SLOT(toggleGroupBox(bool)));

    connect(ui->pushButton_save_to_aq, SIGNAL(clicked()),this,SLOT(saveAQSettings()));
    connect(ui->cmdBtn_ConvertTov68AttPIDs, SIGNAL(clicked()), this, SLOT(convertPidAttValsToFW68Scales()));
    // timer
    connect(&delayedSendRCTimer, SIGNAL(timeout()), this, SLOT(sendRcRefreshFreq()));

    // MainWindow slots
    connect(this, SIGNAL(remoteGuidanceEnabledChanged(bool)), MainWindow::instance(), SLOT(setRemoteGuidanceEnabled(bool)));

    //Process Slots
    ps_master.setProcessChannelMode(QProcess::MergedChannels);
    connect(&ps_master, SIGNAL(finished(int)), this, SLOT(prtstexit(int)));
    connect(&ps_master, SIGNAL(readyReadStandardOutput()), this, SLOT(prtstdout()));
//    connect(&ps_master, SIGNAL(readyReadStandardError()), this, SLOT(prtstderr()));
    connect(&ps_master, SIGNAL(error(QProcess::ProcessError)), this, SLOT(extProcessError(QProcess::ProcessError)));

    setupPortList();
    loadSettings();

    // connect some things only after settings are loaded to prevent erroneous signals
    connect(ui->spinBox_rcGraphRefreshFreq, SIGNAL(valueChanged(int)), this, SLOT(delayedSendRcRefreshFreq()));

    // UAS slots
    setActiveUAS(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)), Qt::UniqueConnection);
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(uasDeleted(UASInterface*)));

    m_initComplete = true;
}

QGCAutoquad::~QGCAutoquad()
{
    if (ps_master.state() == QProcess::Running)
        ps_master.close();
//    if (ps_tracking.state() == QProcess::Running)
//        ps_tracking.close();

    writeSettings();
    if (aqPwmPortConfig)
        aqPwmPortConfig->deleteLater();
#ifdef INCLUDE_ESC32V2_UI
    if (esc32Cfg)
        esc32Cfg->deleteLater();
#endif
    delete ui;
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
    buttonBoxResized();
    emit visibilityChanged(true);
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

void QGCAutoquad::resizeEvent(QResizeEvent *event)
{
    buttonBoxResized();
    QWidget::resizeEvent(event);
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

    ui->portName->setCurrentIndex(ui->portName->findText(settings.value("FW_FLASH_PORT_NAME", "").toString()));
    ui->comboBox_fwPortSpeed->setCurrentIndex(ui->comboBox_fwPortSpeed->findText(settings.value("FW_FLASH_BAUD_RATE", 115200).toString()));

    if (settings.contains("AUTOQUAD_FW_FILE") && settings.value("AUTOQUAD_FW_FILE").toString().length()) {
        ui->fileLabel->setText(settings.value("AUTOQUAD_FW_FILE").toString());
        ui->fileLabel->setToolTip(settings.value("AUTOQUAD_FW_FILE").toString());
        ui->checkBox_verifyFwFlash->setChecked(settings.value("AUTOQUAD_FW_VERIFY", true).toBool());
        setFwType();
    }

    LastFilePath = settings.value("AUTOQUAD_LAST_PATH").toString();

    if (settings.contains("SETTINGS_SPLITTER_SIZES")) {
        ui->splitter_aqWidgetSidebar->restoreState(settings.value("SETTINGS_SPLITTER_SIZES").toByteArray());
        splitterMoved();
    }

    ui->tabWidget_aq_left->setCurrentIndex(settings.value("SETTING_SELECTED_LEFT_TAB", 0).toInt());
	 //ui->toolButton_toggleRadioGraph->setChecked(settings.value("RADIO_VALUES_UPDATE_BTN_STATE", true).toBool());
     ui->groupBox_addlRadioControls->setChecked(settings.value("ADDL_RADIO_CONTROLS_GRP_STATE", false).toBool());
     ui->groupBox_tuningChannels->setChecked(settings.value("TUNABLE_PARAMS_GRP_STATE", false).toBool());
     ui->groupBox_gimbal->setChecked(settings.value("GIMBAL_AXES_GRP_STATE", false).toBool());
     ui->groupBox_autoTrigger->setChecked(settings.value("AUTO_TRIGGERS_GRP_STATE", false).toBool());
     ui->checkBox_showAdvRadioCfg->setChecked(settings.value("SHOW_ADVANCED_SWITCH_SETUP", false).toBool());
	 toggleRadioSwitchAdvancedSetup(ui->checkBox_showAdvRadioCfg->isChecked());

    settings.endGroup();
    settings.sync();
}

void QGCAutoquad::writeSettings()
{
    //QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

    settings.setValue("APP_VERSION", QGCAUTOQUAD::APP_VERSION);

    settings.setValue("AUTOQUAD_FW_FILE", ui->fileLabel->text());
    settings.setValue("AUTOQUAD_FW_VERIFY", ui->checkBox_verifyFwFlash->isChecked());
    settings.setValue("FW_FLASH_PORT_NAME", ui->portName->currentText());
    settings.setValue("FW_FLASH_BAUD_RATE", ui->comboBox_fwPortSpeed->currentText());

    settings.setValue("AUTOQUAD_LAST_PATH", LastFilePath);

    settings.setValue("SETTINGS_SPLITTER_SIZES", ui->splitter_aqWidgetSidebar->saveState());
    settings.setValue("SETTING_SELECTED_LEFT_TAB", ui->tabWidget_aq_left->currentIndex());
	 //settings.setValue("RADIO_VALUES_UPDATE_BTN_STATE", ui->toolButton_toggleRadioGraph->isChecked());
	 settings.setValue("ADDL_RADIO_CONTROLS_GRP_STATE", ui->groupBox_addlRadioControls->isChecked());
	 settings.setValue("TUNABLE_PARAMS_GRP_STATE", ui->groupBox_tuningChannels->isChecked());
	 settings.setValue("GIMBAL_AXES_GRP_STATE", ui->groupBox_gimbal->isChecked());
	 settings.setValue("AUTO_TRIGGERS_GRP_STATE", ui->groupBox_autoTrigger->isChecked());
	 settings.setValue("SHOW_ADVANCED_SWITCH_SETUP", ui->checkBox_showAdvRadioCfg->isChecked());

    settings.sync();
    settings.endGroup();
}


//
// UI handlers
//

void QGCAutoquad::splitterCollapseToggle() {
	 QList<int> sz = ui->splitter_aqWidgetSidebar->sizes();
	 static int leftW = qMax(sz.at(0), ui->tabWidget_aq_left->minimumWidth());
	 QList<int> newsz;
	 if (sz.at(0) > 0) {
		  leftW = sz.at(0);
		  newsz << 0 << leftW + sz.at(1);
		  splitterToggleBtn->setArrowType(Qt::RightArrow);
	 } else {
		  newsz << leftW << sz.at(1) - leftW;
		  splitterToggleBtn->setArrowType(Qt::LeftArrow);
	 }
	 ui->splitter_aqWidgetSidebar->setSizes(newsz);
     buttonBoxResized();
}

void QGCAutoquad::splitterMoved() {
	 if (ui->splitter_aqWidgetSidebar->sizes().at(0) > 0)
		  splitterToggleBtn->setArrowType(Qt::LeftArrow);
	 else
         splitterToggleBtn->setArrowType(Qt::RightArrow);

     buttonBoxResized();
}

void QGCAutoquad::buttonBoxResized()
{
    Qt::ToolButtonStyle tbs;
    int base = 500;
    int w = ui->widget_buttonBox->size().width();

    ui->btn_paramsLoadSD->setVisible(w > base);
    ui->btn_paramsSaveSD->setVisible(w > base);

    tbs = (w > (base += 100)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsRefresh->setToolButtonStyle(tbs);

    tbs = (w > (base += 100)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsLoadDefault->setToolButtonStyle(tbs);

    tbs = (w > (base += 80)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsLoadFlash->setToolButtonStyle(tbs);

    tbs = (w > (base += 60)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsLoadFile->setToolButtonStyle(tbs);

    tbs = (w > (base += 60)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsSaveFile->setToolButtonStyle(tbs);

    tbs = (w > (base += 60)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsSaveFlash->setToolButtonStyle(tbs);

    tbs = (w > (base += 80)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsLoadSD->setToolButtonStyle(tbs);

    tbs = (w > (base += 60)) ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    ui->btn_paramsSaveSD->setToolButtonStyle(tbs);

}

void QGCAutoquad::toggleGroupBox(bool on, QGroupBox *gb)
{
    if (!gb && !sender())
        return;

    QGroupBox *box;
    if (gb)
        box = gb;
    else if (!(box = qobject_cast<QGroupBox *>(sender())))
        return;

    foreach (QWidget *w, box->findChildren<QWidget *>(QRegExp("^widget_"), Qt::FindDirectChildrenOnly))
        w->setVisible(on);

    MG::UTIL::refreshStyleOnWidget(box);
}

void QGCAutoquad::adjustUiForHardware()
{
    //ui->groupBox_commSerial2->setVisible(!aqHardwareVersion || aqHardwareVersion == 6);
    ui->groupBox_commSerial3->setVisible(aqHardwareVersion == 7);
    ui->groupBox_commSerial4->setVisible(aqHardwareVersion == 7);
    QVariant v(0);
    if (aqHardwareVersion == 8)
		  v.setValue(1 | 32);  // Qt::ItemIsSelectable | Qt::ItemIsEnabled
    ui->MOT_ESC_TYPE->model()->setData(ui->MOT_ESC_TYPE->model()->index(1, 0), v, Qt::UserRole - 1);
}

void QGCAutoquad::adjustUiForFirmware()
{
    setupRadioTypes();

    ui->RADIO_TYPE->setVisible(!useRadioSetupParam);
    ui->label_RADIO_TYPE->setVisible(!useRadioSetupParam);
    ui->comboBox_radioSetup2->setVisible(useRadioSetupParam);
    ui->label_radioSetup2->setVisible(useRadioSetupParam);
//    ui->comboBox_multiRadioMode->setVisible(useRadioSetupParam);
//    ui->label_multiRadioMode->setVisible(useRadioSetupParam);
    ui->groupBox_tuningChannels->setVisible(usingQuatos || useTunableParams);
    ui->checkBox_showAdvRadioCfg->setVisible(useNewControlsScheme);

    // radio loss stage 2 failsafe options
    uint8_t idx = ui->SPVR_FS_RAD_ST2->currentIndex();
    ui->SPVR_FS_RAD_ST2->clear();
    ui->SPVR_FS_RAD_ST2->addItem("Land", 0);
    ui->SPVR_FS_RAD_ST2->addItem("RTH, Land", 1);
    if (!aqBuildNumber || aqBuildNumber >= 1304)
        ui->SPVR_FS_RAD_ST2->addItem("Ascend, RTH, Land", 2);
    if (idx < ui->SPVR_FS_RAD_ST2->count())
        ui->SPVR_FS_RAD_ST2->setCurrentIndex(idx);

    // gimbal auto-triggering options
	 ui->groupBox_autoTrigger->setVisible(!aqBuildNumber || aqBuildNumber >= 1378);

    // param widget buttons
    if (paramaq) {
        paramaq->setRestartBtnEnabled(aqCanReboot);
        paramaq->setCalibBtnsEnabled(!aqBuildNumber || aqBuildNumber >= 1760);
    }

}

void QGCAutoquad::adjustUiForQuatos()
{
    ui->widget_attitude_pid->setVisible(!usingQuatos);
    ui->widget_attitude_quatos->setVisible(usingQuatos);
    ui->groupBox_tuningChannels->setVisible(usingQuatos || useTunableParams);
    ui->container_QUATOS_AM1_KNOB->setVisible(usingQuatos && !useTunableParams);
    ui->widget_motor_esc_quatos->setVisible(usingQuatos);
    if (usingQuatos)
        ui->radioButton_attitude_quatos->setChecked(true);
    else
        ui->radioButton_attitude_pid->setChecked(true);
}

void QGCAutoquad::on_tab_aq_settings_currentChanged(int idx)
{
    Q_UNUSED(idx);
    QWidget *arg1 = ui->tab_aq_settings->currentWidget();
    bool vis = !(arg1->objectName() == "AQEsc32ConfigWidget");
    ui->widget_buttonBox->setVisible(vis);
}

void QGCAutoquad::on_SPVR_FS_RAD_ST2_currentIndexChanged(int index)
{
    ui->label_SPVR_FS_ADD_ALT->setVisible(index == 2);
    ui->SPVR_FS_ADD_ALT->setVisible(index == 2);
}

void QGCAutoquad::on_MOT_ESC_TYPE_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    ui->checkBox_escCalibration->setChecked(false);
    ui->checkBox_escCalibration->setEnabled(!ui->MOT_ESC_TYPE->currentIndex());
    ui->groupBox_escPwm->setEnabled(ui->MOT_ESC_TYPE->currentIndex() != 1);
}


//
// Radio Config UI
//

void QGCAutoquad::setupRadioTypes()
{
    uint8_t idx = ui->RADIO_TYPE->currentIndex(),
            idx2 = ui->RADIO_SETUP->currentIndex(),
            idx3 = ui->comboBox_radioSetup2->currentIndex();

    // which radio types are available
    QMap<int, QString> radioTypes;
    radioTypes.insert(0, tr("No Radio"));
    radioTypes.insert(1, tr("Spektrum 11Bit"));
    radioTypes.insert(2, tr("Spektrum 10Bit"));
    radioTypes.insert(3, tr("S-BUS (Futaba, others)"));
    radioTypes.insert(4, tr("PPM"));
    if (!aqBuildNumber || aqBuildNumber >= 1149)
        radioTypes.insert(5, tr("SUMD (Graupner)"));
    if (!aqBuildNumber || aqBuildNumber >= 1350)
        radioTypes.insert(6, tr("M-Link (Multiplex)"));
    if (!aqHardwareVersion || aqHardwareVersion == 8) {
        if (!aqHardwareRevision || aqHardwareRevision < 6)
            radioTypes.insert(7, tr("Deltang (onboard M4v1)"));
        if (!aqHardwareRevision || aqHardwareRevision >= 6)
            radioTypes.insert(8, tr("CYRF (onboard M4v2+)"));
    }

    ui->RADIO_TYPE->blockSignals(true);
    ui->RADIO_SETUP->blockSignals(true);
    ui->comboBox_radioSetup2->blockSignals(true);

    ui->RADIO_TYPE->clear();
    ui->RADIO_SETUP->clear();
    ui->comboBox_radioSetup2->clear();

    QMapIterator<int, QString> i(radioTypes);
    while (i.hasNext()) {
        i.next();
        ui->RADIO_TYPE->addItem(i.value(), i.key()-1);
        ui->RADIO_SETUP->addItem(i.value(), i.key());
        ui->comboBox_radioSetup2->addItem(i.value(), i.key());
    }

    if (idx < ui->RADIO_TYPE->count())
        ui->RADIO_TYPE->setCurrentIndex(idx);
    if (idx2 < ui->RADIO_SETUP->count())
        ui->RADIO_SETUP->setCurrentIndex(idx2);
    if (idx3 < ui->comboBox_radioSetup2->count())
        ui->comboBox_radioSetup2->setCurrentIndex(idx3);

    ui->RADIO_TYPE->blockSignals(false);
    ui->RADIO_SETUP->blockSignals(false);
    ui->comboBox_radioSetup2->blockSignals(false);
}

void QGCAutoquad::setupRadioPorts()
{
    int i, cidx, strtat;
    int pcount = 18;
//    QStringList ports;

    if (ui->comboBox_multiRadioMode->currentIndex())
        pcount = 36;

    foreach (QComboBox* cb, allRadioChanCombos) {
        cidx = cb->currentIndex();
        cb->blockSignals(true);
        cb->clear();
        strtat = 0;
        if (!cb->property("no_off").isValid()) {
            cb->addItem(tr("Off"), 0);
            strtat = 1;
        }
        for (i = 1; i <= pcount; ++i)
            cb->addItem(QString::number(i), strtat++);
        if (cidx < 0 || cidx >= cb->count()) {
            if (cb->property("default_port").isValid())
                cidx = cb->property("default_port").toUInt();
            else
                cidx = 0;
        }
        cb->setCurrentIndex(cidx);
        cb->blockSignals(false);
    }
    validateRadioSettings();

}

bool QGCAutoquad::radioHasPPM()
{
    bool hasPPM = (!useRadioSetupParam && ui->RADIO_TYPE->itemData(ui->RADIO_TYPE->currentIndex()).toInt() == 3) ||
            (useRadioSetupParam && (
                ui->RADIO_SETUP->itemData(ui->RADIO_SETUP->currentIndex()).toInt() == 4 ||
                ui->comboBox_radioSetup2->itemData(ui->comboBox_radioSetup2->currentIndex()).toInt() == 4 ));

    return hasPPM;
}

void QGCAutoquad::radioType_changed(int idx) {

    Q_UNUSED(idx)
    emit hardwareInfoUpdated();

    if (radioHasPPM()) { // PPM
          ui->groupBox_ppmOptions->show();
    } else {
          ui->groupBox_ppmOptions->hide();
    }

    if (useRadioSetupParam && ui->RADIO_SETUP->currentIndex() > 0 && ui->comboBox_radioSetup2->currentIndex() > 0) {
        ui->comboBox_multiRadioMode->show();
        ui->label_multiRadioMode->show();
    } else {
        ui->comboBox_multiRadioMode->setCurrentIndex(0);
        ui->comboBox_multiRadioMode->hide();
        ui->label_multiRadioMode->hide();
    }

//    if (!paramaq)
//        return;

//    bool ok;
//    int prevRadioValue;
//    int newRadioValue;

//    if (useRadioSetupParam) {
//        prevRadioValue = paramaq->getParaAQ("RADIO_SETUP").toInt(&ok);
//        newRadioValue = calcRadioSetting();
//    } else {
//        prevRadioValue = paramaq->getParaAQ("RADIO_TYPE").toInt(&ok);
//        newRadioValue = ui->RADIO_TYPE->itemData(idx).toInt(&ok);
//    }
}

void QGCAutoquad::on_toolButton_radioHelp_clicked()
{
    QString helpText = QGCCore::readFileToString(QGCCore::getDocsFilePath() % "aq_help_radio-settings");
    MainWindow::instance()->showInfoMessage(tr("Radio Settings Help"), helpText);
}

void QGCAutoquad::on_toolButton_adjParamsHelp_clicked()
{
    QString helpText = QGCCore::readFileToString(QGCCore::getDocsFilePath() % "aq_help_adjustable-params");
    MainWindow::instance()->showInfoMessage(tr("Adjustable Parameters Help"), helpText);
}

void QGCAutoquad::onSwitchValueChanged(QSpinBox *origin, QComboBox *target)
{
	if (!origin || !target) {
		if ( !(origin = qobject_cast<QSpinBox *>(sender())) || !origin->property("swpos_ptr").isValid() ||
			  !(target = static_cast<QComboBox *>(origin->property("swpos_ptr").value<void *>())) )
			return;
	}
	if (!origin || !target)
		return;

	target->blockSignals(true);
	if (origin->value() < -250)
		target->setCurrentIndex(2);
	else if (origin->value() > 250)
		target->setCurrentIndex(0);
	else
		target->setCurrentIndex(1);
	target->blockSignals(false);

	if (!mtx_paramsAreLoading)
		validateRadioSettings();
}

void QGCAutoquad::onSwitchPositionChanged(QComboBox *origin, QSpinBox *target)
{
	if (!origin || !target) {
		if ( !(origin = qobject_cast<QComboBox *>(sender())) || !origin->property("value_ptr").isValid() ||
			  !(target = static_cast<QSpinBox *>(origin->property("value_ptr").value<void *>())) )
			return;
	}
	if (!origin || !target)
		return;

	target->blockSignals(true);
	if (origin->currentIndex() == 0)
		target->setValue(501);
	else if (origin->currentIndex() == 1)
		target->setValue(0);
	else
		target->setValue(-501);
	target->blockSignals(false);

	if (!mtx_paramsAreLoading)
		validateRadioSettings();
}

void QGCAutoquad::onTunableParamBtnClick()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!paramaq || !btn)
        return;

    bool ok;
    uint16_t pVal = btn->property("paramValue").toInt(&ok);
    QString pName = btn->property("paramName").toString();
    if (!ok || pName.isEmpty())
        return;

    if (!m_selectAdjParamsDialog)
        m_selectAdjParamsDialog = new SelectAdjustableParamDialog(this, paramaq);

    int ret = m_selectAdjParamsDialog->selectParam(pName, pVal, aqBuildNumber);

    if (ret == QDialog::Rejected)
        return;

    uint16_t oldVal = pVal;
    pVal = m_selectAdjParamsDialog->selParamId();
    btn->setProperty("paramValue", pVal);
    if (pVal) {
        btn->setText(paramaq->getParameterNameById(MAV_DEFAULT_SYSTEM_COMPONENT, pVal));
    } else
        btn->setText(tr("None"));

    if (oldVal != pVal) {
        validateRadioSettings();
        checkTunableParamsChanged();
    }
}

void QGCAutoquad::toggleRadioSwitchAdvancedSetup(bool on)
{
	QComboBox *cb;
	ui->label_colHdr_switchValue->setVisible(on);
	ui->label_colHdr_switchPos->setVisible(!on);
    ui->label_colHdr_switchValue_2->setVisible(on);
    ui->label_colHdr_switchPos_2->setVisible(!on);
	ui->CTRL_DBAND_SWTCH->setVisible(on);
	ui->label_CTRL_DBAND_SWTCH->setVisible(on);
	if (!on)
		ui->CTRL_DBAND_SWTCH->setValue(250);
	foreach (QSpinBox *sb, allRadioSwitchValueBoxes) {
		if (!sb->property("existsOnboard").toBool() || !(cb = static_cast<QComboBox *>(sb->property("swpos_ptr").value<void *>())))
			continue;
		if (!on)
			onSwitchValueChanged(sb, cb);
		sb->setVisible(on);
		cb->setVisible(!on);
	}
	ui->checkBox_showAdvRadioCfg->setChecked(on);
}

void QGCAutoquad::checkRadioSwitchHasAdvancedSetup()
{
	bool has = false;
	foreach (QSpinBox *sb, allRadioSwitchValueBoxes) {
		if (sb->value() != 0 && sb->value() != -501 && sb->value() != 501) {
			has = true;
			break;
		}
	}
	if (ui->CTRL_DBAND_SWTCH->value() != 250)
		has = true;
	toggleRadioSwitchAdvancedSetup(has);
}

// make sure no radio channel assignments conflict
bool QGCAutoquad::validateRadioSettings(/*int idx*/)
{
    if (mtx_paramsAreLoading)
        return false;

    QList<QString> conflictParams, essentialPorts, conflictSwitchValues /*, conflictPorts, portsUsed, activeTunableParamPorts*/;
    QMultiMap<QString, QPair<int, QString> > switchChannelValues;
    QMultiMap<QString, QString> usedChannelParams;
    QPair<int, QString> val;
    QSpinBox *val_sb = NULL;
	 QComboBox *pos_cb = NULL;
    QPushButton *param_pb = NULL;
    QWidget *indicator;
    QString cbname, cbtxt, paramName;
    uint8_t chan;
    bool ret = true, valid, skip, telemActive;
    int dband = ui->CTRL_DBAND_SWTCH->value();
    int oldval;

    radioChannelIndicatorsMap.clear();

    foreach (QComboBox* cb, allRadioChanCombos) {
        cbname = cb->objectName();
        cbtxt = cb->currentText();
        if (!cbtxt.toUInt(&valid) || !valid || !cb->property("existsOnboard").isValid() || !cb->property("existsOnboard").toBool())
            continue;

        paramName = cbname;
        paramName.replace("_chan", "");
		  val_sb = static_cast<QSpinBox *>(cb->property("value_ptr").value<void *>());
		  param_pb = static_cast<QPushButton *>(cb->property("btn_ptr").value<void *>());

        if (cb->property("essentialPort").isValid())
            essentialPorts.append(cbtxt);

        if (useNewControlsScheme) {
            if (!cb->property("essentialPort").isValid() && cb->property("legacy").isValid())
                continue;
            else if (param_pb && !param_pb->property("paramValue").toUInt())
                continue;

            skip = false;
            if (!essentialPorts.contains(cbtxt) && val_sb) {
                if (switchChannelValues.contains(cbtxt))
                    skip = true;
                foreach (val, switchChannelValues.values(cbtxt)) {
                    if (val.first - dband <= val_sb->value() + dband && val_sb->value() - dband <= val.first + dband) {
                        //skip = false;
                        conflictSwitchValues.append(val_sb->objectName());
                        conflictSwitchValues.append(val.second + "_val");
                        conflictParams.append(cbname);
                        conflictParams.append(val.second);
                    }
                }
                switchChannelValues.insert(cbtxt, QPair<int, QString>(val_sb->value(), cbname));
            }
            if (!skip && usedChannelParams.contains(cbtxt)) {
                conflictParams.append(cbname);
                conflictParams.append(usedChannelParams.values(cbtxt));
            }

        }
        // legacy controls setup
        else {
            if (!cb->property("legacy").isValid() && paramName != "GMBL_PSTHR_CHAN")
                continue;
            if (usedChannelParams.contains(cbtxt)) {
                conflictParams.append(cbname);
                conflictParams.append(usedChannelParams.value(cbtxt));
            }
        }

        usedChannelParams.insert(cbtxt, cbname);
    }

    //qDebug() << usedChannelParams << conflictParams;
    telemActive = ui->toolButton_toggleRadioGraph->isChecked();

    foreach (QComboBox* cb, allRadioChanCombos) {
        if ( !cb->property("existsOnboard").isValid() || !cb->property("existsOnboard").toBool())
            continue;

        cbname = cb->objectName();
        cbtxt = cb->currentText();
        chan = cbtxt.toUInt(&valid);
        paramName = cbname;
        paramName.replace("_chan", "");
		  indicator = static_cast<QWidget *>(cb->property("indicator_ptr").value<void *>());
		  val_sb = static_cast<QSpinBox *>(cb->property("value_ptr").value<void *>());
		  pos_cb = static_cast<QComboBox *>(cb->property("swpos_ptr").value<void *>());

        if (conflictParams.contains(cbname)) {
            if (essentialPorts.contains(cbtxt)) {
                cb->setStyleSheet("background-color: rgba(255, 0, 0, 160)");
                ret = false;
            } else
                cb->setStyleSheet("background-color: rgba(255, 140, 0, 130)");
        } else
            cb->setStyleSheet("");

        if (val_sb) {
			  if (conflictSwitchValues.contains(val_sb->objectName())) {
				  val_sb->setStyleSheet("background-color: rgba(255, 140, 0, 130)");
				  if (pos_cb)
					  pos_cb->setStyleSheet("background-color: rgba(255, 140, 0, 130)");
			  }
			  else {
				  val_sb->setStyleSheet("");
				  if (pos_cb)
					  pos_cb->setStyleSheet("");
			  }
        }

        if (indicator) {
            if (indicator->property("ind_type").toString() == "value_lbl") {
					 param_pb = static_cast<QPushButton *>(cb->property("btn_ptr").value<void *>());
                if (!param_pb->property("paramValue").toUInt())
                    chan = 0;
            }
            if (valid && chan && (useNewControlsScheme || indicator->property("legacy_channel").isValid())) {

                if (!useNewControlsScheme && cb->property("legacy").isValid() && cb->property("default_port").isValid())
                    chan = cb->property("default_port").toUInt() + 1;
                else if (indicator->property("telem_index").isValid())
                    chan = indicator->property("telem_index").toUInt() + 1000;
                if (!radioChannelIndicatorsMap.contains(chan)) {
                    radioChannelIndicatorsMap.insert(chan, QList<QPair<QWidget *, QWidget *> >());
                }
                radioChannelIndicatorsMap[chan].append(QPair<QWidget*, QWidget*>(indicator, val_sb));

                indicator->setVisible(true);
                indicator->setDisabled(!telemActive);
                indicator->setProperty("status", 1);
            }
            else {
                indicator->setDisabled(true);
                indicator->setProperty("status", 0);
                oldval = indicator->property("value").toInt();
                indicator->setProperty("value", 0);
                if (indicator->property("ind_type").toString() == "fill" && oldval)
                    MG::UTIL::refreshStyleOnWidget(indicator);
                indicator->setVisible(false);
            }
        }
    }

    if (!useNewControlsScheme)
        checkLegacyChannelsChanged();

    return ret;
}

bool QGCAutoquad::checkTunableParamsChanged()
{
    bool changed = false;

    if (paramaq && ui->toolButton_toggleRadioGraph->isChecked()) {
        quint32 val;
        QComboBox *tunableValChan;
        QDoubleSpinBox *tunableValDblBox;

        foreach (QWidget *w, ui->groupBox_tuningChannels->findChildren<QWidget *>(paramsTunableControls)) {
            if (w->property("channel_ptr").isValid() && w->property("paramValue").isValid()) {
                val = (w->property("paramValue").toUInt() & 0x3FF);
                tunableValDblBox = static_cast<QDoubleSpinBox *>(w->property("scale_ptr").value<void *>());
                tunableValChan = static_cast<QComboBox *>(w->property("channel_ptr").value<void *>());
                if (tunableValDblBox && tunableValChan)
                    val |= ((tunableValChan->currentIndex() & 0x3F) << 10) | (((quint32)(tunableValDblBox->value() * 10000) & 0xFF) << 16);
                else
                    continue;
                if (val != paramaq->getParaAQ(w->objectName()).toUInt()) {
                    changed = true;
                    break;
                }
            }
        }
    }

    ui->label_adjustParamsChannelSwitchWarning->setVisible(changed);

    if (hasAnyTunableParams() != m_configTelemIsRunning)
        toggleConfigTelemetry(!m_configTelemIsRunning && ui->toolButton_toggleRadioGraph->isChecked());

    return changed;
}

bool QGCAutoquad::checkLegacyChannelsChanged()
{
    bool changed = false;

    if (paramaq && !useNewControlsScheme && ui->toolButton_toggleRadioGraph->isChecked()) {
        quint32 val;
        bool ok;
        foreach (QComboBox* cb, allRadioChanCombos) {
            if (!cb->property("existsOnboard").toBool() || !cb->property("legacy").isValid() || !cb->property("default_port").isValid())
                continue;
            val = cb->currentData().toUInt(&ok);
            if (ok && val != paramaq->getParaAQ(cb->objectName()).toUInt()) {
                changed = true;
                break;
            }
        }
    }

    ui->label_legacyChannelSwitchWarning->setVisible(changed);
    return changed;
}

bool QGCAutoquad::hasAnyTunableParams()
{
    bool ret = false;

    quint32 val;
    QComboBox *tunableValChan;

    foreach (QWidget *w, ui->groupBox_tuningChannels->findChildren<QWidget *>(paramsTunableControls)) {
        if (w->property("channel_ptr").isValid() && w->property("paramValue").isValid()) {
            val = w->property("paramValue").toUInt();
            if (!val)
                continue;
            tunableValChan = static_cast<QComboBox *>(w->property("channel_ptr").value<void *>());
            if (tunableValChan && tunableValChan->currentData().toBool()) {
                ret = true;
                break;
            }
        }
    }
	 if (ret)
		 ui->groupBox_tuningChannels->setChecked(true);

    return ret;
}


//
// FW Flashing
//

void QGCAutoquad::setPortName(QString str)
{
    Q_UNUSED(str);

    portName = ui->portName->itemData(ui->portName->currentIndex()).toString();
//#ifdef Q_OS_WIN
//    portName = portName.split("-").first();
//#endif
//    portName = portName.remove(" ");
//    QString portSpeed = ui->comboBox_fwPortSpeed->currentText();
    ui->portName->setToolTip(ui->portName->currentText());
}

void QGCAutoquad::setupPortList()
{
    QString pdispname;
    QString cidxfw = ui->portName->currentText();
    ui->portName->clear();
    // Get the ports available on this system
    foreach (const QextPortInfo &p, QextSerialEnumerator::getPorts()) {
        if (!p.portName.length())
            continue;
        pdispname = p.portName;
        if (p.friendName.length())
            pdispname += " - " + p.friendName.split(QRegExp(" ?\\(")).first();
        ui->portName->addItem(pdispname, p.portName);
    }
    ui->portName->setCurrentIndex(ui->portName->findText(cidxfw));
}

void QGCAutoquad::setFwType() {
    QString typ = "aq";
    // test for esc32 in the fw file name
    if (ui->fileLabel->text().contains(QRegExp("esc32.+\\.hex$", Qt::CaseInsensitive)))
        typ = "esc32";
    // test for aq M4 or v7/8 hardware in fw file name
    else if (ui->fileLabel->text().contains(QRegExp("(aq|autoquad).+(hwv[78]\\.[\\d]|m4).+\\.bin$", Qt::CaseInsensitive)))
        typ = "dfu";

    ui->comboBox_fwType->setCurrentIndex(ui->comboBox_fwType->findData(typ));
}

void QGCAutoquad::fwTypeChange() {
    bool en = ui->comboBox_fwType->itemData(ui->comboBox_fwType->currentIndex()).toString() != "dfu";
    ui->comboBox_fwPortSpeed->setEnabled(en);
    ui->portName->setEnabled(en);
    ui->label_fwPort->setEnabled(en);
    ui->label_fwPortSpeed->setEnabled(en);
    ui->toolButton_fwReloadPorts->setEnabled(en);
    ui->checkBox_verifyFwFlash->setEnabled(en);
}

void QGCAutoquad::selectFWToFlash()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Firmware File"), dir.absoluteFilePath(),
                                            tr("AQ or ESC32 firmware") + " (*.hex *.bin)");

    if (fileName.length())
    {
        QString fileNameLocale = QDir::toNativeSeparators(fileName);
        QFile file(fileNameLocale);
        if (!file.open(QIODevice::ReadOnly))
        {
            MainWindow::instance()->showInfoMessage(tr("Warning!"), tr("Could not open firmware file. %1").arg(file.errorString()));
            return;
        }
        ui->fileLabel->setText(fileNameLocale);
        ui->fileLabel->setToolTip(fileNameLocale);
        fileToFlash = file.fileName();
        LastFilePath = fileToFlash;
        file.close();

        setFwType();
    }
}

void QGCAutoquad::flashFW()
{
    if (ui->comboBox_fwType->currentIndex() == -1) {
        MainWindow::instance()->showCriticalMessage(tr("Error!"), tr("Please select the firwmare type."));
        return;
    }

    if (checkProcRunning())
        return;

    QString fwtype = ui->comboBox_fwType->itemData(ui->comboBox_fwType->currentIndex()).toString();
    QString msg = "";

    if (fwtype == "dfu") {
        msg += tr("Make sure your AQ is connected via USB and is already in bootloader mode.  To enter bootloader mode,"
                  "first connect the BOOT pins (or hold the BOOT button) and then turn the AQ on.\n\n");
#ifndef Q_OS_WIN32
        msg += tr("Please make sure you have the dfu-util program installed on this computer. See http://dfu-util.gnumonks.org/ ."
                  "Mac OS X users should install via MacPorts (http://www.macports.org/).\n\n");
#else
        msg += tr("An automatic restart of AQ will be attempted after flashing, but may not be possible, depending on the USB driver being used. "
                  "You may see an error message at the end of the flash utility output messages, which can be ignored. In this case, simply restart AQ manually.\n\n");
#endif
    } else { // aq and esc serial flash

        if (!portName.length()) {
            MainWindow::instance()->showCriticalMessage(tr("Error!"), tr("Please select an available COM port."));
            return;
        }

        if ( checkAqSerialConnection(portName) )
            msg = tr("WARNING: You are already connected to AutoQuad. If you continue, you will be disconnected and then re-connected afterwards.\n\n");

        if (fwtype == "aq")
            msg += tr("WARNING: Flashing firmware will reset all AutoQuad settings back to default values. Make sure you have your generated parameters and custom settings saved.\n\n");
        else
            msg += tr("WARNING: Flashing firmware will reset all ESC32 settings back to default values. Make sure you have your custom settings saved.\n\n");

        msg += tr("Make sure you are using the %1 port.\n").arg(portName);
        msg += tr("There is a delay before the flashing process shows any progress. Please wait at least 20sec. before you retry!\n\n");
    }
    msg += "Do you wish to continue flashing?";

    QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Firmware Flashing"), msg, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
    if (qrply == QMessageBox::Cancel)
        return;

    if (connectedLink)
        connectedLink->disconnect();

    activeProcessStatusWdgt = ui->textFlashOutput;
    fwFlashActive = true;

    if (fwtype == "aq")
        flashFwStart();
    else if (fwtype == "dfu")
        flashFwDfu();
#ifdef INCLUDE_ESC32V2_UI
    else {
        esc32Cfg->setPortName(portName);
        esc32Cfg->flashFWEsc32();
    }
#endif
}


void QGCAutoquad::flashFwStart()
{
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "stm32flash" + platformExeExt);
    QStringList Arguments;
    Arguments.append(QString("-b"));
    Arguments.append(ui->comboBox_fwPortSpeed->currentText());
    Arguments.append(QString("-w"));
    Arguments.append(QDir::toNativeSeparators(ui->fileLabel->text()));
    if (ui->fileLabel->text().endsWith(".bin", Qt::CaseInsensitive))
        Arguments.append("-s 0x08000000");
    if (ui->checkBox_verifyFwFlash->isChecked())
        Arguments.append("-v");
    Arguments.append(portName);

    QString cmdLine = AppPath;
    foreach (const QString arg, Arguments)
        cmdLine += " " + arg;
    ui->textFlashOutput->append(cmdLine + "\n\n");

    ps_master.start(AppPath , Arguments, QProcess::Unbuffered | QProcess::ReadWrite);
}

void QGCAutoquad::flashFwDfu()
{
#ifdef Q_OS_WIN32
    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "dfu-util" + platformExeExt);
#else
    QString AppPath = "dfu-util";
#endif
    QStringList Arguments;
    Arguments.append("-a 0");                   // alt 0 is start of internal flash
    Arguments.append("-d 0483:df11" );          // device ident stm32
    Arguments.append("-s 0x08000000:leave");    // start address (:leave to exit DFU mode after flash)
    //Arguments.append("-v");                   // verbose
    Arguments.append("-R");                     // reset after upload
    Arguments.append("-D");                     // firmware file
    Arguments.append(QDir::toNativeSeparators(ui->fileLabel->text()));

    QString cmdLine = AppPath;
    foreach (const QString arg, Arguments)
        cmdLine += " " + arg;
    ui->textFlashOutput->append(cmdLine + "\n\n");

    ps_master.start(AppPath , Arguments, QProcess::Unbuffered | QProcess::ReadWrite);
}


//
// Radio values view
//

void QGCAutoquad::toggleRadioValuesUpdate(bool enable)
{
    if (!uas)
        enable = false;

    ui->toolButton_toggleRadioGraph->setChecked(enable);
    ui->label_colHdr_adjValsInd_1->setEnabled(enable);
    ui->label_colHdr_adjValsInd_2->setEnabled(enable);
    ui->label_colHdr_switchInd_1->setEnabled(enable);
    ui->label_colHdr_switchInd_2->setEnabled(enable);

    bool active;
    int oldval;
    QWidget *w;
    QPair<QWidget *, QWidget *> pair;
    QMapIterator<uint8_t, QList<QPair<QWidget *, QWidget *> > > i(radioChannelIndicatorsMap);
    while (i.hasNext()) {
        i.next();
        foreach (pair, radioChannelIndicatorsMap.value(i.key())) {
            w = pair.first;
            if (!w)
                continue;
            active = w->property("status").toBool();
            w->setEnabled(enable && active);
            oldval = pair.first->property("value").toInt();
            pair.first->setProperty("value", 0);
            if (pair.first->property("ind_type").toString() == "fill" && oldval)
                MG::UTIL::refreshStyleOnWidget(pair.first);
            else if (pair.first->property("ind_type").toString() == "value_lbl")
                pair.first->setProperty("text", "N/A");
        }
    }

    toggleConfigTelemetry(enable);
    checkTunableParamsChanged();
    checkLegacyChannelsChanged();
}

void QGCAutoquad::onToggleRadioValuesRefresh(const bool on)
{
    if (!on || !uas)
        toggleRadioValuesUpdate(false);
    else if (!ui->spinBox_rcGraphRefreshFreq->value())
        ui->spinBox_rcGraphRefreshFreq->setValue(4);

    toggleRadioStream(on);
}

void QGCAutoquad::toggleRadioStream(const bool enable)
{
    if (uas)
        uas->enableRCChannelDataTransmission(enable ? ui->spinBox_rcGraphRefreshFreq->value() : 0);
}

void QGCAutoquad::toggleConfigTelemetry(bool enable)
{
    if (!hasAnyTunableParams())
        enable = false;
    if (uas && useTunableParams && enable != m_configTelemIsRunning) {
        uas->startStopTelemetry(enable, 1000000 / ui->spinBox_rcGraphRefreshFreq->value(), AUTOQUADMAV::AQMAV_DATASET_CONFIG);
        if (enable)
            connect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, SLOT(getNewTelemetryF(int,mavlink_aq_telemetry_f_t)));
        else
            disconnect(uas, SIGNAL(TelemetryChangedF(int,mavlink_aq_telemetry_f_t)), this, 0);
        m_configTelemIsRunning = enable;
    }
}

void QGCAutoquad::delayedSendRcRefreshFreq()
{
    delayedSendRCTimer.start();
}

void QGCAutoquad::sendRcRefreshFreq()
{
    delayedSendRCTimer.stop();
    toggleRadioValuesUpdate(ui->spinBox_rcGraphRefreshFreq->value());
    toggleRadioStream(ui->spinBox_rcGraphRefreshFreq->value());
}

void QGCAutoquad::setRadioChannelDisplayValue(int channelId, float normalized)
{
    ++channelId;
    if (!this->isVisible() || !radioChannelIndicatorsMap.contains(channelId))
        return;

    // make sure UI is enabled if data is coming in
    if (!ui->toolButton_toggleRadioGraph->isChecked())
        toggleRadioValuesUpdate(true);

    QProgressBar *bar;
    QSpinBox *sb;
    int oldval;
    int tempval;
    int val = (int)(normalized-1024);
    int dband = ui->CTRL_DBAND_SWTCH->value();

    QPair<QWidget *, QWidget *> pair;
    foreach (pair, radioChannelIndicatorsMap.value(channelId)) {
        if (!pair.first)
            continue;
        tempval = val;

        if (pair.first->property("ind_type").toString() == "bar" && (bar = qobject_cast<QProgressBar *>(pair.first))) {
            bar->setValue(qMax(bar->minimum(), qMin(val, bar->maximum())));
        }
        else if (pair.first->property("ind_type").toString() == "value_lbl")
            pair.first->setProperty("text", QString::number(normalized));
        else if (pair.second && (sb = qobject_cast<QSpinBox *>(pair.second))) {
            if (val >= sb->value() - dband && val <= sb->value() + dband)
                tempval = 1;
            else
                tempval = 0;
        }
        oldval = pair.first->property("value").toInt();
        pair.first->setProperty("value", tempval);

        if (oldval != tempval && pair.first->property("ind_type").toString() == "fill") {
            MG::UTIL::refreshStyleOnWidget(pair.first);
        }
    }

}

void QGCAutoquad::setRssiDisplayValue(float normalized)
{
    if (!ui->progressBar_rssi->isVisible())
        return;

    ui->progressBar_rssi->setValue(qMax(ui->progressBar_rssi->minimum(), qMin((int)(normalized), ui->progressBar_rssi->maximum())));
}

void QGCAutoquad::getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values)
{
    if (!uas || uasId != uas->getUASID() || !this->isVisible() || values.Index != AUTOQUADMAV::AQMAV_DATASET_CONFIG)
        return;

    if (!ui->toolButton_toggleRadioGraph->isChecked()) {
        m_configTelemIsRunning = true;
        toggleConfigTelemetry(false);
    }

    int i = 998;
    setRadioChannelDisplayValue(++i, values.value2);
    setRadioChannelDisplayValue(++i, values.value4);
    setRadioChannelDisplayValue(++i, values.value6);
    setRadioChannelDisplayValue(++i, values.value8);
    setRadioChannelDisplayValue(++i, values.value10);
    setRadioChannelDisplayValue(i, values.value12);
}

//
// UAS Interfaces
//

void QGCAutoquad::setActiveUAS(UASInterface* uas_ext)
{
    if (!uas_ext)
        return;

    removeActiveUAS();
    uas = uas_ext;
    paramaq = new QGCAQParamWidget(uas, this);
    ui->label_params_no_aq->hide();
    ui->tabLayout_paramHandler->addWidget(paramaq);
    if ( LastFilePath == "")
        paramaq->setFilePath(QCoreApplication::applicationDirPath());
    else
        paramaq->setFilePath(LastFilePath);

    // reset system info of connected AQ
    setConnectedSystemInfoDefaults();
    // do this before we recieve any data stream announcements or messages
    onToggleRadioValuesRefresh(ui->toolButton_toggleRadioGraph->isChecked());
    ui->widget_buttonBox->setEnabled(true);

    uas->editCmdResponseFilter(MAV_CMD_AQ_TELEMETRY, true);

    connect(uas, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(handleStatusText(int, int, int, QString)));
    connect(uas, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(setRssiDisplayValue(float)));
    connect(uas, SIGNAL(dataStreamAnnounced(int,uint8_t,uint16_t,bool)), this, SLOT(dataStreamUpdate(int,uint8_t,uint16_t,bool)));
    connect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setRadioChannelDisplayValue(int,float)));
    connect(uas, SIGNAL(heartbeatTimeout(bool,unsigned int)), this, SLOT(setUASstatus(bool,unsigned int)));
    connect(uas, SIGNAL(systemVersionChanged(int,uint32_t,uint32_t,QString,QString)), this, SLOT(uasVersionChanged(int,uint32_t,uint32_t,QString,QString)));

    connect(paramaq, SIGNAL(parameterListUpToDate(uint8_t)), this, SLOT(onParametersLoaded(uint8_t)));
    connect(paramaq, SIGNAL(paramWriteCompleted(int,int,QString)), this, SLOT(onParamWriteCompleted(int,int,QString)));
    connect(paramaq, SIGNAL(parameterListRequested(uint8_t)), this, SLOT(uasConnected(uint8_t)));

#ifdef INCLUDE_DEVEL_WIDGET
    develWdgt->setUas(uas);
#endif

    paramaq->requestParameterList(MAV_DEFAULT_SYSTEM_COMPONENT);

    VisibleWidget = 2;
}

void QGCAutoquad::uasDeleted(UASInterface *mav)
{
    if (uas && mav && mav == uas) {
        removeActiveUAS();
    }
}

void QGCAutoquad::removeActiveUAS()
{
    if (uas) {
        disconnect(uas, 0, this, 0);
        if (paramaq) {
            disconnect(paramaq, 0, this, 0);
            ui->tabLayout_paramHandler->removeWidget(paramaq);
            ui->label_params_no_aq->show();
            paramaq->deleteLater();
            paramaq = NULL;
        }
        uas = NULL;
        toggleRadioValuesUpdate(false);
        ui->widget_buttonBox->setEnabled(false);
    }
}

void QGCAutoquad::setUASstatus(bool timeout, unsigned int ms)
{
    Q_UNUSED(ms);
    if (!uas)
        return;

    ui->widget_buttonBox->setEnabled(!timeout);
    if (timeout)
        toggleRadioValuesUpdate(false);
}

void QGCAutoquad::uasVersionChanged(int uasId, uint32_t fwVer, uint32_t hwVer, QString fwVerStr, QString hwVerStr)
{
    Q_UNUSED(hwVerStr)
    if (!uas || uasId != uas->getUASID())
        return;

    aqFirmwareVersion = fwVerStr;
    aqBuildNumber = fwVer & 0xFFFF;
    aqHardwareVersion = (hwVer >> 24) & 0xFF;
    aqHardwareRevision = (hwVer >> 16) & 0xFF;

    setHardwareInfo();
    setFirmwareInfo();

    QString verStr = QString("AQ FW: v");
    QString ttStr = "AQ Firmare Version: v";
    QString t;
    if (aqFirmwareVersion.length()) {
        t = QString("%1").arg(aqFirmwareVersion);
        verStr += t;
        ttStr += t % QString(" (%1.%2.%3)").arg((fwVer >> 24) & 0xFF).arg((fwVer >> 16) & 0xFF).arg(aqBuildNumber);
    }  else {
        t += tr(" [unknown]");
        verStr += t;
        ttStr += t;
    }
    verStr += " HW: v";
    ttStr += "\nAQ Hardware Version: v";
    if (aqHardwareVersion) {
        t = QString("%1").arg(QString::number(aqHardwareVersion));
        if (aqHardwareRevision > -1)
            t += QString(".%1").arg(QString::number(aqHardwareRevision));
    } else
        t = tr(" [unknown]");
    verStr += t;
    ttStr += t;

    ui->lbl_aq_fw_version->setText(verStr);
    ui->lbl_aq_fw_version->setToolTip(ttStr);

}

void QGCAutoquad::dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off)
{
    if (uas && uas->getUASID() == uasId) {
        if (stream_id == MAV_DATA_STREAM_RC_CHANNELS) {
            if (on_off)
                ui->spinBox_rcGraphRefreshFreq->setValue(rate);
            toggleRadioValuesUpdate(on_off);
        }
        else if (stream_id == MAV_DATA_STREAM_EXTRA3)
            m_configTelemIsRunning = on_off && rate;
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

void QGCAutoquad::getGUIpara(QWidget *parent) {
    if ( !paramaq || !parent)
        return;

    bool ok;
    int precision, tmp;
    unsigned utmp;
    QString paraName, valstr;
    QVariant val;
    QLabel *paraLabel;
    QWidget *paraContainer;
    QWidget *paraIndicator;
    QSpinBox *swValBox;
	 QComboBox *swPosBox;
    QComboBox *tunableValChan;
    QDoubleSpinBox *tunableValDblBox;

    // handle all input widgets
    QList<QWidget*> wdgtList = parent->findChildren<QWidget *>(fldnameRx);
    foreach (QWidget* w, wdgtList) {
        paraName = paramNameGuiToOnboard(w->objectName());
        paraLabel = parent->findChild<QLabel *>(QString("label_%1").arg(w->objectName()));
        paraContainer = parent->findChild<QWidget *>(QString("container_%1").arg(w->objectName()));
        paraIndicator = NULL;
        if (w->property("indicator_ptr").isValid())
				paraIndicator = static_cast<QWidget *>(w->property("indicator_ptr").value<void *>());
        swValBox = NULL;
		  swPosBox = NULL;
		  if (w->property("value_ptr").isValid()) {
			  swValBox = static_cast<QSpinBox *>(w->property("value_ptr").value<void *>());
			  swPosBox = static_cast<QComboBox *>(w->property("swpos_ptr").value<void *>());
		  }

        if (!paramaq->paramExistsAQ(paraName)) {
            w->hide();
            w->setProperty("existsOnboard", false);
            if (paraLabel)
                paraLabel->hide();
            if (paraContainer)
                paraContainer->hide();
				if (swValBox) {
					swValBox->hide();
					swValBox->setProperty("existsOnboard", false);
				}
				if (swPosBox) {
					swPosBox->hide();
					swPosBox->setProperty("existsOnboard", false);
				}
            if (paraIndicator)
                paraIndicator->hide();
            if (w->property("channel_ptr").isValid()) {
					w = static_cast<QWidget *>(w->property("channel_ptr").value<void *>());
               if (w)
                   w->setProperty("existsOnboard", false);
            }
            continue;
        }

        w->show();
        w->setProperty("existsOnboard", true);
        if (paraLabel){
            paraLabel->setToolTip(paraName);
            paraLabel->show();
        }
        if (paraContainer)
            paraContainer->show();
		  if (swValBox) {
			  swValBox->show();
			  swValBox->setProperty("existsOnboard", true);
		  }
		  if (swPosBox) {
			  swPosBox->show();
			  swPosBox->setProperty("existsOnboard", false);
		  }
        if (paraIndicator)
            paraIndicator->show();

        ok = true;
        precision = 6;
        val = paramaq->getParaAQ(paraName);
        if (paraName == "GMBL_SCAL_PITCH" || paraName == "GMBL_SCAL_ROLL")
            val = fabs(val.toFloat());
        else if (paraName == "RADIO_SETUP")
            val = val.toInt() & 0x0f;
        else if (w->property("value_ptr").isValid()) {
            utmp = val.toUInt();
            val = utmp & 0xFF;
            if (val.toBool() && swValBox) {
                tmp = (utmp >> 8) & 0x7FF;
                if (!(utmp & (1<<19)))
                    tmp = -tmp;
                swValBox->setValue(tmp);
            }
        }
        else if (w->property("channel_ptr").isValid()) {
            utmp = val.toUInt();
            val = utmp & 0x3FF; // param ID
            if (val.toInt())
                valstr = paramaq->getParameterNameById(MAV_DEFAULT_SYSTEM_COMPONENT, val.toInt());
            else
                valstr = tr("None");
            tmp = (utmp >> 10) & 0x3F; // channel
				tunableValDblBox = static_cast<QDoubleSpinBox *>(w->property("scale_ptr").value<void *>());
				tunableValChan = static_cast<QComboBox *>(w->property("channel_ptr").value<void *>());
            if (tunableValDblBox && tunableValChan) {
                tunableValChan->setProperty("existsOnboard", true);
                tunableValChan->setCurrentIndex(tmp);
                if (val.toBool())
                    tunableValDblBox->setValue(((utmp >> 16) & 0xFF) / 10000.0f);
            }
        }

        if (QLineEdit* le = qobject_cast<QLineEdit *>(w)){
            valstr.setNum(val.toFloat(), 'g', precision);
            le->setText(valstr);
            le->setValidator(new QDoubleValidator(-1000000.0, 1000000.0, 8, le));
        } else if (QComboBox* cb = qobject_cast<QComboBox *>(w)) {
            if (cb->isEditable()) {
                if ((tmp = cb->findText(val.toString())) > -1)
                    cb->setCurrentIndex(tmp);
                else {
                    cb->insertItem(0, val.toString());
                    cb->setCurrentIndex(0);
                }
            }
            else if ((tmp = cb->findData(val)) > -1)
                cb->setCurrentIndex(tmp);
            else
                cb->setCurrentIndex(abs(val.toInt(&ok)));
        } else if (QDoubleSpinBox* dsb = qobject_cast<QDoubleSpinBox *>(w)) {
            dsb->setValue(val.toDouble(&ok));
        }
        else if (QSpinBox* sb = qobject_cast<QSpinBox *>(w))
            sb->setValue(val.toInt(&ok));
        else if (QPushButton* pb = qobject_cast<QPushButton *>(w)) {
            pb->setText(valstr);
            pb->setProperty("paramName", paraName);
            pb->setProperty("paramValue", val);
            pb->setEnabled(true);
        }
        else
            continue;

        //if (!ok)
            // TODO: notify the user, or something...
    }

    if (parent->objectName() == "tab_aq_settings") {
        // radio port 2 and mode select boxes
        if (useRadioSetupParam) {
            tmp = paramaq->getParaAQ("RADIO_SETUP").toInt();
            ui->comboBox_radioSetup2->setCurrentIndex(ui->comboBox_radioSetup2->findData((tmp >> 4) & 0x0f));
            ui->comboBox_multiRadioMode->setCurrentIndex(ui->comboBox_multiRadioMode->findData((tmp >> 12) & 0x0f));
        }

        // gimbal pitch/roll revese checkboxes
        ui->reverse_gimbal_pitch->setChecked(paramaq->getParaAQ("GMBL_SCAL_PITCH").toFloat() < 0);
        ui->reverse_gimbal_roll->setChecked(paramaq->getParaAQ("GMBL_SCAL_ROLL").toFloat() < 0);
        ui->checkBox_escCalibration->setChecked(paramaq->paramExistsAQ("MOT_ESC_TYPE") && (paramaq->getParaAQ("MOT_ESC_TYPE").toUInt() & 0x800000));

        on_SPVR_FS_RAD_ST2_currentIndexChanged(ui->SPVR_FS_RAD_ST2->currentIndex());
        on_MOT_ESC_TYPE_currentIndexChanged(0);
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
//        qDebug() << paraName << val;

        foreach (QAbstractButton* abtn, g->buttons()) {
            if (paramaq->paramExistsAQ(paraName)) {
					 //abtn->setEnabled(true);
                if (g->exclusive()) { // individual values
                    abtn->setChecked(val.toInt() == g->id(abtn));
                } else { // bitmask
                    abtn->setChecked((val.toInt() & g->id(abtn)));
                }
				}
//				else {
//                abtn->setEnabled(false);
//            }
        }
    }
}

void QGCAutoquad::onParametersLoaded(uint8_t component)
{
    if (component != MAV_DEFAULT_SYSTEM_COMPONENT)
        return;

    showStatusMessage();

    motPortTypeCAN = paramaq->paramExistsAQ("MOT_CAN") || paramaq->paramExistsAQ("MOT_CANL");
    motPortTypeCAN_H = paramaq->paramExistsAQ("MOT_CANH");
    maxMotorPorts = paramaq->paramExistsAQ("MOT_PWRD_16_P") ? 16 : 14;
    useRadioSetupParam = paramaq->paramExistsAQ("RADIO_SETUP");
    useNewControlsScheme = paramaq->paramExistsAQ("NAV_CTRL_PH");
    useTunableParams = paramaq->paramExistsAQ("CONFIG_ADJUST_P1");
    remoteGuidanceEnabled = paramaq->paramExistsAQ("NAV_CTRL_GUIDED");

    emit firmwareInfoUpdated();
    emit remoteGuidanceEnabledChanged(remoteGuidanceEnabled);

    loadParametersToUI();
}

void QGCAutoquad::loadParametersToUI() {

    mtx_paramsAreLoading = true;
    getGUIpara(ui->tab_aq_settings);
    populateButtonGroups(this);
    aqPwmPortConfig->loadOnboardConfig();

    // check for old PIDs and offer to convert them if running newer firmware
    // TODO: remove me eventually
    if (paramaq->paramExistsAQ("MOT_CAN") &&
            paramaq->getParaAQ("CTRL_TLT_RTE_D").toFloat() < 15000.0f)
        ui->cmdBtn_ConvertTov68AttPIDs->show();
    else
        ui->cmdBtn_ConvertTov68AttPIDs->hide();

    // convert old radio type value if switching to new system
    if (useRadioSetupParam && paramaq->getParaAQ("RADIO_SETUP").toInt() == 0 && paramaq->paramExistsAQ("RADIO_TYPE")) {
        int idx = ui->RADIO_SETUP->findData(paramaq->getParaAQ("RADIO_TYPE").toInt() + 1);
        ui->RADIO_SETUP->setCurrentIndex(idx);
        radioType_changed(idx);
    }

    if (paramaq->paramExistsAQ("QUATOS_ENABLE"))
        setAqHasQuatos(paramaq->getParaAQ("QUATOS_ENABLE").toBool());

    mtx_paramsAreLoading = false;
    paramsLoadedForAqBuildNumber = aqBuildNumber;
	 checkRadioSwitchHasAdvancedSetup();
    validateRadioSettings();
    checkTunableParamsChanged();
    checkLegacyChannelsChanged();

}

bool QGCAutoquad::checkAqConnected(bool interactive) {

    if ( !paramaq || !uas || uas->getCommunicationStatus() != uas->COMM_CONNECTED ) {
        if (interactive)
            MainWindow::instance()->showCriticalMessage("Error", "No AutoQuad connected!");
        return false;
    } else
        return true;
}

void QGCAutoquad::setAqHasQuatos(const bool yes)
{
    if (usingQuatos != yes) {
        usingQuatos = yes;
        emit aqHasQuatosChanged(usingQuatos);
    }
    adjustUiForQuatos();
}

bool QGCAutoquad::saveSettingsToAq(QWidget *parent, bool interactive)
{
    float val_uas, val_local;
    quint32 utmp;
    QString paraName;
    QSpinBox *swValBox;
    QComboBox *tunableValChan;
    QDoubleSpinBox *tunableValDblBox;
    QStringList errors;
    bool ok, chkstate;
    quint8 errLevel = 0;  // 0=no error; 1=soft error; 2=hard error
    QMap<QString, QPair<float, float> > changeList; // param name, old val, new val
    QMessageBox msgBox;
    QVariant tmp;

    if ( !checkAqConnected(interactive) )
        return false;

    QList<QWidget*> wdgtList = parent->findChildren<QWidget *>(fldnameRx);
    QList<QObject*> objList = *reinterpret_cast<QList<QObject *>*>(&wdgtList);
    if (!QString::compare(parent->objectName(), "tab_aq_settings")) {
        QList<QButtonGroup *> grpList = this->findChildren<QButtonGroup *>(fldnameRx);
        objList.append(*reinterpret_cast<QList<QObject *>*>(&grpList));
    }

    foreach (QObject* w, objList) {
        paraName = paramNameGuiToOnboard(w->objectName());

        if (!paramaq->paramExistsAQ(paraName))
            continue;

        ok = true;
        val_uas = paramaq->getParaAQ(paraName).toFloat(&ok);

        if (QLineEdit* le = qobject_cast<QLineEdit *>(w))
            val_local = le->text().toFloat(&ok);
        else if (QComboBox* cb = qobject_cast<QComboBox *>(w)) {
            if (cb->isEditable()) {
                val_local = cb->currentText().toFloat(&ok);
            }
            else {
                tmp = cb->itemData(cb->currentIndex());
                if (tmp.isValid())
                    val_local = tmp.toFloat(&ok);
                else
                    val_local = static_cast<float>(cb->currentIndex());
            }
        } else if (QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox *>(w))
            val_local = (float)sb->value();
        else if (QSpinBox* sb = qobject_cast<QSpinBox *>(w))
            val_local = (float)sb->value();
        else if (QButtonGroup* bg = qobject_cast<QButtonGroup *>(w)) {
            val_local = 0.0f;
            foreach (QAbstractButton* abtn, bg->buttons()) {
                if (abtn->isChecked()) {
                    if (bg->exclusive()) {
                        val_local = bg->id(abtn);
                        break;
                    } else
                        val_local += bg->id(abtn);
                }
            }
        }
        else if (QPushButton* pb = qobject_cast<QPushButton *>(w))
            val_local = pb->property("paramValue").toFloat(&ok);
        else
            continue;

        if (!ok){
            errors.append(paraName);
            continue;
        }

        // special case for reversing gimbal servo direction
        if (paraName == "GMBL_SCAL_PITCH" || paraName == "GMBL_SCAL_ROLL" || paraName == "SIG_BEEP_PRT") {
            chkstate = false;
            if (paraName == "GMBL_SCAL_PITCH")
                chkstate = parent->findChild<QCheckBox *>("reverse_gimbal_pitch")->checkState();
            else if (paraName == "GMBL_SCAL_ROLL")
                chkstate = parent->findChild<QCheckBox *>("reverse_gimbal_roll")->checkState();
            else if (paraName == "SIG_BEEP_PRT")
                chkstate = parent->findChild<QCheckBox *>("checkBox_useSpeaker")->checkState();

            if (chkstate)
                val_local = 0.0f - val_local;
        }
        else if (paraName == "RADIO_SETUP") {
            val_local = (float)calcRadioSetting();
        }
        else if (paraName == "MOT_ESC_TYPE" && ui->checkBox_escCalibration->isChecked()) {
            val_local = ((uint32_t)val_local | 0x800000);
        }
        else if (w->property("value_ptr").isValid()) {
            utmp = ((quint32)val_local & 0xFF);
				swValBox = static_cast<QSpinBox *>(w->property("value_ptr").value<void *>());
            if (swValBox) {
                utmp |= abs(swValBox->value()) << 8;
                if (swValBox->value() > 0)
                    utmp |= (1<<19);
            }
            val_local = utmp;
        }
        else if (w->property("channel_ptr").isValid()) {
            utmp = ((quint32)val_local & 0x3FF);
				tunableValDblBox = static_cast<QDoubleSpinBox *>(w->property("scale_ptr").value<void *>());
				tunableValChan = static_cast<QComboBox *>(w->property("channel_ptr").value<void *>());
            if (tunableValDblBox && tunableValChan)
                utmp |= ((tunableValChan->currentIndex() & 0x3F) << 10) | (((quint32)(tunableValDblBox->value() * 10000) & 0xFF) << 16);
            val_local = utmp;
        }

        if (fabs(val_uas - val_local) > 2.0f * FLT_EPSILON)
            changeList.insert(paraName, QPair<float, float>(val_uas, val_local));
    }

    if (errors.size()) {
        errors.insert(0, tr("One or more parameter(s) could not be saved:"));
        if (errors.size() >= changeList.size())
            errLevel = 2;
        else
            errLevel = 1;
    }

    errLevel = aqPwmPortConfig->saveOnboardConfig(&changeList, &errors);

    if (errLevel) {

        if (errLevel > 1){
            msgBox.setText(tr("Parameter save error.").leftJustified(300, ' '));
            msgBox.setInformativeText(tr("Cannot save due to error(s). Click the Details button for more information."));
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.setIcon(QMessageBox::Critical);
        } else {
            msgBox.setText(tr("Parameter save warning.").leftJustified(300, ' '));
            msgBox.setInformativeText(tr("Possible problem(s) exist. Click the Details button for more information.\n\nDo you wish to ignore this and continue saving?"));
            msgBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            msgBox.setIcon(QMessageBox::Warning);
        }
        msgBox.setDetailedText(errors.join("\n\n"));

        int ret = msgBox.exec();
        if (errLevel > 1 || ret == QMessageBox::Cancel)
            return false;

    }

    return saveParamChanges(changeList, interactive);
}

void QGCAutoquad::saveAQSettings() {
    if (!validateRadioSettings()) {
        MainWindow::instance()->showCriticalMessage(tr("Error"), tr("You have the same port assigned to multiple controls!"));
        return;
    }
    saveSettingsToAq(ui->tab_aq_settings);
}

bool QGCAutoquad::saveParamChanges(QMap<QString, QPair<float, float> > changeList, bool interactive)
{
    if ( changeList.size() ) {
        paramSaveType = 1;  // save to volatile
        restartAfterParamSave = false;

        if (interactive && !showSaveDialog(changeList))
            return false;

        QMapIterator<QString, QPair<float, float> > i(changeList);
        while (i.hasNext()) {
            i.next();
            paramaq->setParaAQ(i.key(), i.value().second);
        }

        if (paramSaveType == 2) {
            uas->writeParametersToStorageAQ();
        }

        if (restartAfterParamSave) {
            showStatusMessage(tr("Restarting flight controller..."), 6000);
            QTimer::singleShot(2000, paramaq, SLOT(restartUas()));
        }

        return true;
    }
    else {
        if (interactive)
            MainWindow::instance()->showInfoMessage(tr("Warning"), tr("No changed parameters detected.  Nothing to save."));
        return false;
    }
}

bool QGCAutoquad::showSaveDialog(QMap<QString, QPair<float, float> > changeList)
{
    QString msg, msgBoxText, val1, val2, restartFlag;
    bool restartRequired = false;

    paramSaveType = 0;

    msgBoxText = tr("%n parameter(s) modified:<br>", "one or more params have changed", changeList.size());
    msg = tr("<table border=\"0\"><thead><tr><th>Parameter </th><th>Old Value </th><th>New Value </th></tr></thead><tbody>\n");

    QMapIterator<QString, QPair<float, float> > i(changeList);
    while (i.hasNext()) {
        i.next();
        val1.setNum(i.value().first, 'g', 8);
        val2.setNum(i.value().second, 'g', 8);
        restartFlag = "";
        // check if restart is required for this param
        if (i.key().contains(paramsReqRestartRx)) {
            restartRequired = true;
            restartFlag = "* ";
        }

        msg += QString("<tr><td style=\"padding: 1px 7px 0 1px;\"><span style=\"color: rgba(255, 0, 0, 200); font-weight: bold;\">%1</span>%2</td><td>%3 </td><td>%4</td></tr>\n").arg(restartFlag, i.key(), val1, val2);
    }

    msg += "</tbody></table>\n";
    if (restartRequired)
        msgBoxText += "<span style=\"color: rgba(255, 0, 0, 200); font-weight: bold;\">* restart required</span>";

    QDialog* dialog = new QDialog(this);
    dialog->setSizeGripEnabled(true);
    dialog->setWindowTitle(tr("Verify Changed Parameters"));
    dialog->setWindowModality(Qt::ApplicationModal);
    dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSizePolicy sizepol(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::Label);
    sizepol.setVerticalStretch(0);
    QLabel* prompt = new QLabel(msgBoxText, dialog);
    prompt->setTextFormat(Qt::RichText);
    prompt->setSizePolicy(sizepol);
    QLabel* prompt2 = new QLabel(tr("Do you wish to continue?"), dialog);
    prompt2->setSizePolicy(sizepol);

    QCheckBox* restartOption = new QCheckBox(tr("Restart after save?"), dialog);
    restartOption->setToolTip(tr("<html><p>Selecting this option will attempt to automatically restart the flight controller after saving parameters. \
                                 Only do this when saving to permanent memory.  You may loose the link to the flight controller and need to reconnect.</p></html>"));
    restartOption->setObjectName("chkbox_restart");
    restartOption->setSizePolicy(sizepol);
    restartOption->setVisible(aqCanReboot);

    QTextEdit* message = new QTextEdit(msg, dialog);
    message->setReadOnly(true);
    message->setAcceptRichText(true);

    QDialogButtonBox* bbox = new QDialogButtonBox(Qt::Horizontal, dialog);
    QPushButton *btn_saveToRam = bbox->addButton(tr("Save to Volatile Memory"), QDialogButtonBox::AcceptRole);
    btn_saveToRam->setToolTip(tr("The settings will be immediately active and persist UNTIL the flight controller is restarted."));
    btn_saveToRam->setObjectName("btn_saveToRam");
    btn_saveToRam->setAutoDefault(false);
    QPushButton *btn_saveToRom = bbox->addButton(tr("Save to Permanent Memory"), QDialogButtonBox::AcceptRole);
    btn_saveToRom->setToolTip(tr("The settings will be immediately active and persist AFTER flight controller is restarted."));
    btn_saveToRom->setObjectName("btn_saveToRom");
    btn_saveToRom->setAutoDefault(false);
    QPushButton *btn_cancel = bbox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    btn_cancel->setToolTip(tr("Do not save any settings."));
    btn_cancel->setDefault(true);

    QVBoxLayout* dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setSpacing(8);
    dlgLayout->addWidget(prompt);
    dlgLayout->addWidget(message);
    QHBoxLayout* promptLayout = new QHBoxLayout;
    promptLayout->setSpacing(8);
    promptLayout->addWidget(prompt2);
    promptLayout->addWidget(restartOption);
    promptLayout->setAlignment(restartOption, Qt::AlignRight);
    dlgLayout->addLayout(promptLayout);
    dlgLayout->addWidget(bbox);

    dialog->setLayout(dlgLayout);

    connect(btn_cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect(btn_saveToRam, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(btn_saveToRom, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(bbox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(saveDialogButtonClicked(QAbstractButton*)));
    connect(restartOption, SIGNAL(clicked(bool)), this, SLOT(saveDialogRestartOptionChecked(bool)));

    bool dlgret = dialog->exec();
    dialog->deleteLater();

    return (dlgret != QDialog::Rejected && paramSaveType);
}

void QGCAutoquad::saveDialogButtonClicked(QAbstractButton* btn) {
    paramSaveType = 0;
    if (btn->objectName() == "btn_saveToRam")
        paramSaveType = 1;
    else if (btn->objectName() == "btn_saveToRom")
        paramSaveType = 2;
}

void QGCAutoquad::saveDialogRestartOptionChecked(bool chk) {
    restartAfterParamSave = chk;
}

void QGCAutoquad::onParametersChanged(int component, QMap<QString, QPair<float, float> > changes)
{
    if (component == MAV_DEFAULT_SYSTEM_COMPONENT)
        saveParamChanges(changes, true);
}

QString QGCAutoquad::paramNameGuiToOnboard(QString paraName) {
    paraName = paraName.replace(dupeFldnameRx, "");

    if (!paramaq)
        return paraName;

    // check for old param names
    QString tmpstr;
    if (paraName.indexOf(QRegExp("NAV_ALT_SPED_.+")) > -1 && !paramaq->paramExistsAQ(paraName)){
        tmpstr = paraName.replace(QRegExp("NAV_ALT_SPED_(.+)"), "NAV_ATL_SPED_\\1");
        if (paramaq->paramExistsAQ(tmpstr))
            paraName = tmpstr;
    }
    else if (paraName.indexOf(QRegExp("QUATOS_.+")) > -1 && !paramaq->paramExistsAQ(paraName)) {
        tmpstr = paraName.replace(QRegExp("QUATOS_(.+)"), "L1_ATT_\\1");
        if (paramaq->paramExistsAQ(tmpstr))
            paraName = tmpstr;
    }

    // ignore depricated radio_type param
    if (paraName == "RADIO_TYPE" && useRadioSetupParam)
        paraName += "_void";

    return paraName;
}

int QGCAutoquad::calcRadioSetting()
{
    int radioSetup = ui->RADIO_SETUP->itemData(ui->RADIO_SETUP->currentIndex()).toInt() |
                    (ui->comboBox_radioSetup2->itemData(ui->comboBox_radioSetup2->currentIndex()).toInt() << 4) |
                    (ui->comboBox_multiRadioMode->itemData(ui->comboBox_multiRadioMode->currentIndex()).toInt() << 12);

    //qDebug() << radioSetup;
    return radioSetup;
}

void QGCAutoquad::convertPidAttValsToFW68Scales() {
    float v;
    bool ok;
    QList<QLineEdit *> attPIDs = this->findChildren<QLineEdit *>(QRegExp("^CTRL_(TLT_(RTE|ANG)|YAW_RTE)_[PIDOM]{1,2}$"));
    foreach (QLineEdit* le, attPIDs) {
        v = le->text().toFloat(&ok);
        if (ok)
            le->setText(QString::number(v * 4.82f));
    }
    // don't forget CTRL_MAX which is a int spin box
    ui->CTRL_MAX->setValue(ui->CTRL_MAX->value() * 4.82f);

    if (ok) {
        QString msg = tr("<html><p>The <b>Tilt Rate</b>, <b>Tilt Angle</b>, and <b>Yaw Rate</b> PIDs, and the <b>Max. Ctrl. Per Axis</b> (CTRL_MAX) parameter have been converted \
                and are displayed here, but have NOT been sent to AQ (Ctrl. Max. is shown on the Radio & Controls setup screen).</p>\
                <p>To return to the old values, simply refresh the onboard parameters list.</p>\
                <p>Please note that the conversions are approximate. Each value (except the F term!) has been multipled by 4.82 You may want to round some of the numbers a bit.</p>\
                <p>You may also wish to refer to the <a href='http://code.google.com/p/autoquad/source/diff?spec=svn234&r=234&format=side&path=/trunk/onboard/config_default.h#sc_svn233_59'>\
                original code changes</a> for reference.</p></html>");
        MainWindow::instance()->showInfoMessage(tr("Attitude PID values converted."), msg);
        ui->cmdBtn_ConvertTov68AttPIDs->hide();
    }
}


void QGCAutoquad::on_btn_paramsRefresh_clicked()
{
    if (!paramaq)
        return;
    paramaq->requestParameterList(MAV_DEFAULT_SYSTEM_COMPONENT);
}

void QGCAutoquad::on_btn_paramsLoadDefault_clicked()
{
    if (!paramaq)
        return;
    paramaq->loadOnboardDefaults();
}

void QGCAutoquad::on_btn_paramsLoadFlash_clicked()
{
    if (!paramaq)
        return;
    paramaq->readParameters();
}

void QGCAutoquad::on_btn_paramsSaveFlash_clicked()
{
    if (!uas)
        return;
    uas->writeParametersToStorageAQ();
}

void QGCAutoquad::on_btn_paramsLoadSD_clicked()
{
    if (!paramaq)
        return;
    paramaq->loadParaFromSD();
}

void QGCAutoquad::on_btn_paramsSaveSD_clicked()
{
    if (!paramaq)
        return;
    paramaq->saveParaToSD();
}

void QGCAutoquad::on_btn_paramsLoadFile_clicked()
{
    if (!paramaq)
        return;

    connect(paramaq, SIGNAL(parametersChanged(int,QMap<QString,QPair<float,float> >)), this, SLOT(onParametersChanged(int,QMap<QString,QPair<float,float> >)));
    bool ret = paramaq->loadParameters();
    disconnect(paramaq, SIGNAL(parametersChanged(int,QMap<QString,QPair<float,float> >)), this, 0);
    if (!ret)
        showStatusMessage(tr("No changed parameters detected, nothing to load."), 4000);
}

void QGCAutoquad::on_btn_paramsSaveFile_clicked()
{
    if (!paramaq)
        return;
    paramaq->saveParameters();
}


//
// Miscellaneous
//


bool QGCAutoquad::checkProcRunning(bool warn) {
    if (ps_master.state() == QProcess::Running) {
        if (warn)
            MainWindow::instance()->showCriticalMessage(
                        tr("Process already running."),
                        tr("There appears to be an external process (calculation step or firmware flashing) already running. Please abort it first."));
        return true;
    }
    return false;
}

void QGCAutoquad::prtstexit(int stat) {
    prtstdout();
    if ( fwFlashActive ) {  // firmware flashing mode
        ui->flashButton->setEnabled(true);
        if (!stat)
            MainWindow::instance()->showInfoMessage(tr("Restart the device."), tr("Please cycle power to the AQ/ESC or press the AQ reset button to reboot."));
        fwFlashActive = false;
        if (connectedLink) {
            connectedLink->connect();
        }
    }
}

void QGCAutoquad::prtstdout() {
    QString output = ps_master.readAllStandardOutput();
    if (output.contains(QRegExp("\\[(uWrote|H)"))) {
        output = output.replace(QRegExp(".\\[[uH]"), "");
        activeProcessStatusWdgt->clear();
    }
    activeProcessStatusWdgt->insertPlainText(output);
    activeProcessStatusWdgt->ensureCursorVisible();
}


/**
 * @brief Translate process error code to a useful messgae
 * @param err Error code
 */
QString QGCAutoquad::extProcessErrorToString(QProcess::ProcessError err)
{
    QString msg;
    switch(err) {
        case QProcess::FailedToStart:
            msg = tr("Failed to start.");
            break;
        case QProcess::Crashed:
            msg = tr("Process terminated (aborted or crashed).");
            break;
        case QProcess::Timedout:
            msg = tr("Timeout waiting for process.");
            break;
        case QProcess::WriteError:
            msg = tr("Cannot write to process, exiting.");
            break;
        case QProcess::ReadError:
            msg = tr("Cannot read from process, exiting.");
            break;
        default:
            msg = tr("Unknown error");
            break;
    }
    return msg;
}

/**
 * @brief Handle external process error code
 * @param err Error code
 */
void QGCAutoquad::extProcessError(QProcess::ProcessError err) {
    activeProcessStatusWdgt->append(extProcessErrorToString(err));
    fwFlashActive = false;
}


void QGCAutoquad::uasConnected(uint8_t component)
{
    if (component == MAV_DEFAULT_SYSTEM_COMPONENT) {
        uas->sendCommmandToAq(MAV_CMD_AQ_REQUEST_VERSION, 1);
        showStatusMessage(tr("Parameter list requested, please wait..."));
    }
}

void QGCAutoquad::setConnectedSystemInfoDefaults()
{
    aqFirmwareVersion = QString("");
    aqFirmwareRevision = 0;
    aqHardwareVersion = 0;
    aqHardwareRevision = 0;
    aqBuildNumber = 0;
    maxMotorPorts = 16;
    motPortTypeCAN = true;
    motPortTypeCAN_H = true;
    useRadioSetupParam = true;
    aqCanReboot = false;
    useNewControlsScheme = true;
    useTunableParams = true;
    paramsLoadedForAqBuildNumber = 0;

    setAqHasQuatos(false);  // assume no Quatos unless told otherwise for this UAV
    ui->lbl_aq_fw_version->setText("AutoQuad Firmware v. [unknown]");

    emit firmwareInfoUpdated();
}

void QGCAutoquad::setHardwareInfo()
{
    pwmPortTimers.clear();
    switch (aqHardwareVersion) {
     case 8:
        maxPwmPorts = 9;
        pwmPortTimers << 3 << 3 << 4 << 4 << 4 << 4 << 8 << 8 << 9;
        break;
    case 7:
        maxPwmPorts = 9;
        pwmPortTimers << 1 << 1 << 1 << 1 << 4 << 4 << 9 << 9 << 11;
        break;
    case 6:
    default:
        maxPwmPorts = 14;
        pwmPortTimers << 1 << 1 << 1 << 1 << 4 << 4 << 4 << 4 << 9 << 9 << 2 << 2 << 10 << 11;
        break;
    }
    emit hardwareInfoUpdated();
}

void QGCAutoquad::setFirmwareInfo()
{
    if (aqBuildNumber) {
        motPortTypeCAN_H = (aqBuildNumber >= 1663);
        maxMotorPorts = (aqBuildNumber < 1423) ? 14 : 16;
        motPortTypeCAN = (aqBuildNumber >= 1418);
        useRadioSetupParam = (aqBuildNumber >= 1790);
    }
    aqCanReboot = (aqBuildNumber >= 1740);
    emit firmwareInfoUpdated();
}

QStringList QGCAutoquad::getAvailablePwmPorts(void) {
    QStringList portsList;
    unsigned short maxport = maxPwmPorts;

    if (radioHasPPM())
        maxport--;

    for (int i=1; i <= maxport; i++)
        portsList.append(QString::number(i));

    return portsList;
}

void QGCAutoquad::handleStatusText(int uasId, int compid, int severity, QString text) {
    Q_UNUSED(severity);
    Q_UNUSED(compid);
    if (uasId != uas->getUASID())
        return;

    if (text.contains("Quatos enabled", Qt::CaseInsensitive)) {
        setAqHasQuatos(true);
    }
}

bool QGCAutoquad::checkAqSerialConnection(QString port) {
    bool IsConnected = false;
    connectedLink = NULL;

    if (!checkAqConnected(false))
        return false;

    if ( uas != NULL ) {
        for ( int i=0; i < uas->getLinks()->count(); i++) {
            connectedLink = uas->getLinks()->at(i);
            //qDebug() << connectedLink->getName();
            if ( connectedLink->isConnected() == true && (port == "" ||  connectedLink->getName().contains(port))) {
                IsConnected = true;
                break;
            }
        }
    }
    if (!IsConnected)
        connectedLink = NULL;

    return IsConnected;
}

void QGCAutoquad::onParamWriteCompleted(int component, int writeStatCode, QString writeStatus)
{
    Q_UNUSED(component)
    showStatusMessage(writeStatus, writeStatCode ? 0 : 4000);
    qDebug() << writeStatus;
}

void QGCAutoquad::showStatusMessage(const QString &msg, const uint32_t timeout)
{
    if (statusMsgTimer.remainingTime() > 0 && msg.isEmpty())
        return;

    if (msg.isEmpty())
        ui->statusDisplay->hide();
    else {
        statusMsgTimer.stop();
        ui->statusDisplay->show();
        ui->statusDisplay->setText(msg);
    }

    if (timeout) {
        statusMsgTimer.setInterval(timeout);
        statusMsgTimer.setSingleShot(true);
        connect(&statusMsgTimer, SIGNAL(timeout()), this, SLOT(showStatusMessage()));
        statusMsgTimer.start();
        //QTimer::singleShot(timeout, this, SLOT(showStatusMessage()));
    }
}

