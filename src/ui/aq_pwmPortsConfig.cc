
#include "MainWindow.h"
#include "aq_pwmPortsConfig.h"
#include "ui_aq_pwmPortsConfig.h"
#include "qgcautoquad.h"
#include <algorithm>

AQPWMPortsConfig::AQPWMPortsConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQPWMPortsConfig)
{

    // find QGCAutoquad class
    aq = qobject_cast<QGCAutoquad *>(parent);
    if (!aq)
        return;

    // get reference to mavlink parameter handler (to read/write params)
    paramHandler = aq->getParamHandler();

    // other defaults
    portOrder2Param = "TELEMETRY_RATE";
    mixFilesPath = aq->aqMotorMixesPath;
    mixImagesPath = mixFilesPath % "images/";
    motMixLastFile = mixFilesPath + QString("/motorMixing.mix");

    ui->setupUi(this);

    // assign IDs to mix type radio buttons
    ui->buttonGroup_motorMix->setId(ui->radioButton_mixType_custom, 0);
    ui->buttonGroup_motorMix->setId(ui->radioButton_mixType_predefined, 1);

    // set up mixing table view
    ui->table_motMix->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->table_motMix->horizontalHeader()->setResizeMode(COL_MOTOR, QHeaderView::Fixed);
    ui->table_motMix->horizontalHeader()->resizeSection(COL_MOTOR, 45);
    PwmPortsComboBoxDelegate *delegate = new PwmPortsComboBoxDelegate(ui->table_motMix, this);
    ui->table_motMix->setItemDelegateForColumn(COL_PORT, delegate);
    ui->table_motMix->setItemDelegateForColumn(COL_TYPE, delegate);
    dataChangeType = Qt::EditRole;

    // set up motor/frame layout graphics view background image
    QImage img(":/files/images/contrib/aq_motormix_frame_background.png");
    frameLayout_bgItem = new QGraphicsPixmapItem(QPixmap::fromImage(img));
    scene_frameLayout = new QGraphicsScene();
    scene_frameLayout->setSceneRect(QRectF(0, 0, 249, 249));
    scene_frameLayout->addItem(frameLayout_bgItem);
    ui->graphicsView_frameLayout->setScene(scene_frameLayout);
    ui->graphicsView_frameLayout->show();

    // populate preconfigured frame types
    loadFrameTypes();

    // number of motors selector for custom type
    ui->comboBox_numOfMotors->addItem("Select...", 0);
    for (int i=1; i <= aq->maxMotorPorts; ++i)
        ui->comboBox_numOfMotors->addItem(QString::number(i), i);

    //ui->comboBox_numOfMotors->addItems(aq->getAvailablePwmPorts());
    ui->comboBox_numOfMotors->setCurrentIndex(0);

    // list of all port selector combo boxes, for easy traversal
    allPortSelectors.append(ui->groupBox_gimbal->findChildren<QComboBox *>());
    allPortSelectors.append(ui->groupBox_signaling->findChildren<QComboBox *>());

    // set up a simple item model for port number selector boxes
    model_portNumbers = new QStringListModel(this);
    portNumbersModel_updated();;

    // assign item model to all port selectors
    foreach (QComboBox* cb, allPortSelectors) {
        cb->setModel(model_portNumbers);
        cb->setCurrentIndex(0);
        connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(portSelector_currentIndexChanged(int)));
    }

    // set up default mixing type view
    motorMixType = 1;
    changeMixType();

    // connect to hardware info update signal
    connect(aq, SIGNAL(hardwareInfoUpdated()), this, SLOT(portNumbersModel_updated()));
    connect(aq, SIGNAL(firmwareInfoUpdated()), this, SLOT(firmwareVersion_updated()));

    // connect GUI controls related to motor table
    motorTableConnections(true);

    // other GUI connections
    connect(ui->toolButton_loadFile, SIGNAL(clicked()), this, SLOT(loadFile_clicked()));
    connect(ui->toolButton_saveFile, SIGNAL(clicked()), this, SLOT(saveFile_clicked()));
    connect(ui->toolButton_loadImage, SIGNAL(clicked()), this, SLOT(loadImage_clicked()));
    connect(ui->toolButton_allToCAN, SIGNAL(clicked()), this, SLOT(allToCAN_clicked()));
    connect(ui->toolButton_allToPWM, SIGNAL(clicked()), this, SLOT(allToPWM_clicked()));
}

AQPWMPortsConfig::~AQPWMPortsConfig()
{
    delete ui;
}


void AQPWMPortsConfig::motorTableConnections(bool enable) {
    if (enable) {
        connect(ui->buttonGroup_motorMix, SIGNAL(buttonClicked(int)), this, SLOT(motorMix_buttonClicked(int)));
        connect(ui->comboBox_mixSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(mixSelector_currentIndexChanged(int)));
        connect(ui->comboBox_numOfMotors, SIGNAL(currentIndexChanged(int)), this, SLOT(numOfMotors_currentIndexChanged(int)));
    } else {
        disconnect(ui->buttonGroup_motorMix, SIGNAL(buttonClicked(int)), this, SLOT(motorMix_buttonClicked(int)));
        disconnect(ui->comboBox_mixSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(mixSelector_currentIndexChanged(int)));
        disconnect(ui->comboBox_numOfMotors, SIGNAL(currentIndexChanged(int)), this, SLOT(numOfMotors_currentIndexChanged(int)));
    }
}

