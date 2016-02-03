#include "AQEsc32ConfigWidget.h"
#include "ui_AQEsc32ConfigWidget.h"
#include "MainWindow.h"

#include <qextserialenumerator.h>
#include <QMessageBox>
#include <QSettings>

AQEsc32ConfigWidget::AQEsc32ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQEsc32ConfigWidget),
    esc32(NULL),
    m_outputBrowser(NULL),
    FlashEsc32Active(false),
    FwFileForEsc32("")
{

    ui->setupUi(this);

    ui->STARTUP_MODE->addItem(tr("Open Loop"),0);
    ui->STARTUP_MODE->addItem(tr("CL RPM"),1);
    ui->STARTUP_MODE->addItem(tr("CL Thrust"),2);
    ui->STARTUP_MODE->addItem(tr("Servo (v1.5+)"),3);

    ui->label_portName_esc32->hide();
    ui->pushButton_logging->hide();

    ui->pushButton_start_calibration->setToolTip("WARNING: EXPERIMENTAL!!");

    connect(ui->toolButton_esc32ReloadPorts, SIGNAL(clicked()), this, SLOT(setupPortList()));
    //connect(LinkManager::instance(), SIGNAL(newLink(LinkInterface*)), this, SLOT(addLink(LinkInterface*)));

//    connect(ui->comboBox_port_esc32, SIGNAL(editTextChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->comboBox_port_esc32, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->comboBox_esc32PortSpeed, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->pushButton_connect_to_esc32, SIGNAL(clicked()), this, SLOT(btnConnectEsc32()));
    connect(ui->pushButton_read_config, SIGNAL(clicked()), this, SLOT(btnReadConfigEsc32()));
    connect(ui->pushButton_send_to_esc32, SIGNAL(clicked()), this, SLOT(btnSaveToEsc32()));
    connect(ui->pushButton_esc32_read_arm_disarm, SIGNAL(clicked()), this, SLOT(btnArmEsc32()));
    connect(ui->pushButton_esc32_read_start_stop, SIGNAL(clicked()), this, SLOT(btnStartStopEsc32()));
    connect(ui->pushButton_send_rpm, SIGNAL(clicked()), this, SLOT(btnSetRPM()));
    connect(ui->pushButton_start_calibration, SIGNAL(clicked()), this, SLOT(Esc32StartCalibration()));
    connect(ui->pushButton_logging, SIGNAL(clicked()), this, SLOT(Esc32StartLogging()));
    connect(ui->pushButton_read_load_def, SIGNAL(clicked()), this, SLOT(Esc32LoadDefaultConf()));
    connect(ui->pushButton_reload_conf, SIGNAL(clicked()), this, SLOT(Esc32ReLoadConf()));
    connect(ui->pushButton_esc32_saveToFile, SIGNAL(clicked()), this, SLOT(Esc32SaveParamsToFile()));
    connect(ui->pushButton_esc32_loadFromFile, SIGNAL(clicked()), this, SLOT(Esc32LoadParamsFromFile()));

    setupPortList();
    loadSettings();

}

AQEsc32ConfigWidget::~AQEsc32ConfigWidget()
{
    if (esc32)
        btnConnectEsc32();
    writeSettings();
    delete ui;
}

void AQEsc32ConfigWidget::changeEvent(QEvent *e)
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

void AQEsc32ConfigWidget::loadSettings()
{
    // Load defaults from settings
    QSettings settings;
    settings.beginGroup("AUTOQUAD_SETTINGS");
    ui->comboBox_port_esc32->setCurrentIndex(ui->comboBox_port_esc32->findText(settings.value("ESC32_FLASH_PORT_NAME", "").toString()));
    ui->comboBox_esc32PortSpeed->setCurrentIndex(ui->comboBox_esc32PortSpeed->findText(settings.value("ESC32_BAUD_RATE", 230400).toString()));
    settings.endGroup();
    settings.sync();
}

void AQEsc32ConfigWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("AUTOQUAD_SETTINGS");
    settings.setValue("ESC32_FLASH_PORT_NAME", ui->comboBox_port_esc32->currentText());
    settings.setValue("ESC32_BAUD_RATE", ui->comboBox_esc32PortSpeed->currentText());
    settings.sync();
    settings.endGroup();
}

