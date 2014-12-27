/*
 * ESC Teletry Widget
 * Author: Maxim Paperno
 * Date: Dec-2014
 * Copyright: (c)2014 Maxim Paperno
 *
 */

#include "ESCtelemetryWidget.h"
#include "ui_ESCtelemetryWidget.h"

#include "GAudioOutput.h"

#include <QSettings>
#include <QDockWidget>
#include <QDialog>
#include <QLCDNumber>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QFormLayout>
#include <QFrame>
#include <QSpacerItem>


ESCtelemetryWidget::ESCtelemetryWidget(QWidget *parent) :
    QWidget(parent),
    m_uas(NULL),
    m_optionsDialog(NULL),
    m_refreshFreq(1),
    m_dataAvailable(false),
    m_firstDataAvailable(false),
    m_dataTimeout(false),
    m_uasTimeout(false),
    m_showTable(true),
    m_showAlertsOpts(true),
    m_hasV2data(false),
    m_hasV3data(false),
    m_enableAudioAlerts(true),
    m_mutexAlertRowsUpdating(false),
    m_maxDataAge(2000),
    ui(new Ui::ESCtelemetryWidget)
{

    // The following string lists correspond to various ENUM values in the header file.
    // ! The list sizes must match the enum counts !

    // layoutColumns
    m_columnTitles << tr("ID")
                   << tr("Ver")
                   << tr("Volts")
                   << tr("Amps")
                   << tr("Watts")
                   << tr("RPM")
                   << tr("% Duty")
                   << tr("°C")
                   << tr("Err Cnt")
                   << tr("State")
                   << tr("DCode")
                   << tr("Disrm Rsn")
                   << tr("Age (ms)")
                   << tr("Status")
    ;
    // layoutColumns
    m_columnTips << tr("Motor Port Number / ESC ID")
                   << tr("ESC Version")
                   << tr("Voltage")
                   << tr("Current Draw")
                   << tr("Calculated power draw in watts")
                   << tr("Rotations per minute")
                   << tr("FET duty cycle")
                   << tr("ESC temperature at MCU (ESC32 v3 only)")
                   << tr("Commutations error count (ESC32 v2 only)")
                   << tr("Current operating state (internal code)")
                   << tr("Reason for last disarm (internal code)")
                   << tr("Reason for last disarm in plain text")
                   << tr("Age of status data at flight controller.")
                   << tr("Operating state and/or error descriptions.")
    ;
    // escStates
    m_escStateNames << tr("Disarmed")
                    << tr("Stopped")
                    << tr("No Comm")
                    << tr("Starting")
                    << tr("Running")
    ;
    // escDisarmReasons
    m_escDisarmReasonNames  << tr("Startup")
                            << tr("Bad Detects")
                            << tr("Crossing Timeout")
                            << tr("PWM Timeout")
                            << tr("Low Voltage")
                            << tr("CLI User")
                            << tr("Binary User")
                            << tr("CAN User")
                            << tr("CAN Timeout")
    ;
    // escV3DisarmReasons
    m_escV3DisarmReasonNames << tr("None")
                             << tr("CLI User")
                             << tr("Binary User")
                             << tr("CAN User")
                             << tr("Startup")
                             << tr("Bad Detects")
                             << tr("Crossing Timeout")
                             << tr("PWM Timeout")
                             << tr("Low Voltage")
                             << tr("CAN Timeout")
    ;
    // alertTypes
    m_alertTitles << tr("Total Amps")
                  << tr("Avg Volts")
                  << tr("Avg Watts")
                  << tr("ESC Amps")
                  << tr("ESC Volts")
                  << tr("ESC Watts")
                  << tr("ESC RPM")
                  << tr("ESC Temp")
                  << tr("ESC Errors")
    ;
    // alertTypes
    m_alertSettingNames << "ESCTELEM_ALERTS_TTL_A"
                        << "ESCTELEM_ALERTS_AVG_V"
                        << "ESCTELEM_ALERTS_AVG_W"
                        << "ESCTELEM_ALERTS_ESC_A"
                        << "ESCTELEM_ALERTS_ESC_V"
                        << "ESCTELEM_ALERTS_ESC_W"
                        << "ESCTELEM_ALERTS_ESC_R"
                        << "ESCTELEM_ALERTS_ESC_T"
                        << "ESCTELEM_ALERTS_ESC_E"
    ;

    // default visible columnts
    m_visibleColumnsList << COL_ESC_ID << COL_VOLTS << COL_AMPS << COL_RPM << COL_TEMP << COL_ERR_COUNT << COL_STAT;

    // set up data model, proxy model for sorting, and table to display them.
    m_dataModel = new QStandardItemModel(this);
    m_dataModel->setColumnCount(COL_ENUM_COUNT);
    m_dataModel->setHorizontalHeaderLabels(m_columnTitles);
    for (int i=0; i < COL_ENUM_COUNT; ++i)
        m_dataModel->setHeaderData(i, Qt::Horizontal, m_columnTips.at(i), Qt::ToolTipRole);

    m_tableProxyModel = new QSortFilterProxyModel(this);
    m_tableProxyModel->setSourceModel(m_dataModel);
    m_tableProxyModel->setDynamicSortFilter(true);

    ui->setupUi(this);

    ui->tableView->setModel(m_tableProxyModel);
#if QT_VERSION >= 0x050000
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#else
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#endif
    ui->tableView->horizontalHeader()->resizeSection(COL_ESC_ID, 35);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);
    ui->tableView->horizontalHeader()->setSortIndicator(COL_ESC_ID, Qt::AscendingOrder);
    //ui->tableView->horizontalHeader()->resizeSection(COL_DUTY, 60);
    ui->tableView->hide();

    // how long between frequency spin box changed and actual request is sent to UAV
    m_delayedSendFreqTimer.setInterval(800);
    // audio alerts, if enabled, will be played this often
    m_audioAlertsTimer.setInterval(3000);

    //initialize various things
    resetDisplayDataAvailable(false);
    initAlertsMap();
    setControlsUiEnabled(false);
    setDisplayUiEnabled();
    setWidgetTitle();
    loadSettings();
    loadAlertSettings();
    // the following inits should happen after setting are loaded
    setVisibleColumns();
    setWatchdogInterval();

    connect(UASManager::instance(), SIGNAL(activeUASSet(int)), this, SLOT(setUAS(int)));
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(uasDeleted(UASInterface*)));
    connect(&m_delayedSendFreqTimer, SIGNAL(timeout()), this, SLOT(sendRefreshFreq()));
    connect(&m_audioAlertsTimer, SIGNAL(timeout()), this, SLOT(audioAlertTimeout()));
    connect(&m_watchdogTimer, SIGNAL(timeout()), this, SLOT(watchdogTimeout()));
    connect(ui->toolButton_toggleRefresh, SIGNAL(clicked(bool)),this,SLOT(onToggleRefresh(bool)));
    connect(ui->toolButton_options, SIGNAL(clicked()),this,SLOT(optionsDialog()));
    connect(GAudioOutput::instance(), SIGNAL(mutedChanged(bool)), this, SLOT(toggleAlertTimer()));

}