void AQPWMPortsConfig::portConfigConnections(bool enable) {
    if (enable) {
        connect(ui->table_motMix, SIGNAL(cellChanged(int,int)), this, SLOT(motorPortsConfig_updated(int,int)));
    } else {
        disconnect(ui->table_motMix, SIGNAL(cellChanged(int,int)), this, SLOT(motorPortsConfig_updated(int,int)));
    }
}


void AQPWMPortsConfig::changeMixType(void) {

    motorTableConnections(false);

    if (motorMixType == 0) { // custom
        ui->radioButton_mixType_custom->setChecked(true);
        ui->comboBox_mixSelector->setCurrentIndex(0);
        mixConfigId = 0;

        ui->label_mixSelector->hide();
        ui->comboBox_mixSelector->hide();
        ui->label_numOfMotors->show();
        ui->comboBox_numOfMotors->show();

        if (ui->table_motMix->rowCount())
            ui->comboBox_numOfMotors->setCurrentIndex(ui->comboBox_numOfMotors->findText(QString("%1").arg(ui->table_motMix->rowCount()-1)));
        else
            ui->comboBox_numOfMotors->setCurrentIndex(0);

    } else { // predefined
        ui->radioButton_mixType_predefined->setChecked(true);

        ui->label_mixSelector->show();
        ui->comboBox_mixSelector->show();
        ui->label_numOfMotors->hide();
        ui->comboBox_numOfMotors->hide();

        if (ui->comboBox_mixSelector->currentIndex()) {
            QString file = QDir::toNativeSeparators(mixFilesPath % "/" % ui->comboBox_mixSelector->itemData(ui->comboBox_mixSelector->currentIndex()).toString());
            QFileInfo fInfo(file);
            if (fInfo.exists() && fInfo.isReadable()) {
                QSettings mixSettings(file, QSettings::IniFormat);
                frameImageFile = mixSettings.value("META/ImageFile", fInfo.fileName()).toString();
                mixConfigId = mixSettings.value("META/ConfigId", 0).toInt();
            }
        }

    }

    setFrameImage(frameImageFile);

    motorTableConnections(true);
}