void AQEsc32ConfigWidget::setupPortList()
{
    QString pdispname;
    QString cidxesc = ui->comboBox_port_esc32->currentText();
    ui->comboBox_port_esc32->clear();
    // Get the ports available on this system
    foreach (const QextPortInfo &p, QextSerialEnumerator::getPorts()) {
        if (!p.portName.length())
            continue;
        pdispname = p.portName;
        if (p.friendName.length())
            pdispname += " - " + p.friendName.split(QRegExp(" ?\\(")).first();
        ui->comboBox_port_esc32->addItem(pdispname, p.portName);
    }
    ui->comboBox_port_esc32->setCurrentIndex(ui->comboBox_port_esc32->findText(cidxesc));
}

void AQEsc32ConfigWidget::setPortNameEsc32(QString port)
{
    Q_UNUSED(port);

    portNameEsc32 = ui->comboBox_port_esc32->itemData(ui->comboBox_port_esc32->currentIndex()).toString();
//    portNameEsc32 = ui->comboBox_port_esc32->currentText();
//#ifdef Q_OS_WIN
//    portNameEsc32 = portNameEsc32.split("-").first();
//#endif
//    portNameEsc32 = portNameEsc32.remove(" ");
    QString portSpeed = ui->comboBox_esc32PortSpeed->currentText();
    ui->label_portName_esc32->setText(QString("%1 @ %2 bps").arg(portNameEsc32).arg(portSpeed));
    ui->comboBox_port_esc32->setToolTip(ui->comboBox_port_esc32->currentText());
}


void AQEsc32ConfigWidget::flashFWEsc32() {

    if (esc32)
        esc32->Disconnect();

    m_outputBrowser->append(tr("Testing for ESC32 bootloader mode...\n"));

    QProcess stmflash;
    stmflash.setProcessChannelMode(QProcess::MergedChannels);

    QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "stm32flash");

    m_outputBrowser->append(AppPath + " " + portName + "\n");

    stmflash.start(AppPath , QStringList() << portName);
    if (!stmflash.waitForFinished(3000)) {
        m_outputBrowser->append(tr("stm32flash failed to connect on %1 with error: %2\n").arg(portName).arg(stmflash.errorString()));
        return;
    } else {
        QByteArray stmout = stmflash.readAll();
        //qDebug() << stmout;
        if (stmout.contains("Version")) {
            m_outputBrowser->append(tr("ESC32 in bootloader mode already, flashing...\n"));
            emit flashFwStart();
            return;
        } else {
            m_outputBrowser->append(tr("ESC32 not in bootloader mode...\n"));
        }
    }

    esc32 = new AQEsc32();
    connect(esc32, SIGNAL(Esc32Connected()), this, SLOT(Esc32Connected()));
    connect(esc32, SIGNAL(ESc32Disconnected()), this, SLOT(ESc32Disconnected()));
    connect(esc32, SIGNAL(EnteredBootMode()), this, SLOT(Esc32BootModOk()));
    connect(esc32, SIGNAL(NoBootModeArmed(QString)), this, SLOT(Esc32BootModFailure(QString)));
    connect(esc32, SIGNAL(BootModeTimeout()), this, SLOT(Esc32BootModeTimeout()));

    m_outputBrowser->append("Attempting to force bootloader mode. Connecting to ESC32...\n");

    FlashEsc32Active = true;
    esc32->Connect(portName, ui->comboBox_esc32PortSpeed->currentText());

}

void AQEsc32ConfigWidget::Esc32BootModOk() {
    FlashEsc32Active = false;
    esc32->Disconnect();
//    QTimer* tim = new QTimer(this);
//    tim->setSingleShot(true);
//    connect(tim, SIGNAL(timeout()), this, SLOT(flashFwStart()));
//    tim->start(2500);
    emit flashFwStart();
}

void AQEsc32ConfigWidget::Esc32BootModFailure(QString err) {
    FlashEsc32Active = false;
    esc32->Disconnect();

    m_outputBrowser->append(tr("Failed to enter bootloader mode.\n"));
    if (err.contains("armed"))
        err += tr("\n\nESC appears to be active/armed.  Please disarm first!");
    else
        err += tr("\n\nYou may need to short the BOOT0 pins manually to enter bootloader mode.  Then attempt flashing again.");
    MainWindow::instance()->showCriticalMessage("Error!", err);
}