ESCtelemetryWidget::~ESCtelemetryWidget()
{
    writeSettings();
    delete ui;
}

void ESCtelemetryWidget::changeEvent(QEvent *e)
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

void ESCtelemetryWidget::loadSettings()
{
    QSettings settings;

    settings.beginGroup("ESC_TELEMETRY_WIDGET");
    m_refreshFreq = settings.value("REFRESH_FREQ", m_refreshFreq).toInt();
    m_showTable = settings.value("SHOW_TABLE", m_showTable).toBool();
    m_showAlertsOpts = settings.value("SHOW_ALERT_OPTIONS", m_showAlertsOpts).toBool();
    m_enableAudioAlerts = settings.value("ENABLE_AUDIO_ALERTS", m_enableAudioAlerts).toBool();
    m_visibleColumnsList = settings.value("VISIBLE_COLUMNS", m_visibleColumnsList).toList();
    ui->tableView->horizontalHeader()->restoreState(settings.value("TABLE_HHEADER_STATE", ui->tableView->horizontalHeader()->saveState()).toByteArray());
    settings.endGroup();
    settings.sync();
}

void ESCtelemetryWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("ESC_TELEMETRY_WIDGET");

    settings.setValue("REFRESH_FREQ", m_refreshFreq);
    settings.setValue("SHOW_TABLE", m_showTable);
    settings.setValue("SHOW_ALERT_OPTIONS", m_showAlertsOpts);
    settings.setValue("ENABLE_AUDIO_ALERTS", m_enableAudioAlerts);
    settings.setValue("VISIBLE_COLUMNS", m_visibleColumnsList);
    settings.setValue("TABLE_HHEADER_STATE", ui->tableView->horizontalHeader()->saveState());

    settings.sync();
    settings.endGroup();
}

void ESCtelemetryWidget::loadAlertSettings(const int uasid)
{
    QSettings settings;
    QString group("ESC_TELEMETRY_WIDGET");
    if (uasid)
        group = QString("MAV%1").arg(uasid);
    settings.beginGroup(group);
    for (int i=0; i < ALERT_ENUM_COUNT; ++i) {
        alerts_t a = m_alertsMap.value(i);
        a.data->fromVariantList(settings.value(a.settingsName, AlertLevels().toVariantList()).toList());
    }
    settings.sync();
    settings.endGroup();
}

void ESCtelemetryWidget::writeAlertSettings(const int uasid)
{
    QSettings settings;
    QString group("ESC_TELEMETRY_WIDGET");
    if (uasid)
        group = QString("MAV%1").arg(uasid);
    settings.beginGroup(group);
    for (int i=0; i < ALERT_ENUM_COUNT; ++i) {
        alerts_t a = m_alertsMap.value(i);
        settings.setValue(a.settingsName, a.data->toVariantList());
    }
    settings.sync();
    settings.endGroup();
}

void ESCtelemetryWidget::loadUASSettings()
{
    if (!m_uas)
        return;

    QSettings settings;
    int uasid = m_uas->getUASID();
    // test if we have alert settings for this uav
    if (!settings.contains(QString("MAV%1/%2").arg(uasid).arg(m_alertsMap.value(0).settingsName)))
        uasid = 0;  // load defaults
    loadAlertSettings(uasid);

    // check for set battery voltage warning levels for this mav
    if (!uasid) {
        AlertLevels *data = m_alertsMap.value(ALERT_AVG_V).data;
        float warn = m_uas->getBatteryWarnVoltage();
        float err = m_uas->getBatteryEmptyVoltage();
        if (warn >= 0.0 && warn > err) {
            data->enableFlags |= 1<<0;
            data->warnL = warn;
        }
        if (err >= 0.0 && err < warn) {
            data->enableFlags |= 1<<2;
            data->errL = err;
        }
    }
}

void ESCtelemetryWidget::writeUASSettings()
{
    if (!m_uas)
        return;
    writeAlertSettings(m_uas->getUASID());
}