void AQPWMPortsConfig::setFrameImage(QString file) {

    if (scene_frameLayout->items().contains(frameLayout_fgItem))
        scene_frameLayout->removeItem(frameLayout_fgItem);

    if (file.length()) {
        if (file.contains(".mix", Qt::CaseInsensitive))
            file = mixImagesPath % file.replace(".mix", ".png", Qt::CaseInsensitive);

        QFileInfo fInfo(file);
        if (fInfo.exists() && fInfo.isReadable() && fInfo.size() < 5*1024*1024) {
            QImage img(file);
            QPixmap pm = QPixmap::fromImage(img);
            frameLayout_fgItem = new QGraphicsPixmapItem(frameLayout_bgItem);
            frameLayout_fgItem->setPixmap(pm.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            frameLayout_fgItem->setPos(10,9);
        } else
            MainWindow::instance()->showStatusMessage(tr("Could not load image file: %1").arg(file));
    }
}


QStringList AQPWMPortsConfig::getMixFileList(void) {
    QDir mixDir(mixFilesPath);
    mixDir.setFilter(QDir::Files | QDir::Readable);
    mixDir.setSorting(QDir::Name);
    QStringList MixFileFilter;
    MixFileFilter.append(("*.mix"));
    mixDir.setNameFilters(MixFileFilter);

    QStringList mixFiles = mixDir.entryList();

    return mixFiles;
}


QString AQPWMPortsConfig::getMixFileByConfigId(int configId) {
    QString mixFile, file;
    QStringList mixFiles = getMixFileList();
    int cid = 0;
    for (int i = 0; i < mixFiles.size(); ++i) {
        mixFile = QDir::toNativeSeparators(mixFilesPath % "/" % mixFiles.at(i));
        QSettings mixSettings(mixFile, QSettings::IniFormat);
        cid = mixSettings.value("META/ConfigId", 0).toInt();
        if (cid == configId) {
            file = mixFiles.at(i);
            break;
        }
    }
    if (!file.length())
        MainWindow::instance()->showStatusMessage(tr("Could not find ConfigID %1 in any .mix file!").arg(configId));

    return file;
}

QString AQPWMPortsConfig::motorPortTypeName(uint8_t type) {
    switch (type) {
    case MOT_PORT_TYPE_CAN :
        return "CAN";
    case MOT_PORT_TYPE_PWM :
    default:
        return "PWM";
    }
}

uint8_t AQPWMPortsConfig::motorPortTypeId(QString type) {
    if (type == "CAN")
        return MOT_PORT_TYPE_CAN;
    else
        return MOT_PORT_TYPE_PWM;
}

void AQPWMPortsConfig::drawMotorsTable(void) {
    QColor ttlLineBg(Qt::gray);
    QColor ttlLineFg(Qt::black);
    motorPortSettings mot;
//    QTableWidgetItem * wi;

    portConfigConnections(false);

    motorConfigErrors.clear();
    ui->table_motMix->clearContents();
    ui->table_motMix->setRowCount(0);

    if (motorPortsConfig.size()) {
        for (int i=0; i < motorPortsConfig.size(); ++i) {
            mot = motorPortsConfig.at(i);
            if (mot.port) {
                ui->table_motMix->setRowCount(i+1);
                ui->table_motMix->setItem(i, COL_MOTOR, new QTableWidgetItem(QChar('A' + i)));
                ui->table_motMix->item(i, COL_MOTOR)->setFlags(ui->table_motMix->item(i, 0)->flags() ^ Qt::ItemIsEditable);
                ui->table_motMix->item(i, COL_MOTOR)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                ui->table_motMix->item(i, COL_MOTOR)->setBackgroundColor(ttlLineBg);
                ui->table_motMix->item(i, COL_MOTOR)->setTextColor(ttlLineFg);
                ui->table_motMix->setItem(i, COL_PORT, new QTableWidgetItem(QString::number(mot.port)));
                ui->table_motMix->item(i, COL_PORT)->setData(Qt::UserRole, mot.port);
                ui->table_motMix->setItem(i, COL_THROT, new QTableWidgetItem(QString::number(mot.throt)));
                ui->table_motMix->setItem(i, COL_PITCH, new QTableWidgetItem(QString::number(mot.pitch)));
                ui->table_motMix->setItem(i, COL_ROLL, new QTableWidgetItem(QString::number(mot.roll)));
                ui->table_motMix->setItem(i, COL_YAW, new QTableWidgetItem(QString::number(mot.yaw)));
                ui->table_motMix->setItem(i, COL_TYPE, new QTableWidgetItem(motorPortTypeName(mot.type)));
                ui->table_motMix->item(i, COL_TYPE)->setData(Qt::UserRole, mot.type);
            }
        }

        int i = ui->table_motMix->rowCount();
        ui->table_motMix->setRowCount(i+1);
        //ui->table_motMix->setItemDelegateForRow(i, new QStyledItemDelegate(ui->table_motMix));
        ui->table_motMix->setRowHeight(i, 20);
        ui->table_motMix->setItem(i, COL_MOTOR, new QTableWidgetItem("Totals"));
        ui->table_motMix->item(i, COL_MOTOR)->setFlags(ui->table_motMix->item(i, 0)->flags() ^ Qt::ItemIsEditable);
        ui->table_motMix->setSpan(i, COL_MOTOR, 1, 2);
        ui->table_motMix->item(i, COL_MOTOR)->setBackgroundColor(ttlLineBg);
        ui->table_motMix->item(i, COL_MOTOR)->setTextColor(ttlLineFg);
        for (int ii=COL_THROT; ii <= COL_TYPE; ++ii){
            ui->table_motMix->setItem(i, ii, new QTableWidgetItem(ii != COL_TYPE ? "0" : ""));
            ui->table_motMix->item(i, ii)->setFlags(ui->table_motMix->item(i, ii)->flags() ^ Qt::ItemIsEditable);
            ui->table_motMix->item(i, ii)->setBackgroundColor(ttlLineBg);
            ui->table_motMix->item(i, ii)->setTextColor(ttlLineFg);
            //ui->table_motMix->item(i, ii)->setIcon(QIcon(QPixmap(":/files/images/actions/cross-small.png")));
        }
    }

    portConfigConnections(true);
    validateForm();
}


bool AQPWMPortsConfig::updateMotorSums(void) {
    int lastRow = ui->table_motMix->rowCount() - 1;

    if (lastRow < 1)
        return false;

    bool ok;
    float throt=0, pitch=0, roll=0, yaw=0;
    QIcon icnBad(QPixmap(":/files/images/actions/cross-small.png"));
    QIcon icnGood(QPixmap(":/files/images/actions/tick-small.png"));

    portConfigConnections(false);

    errorInMotorConfigTotals = false;

    for (int i=0; i < lastRow; ++i) {
        throt += ui->table_motMix->item(i, COL_THROT)->data(Qt::EditRole).toFloat();
        pitch += ui->table_motMix->item(i, COL_PITCH)->data(Qt::EditRole).toFloat();
        roll += ui->table_motMix->item(i, COL_ROLL)->data(Qt::EditRole).toFloat();
        yaw += ui->table_motMix->item(i, COL_YAW)->data(Qt::EditRole).toFloat();
    }

    ui->table_motMix->item(lastRow, COL_THROT)->setData(Qt::DisplayRole, throt);
    ui->table_motMix->item(lastRow, COL_PITCH)->setData(Qt::DisplayRole, pitch);
    ui->table_motMix->item(lastRow, COL_ROLL)->setData(Qt::DisplayRole, roll);
    ui->table_motMix->item(lastRow, COL_YAW)->setData(Qt::DisplayRole, yaw);

    ui->table_motMix->item(lastRow, COL_THROT)->setIcon((ok = throt > 0) ? icnGood : icnBad);
    if (!ok) errorInMotorConfigTotals = true;
    ui->table_motMix->item(lastRow, COL_PITCH)->setIcon((ok = pitch == 0) ? icnGood : icnBad);
    if (!ok) errorInMotorConfigTotals = true;
    ui->table_motMix->item(lastRow, COL_ROLL)->setIcon((ok = roll == 0) ? icnGood : icnBad);
    if (!ok) errorInMotorConfigTotals = true;
    ui->table_motMix->item(lastRow, COL_YAW)->setIcon((ok = yaw == 0) ? icnGood : icnBad);
    if (!ok) errorInMotorConfigTotals = true;

    portConfigConnections(true);
    return true;
}


void AQPWMPortsConfig::loadFileConfig(QString file) {
    QString mot;
    float throt, pitch, roll, yaw;
    uint8_t type;
    QStringList pOrder;
    QList<motorPortSettings> fileConfig;
    uint16_t motCAN;

    QFileInfo fInfo(file);

    if (!fInfo.exists() || !fInfo.isReadable()) {
        MainWindow::instance()->showStatusMessage(tr("Could not open file: '%1'").arg(file));
        return;
    }

    QSettings mixSettings(file, QSettings::IniFormat);

    mixConfigId = mixSettings.value("META/ConfigId", 0).toUInt();
    pOrder = mixSettings.value("META/PortOrder").toStringList();
    motCAN = mixSettings.value("META/MOT_CAN", 0).toInt();

    motorTableConnections(false);

    motorPortsConfig.clear();
    for (int i=1; i <= 16; ++i) {
        mot = QString::number(i);
        throt = mixSettings.value("Throttle/Motor" % mot, 0).toFloat();
        pitch = mixSettings.value("Pitch/Motor" % mot, 0).toFloat();
        roll = mixSettings.value("Roll/Motor" % mot, 0).toFloat();
        yaw = mixSettings.value("Yaw/Motor" % mot, 0).toFloat();
        type = (motCAN >> (i-1)) & 1;

        if (throt || pitch || roll || yaw) {
            fileConfig.append(motorPortSettings(i, throt, pitch, roll, yaw, type));
        }
    }

    if (!pOrder.size())
        motorPortsConfig.append(fileConfig);
    else {
        for (int i=0; i < pOrder.size(); ++i) {
            for (int ii=0; ii < fileConfig.size(); ++ii) {
                if (fileConfig.at(ii).port == pOrder.at(i).toInt()) {
                    motorPortsConfig.append(fileConfig.takeAt(ii));
                    break;
                }
            }
        }
    }

    drawMotorsTable();

    if (mixConfigId) {
        motorMixType = 1;
        ui->comboBox_mixSelector->setCurrentIndex(ui->comboBox_mixSelector->findData(fInfo.fileName()));
    } else {
        motorMixType = 0;
        ui->comboBox_mixSelector->setCurrentIndex(0);
    }

    customFrameImg = false;
    frameImageFile = mixSettings.value("META/ImageFile", "").toString();
    if (frameImageFile.length())
        customFrameImg = true;
    else
        frameImageFile = fInfo.fileName();

    if (motCAN) {
        ui->toolButton_allToCAN->show();
        ui->toolButton_allToPWM->show();
        ui->table_motMix->showColumn(COL_TYPE);
    }

    changeMixType();
    //setFrameImage(frameImageFile);

    //motorTableConnections(true);
}


void AQPWMPortsConfig::saveConfigFile(QString file) {
    QString pname;
    motorPortSettings pconfig, mot;
    QStringList pOrder;
    uint16_t motCAN = 0;

    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage(tr("Error"), tr("Could not open file for writing: '%1'").arg(file));
        return;
    }
    f.close();

    QSettings mixSettings(file, QSettings::IniFormat);

    // determine port order
    for (int ii=0; ii < motorPortsConfig.size(); ++ii)
        pOrder.append(QString::number(motorPortsConfig.at(ii).port));

    mixSettings.setValue("META/ConfigId", 0);
    mixSettings.setValue("META/Motors", motorPortsConfig.size());
    mixSettings.setValue("META/PortOrder", pOrder);
    mixSettings.setValue("META/ImageFile", frameImageFile);

    for (int i=1; i <= 16; ++i) {
        pconfig = motorPortSettings(i);
        for (int ii=0; ii < motorPortsConfig.size(); ++ii) {
            mot = motorPortsConfig.at(ii);
            if (mot.port == i) {
                pconfig = mot;
                break;
            }
        }

        pname = QString("/Motor%1").arg(i);
        mixSettings.setValue("Throttle" % pname, QString::number(pconfig.throt));
        mixSettings.setValue("Pitch" % pname, QString::number(pconfig.pitch));
        mixSettings.setValue("Roll" % pname, QString::number(pconfig.roll));
        mixSettings.setValue("Yaw" % pname, QString::number(pconfig.yaw));

        if (pconfig.type == MOT_PORT_TYPE_CAN)
            motCAN |= 1 << (pconfig.port - 1);
    }

    mixSettings.setValue("META/MOT_CAN", motCAN);

    mixSettings.sync();
}