void AQEsc32ConfigWidget::Esc32BootModeTimeout() {
    Esc32BootModFailure(tr("Bootloader mode timeout."));
}

void AQEsc32ConfigWidget::Esc32GotFirmwareVersion(QString ver) {
    ui->label_esc32_fw_version->setText(tr("FW version: %1").arg(ver));
    bool ok;
    int maj = ver.section(".", 0, 0).toInt(&ok);
    ok = !ok || maj < 3;
    ui->pushButton_send_to_esc32->setEnabled(ok);
    ui->pushButton_esc32_read_start_stop->setEnabled(ok);
    ui->pushButton_start_calibration->setEnabled(ok);
}

void AQEsc32ConfigWidget::btnConnectEsc32()
{
    if (!esc32) {
        QString port = portNameEsc32;
        QString baud = ui->comboBox_esc32PortSpeed->currentText();

//        if (checkAqSerialConnection(port)) {
//            QString msg = QString("WARNING: You are already connected to AutoQuad! If you continue, you will be disconnected.\n\nDo you wish to continue connecting to ESC32?").arg(port);
//            QMessageBox::StandardButton qrply = QMessageBox::warning(this, tr("Confirm Disconnect AutoQuad"), msg, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
//            if (qrply == QMessageBox::Cancel)
//                return;

//            connectedLink->disconnect();
//        }

        esc32 = new AQEsc32();
        connect(esc32, SIGNAL(Esc32Connected()), this, SLOT(Esc32Connected()));
        connect(esc32, SIGNAL(ESc32Disconnected()), this, SLOT(ESc32Disconnected()));
        connect(esc32, SIGNAL(getCommandBack(int)), this, SLOT(Esc32CaliGetCommand(int)));
        connect(esc32, SIGNAL(ShowConfig(QString)), this, SLOT(Esc32LoadConfig(QString)));
        connect(esc32, SIGNAL(Esc32ParaWritten(QString)), this, SLOT(ParaWrittenEsc32(QString)));
        connect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)), this, SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        connect(esc32, SIGNAL(finishedCalibration(int)), this, SLOT(Esc32CalibrationFinished(int)));
        connect(esc32, SIGNAL(GotFirmwareVersion(QString)), this, SLOT(Esc32GotFirmwareVersion(QString)));

        esc32->Connect(port, baud);
    }
    else {
        esc32->Disconnect();
    }
}

void AQEsc32ConfigWidget::Esc32LoadConfig(QString Config)
{
    paramEsc32.clear();
    Config.remove(QRegExp("^.*\\[J"));
    Config.remove("\n");
    QStringList RowList = Config.split("\r");
    for ( int j = 0; j< RowList.length(); j++) {
        QStringList ParaList = RowList.at(j).split(" ", QString::SkipEmptyParts);
        if ( ParaList.length() >= 3)
            paramEsc32.insert(ParaList.at(0),ParaList.at(2));
    }

    Esc32ShowConfig(paramEsc32);
    Esc32UpdateStatusText(tr("Loaded current config."));
}

void AQEsc32ConfigWidget::Esc32ShowConfig(QMap<QString, QString> paramPairs, bool disableMissing) {
    QList<QLineEdit*> edtList = this->findChildren<QLineEdit*>(QRegExp("^[A-Z]{2,}") );
    for ( int i = 0; i<edtList.count(); i++) {
        QString ParaName = edtList.at(i)->objectName();
        if ( paramPairs.contains(ParaName) )
        {
            edtList.at(i)->setEnabled(true);
            edtList.at(i)->setText(paramPairs.value(ParaName));
        }
        else if (disableMissing)
            edtList.at(i)->setEnabled(false);
    }

    if (paramPairs.contains("STARTUP_MODE"))
        ui->STARTUP_MODE->setCurrentIndex(paramPairs.value("STARTUP_MODE").toInt());
    if (paramPairs.contains("BAUD_RATE"))
        ui->BAUD_RATE->setCurrentIndex(ui->BAUD_RATE->findText(paramPairs.value("BAUD_RATE")));
    if (paramPairs.contains("ESC_ID"))
        ui->ESC_ID->setCurrentIndex(paramPairs.value("ESC_ID").toInt());
    if (paramPairs.contains("DIRECTION"))
        ui->DIRECTION->setCurrentIndex(paramPairs.value("DIRECTION").toInt() == 1 ? 0 : 1);

    if (disableMissing) {
        ui->groupBox_ESC32_ServoSettings->setVisible(paramPairs.contains("SERVO_DUTY"));
        ui->ESC_ID->setEnabled(paramPairs.contains("ESC_ID"));
        ui->DIRECTION->setEnabled(paramPairs.contains("DIRECTION"));
    }
}