void ESCtelemetryWidget::setWidgetTitle()
{
    QString ttl = tr("ESC Telemetry");
    QString sys = tr("[not connected]");
    if (m_uas && !m_uasTimeout) {
        sys = m_uas->getUASName();
        ttl += tr("  for: %1").arg(sys);
        if (m_dataTimeout)
            ttl += tr(" [DATA TIMEOUT]");
        else if (!m_dataAvailable)
            ttl += tr(" [stopped]");
    } else
        ttl += QString(" %1").arg(sys);

    //ui->label_systemName->setText(QString("System: %1").arg(sys));

    QDockWidget *pwin = qobject_cast<QDockWidget *>(this->parent());
    if (pwin)
        pwin->setWindowTitle(ttl);
    else
        this->setWindowTitle(ttl);
}

void ESCtelemetryWidget::initAlertsMap()
{
    m_alertsMap.clear();
    for (int i=0; i < ALERT_ENUM_COUNT; ++i) {
        alerts_t a;
        a.labelTxt = m_alertTitles.at(i);
        a.settingsName = m_alertSettingNames.at(i);
        a.data = new AlertLevels();
        m_alertsMap.insert(i, a);
    }
}

void ESCtelemetryWidget::setUAS(int id)
{
    if (m_uas)
        uasDeleted(m_uas);

    // Connect the new UAS
    m_uas = UASManager::instance()->getUASForId(id);
    if (m_uas) {
        loadUASSettings();
        setControlsUiEnabled(true);
        setWidgetTitle();

        connect(m_uas, SIGNAL(heartbeatTimeout(bool,unsigned int)), this, SLOT(setUASstatus(bool,unsigned int)));
        connect(m_uas, SIGNAL(dataStreamAnnounced(int,uint8_t,uint16_t,bool)), this, SLOT(dataStreamUpdate(int,uint8_t,uint16_t,bool)));
        connect(m_uas, SIGNAL(escTelemetryUpdate(uint8_t,uint8_t,uint16_t,uint8_t,float,float,uint16_t,float,float,uint16_t,uint8_t)),
                this, SLOT(escTelemetryRcv(uint8_t,uint8_t,uint16_t,uint8_t,float,float,uint16_t,float,float,uint16_t,uint8_t)));
    }
}

void ESCtelemetryWidget::uasDeleted(UASInterface *mav)
{
    if (m_uas && mav->getUASID() == m_uas->getUASID()) {
        disconnect(m_uas, 0, this, 0);
        m_uas = NULL;
        toggleUpdate(false);
        resetDisplayDataAvailable(false);
        loadAlertSettings();
        setWidgetTitle();
        m_dataModel->removeRows(0, m_dataModel->rowCount());
        m_hasV2data = false;
        m_hasV3data = false;
    }
}

void ESCtelemetryWidget::setUASstatus(bool timeout, unsigned int ms)
{
    Q_UNUSED(ms);
    m_uasTimeout = timeout;
    if (m_uas) {
        setControlsUiEnabled(!timeout);
        if (timeout)
            toggleUpdate(false);
        else
            toggleAlertTimer();
    }
    setWidgetTitle();
}

void ESCtelemetryWidget::toggleUpdate(bool enable)
{
    if (!m_uas || m_uasTimeout)
        enable = false;

    m_dataAvailable = enable;
    setDisplayUiEnabled();
    setWatchdogInterval();
    toggleAlertTimer();
    setWidgetTitle();
    ui->toolButton_toggleRefresh->setChecked(enable);
}

void ESCtelemetryWidget::toggleTelemetryMessage(const bool enable)
{
    if (m_uas) {
        m_uas->enableDataStream(MAV_DATA_STREAM_PROPULSION, enable ? m_refreshFreq : 0);
    }
}

void ESCtelemetryWidget::toggleAlertTimer()
{
    if (m_enableAudioAlerts && m_uas && m_dataAvailable && !GAudioOutput::instance()->isMuted())
        m_audioAlertsTimer.start();
    else
        m_audioAlertsTimer.stop();
}

void ESCtelemetryWidget::dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off)
{
    if (m_uas && m_uas->getUASID() == uasId && stream_id == MAV_DATA_STREAM_PROPULSION) {
        if (on_off)
            m_refreshFreq = rate;
    }
}

int ESCtelemetryWidget::setPrecision(const float val) {
    int ret = 2;
    if (val >= 1000.0)
        ret = 0;
    else if (val >= 100.0)
        ret = 1;
// so far all our values are to 100th precision at most. if we had 1000ths, we could enable the next block.
//    else if (val < 10.0)
//        ret = 3;

    return ret;
}

int ESCtelemetryWidget::getStatusType(const ESCtelemetryWidget::AlertLevels *data, const float &val)
{
    int stat = STATUS_OK;
    if ((data->enableFlags & 8) && val > data->errH)
        stat = STATUS_ERR_H;
    else if ((data->enableFlags & 4) && val < data->errL)
        stat = STATUS_ERR_L;
    else if ((data->enableFlags & 2) && val > data->warnH)
        stat = STATUS_WARN_H;
    else if ((data->enableFlags & 1) && val < data->warnL)
        stat = STATUS_WARN_L;

    return stat;
}

QString ESCtelemetryWidget::getPropertyForStatus(const int stat)
{
    QString statprop = "ok";
    if (stat & STATUS_ERR)
        statprop = "error";
    else if (stat & STATUS_WARN)
        statprop = "warning";
    if (statprop != "ok") {
        if (stat & STATUS_HIGH)
            statprop += "H";
        else if (stat & STATUS_LOW)
            statprop += "L";
    }
    return statprop;
}

QBrush ESCtelemetryWidget::getColorForStatus(const int stat)
{
    QBrush color_error(QColor(255, 0, 0, 180)),
            color_warn(QColor(255, 140, 0, 180)),
            color_nodata(QColor(193, 193, 193, 180)),
            color_ok(Qt::NoBrush),
            ret = color_ok;

    if (stat & STATUS_ERR)
        ret = color_error;
    else if (stat & STATUS_WARN)
        ret = color_warn;
    else if (stat == STATUS_NODATA)
        ret = color_nodata;

    return ret;
}