void AQPWMPortsConfig::loadCustomConfig(int numMotors) {
    if (motorPortsConfig.size() > numMotors) {
        while (motorPortsConfig.size() > numMotors)
            motorPortsConfig.takeLast();
    } else {
        while (motorPortsConfig.size() < numMotors)
            motorPortsConfig.append(motorPortSettings(motorPortsConfig.size()+1));
    }
    drawMotorsTable();
}


void AQPWMPortsConfig::loadOnboardConfig(void) {
    paramHandler = aq->getParamHandler();

    if (!paramHandler)
        return;

    QString pname;
    float throt, pitch, roll, yaw, motConfig;
    uint8_t type;
    bool orderType = 0; // 0=param order (1-14), 1=order from onboard config bitmask
    uint32_t portOrder=0, portOrder2=0;
    uint16_t motCAN = 0;
    QList<motorPortSettings> onboardConfig;

    portOrder2Param = (paramHandler->paramExistsAQ("MOT_CONFIG_PO2")) ? "MOT_CONFIG_PO2" : "TELEMETRY_RATE";

    motConfig = paramHandler->getParaAQ("MOT_FRAME").toFloat();
    portOrder = *(uint32_t *) &motConfig;
    motConfig = paramHandler->getParaAQ(portOrder2Param).toFloat();
    portOrder2 = *(uint32_t *) &motConfig;
    if (paramHandler->paramExistsAQ("MOT_CAN")) {
        motCAN = paramHandler->getParaAQ("MOT_CAN").toInt();
        ui->toolButton_allToCAN->show();
        ui->toolButton_allToPWM->show();
        ui->table_motMix->showColumn(COL_TYPE);
    } else {
        ui->toolButton_allToCAN->hide();
        ui->toolButton_allToPWM->hide();
        ui->table_motMix->hideColumn(COL_TYPE);
    }

    mixConfigId = portOrder & 0xFF; // 8bits
    if (portOrder > 65)
        orderType = 1;

    motorPortsConfig.clear();

    for (int i=1; i <= aq->maxMotorPorts; ++i) {
        pname = QString("MOT_PWRD_%1_").arg(i, 2, 10, QLatin1Char('0'));
        throt = paramHandler->getParaAQ(pname % "T").toFloat();
        pitch = paramHandler->getParaAQ(pname % "P").toFloat();
        roll = paramHandler->getParaAQ(pname % "R").toFloat();
        yaw = paramHandler->getParaAQ(pname % "Y").toFloat();
        type = (motCAN >> (i-1)) & 1;

        if (throt || pitch || roll || yaw) {
            if (orderType) // store a temp copy because ordering is defined elsewhere
                onboardConfig.append(motorPortSettings(i, throt, pitch, roll, yaw, type));
            else
                motorPortsConfig.append(motorPortSettings(i, throt, pitch, roll, yaw, type));
        }
    }

    if (orderType) {
        motorPortSettings mot;
        QList<int> po;
        int p;

        if (onboardConfig.size() <= 6)
            portOrder2 = 0;
        uint64_t co = static_cast<uint64_t>(portOrder2) << 32 | static_cast<uint64_t>(portOrder);

        for (int b = 8; b <= 60; b=b+4) {
            p = (co >> b) & 0xF;
            if (p)
                po.append(p);
            else
                break;
        }
        for (int i=0; i < po.size(); ++i) {
            for (int ii=0; ii < onboardConfig.size(); ++ii) {
                mot = onboardConfig.at(ii);
                if (mot.port == po.at(i)) {
                    motorPortsConfig.append(mot);
                    onboardConfig.removeAt(ii);
                    break;
                }
            }
        }
        if (onboardConfig.size()) {
            while (onboardConfig.size())
                motorPortsConfig.append(onboardConfig.takeAt(0));
        }
    }

    motorTableConnections(false);
    drawMotorsTable();

    if (mixConfigId) {
        QString file = getMixFileByConfigId(mixConfigId);
        if (file.length()) {
            motorMixType = 1;
            ui->comboBox_mixSelector->setCurrentIndex(ui->comboBox_mixSelector->findData(file));
        } else
            motorMixType = 0;
    } else {
        motorMixType = 0;
    }

    changeMixType();

    aq->getGUIpara(ui->groupBox_gimbal);
    aq->getGUIpara(ui->groupBox_signaling);

    ui->checkBox_useSpeaker->setChecked(paramHandler->getParaAQ("SIG_BEEP_PRT").toFloat() < 0);

    ui->groupBox_signaling->setEnabled(ui->SIG_LED_1_PRT->isEnabled());

}