void AQEsc32ConfigWidget::btnSaveToEsc32() {

    bool something_gos_wrong = false;
    int rettryToStore = 0, timeout = 0;
    QString ParaName, valueText, valueEsc32;
    QMap<QString, QString> changedParams;

    QList<QLineEdit*> edtList = ui->gridLayout->findChildren<QLineEdit*>();
    for ( int i = 0; i<edtList.count(); i++) {
        ParaName = edtList.at(i)->objectName();
        valueText = edtList.at(i)->text();
        if ( paramEsc32.contains(ParaName) )
        {
            valueEsc32 = paramEsc32.value(ParaName);
            if ( valueEsc32 != valueText || skipParamChangeCheck)
                changedParams.insert(ParaName, valueText);
        }
    }

    valueEsc32 = paramEsc32.value("STARTUP_MODE");
    if (valueEsc32.toInt() != ui->STARTUP_MODE->currentIndex() || skipParamChangeCheck)
        changedParams.insert("STARTUP_MODE", QString::number(ui->STARTUP_MODE->currentIndex()));

    valueEsc32 = paramEsc32.value("BAUD_RATE");
    if (valueEsc32 != ui->BAUD_RATE->currentText() || skipParamChangeCheck)
        changedParams.insert("BAUD_RATE", ui->BAUD_RATE->currentText());

    if ( paramEsc32.contains("ESC_ID") ) {
        valueEsc32 = paramEsc32.value("ESC_ID");
        if (valueEsc32.toInt() != ui->ESC_ID->currentIndex() || skipParamChangeCheck)
            changedParams.insert("ESC_ID", QString::number(ui->ESC_ID->currentIndex()));
    }
    if ( paramEsc32.contains("DIRECTION") ) {
        valueEsc32 = paramEsc32.value("DIRECTION");
        valueText = ui->DIRECTION->currentIndex() == 0 ? "1" : "-1";
        if (valueEsc32 != valueText || skipParamChangeCheck)
            changedParams.insert("DIRECTION", valueText);
    }

    Esc32UpdateStatusText("Writing config...");

    QMapIterator<QString, QString> i(changedParams);
    while (i.hasNext()) {
        i.next();
        WaitForParaWriten = 1;
        ParaNameWritten = i.key();
        esc32->SavePara(ParaNameWritten, i.value());
        timeout = 0;
        while(WaitForParaWriten >0) {
            if (paramEsc32Written.contains(ParaNameWritten)) {
                paramEsc32Written.remove(ParaNameWritten);
                break;
            }
            timeout++;
            if ( timeout > 500000) {
                something_gos_wrong = true;
                rettryToStore++;
                break;
            }
            QCoreApplication::processEvents();
        }
        if ((rettryToStore >= 3) && (something_gos_wrong))
            break;
    }

    if (changedParams.size()&& !something_gos_wrong) {
        skipParamChangeCheck = false;
        Esc32UpdateStatusText(tr("Wrote %1 params.").arg(changedParams.size()));
        saveEEpromEsc32();
    } else if (something_gos_wrong) {
        Esc32UpdateStatusText("Error saving config.");
        MainWindow::instance()->showCriticalMessage("Error!", tr("Something went wrong trying to store the configuration. Please retry!"));
    } else
        Esc32UpdateStatusText("No changed params.");

}