void ESCtelemetryWidget::escTelemetryRcv(uint8_t escId, uint8_t version, uint16_t time, uint8_t state, float volts, float amps, uint16_t rpm, float duty, float temp, uint16_t errCount, uint8_t errCode)
{
    QMap<int, QModelIndex> idxmap;
    QString n("%1"), statusTxt, dsrmRsnName;
    int rowStatus;
    int valStatus;
    int maxValStatus;
    bool nodata;
    float watts;
    QBrush bgcolor;

    if (!m_dataAvailable)
        toggleUpdate(true);

    m_watchdogTimer.start();

    // new esc ID, make new model items for it (new data row)
    if (!m_modelIndexMap.contains(escId)) {
        QStandardItem *itm;
        int rowId = m_dataModel->rowCount();
        for (int i=0; i < COL_ENUM_COUNT; ++i) {
            itm = new QStandardItem();
            m_dataModel->setItem(rowId, i, itm);
            idxmap.insert(i, m_dataModel->indexFromItem(itm));
        }
        m_dataModel->itemFromIndex(idxmap.value(COL_ESC_ID))->setTextAlignment(Qt::AlignCenter);
        m_dataModel->setData(idxmap.value(COL_ESC_ID), false, Qt::UserRole);
        m_dataModel->setData(idxmap.value(COL_ESC_ID), escId);
        m_modelIndexMap.insert(escId, idxmap);
        idxmap.clear();

        if (version >= 30)
            m_hasV3data = true;
        else if (version >= 20)
            m_hasV2data = true;

        setVisibleColumns();
        ui->tableView->resizeRowsToContents();
    }

    // get map of model item indexes key'd by layoutColumns enum.
    idxmap = m_modelIndexMap.value(escId);

    maxValStatus = 0;
    nodata = time == 0xffff;
    rowStatus = STATUS_OK;
    statusTxt = m_escStateNames[state];
    dsrmRsnName = version < 30 ? m_escDisarmReasonNames[errCode] : m_escV3DisarmReasonNames[errCode];

    // check if has had at least one valid data packet but now has no data
    if (nodata && m_dataModel->data(idxmap.value(COL_ESC_ID), Qt::UserRole).toBool()) {
        rowStatus = STATUS_ERR;
        statusTxt = "TIMEOUT";
    }
    // disarmed for a bad reason
    else if (!nodata && state == ESC_STATE_DISARMED && errCode && (
                 (version < 30 && errCode != REASON_CLI_USER && errCode != REASON_BINARY_USER && errCode != REASON_CAN_USER) ||
                 (version >= 30 && errCode > REASON_V3_STARTUP) )
             ) {
        rowStatus = STATUS_ERR;
        statusTxt += QString(" (%1)").arg(dsrmRsnName);
    }
    // no valid data received at all
    else if (nodata) {
        rowStatus = STATUS_NODATA;
        statusTxt = "NO DATA";
    }

    // got valid data
    if (!nodata)
        m_dataModel->itemFromIndex(idxmap.value(COL_ESC_ID))->setData(true, Qt::UserRole);  // mark as having recieved at least one valid data packet

    bgcolor = getColorForStatus(rowStatus);

    m_dataModel->setData(idxmap.value(COL_ESC_ID), escId);
    m_dataModel->itemFromIndex(idxmap.value(COL_ESC_ID))->setBackground(bgcolor);

    m_dataModel->setData(idxmap.value(COL_VERSION), n.arg(version / 10.0, 2, 'f', 1, QLatin1Char('0')));
    m_dataModel->itemFromIndex(idxmap.value(COL_VERSION))->setBackground(bgcolor);

    valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_V).data, volts);
    maxValStatus |= valStatus;
    m_dataModel->setData(idxmap.value(COL_VOLTS), n.arg(volts, 5, 'f', setPrecision(volts), QLatin1Char('0')));
    m_dataModel->itemFromIndex(idxmap.value(COL_VOLTS))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);

    valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_A).data, amps);
    maxValStatus |= valStatus;
    m_dataModel->setData(idxmap.value(COL_AMPS), n.arg(amps, 5, 'f', setPrecision(amps), QLatin1Char('0')));
    m_dataModel->itemFromIndex(idxmap.value(COL_AMPS))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);

    watts = amps * volts;
    valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_W).data, watts);
    maxValStatus |= valStatus;
    m_dataModel->setData(idxmap.value(COL_WATTS), n.arg(watts, 5, 'f', setPrecision(watts), QLatin1Char('0')));
    m_dataModel->itemFromIndex(idxmap.value(COL_WATTS))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);

    valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_R).data, rpm);
    maxValStatus |= valStatus;
    m_dataModel->setData(idxmap.value(COL_RPM), rpm);
    m_dataModel->itemFromIndex(idxmap.value(COL_RPM))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);

    m_dataModel->setData(idxmap.value(COL_DUTY), n.arg(duty, 5, 'f', setPrecision(duty), QLatin1Char('0')));
    m_dataModel->itemFromIndex(idxmap.value(COL_DUTY))->setBackground(bgcolor);

    if (version >= 30) {
        valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_T).data, temp);
        maxValStatus |= valStatus;
        m_dataModel->setData(idxmap.value(COL_TEMP), n.arg(temp, 5, 'f', setPrecision(temp), QLatin1Char('0')));
        m_dataModel->itemFromIndex(idxmap.value(COL_TEMP))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);
        m_dataModel->setData(idxmap.value(COL_ERR_COUNT), "N/A");
        m_dataModel->itemFromIndex(idxmap.value(COL_ERR_COUNT))->setBackground(getColorForStatus(STATUS_NODATA));
    } else {
        valStatus = getStatusType(m_alertsMap.value(ALERT_ESC_E).data, errCount);
        maxValStatus |= valStatus;
        m_dataModel->setData(idxmap.value(COL_ERR_COUNT), errCount);
        m_dataModel->itemFromIndex(idxmap.value(COL_ERR_COUNT))->setBackground(rowStatus == STATUS_OK ? getColorForStatus(valStatus) : bgcolor);
        m_dataModel->setData(idxmap.value(COL_TEMP), "N/A");
        m_dataModel->itemFromIndex(idxmap.value(COL_TEMP))->setBackground(getColorForStatus(STATUS_NODATA));
    }
    m_dataModel->setData(idxmap.value(COL_AGE), time);
    m_dataModel->itemFromIndex(idxmap.value(COL_AGE))->setBackground(bgcolor);

    m_dataModel->setData(idxmap.value(COL_STATE), state);
    m_dataModel->itemFromIndex(idxmap.value(COL_STATE))->setBackground(bgcolor);

    m_dataModel->setData(idxmap.value(COL_DISARM_CODE), errCode);
    m_dataModel->itemFromIndex(idxmap.value(COL_DISARM_CODE))->setBackground(bgcolor);

    m_dataModel->setData(idxmap.value(COL_DISARM_REASON), dsrmRsnName);
    m_dataModel->setData(idxmap.value(COL_DISARM_REASON), dsrmRsnName, Qt::ToolTipRole);
    m_dataModel->itemFromIndex(idxmap.value(COL_DISARM_REASON))->setBackground(bgcolor);

    m_dataModel->setData(idxmap.value(COL_STAT), statusTxt);
    m_dataModel->setData(idxmap.value(COL_STAT), statusTxt, Qt::ToolTipRole);
    m_dataModel->itemFromIndex(idxmap.value(COL_STAT))->setBackground(bgcolor);

    if (rowStatus)
        rowStatus |= maxValStatus;

    if (rowStatus & STATUS_ERR) {
        if (!m_valueErrors.contains(escId))
            m_valueErrors.append(escId);
    } else
        m_valueErrors.removeAll(escId);
    if (rowStatus & STATUS_WARN) {
        if (!m_valueWarnings.contains(escId))
            m_valueWarnings.append(escId);
    } else
        m_valueWarnings.removeAll(escId);

    float ttlA = 0.0;
    float ttlV = 0.0;
    float ttlW = 0.0;
    float avgV = 0.0;
    float avgW = 0.0;
    int ttlRows = 0;
    for (int r=0; r < m_dataModel->rowCount(); ++r) {
        if (m_dataModel->item(r, COL_AGE)->data(Qt::EditRole).toUInt() < m_maxDataAge) {
            ttlRows++;
            ttlA += m_dataModel->item(r, COL_AMPS)->data(Qt::EditRole).toFloat();
            ttlV += m_dataModel->item(r, COL_VOLTS)->data(Qt::EditRole).toFloat();
            ttlW += m_dataModel->item(r, COL_WATTS)->data(Qt::EditRole).toFloat();
        }
    }
    if (ttlRows) {
        avgV = ttlV / (float)ttlRows;
        avgW = ttlW / (float)ttlRows;
    }
    m_ttlA = ttlA;
    m_avgV = avgV;
    m_avgW = avgW;

    updateLCD(ui->lcdNumber_ttlAmps, m_ttlA, ALERT_TOT_A);
    updateLCD(ui->lcdNumber_avgVolts, m_avgV, ALERT_AVG_V);
    updateLCD(ui->lcdNumber_avgWatts, m_avgW, ALERT_AVG_W);

}