quint8 AQPWMPortsConfig::saveOnboardConfig(QMap<QString, QList<float> > *changeList, QStringList *errors) {

    if (!aq->checkAqConnected(true))
        return 2;

    paramHandler = aq->getParamHandler();

    QString pname; //, porder;
    float val, val_uas;
    uint32_t portOrder=0, portOrder2=0;
    uint16_t motCAN = 0;
    motorPortSettings mot, pconfig;
    QMap<QString, float> configMap;
    QList<float> changeVals;
    quint8 err = 0; // 0=no error, 1=soft error, 2=fatal error

    validateForm();

    if (errorInMotorConfigTotals) {
        errors->append(tr("There is an unbalanced total value in one or more of the motor configuration columns. This may lead to upredictable behavior."));
        err = 1;
    }
    if (errorInMotorConfig) {
        errors->append(tr("You have invalid values in the motor configuration table."));
        err = 2;
    }
    if (errorInPortConfig) {
        errors->append(tr("You have selected the same port for multiple outputs."));
        err = 2;
    }
    if (errorInTimerConfig) {
        errors->append(tr("You have selected motor and gimbal ports, or gimbal roll/pitch and trigger ports, which use the same hardware timers. Please check the port number chart image for a reference."));
        err = 2;
    }
    if (errorInPortNumberType) {
        errors->append(tr("You have selected a PWM output port which does not exist on your current hardware."));
        err = 2;
    }

    if (err > 1)
        return err;

    for (int i=1; i <= aq->maxMotorPorts; ++i) {
        pconfig = motorPortSettings(i);
        for (int ii=0; ii < motorPortsConfig.size(); ++ii) {
            mot = motorPortsConfig.at(ii);
            if (mot.port == i) {
                pconfig = mot;
                break;
            }
        }
        pname = QString("MOT_PWRD_%1_").arg(i, 2, 10, QLatin1Char('0'));
        configMap.insert(pname % "T", pconfig.throt);
        configMap.insert(pname % "P", pconfig.pitch);
        configMap.insert(pname % "R", pconfig.roll);
        configMap.insert(pname % "Y", pconfig.yaw);

        if (pconfig.type == MOT_PORT_TYPE_CAN)
            motCAN |= 1 << (pconfig.port - 1);
    }

    if (paramHandler->paramExistsAQ("MOT_CAN")) {
        val = *(float *) &motCAN;
        configMap.insert("MOT_CAN", motCAN);
        //  qDebug() << qSetRealNumberPrecision(20) << motCAN << val;
    } else if (motCAN) {
        errors->append(tr("You have selected CAN motor output type but your current firmware does not appear to support it."));
        err = 1;
    }

    // save port order (bit mask: first 8 bits are confgId, next 4 bits is first port used, next 4 is 2nd port used, etc., up to 32 bits (6 ports plus id))
    int p = qMin(5, motorPortsConfig.size()-1);
    for (; p >= 0; --p)
        portOrder |= static_cast<uint32_t>(motorPortsConfig.at(p).port) << (8 + (4 * p)); // save 8 bits for configId

    // if more than 6 motors, need 2nd bitmask to store the other motor positions
    // 2nd port order (bit mask: first 4 bits is 7th port used, next 4 is 8th port used, etc., up to 32 bits (8 ports))
    if (motorPortsConfig.size() > 6) {
        p = qMin(7, motorPortsConfig.size()-7);
        for (; p >= 0; --p)
            portOrder2 |= motorPortsConfig.at(p+6).port << (4 * p);
    }
    portOrder |= mixConfigId;

    // trick to convert ulong into float while preserving bitmask
    val = *(float *) &portOrder;
    configMap.insert("MOT_FRAME", val);
//  qDebug() << qSetRealNumberPrecision(20) << portOrder << val;
//  portOrder = *(uint32_t *) &val;

    if (portOrder2){
        val = *(float *) &portOrder2;
        configMap.insert(portOrder2Param, val);
//      qDebug() << qSetRealNumberPrecision(20) << portOrder2 << val;
//      portOrder2 = *(uint32_t *) &val;
    }

//  qDebug() << qSetRealNumberPrecision(20) << portOrder << portOrder2;

    QMapIterator<QString, float> mi(configMap);
    while (mi.hasNext()) {
        mi.next();
        if (paramHandler->paramExistsAQ(mi.key())) {
            val_uas = paramHandler->getParaAQ(mi.key()).toFloat();
            if (val_uas != mi.value()) {
                changeVals.clear();
                changeVals.append(val_uas);
                changeVals.append(mi.value());
                changeList->insert(mi.key(), changeVals);
            }
        } else {
            errors->append(mi.key());
            err = 1;
        }
    }

    return err;
}