void AQEsc32ConfigWidget::Esc32SaveParamsToFile()
{
    QString suggestPath = "./esc32.txt";
    if (LastFilePath.length()) {
        QFileInfo fi(LastFilePath);
        suggestPath = fi.absolutePath() + "/" + suggestPath;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Select or Create ESC32 Settings File"), suggestPath,
                                                    tr("Parameter File") + " (*.txt);;" + tr("All File Types") + " (*.*)");
    if (!fileName.length())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage("Error!", tr("Could not open params file. %1").arg(file.errorString()));
        return;
    }

    LastFilePath = fileName;
    Esc32UpdateStatusText("Saving to file...");

    QTextStream in(&file);

    QList<QLineEdit*> edtList = ui->gridLayout->findChildren<QLineEdit*> (QRegExp("^[A-Z]{2,}"));
    for ( int i = 0; i<edtList.count(); i++) {
        if (edtList.at(i)->text().length())
            in << edtList.at(i)->objectName() << "\t" << edtList.at(i)->text() << "\n";
    }

    in << "STARTUP_MODE" << "\t" << ui->STARTUP_MODE->currentIndex() << "\n";
    in << "BAUD_RATE" << "\t" << ui->BAUD_RATE->currentText() << "\n";
    in << "ESC_ID" << "\t" << ui->ESC_ID->currentIndex() << "\n";
    in << "DIRECTION" << "\t" << (ui->DIRECTION->currentIndex() == 0 ? 1 : -1) << "\n";

    in.flush();
    file.close();

    Esc32UpdateStatusText("Saved to file.");
}

void AQEsc32ConfigWidget::Esc32LoadParamsFromFile() {
    QString dirPath = QDir::toNativeSeparators(LastFilePath);
    QFileInfo dir(dirPath);

    // use native file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Saved Parameters File"), dir.absoluteFilePath(),
                                            tr("Parameter File") + " (*.txt);;" + tr("All File Types") + " (*.*)");

    if (!fileName.length())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage("Error!", tr("Could not open params file. %1").arg(file.errorString()));
        return;
    }

    LastFilePath = fileName;

    QTextStream in(&file);
    QMap<QString, QString> loadedParams;
    while (!in.atEnd()) {
        QString line = in.readLine();
        qDebug() << line;
        if (line.contains(QRegExp("^[A-Z\\d_]{2,}[\\t ]+[\\d\\+\\-]"))) {
            QStringList paramPair = line.split(QRegExp("[\\t ]"), QString::SkipEmptyParts);
            qDebug() << paramPair.at(0);
            if (paramPair.size() == 2)
                loadedParams.insert(paramPair.at(0), paramPair.at(1));
        }
    }

    Esc32UpdateStatusText("Loaded from file.");

    Esc32ShowConfig(loadedParams, false);
}

void AQEsc32ConfigWidget::btnArmEsc32()
{
    if ( !esc32)
        return;
     if ( !esc32_armed)
        esc32->sendCommand(esc32->BINARY_COMMAND_ARM,0.0f, 0.0f, 0, false);
     else
        esc32->sendCommand(esc32->BINARY_COMMAND_DISARM,0.0f, 0.0f, 0, false);

}

void AQEsc32ConfigWidget::btnStartStopEsc32()
{
    if ( !esc32)
        return;

    if ( !esc32_running)
        esc32->sendCommand(esc32->BINARY_COMMAND_START,0.0f, 0.0f, 0, false);
    else
        esc32->sendCommand(esc32->BINARY_COMMAND_STOP,0.0f, 0.0f, 0, false);
}