void ESCtelemetryWidget::updateLCD(QLCDNumber *wdgt, const float &val, const int typ)
{
    int valkey = -(typ+1);
    int stat = getStatusType(m_alertsMap.value(typ).data, val);
    QString oldprop = wdgt->property("status").toString();
    QString newprop = getPropertyForStatus(stat);
    wdgt->display(QString("%1").arg(val, 5, 'f', setPrecision(val), QLatin1Char('0')));
    wdgt->setProperty("status", newprop);
    if (newprop != oldprop) {
        if (stat & STATUS_ERR) {
            if (!m_valueErrors.contains(valkey))
                m_valueErrors.append(valkey);
        } else
            m_valueErrors.removeAll(valkey);
        if (stat & STATUS_WARN) {
            if (!m_valueWarnings.contains(valkey))
                m_valueWarnings.append(valkey);
        } else
            m_valueWarnings.removeAll(valkey);

        wdgt->style()->unpolish(wdgt);
        wdgt->style()->polish(wdgt);
        wdgt->update();
    }
}

void ESCtelemetryWidget::delayedSendRefreshFreq(int rate)
{
    if (m_refreshFreq != rate) {
        m_refreshFreq = rate;
        m_delayedSendFreqTimer.start();
        setWatchdogInterval();
    }
}

void ESCtelemetryWidget::sendRefreshFreq()
{
    m_delayedSendFreqTimer.stop();
    toggleUpdate(m_refreshFreq);
    toggleTelemetryMessage(m_refreshFreq);
}

void ESCtelemetryWidget::resetDisplayDataAvailable(const bool enable)
{
    m_firstDataAvailable = enable;
    if (enable) {
        ui->label_nodata->hide();
        ui->gridLayout->removeWidget(ui->label_nodata);
        ui->gridLayout->addWidget(ui->frame_lcd, 0, 0);
        ui->frame_lcd->show();
        setTableVisible();
    } else {
        ui->tableView->hide();
        ui->frame_lcd->hide();
        ui->gridLayout->removeWidget(ui->frame_lcd);
        ui->gridLayout->removeWidget(ui->label_nodata);
        ui->gridLayout->addWidget(ui->label_nodata, 0, 0);
        ui->label_nodata->show();
    }
}