void AQPWMPortsConfig::loadFrameTypes(void) {
    QString currentType = "";

    if (ui->comboBox_mixSelector->currentIndex() > -1)
        currentType = ui->comboBox_mixSelector->itemData(ui->comboBox_mixSelector->currentIndex()).toString();

    QStringList mixFiles = getMixFileList();

    ui->comboBox_mixSelector->clear();
    ui->comboBox_mixSelector->addItem("Select...", QString(""));
    for (int i = 0; i < mixFiles.size(); ++i) {
        QString mixName = mixFiles.at(i);
        mixName.replace(".mix", "", Qt::CaseInsensitive);
        mixName.replace("_", " ");
        ui->comboBox_mixSelector->addItem(mixName, mixFiles.at(i));
    }

    if (currentType.length())
        ui->comboBox_mixSelector->setCurrentIndex(ui->comboBox_mixSelector->findData(currentType));

}


bool AQPWMPortsConfig::validateForm(void) {
    QStringList usedPorts, dupePorts, usedMotorPorts, dupeMotorPorts, timConflictPorts, invalidPwmPorts;
    QMap<uint8_t, QStringList> motorUsedTimers;
    uint8_t tim, iport;
    QString port;
    bool ok, ignoreTimer, isPwm;
    float val;
    QBrush color_error(QColor(255, 0, 0, 200)),
            color_warn(QColor(255, 140, 0, 200)),
            color_ok(Qt::NoBrush),
            bgcolor;

    // motor sums will set errorInMotorConfigTotals flag
    updateMotorSums();
    errorInMotorConfig = false;  // reset error flags
    errorInPortConfig = false;
    errorInTimerConfig = false;
    errorInPortNumberType = false;

    ignoreTimer = (ui->GMBL_PWM_FREQ->value() == 400);

    portConfigConnections(false);  // disable motor config signals to prevent loops

    // loop over each row of the motors table, looking for errors and marking them
    for (int r=0; r < ui->table_motMix->rowCount() - 1; ++r) {

        port = ui->table_motMix->item(r, COL_PORT)->data(Qt::EditRole).toString();
        iport = port.toUInt();
        isPwm = ui->table_motMix->item(r, COL_TYPE)->data(Qt::UserRole).toInt() == MOT_PORT_TYPE_PWM;

        // save and check used pwm port numbers
        if (isPwm) {
            if (usedPorts.contains(port))
                dupePorts.append(port);
            else
                usedPorts.append(port);

            // save this timer as used
            if (iport <= aq->pwmPortTimers.size()) {
                tim = aq->pwmPortTimers.at(iport-1);
                if (!motorUsedTimers.contains(tim))
                    motorUsedTimers.insert(tim, QStringList(port));
                else {
                    QStringList pl = motorUsedTimers.value(tim);
                    pl.append(port);
                    motorUsedTimers.insert(tim, pl);
                }
            }

            // check if any pwm ports don't exist in hardware
            if (port.toUInt() > aq->maxPwmPorts)
                invalidPwmPorts.append(port);
        }

        // save and check used motor port numbers
        if (usedMotorPorts.contains(port))
            dupeMotorPorts.append(port);
        else
            usedMotorPorts.append(port);

        // loop over each of the value columns (throt, pitch, roll, yaw) to check valid values
        for (int c=COL_PORT+1; c <= COL_PORT + 4; ++c) {
            val = ui->table_motMix->item(r, c)->data(Qt::EditRole).toFloat(&ok);
            // validate the value
            if (!ok || val < -100 || val > 100 || (c == COL_THROT && val < 0)) {
                bgcolor = color_error;
                errorInMotorConfig = true;
            } else
                bgcolor = color_ok;

            ui->table_motMix->item(r, c)->setBackground(bgcolor);
        }

    }
    // loop over each port number combo box to check for more duplicates
    foreach (QComboBox* cb, allPortSelectors) {
        if (cb->currentIndex() <= 0)
            continue;

        port = cb->currentText();
        iport = port.toUInt();

        if (usedPorts.contains(port)) {
            // special exception to allow tilt port to == pitch port
            if (cb->objectName() == "GMBL_TILT_PORT" && ui->GMBL_PITCH_PORT->currentText() == port)
                continue;
            dupePorts.append(port);
        }
        else
            usedPorts.append(port);

        if (iport > aq->pwmPortTimers.size())
            continue;
        tim = aq->pwmPortTimers.at(iport-1);
        // check if this timer conflicts with any motor ports
        if (!ignoreTimer && cb->objectName().startsWith("GMBL_") && motorUsedTimers.contains(tim)) {
            for (int i=0; i < aq->pwmPortTimers.size(); ++i)
                if (aq->pwmPortTimers.at(i) == tim)
                    timConflictPorts.append(QString::number(i+1));
        }
    }

    if (dupePorts.size() || dupeMotorPorts.size())
        errorInPortConfig = true;
    if (timConflictPorts.size())
        errorInTimerConfig = true;
    if (invalidPwmPorts.size())
        errorInPortNumberType = true;

    // if we have duplicate port numbers, go find them and highlight them
    // first look in the motors table
    for (int r=0; r < ui->table_motMix->rowCount() - 1; ++r) {
        port = ui->table_motMix->item(r, COL_PORT)->data(Qt::EditRole).toString();
        isPwm = ui->table_motMix->item(r, COL_TYPE)->data(Qt::UserRole).toInt() == MOT_PORT_TYPE_PWM;
        if ((isPwm && dupePorts.contains(port)) || dupeMotorPorts.contains(port) || invalidPwmPorts.contains(port))
            bgcolor = color_error;
        else if (isPwm && timConflictPorts.contains(port))
            bgcolor = color_warn;
        else
            bgcolor = color_ok;
        ui->table_motMix->item(r, COL_PORT)->setBackground(bgcolor);
    }
    // now in each port combo box in the rest of the form
    foreach (QComboBox* cb, allPortSelectors) {
        if (dupePorts.contains(cb->currentText()))
            cb->setStyleSheet("background-color: rgba(255, 0, 0, 200);");
        else if (timConflictPorts.contains(cb->currentText()))
            cb->setStyleSheet("background-color: rgba(255, 140, 0, 200);");
        else
            cb->setStyleSheet("");
    }

    portConfigConnections(true); // done possibly updating the data, re-enable motor config signals

    return !errorInMotorConfig && !errorInPortConfig && !errorInTimerConfig && !errorInMotorConfigTotals && !errorInPortNumberType;
}