void AQEsc32ConfigWidget::ParaWrittenEsc32(QString ParaName) {
    if ( ParaNameWritten == ParaName) {
        WaitForParaWriten = 0;

        QList<QLineEdit*> edtList = ui->gridLayout->findChildren<QLineEdit*> ();
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

void AQEsc32ConfigWidget::CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2) {
    Q_UNUSED(V2);
    switch (CommandName) {
    case AQEsc32::BINARY_COMMAND_ARM :
        ui->pushButton_esc32_read_arm_disarm->setText(tr("disarm"));
        esc32_armed = true;
        break;
    case AQEsc32::BINARY_COMMAND_DISARM :
        ui->pushButton_esc32_read_arm_disarm->setText(tr("arm"));
        esc32_armed = false;
        break;
    case AQEsc32::BINARY_COMMAND_START :
        ui->pushButton_esc32_read_start_stop->setText(tr("stop"));
        esc32_running = true;
        break;
    case AQEsc32::BINARY_COMMAND_STOP :
        ui->pushButton_esc32_read_start_stop->setText(tr("start"));
        esc32_running = false;
        break;
    case AQEsc32::BINARY_COMMAND_RPM :
        ui->spinBox_rpm->setValue(V1.toInt());
        break;
    case AQEsc32::BINARY_COMMAND_CONFIG :
        switch (V1.toInt()) {
        case 0 :
            Esc32UpdateStatusText(tr("Loaded config from flash."));
            skipParamChangeCheck = false;
            break;
        case 1 :
            Esc32UpdateStatusText(tr("Wrote config to flash."));
            skipParamChangeCheck = false;
            break;
        case 2 :
            Esc32UpdateStatusText(tr("Loaded default config."));
            skipParamChangeCheck = true;
            break;
        }
        break;
    }
}

void AQEsc32ConfigWidget::btnSetRPM()
{
     if (esc32_running && esc32_armed) {
         if ( ui->FF1TERM->text().toFloat() == 0.0f) {
              MainWindow::instance()->showCriticalMessage(tr("Error!"), tr("The Parameter FF1Term is 0.0, can't set the RPM! Please change it and write config to ESC."));
              return;
         }
         float rpm = (float)ui->spinBox_rpm->value();
         esc32->sendCommand(esc32->BINARY_COMMAND_RPM, rpm, 0.0f, 1, false);
    }
}

void AQEsc32ConfigWidget::saveEEpromEsc32()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Question"));
    msgBox.setInformativeText(tr("The values have been transmitted to Esc32! Do you want to store the parameters into permanent memory (ROM)?"));
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes :
        esc32->sendCommand(esc32->BINARY_COMMAND_CONFIG,1.0f, 0.0f, 1, false);
        break;
    default :
        return;
    }
}

void AQEsc32ConfigWidget::Esc32Connected(){
    if ( !FlashEsc32Active ){
        esc32->CheckVersion();
        esc32->ReadConfigEsc32();
        skipParamChangeCheck = false;
        esc32_connected = true;
    } else {
        m_outputBrowser->append(tr("Serial link connected. Attemtping bootloader mode...\n"));
        esc32->SetToBootMode();
    }
    ui->pushButton_connect_to_esc32->setText(tr("disconnect"));
}

void AQEsc32ConfigWidget::ESc32Disconnected() {
    disconnect(esc32, 0, this, 0);
    esc32 = NULL;
    esc32_connected = false;
    ui->pushButton_connect_to_esc32->setText(tr("connect esc32"));
    ui->label_esc32_fw_version->setText(tr("FW version: [not connected]"));
    Esc32UpdateStatusText(tr("Disconnected."));
}

void AQEsc32ConfigWidget::Esc32StartLogging() {
    if (!esc32)
        return;

    esc32->StartLogging();
}

void AQEsc32ConfigWidget::Esc32StartCalibration() {
    if (!esc32)
        return;

    QString Esc32LoggingFile = "";
    QString Esc32ResultFile = "";

     if ( !esc32_calibrating) {
        QMessageBox InfomsgBox;
        InfomsgBox.setText(tr("<p style='color: red; font-weight: bold;'>WARNING!! EXPERIMENTAL FEATURE! BETTER TO USE Linux/OS-X COMMAND-LINE TOOLS!</p> \
<p>This is the calibration routine for ESC32!</p> \
<p>Please be careful with the calibration function! The motor will spin up to full throttle! Please stay clear of the motor & propeller!</p> \
<p><b style='color: red;'>Proceed at your own risk!</b>  You will have one more chance to cancel before the procedure starts.</p>"));
        InfomsgBox.setWindowModality(Qt::ApplicationModal);
        InfomsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        InfomsgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = InfomsgBox.exec();
        if (ret == QMessageBox::Cancel)
            return;

        ret = QMessageBox::question(this, tr("Question"), tr("Which calibration do you want to do?"), "RpmToVoltage", "CurrentLimiter");
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
        msgBox.setText(tr("<p style='font-weight: bold;'>Again, be carful! You can abort using the Stop Calibration button, but the fastest stop is to pull the battery!</p> \
<p style='font-weight: bold;'>To start the calibration procedure, press Yes.</p><p style='color: red; font-weight: bold;'>This is your final warning!</p>"));
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        ret = msgBox.exec();
        if ( ret == QMessageBox::Cancel)
            return;

        if ( ret == QMessageBox::Yes) {
            float maxAmps = (float)ui->DoubleMaxCurrent->value();

            esc32->SetCalibrationMode(this->Esc32CalibrationMode);
            esc32->StartCalibration(maxAmps,Esc32LoggingFile,Esc32ResultFile);
            esc32_calibrating = true;
            ui->pushButton_start_calibration->setText(tr("stop calibration"));
        }
    }
     else // stop calibration
    {
        esc32_calibrating = false;
        ui->pushButton_start_calibration->setText(tr("start calibration"));
        esc32->StopCalibration(true);
    }
}