void ESCtelemetryWidget::setDisplayUiEnabled()
{
    //ui->tableView->setEnabled(m_dataAvailable);
    if (m_dataAvailable && !m_firstDataAvailable)
        resetDisplayDataAvailable(true);
    ui->lcdNumber_ttlAmps->setEnabled(m_dataAvailable);
    ui->lcdNumber_avgVolts->setEnabled(m_dataAvailable);
    ui->lcdNumber_avgWatts->setEnabled(m_dataAvailable);
}

void ESCtelemetryWidget::setControlsUiEnabled(const bool enable)
{
    ui->toolButton_toggleRefresh->setEnabled(enable);
}

void ESCtelemetryWidget::setTableVisible()
{
    bool show = m_showTable && !ui->label_nodata->isVisible();
    ui->tableView->setVisible(show);
    ui->tableView->setEnabled(show);
}

void ESCtelemetryWidget::setVisibleColumns() {
    bool show;
    for (int i=0; i < COL_ENUM_COUNT; ++i) {
        show = m_visibleColumnsList.contains(i);
        if ((i == COL_ERR_COUNT && !m_hasV2data) || (i == COL_TEMP && !m_hasV3data))
            show = false;
        if (show)
            ui->tableView->showColumn(i);
        else
            ui->tableView->hideColumn(i);
    }
}

void ESCtelemetryWidget::setWatchdogInterval()
{
    if (!m_refreshFreq || !m_dataAvailable)
        m_watchdogTimer.stop();
    else {
        m_watchdogTimer.setInterval(1000 / m_refreshFreq * 7);
        m_dataTimeout = false;
    }
}

void ESCtelemetryWidget::setAlertRowRanges(const int row, const int changedValIdx)
{
    QDoubleSpinBox *warnL = qobject_cast<QDoubleSpinBox *>(m_gridAlerts->itemAtPosition(row, 3)->widget());
    QDoubleSpinBox *warnH = qobject_cast<QDoubleSpinBox *>(m_gridAlerts->itemAtPosition(row, 6)->widget());
    QDoubleSpinBox *errL = qobject_cast<QDoubleSpinBox *>(m_gridAlerts->itemAtPosition(row, 9)->widget());
    QDoubleSpinBox *errH = qobject_cast<QDoubleSpinBox *>(m_gridAlerts->itemAtPosition(row, 12)->widget());

    if (!warnL || !warnH || !errL || !errH)
        return;

    m_mutexAlertRowsUpdating = true;

    if (changedValIdx == 0) {  // warnL
        warnH->setValue(qMax(warnL->value(), warnH->value()));
        errH->setValue(qMax(errH->value(), warnH->value()));
        errL->setValue(qMin(warnL->value(), errL->value()));
    }

    if (changedValIdx == 1) {  // warnH
        errH->setValue(qMax(errH->value(), warnH->value()));
        warnL->setValue(qMin(warnL->value(), warnH->value()));
        errL->setValue(qMin(errL->value(), warnL->value()));
    }

    if (changedValIdx == 2) {  // errL
        errH->setValue(qMax(errL->value(), errH->value()));
        warnH->setValue(qMax(errL->value(), warnH->value()));
        warnL->setValue(qMax(errL->value(), warnL->value()));
    }

    if (changedValIdx == 3) {  // errH
        warnH->setValue(qMin(errH->value(), warnH->value()));
        warnL->setValue(qMin(warnL->value(), warnH->value()));
        errL->setValue(qMin(errL->value(), warnL->value()));
    }

    m_mutexAlertRowsUpdating = false;
}

void ESCtelemetryWidget::watchdogTimeout()
{
    m_watchdogTimer.stop();
    m_dataTimeout = true;
    toggleUpdate(false);
}

void ESCtelemetryWidget::audioAlertTimeout()
{
    if (!m_enableAudioAlerts) {
        toggleAlertTimer();
        return;
    }
    if (m_valueErrors.size())
        GAudioOutput::instance()->beep();
    else if (m_valueWarnings.size())
        GAudioOutput::instance()->notifyNegative();
}

void ESCtelemetryWidget::onToggleRefresh(const bool on)
{
    if (!on || !m_uas)
        toggleUpdate(false);
    else if (!m_refreshFreq)
        m_refreshFreq = 1;

    toggleTelemetryMessage(on);
}

void ESCtelemetryWidget::onAudioAlertsSelected(const bool on)
{
    m_enableAudioAlerts = on;
    toggleAlertTimer();
}

void ESCtelemetryWidget::onColumnSelected(const bool on)
{
    if (!sender())
        return;
    int i = qobject_cast<QCheckBox *>(sender())->property("col_id").toInt();
    if (on && ! m_visibleColumnsList.contains(i))
        m_visibleColumnsList.append(i);
    else if (!on)
        m_visibleColumnsList.removeAll(i);

    setVisibleColumns();
}

void ESCtelemetryWidget::onShowTableSelected(const bool on)
{
    m_showTable = on;
    setTableVisible();
}

void ESCtelemetryWidget::onAlertCheckboxSelected(const bool on)
{
    if (!sender())
        return;

    AlertLevels *data = sender()->property("ptr").value<AlertLevels *>();
    if (!data)
        return;

    unsigned bit = sender()->property("bit").toUInt();
    data->enableFlags = (data->enableFlags & ~(1 << bit)) | ((unsigned)on << bit);
}