void AQPWMPortsConfig::setAllMotorPortTypes(quint8 type) {
    for (int i=0; i < motorPortsConfig.size(); ++i)
        motorPortsConfig[i].type = type;

    drawMotorsTable();
}


void AQPWMPortsConfig::motorPortsConfig_updated(int row, int col) {
    bool ok;
    int role = (col == COL_PORT || col == COL_TYPE) ? Qt::UserRole : Qt::EditRole;

    if (dataChangeType == Qt::EditRole)
        validateForm();

    if (errorInMotorConfig || dataChangeType != role || row >= motorPortsConfig.size() || col == COL_MOTOR)
        return;

    float newVal = ui->table_motMix->item(row, col)->data(role).toFloat(&ok);
    if (!ok)
        return;

    switch (col) {
    case COL_PORT:
        motorPortsConfig[row].port = static_cast<uint16_t>(newVal);
        break;
    case COL_THROT:
        motorPortsConfig[row].throt = newVal;
        break;
    case COL_PITCH:
        motorPortsConfig[row].pitch = newVal;
        break;
    case COL_ROLL:
        motorPortsConfig[row].roll = newVal;
        break;
    case COL_YAW:
        motorPortsConfig[row].yaw = newVal;
        break;
    case COL_TYPE:
        motorPortsConfig[row].type = static_cast<uint8_t>(newVal);
        break;
    }

}


void AQPWMPortsConfig::portNumbersModel_updated(void) {
    QStringList currVals;
    int idx;

    for (int i=0; i < allPortSelectors.size(); ++i)
        currVals << allPortSelectors.at(i)->currentText();

    mtx_portModelIsUpdating = true;

    QStringList ports = aq->getAvailablePwmPorts();
    ports.prepend("off");
    model_portNumbers->setStringList(ports);

    for (int i=0; i < allPortSelectors.size(); ++i) {
        idx = allPortSelectors.at(i)->findText(currVals.at(i));
        allPortSelectors.at(i)->setCurrentIndex((idx > -1) ? idx : 0);
    }

    mtx_portModelIsUpdating = false;
}

void AQPWMPortsConfig::firmwareVersion_updated(void) {
    if (aq->maxMotorPorts > ui->comboBox_numOfMotors->count())
        for (int i=ui->comboBox_numOfMotors->count()-1; i <= aq->maxMotorPorts; ++i)
            ui->comboBox_numOfMotors->addItem(QString::number(i), i);
    else if (aq->maxMotorPorts < ui->comboBox_numOfMotors->count())
        for (int i=ui->comboBox_numOfMotors->count(); i > aq->maxMotorPorts + 1; --i)
            ui->comboBox_numOfMotors->removeItem(i-1);
}