void AQEsc32ConfigWidget::Esc32CalibrationFinished(int mode) {
    esc32_calibrating = false;
    ui->pushButton_start_calibration->setText(tr("start calibration"));
    //Emergency exit
    if ( mode == 99) {
        //No values from esc32
        QMessageBox InfomsgBox;
        InfomsgBox.setText(tr("Something went wrong in data logging, Aborted!"));
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
        //Esc32LoggingFile
        QMessageBox InfomsgBox;
        InfomsgBox.setText(tr("Updated the fields with FF1Term and FF2Term!"));
        InfomsgBox.exec();
        return;
    }
     else if ( mode == 2) {
        ui->CL1TERM->setText(QString::number(esc32->getCL1()));
        ui->CL2TERM->setText(QString::number(esc32->getCL2()));
        ui->CL3TERM->setText(QString::number(esc32->getCL3()));
        ui->CL4TERM->setText(QString::number(esc32->getCL4()));
          ui->CL5TERM->setText(QString::number(esc32->getCL5()));
        QMessageBox InfomsgBox;
        InfomsgBox.setText(tr("Updated the fields with Currentlimiter 1 to Currentlimiter 5!"));
        InfomsgBox.exec();
        return;
    }
}

void AQEsc32ConfigWidget::btnReadConfigEsc32() {
    Esc32UpdateStatusText(tr("Requesting config..."));
    esc32->ReadConfigEsc32();
    skipParamChangeCheck = false;
    Esc32UpdateStatusText(tr("Loaded current config."));
}

void AQEsc32ConfigWidget::Esc32LoadDefaultConf() {
    Esc32UpdateStatusText(tr("Loading defaults..."));
    esc32->sendCommand(esc32->BINARY_COMMAND_CONFIG,2.0f, 0.0f, 1, false);
    esc32->ReadConfigEsc32();
}

void AQEsc32ConfigWidget::Esc32ReLoadConf() {
    Esc32UpdateStatusText(tr("Loading stored config..."));
    esc32->sendCommand(esc32->BINARY_COMMAND_CONFIG,0.0f, 0.0f, 1, false);
    esc32->ReadConfigEsc32();
}

void AQEsc32ConfigWidget::Esc32CaliGetCommand(int Command){
    esc32->SetCommandBack(Command);
}

void AQEsc32ConfigWidget::Esc32UpdateStatusText(QString text){
    ui->label_esc32_configStatusText->setText("<i>" + text + "</i>");
    ui->label_esc32_configStatusText->setToolTip(text);
}

void AQEsc32ConfigWidget::setPortName(const QString &value)
{
    portName = value;
}

void AQEsc32ConfigWidget::setAqBinFolderPath(const QString &value)
{
    aqBinFolderPath = value;
}

void AQEsc32ConfigWidget::setLastFilePath(const QString &value)
{
    LastFilePath = value;
}

void AQEsc32ConfigWidget::setOutputBrowser(QTextBrowser *outputBrowser)
{
    m_outputBrowser = outputBrowser;
}

void AQEsc32ConfigWidget::setBaudRates(const QStringList &value)
{
    baudRates = value;
    ui->comboBox_esc32PortSpeed->clear();
    ui->comboBox_esc32PortSpeed->addItems(baudRates);
    ui->BAUD_RATE->clear();
    ui->BAUD_RATE->addItems(baudRates);
    loadSettings();
}