void ESCtelemetryWidget::onAlertValueChanged(const double val)
{
    if (!sender())
        return;

    AlertLevels *data = sender()->property("ptr").value<AlertLevels *>();
    if (!data)
        return;

    int type = sender()->property("type").toUInt();
    data->setValueByIndex(type, val);

    if (!m_mutexAlertRowsUpdating) {
        if (QCheckBox *chk = (QCheckBox *)sender()->property("chkptr").value<void *>())
            chk->setChecked(true);

        bool ok;
        int type = sender()->property("type").toUInt(&ok);
        int row = sender()->property("rownum").toInt(&ok);
        if (ok)
            setAlertRowRanges(row, type);
    }
}

QLabel* ESCtelemetryWidget::makeOptionsLabel(const QString &title, const Qt::Alignment align)
{
    QString css = "font-weight: bold; font-size: 10px;";
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel *label = new QLabel(title, m_optionsDialog);
    label->setSizePolicy(sizePolicy);
    label->setStyleSheet(css);
    label->setAlignment(align);

    return label;
}

QCheckBox* ESCtelemetryWidget::makeAlertCheckbox(AlertLevels *data, const unsigned &typ)
{
    QSizePolicy sizePolicy_ff(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QCheckBox *chk = new QCheckBox("", m_optionsDialog);
    chk->setSizePolicy(sizePolicy_ff);
    chk->setStyleSheet("padding-right: 0px;");
    chk->setChecked(data->enableFlags & (1<<typ));
    chk->setProperty("bit", typ);
    chk->setProperty("ptr", QVariant::fromValue<AlertLevels *>(data));
    connect(chk, SIGNAL(toggled(bool)), this, SLOT(onAlertCheckboxSelected(bool)));

    return chk;
}

QDoubleSpinBox* ESCtelemetryWidget::makeAlertSpinbox(AlertLevels *data, const unsigned &typ)
{
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QDoubleSpinBox *sb = new QDoubleSpinBox(m_optionsDialog);
    sb->setSizePolicy(sizePolicy);
    //sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sb->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    sb->setKeyboardTracking(false);
    sb->setRange(-999.99, 99999.99);
    sb->setValue(data->getValueByIndex(typ));
    sb->setProperty("type", typ);
    sb->setProperty("ptr", QVariant::fromValue<AlertLevels *>(data));
    connect(sb, SIGNAL(valueChanged(double)), this, SLOT(onAlertValueChanged(double)));

    return sb;
}

void ESCtelemetryWidget::optionsDialog()
{
    QSizePolicy sizePolicy_ff(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSizePolicy sizePolicy_pf(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QSizePolicy sizePolicy_fp(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_optionsDialog = new QDialog(this);
    m_optionsDialog->setSizeGripEnabled(true);
    m_optionsDialog->setWindowTitle(tr("ESC Telemetry Options"));
    m_optionsDialog->setWindowModality(Qt::ApplicationModal);
    m_optionsDialog->setWindowFlags(m_optionsDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* dlgLayout = new QVBoxLayout;
    dlgLayout->setSpacing(8);

    QLabel *label_freq = new QLabel(m_optionsDialog);
    label_freq->setSizePolicy(sizePolicy_ff);
    label_freq->setText(tr("Update Frequency:"));
#ifndef QT_NO_TOOLTIP
    label_freq->setToolTip(tr("Telemetry update frequency in Hz (times per second)"));
#endif

    QSpinBox *spinBox_RefreshFreq = new QSpinBox(m_optionsDialog);
    spinBox_RefreshFreq->setObjectName(QString::fromUtf8("spinBox_RefreshFreq"));
    spinBox_RefreshFreq->setSizePolicy(sizePolicy_ff);
    spinBox_RefreshFreq->setMaximum(100);
    spinBox_RefreshFreq->setMinimum(0);
    spinBox_RefreshFreq->setValue(m_refreshFreq);
    spinBox_RefreshFreq->setSuffix(tr(" Hz"));
#ifndef QT_NO_TOOLTIP
    spinBox_RefreshFreq->setToolTip(tr("<html><head/><body><p>Refresh frequency of telemetry data.  Higher frequencies consume more processor power on this computer and the flight controller, and more data bandwidth on your telemetry link.</p></body></html>"));
#endif

    QCheckBox* checkBox_showTable = new QCheckBox(tr("Show Data Table"), m_optionsDialog);
    checkBox_showTable->setSizePolicy(sizePolicy_pf);
    checkBox_showTable->setChecked(m_showTable);

    QCheckBox* checkBox_audio = new QCheckBox(tr("Play Audio Alerts"), m_optionsDialog);
    checkBox_audio->setSizePolicy(sizePolicy_pf);
    checkBox_audio->setChecked(m_enableAudioAlerts);
#ifndef QT_NO_TOOLTIP
    checkBox_audio->setToolTip(tr("<html><head/><body><p>Note: Audio alerts will never play when the 'Mute Audio Output' program option is selected.</p></body></html>"));
#endif

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(2);
    vLayout->addWidget(checkBox_showTable, 0, Qt::AlignLeft | Qt::AlignVCenter);
    vLayout->addWidget(checkBox_audio, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(4);
    hLayout->addWidget(label_freq, 0, Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addWidget(spinBox_RefreshFreq, 1, Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addLayout(vLayout);

    dlgLayout->addLayout(hLayout);

    QGroupBox *grpBox_columns = new QGroupBox(m_optionsDialog);
    grpBox_columns->setTitle(tr("Visible Table Columns"));
    grpBox_columns->setEnabled(m_showTable);

    QFormLayout* columnsLayout = new QFormLayout;
    columnsLayout->setVerticalSpacing(3);
    columnsLayout->setHorizontalSpacing(4);
    columnsLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    columnsLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    for (int i=0; i < COL_ENUM_COUNT; ++i) {
        if (i == COL_ESC_ID)
            continue;
        QCheckBox* chk = new QCheckBox(m_columnTitles.at(i), m_optionsDialog);
        chk->setSizePolicy(sizePolicy_pf);
        chk->setProperty("col_id", i);
        chk->setChecked(m_visibleColumnsList.contains(i));
        QLabel *lbl = new QLabel(m_columnTips.at(i), m_optionsDialog);
        lbl->setSizePolicy(sizePolicy_fp);
        columnsLayout->addRow(chk, lbl);
        connect(chk, SIGNAL(clicked(bool)), this, SLOT(onColumnSelected(bool)));
    }

    grpBox_columns->setLayout(columnsLayout);
    dlgLayout->addWidget(grpBox_columns);

    QGroupBox *grpBox_alerts = new QGroupBox(m_optionsDialog);
    if (m_uas)
        grpBox_alerts->setTitle(tr("Alert Settings for %1").arg(m_uas->getUASName()));
    else
        grpBox_alerts->setTitle(tr("Default Alert Settings for New Systems"));
    grpBox_alerts->setCheckable(true);
    grpBox_alerts->setFlat(true);
    grpBox_alerts->setChecked(m_showAlertsOpts);

    QVBoxLayout *layout_alerts = new QVBoxLayout(grpBox_alerts);
    layout_alerts->setContentsMargins(0,0,0,0);

    QFrame *frame_alerts = new QFrame(grpBox_alerts);
    frame_alerts->setVisible(m_showAlertsOpts);

    layout_alerts->addWidget(frame_alerts);

    m_gridAlerts = new QGridLayout(frame_alerts);
    m_gridAlerts->setVerticalSpacing(5);
    m_gridAlerts->setHorizontalSpacing(1);
    m_gridAlerts->setContentsMargins(5, 5, 5, 4);

    m_gridAlerts->addWidget(makeOptionsLabel(tr("Warning When")), 0, 2, 1, 5, Qt::AlignHCenter);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Error When")), 0, 8, 1, 5, Qt::AlignHCenter);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Value"), Qt::AlignLeft), 1, 0, 1, 1);
    m_gridAlerts->addItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 1, 1, 1);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Below (V<x)")), 1, 2, 1, 2, Qt::AlignHCenter);
    m_gridAlerts->addItem(new QSpacerItem(15, 1, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 4, 1, 1);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Above (V>x)")), 1, 5, 1, 2, Qt::AlignHCenter);
    m_gridAlerts->addItem(new QSpacerItem(25, 1, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 7, 1, 1);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Below (V<x)")), 1, 8, 1, 2, Qt::AlignHCenter);
    m_gridAlerts->addItem(new QSpacerItem(15, 1, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 10, 1, 1);
    m_gridAlerts->addWidget(makeOptionsLabel(tr("Above (V>x)")), 1, 11, 1, 2, Qt::AlignHCenter);

    m_gridAlerts->setColumnStretch(3, 1);
    m_gridAlerts->setColumnStretch(6, 1);
    m_gridAlerts->setColumnStretch(9, 1);
    m_gridAlerts->setColumnStretch(12, 1);

    for (int i=0; i < ALERT_ENUM_COUNT; ++i) {
        int row = 2+i;
        alerts_t a = m_alertsMap.value(i);
        QLabel *label = new QLabel(a.labelTxt, m_optionsDialog);
        label->setSizePolicy(sizePolicy_ff);
        m_gridAlerts->addWidget(label, row, 0, 1, 1);
        int col = 2;
        for (int j=0; j < 4; ++j) {
            QCheckBox *chk = makeAlertCheckbox(a.data, j);
            QDoubleSpinBox *spn = makeAlertSpinbox(a.data, j);
            spn->setProperty("chkptr", QVariant::fromValue<void *>(chk));
            spn->setProperty("rownum", row);
            m_gridAlerts->addWidget(chk, row, col++, 1, 1);
            m_gridAlerts->addWidget(spn, row, col++, 1, 1);
            col++;  // allow for spacer
        }
        //setAlertRowRanges(row);
    }

    dlgLayout->addWidget(grpBox_alerts);

    QDialogButtonBox* bbox = new QDialogButtonBox(Qt::Horizontal, m_optionsDialog);
    QPushButton *btn_cancel = bbox->addButton(tr("Close"), QDialogButtonBox::RejectRole);
    btn_cancel->setDefault(true);

    dlgLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    dlgLayout->addWidget(bbox, Qt::AlignBottom);

    m_optionsDialog->setLayout(dlgLayout);

    connect(btn_cancel, SIGNAL(clicked()), m_optionsDialog, SLOT(reject()));
    connect(spinBox_RefreshFreq, SIGNAL(valueChanged(int)), this, SLOT(delayedSendRefreshFreq(int)));
    connect(checkBox_showTable, SIGNAL(clicked(bool)), this, SLOT(onShowTableSelected(bool)));
    connect(checkBox_showTable, SIGNAL(clicked(bool)), grpBox_columns, SLOT(setEnabled(bool)));
    connect(checkBox_audio, SIGNAL(clicked(bool)), this, SLOT(onAudioAlertsSelected(bool)));
    connect(grpBox_alerts, SIGNAL(toggled(bool)), frame_alerts, SLOT(setVisible(bool)));

    m_optionsDialog->exec();
    m_showAlertsOpts = grpBox_alerts->isChecked();
    m_gridAlerts->deleteLater();
    m_optionsDialog->deleteLater();
    writeSettings();
    if (m_uas)
        writeUASSettings();
    else
        writeAlertSettings();

}