QComboBox* AQPWMPortsConfig::makeMotorPortCombo(QWidget *parent) {
    QComboBox *editor = new QComboBox(parent);
//    QStringList ports = aq->getAvailablePwmPorts();
//    editor->addItems(ports);

    for (int i=1; i <= aq->maxMotorPorts; ++i)
        editor->addItem(QString::number(i), i);

    return editor;
}

QComboBox* AQPWMPortsConfig::makeMotorPortTypeCombo(QWidget *parent) {
    QComboBox *editor = new QComboBox(parent);

    editor->addItem("PWM", MOT_PORT_TYPE_PWM);
    if (aq->motPortTypeCAN)
        editor->addItem("CAN", MOT_PORT_TYPE_CAN);

    return editor;
}


void AQPWMPortsConfig::motorMix_buttonClicked(int btn) {
    motorMixType = btn;
    if (!customFrameImg)
        frameImageFile = "";
    changeMixType();
}


void AQPWMPortsConfig::mixSelector_currentIndexChanged(int index) {
    QString file = ui->comboBox_mixSelector->itemData(index).toString();
    if (file.length())
        loadFileConfig(QDir::toNativeSeparators(mixFilesPath % "/" % file));
}


void AQPWMPortsConfig::numOfMotors_currentIndexChanged(int index) {
    if (index)
        loadCustomConfig(ui->comboBox_numOfMotors->itemText(index).toInt());
}


void AQPWMPortsConfig::portSelector_currentIndexChanged(int /*index*/) {
    if (!mtx_portModelIsUpdating)
        validateForm();
}


void AQPWMPortsConfig::loadFile_clicked() {
    QFileInfo dir(motMixLastFile);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select or Create AQ Motor Mix File"), dir.absoluteFilePath(),
                                                    tr("AQ Mixing Table (*.mix);;All File Types (*.*)"));

    if (!fileName.length())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage("Error!", tr("Could not open file. %1").arg(file.errorString()));
        return;
    } else
        file.close();

    motMixLastFile = fileName;
    loadFileConfig(fileName);
}

void AQPWMPortsConfig::saveFile_clicked() {
    ui->toolButton_saveFile->setFocus();  // this makes sure the table model gets updated

    if (!motorPortsConfig.size()) {
        MainWindow::instance()->showCriticalMessage("Error", "There is nothing to save...");
        return;
    }

    QFileInfo dir(motMixLastFile);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), dir.absoluteFilePath(), tr("AQ Mixing Table (*.mix)"));

    if (!fileName.length())
        return;

    if (!fileName.endsWith(".mix"))
        fileName += ".mix";

    motMixLastFile = fileName;
    saveConfigFile(fileName);
}

void AQPWMPortsConfig::loadImage_clicked() {

    static QString dirPath;
    if ( dirPath == "")
        dirPath = mixImagesPath;

    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Image Files (*.png *.svg *.gif *.jpg *.jpeg *.bmp);;All files (*.*)"));
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec()) {
        QStringList fileNames = dialog.selectedFiles();

        if (fileNames.size() > 0) {
            QString fileName = fileNames.first();
            QFileInfo fInfo(fileName);

            if (!fInfo.exists() || !fInfo.isReadable()) {
                MainWindow::instance()->showCriticalMessage(tr("Error"), tr("Could not open file: '%1'").arg(fileName));
                return;
            }

            dirPath = fileName;
            frameImageFile = fileName;
            customFrameImg = true;
            setFrameImage(fileName);
        }
    }
}

void AQPWMPortsConfig::allToCAN_clicked() {
    setAllMotorPortTypes(MOT_PORT_TYPE_CAN);
}

void AQPWMPortsConfig::allToPWM_clicked() {
    setAllMotorPortTypes(MOT_PORT_TYPE_PWM);
}


// ----------------------------------------------
// Combo Box Delegate
// ----------------------------------------------

PwmPortsComboBoxDelegate::PwmPortsComboBoxDelegate(QObject *parent, AQPWMPortsConfig *aqPwmPortConfig) :
    QStyledItemDelegate(parent), aqPwmPortConfig(aqPwmPortConfig) {}
PwmPortsComboBoxDelegate::~PwmPortsComboBoxDelegate() {}

QWidget *PwmPortsComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if (index.column() == aqPwmPortConfig->COL_PORT)
        return aqPwmPortConfig->makeMotorPortCombo(parent);
    else
        return aqPwmPortConfig->makeMotorPortTypeCombo(parent);
}

void PwmPortsComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *edit = qobject_cast<QComboBox *>(editor);
    if (edit) {
        int idx = index.data(Qt::UserRole).toInt();
        if (index.column() == aqPwmPortConfig->COL_PORT)
            idx -= 1;
        //qDebug() << "idx" << idx;
        if (edit->count() > idx)
            edit->setCurrentIndex(idx);
    }
}

void PwmPortsComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *edit = qobject_cast<QComboBox *>(editor);
    //qDebug() << "editor" << edit->itemData(edit->currentIndex(), Qt::UserRole).toString() << edit->currentText();
    if (edit) {
        aqPwmPortConfig->dataChangeType = Qt::UserRole;
        model->setData(index, edit->itemData(edit->currentIndex(), Qt::UserRole).toInt(), Qt::UserRole);
        aqPwmPortConfig->dataChangeType = Qt::EditRole;
        model->setData(index, edit->currentText());
    }
    //qDebug() << "model" << model->data(index, Qt::UserRole).toString() << model->data(index, Qt::EditRole).toString();
}
