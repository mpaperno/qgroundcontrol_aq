/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Implementation of class UASInfoWidget
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include "MG.h"
#include "QGC.h"
#include "UASInfoWidget.h"
#include "UASManager.h"

#include <QTimer>
#include <QDir>
#include <cstdlib>
#include <cmath>
//#include <QDockWidget>

#include <QDebug>

UASInfoWidget::UASInfoWidget(QWidget *parent, QString name) : QWidget(parent)
{
    ui.setupUi(this);
    this->name = name;

    activeUAS = NULL;

    //instruments = new QMap<QString, QProgressBar*>();

    // Set default battery type
    //    setBattery(0, LIPOLY, 3);
    startTime = MG::TIME::getGroundTimeNow();
    //    startVoltage = 0.0f;

    //    lastChargeLevel = 0.5f;
    //    lastRemainingTime = 1;

    // Set default values
    /** Set two voltage decimals and zero charge level decimals **/
    this->voltageDecimals = 2;
    this->loadDecimals = 2;

    this->voltage = 0;
    this->chargeLevel = 0;
    this->load = 0;
    this->rssi = 0;
    this->gpsFixType = 0;
    this->gpsEph = -1;
    this->gpsEpv = -1;
    receiveLoss = 0;
    sendLoss = 0;
    changed = true;
    m_uasTimeout = false;
    errors = QMap<QString, int>();

    updateTimer = new QTimer(this);
    updateTimer->setInterval(updateInterval);
    updateTimer->stop();
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(addUAS(UASInterface*)));
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(removeUAS(UASInterface*)));

    this->setVisible(false);
}

UASInfoWidget::~UASInfoWidget()
{

}

void UASInfoWidget::showEvent(QShowEvent* event)
{
    // React only to internal (pre-display)
    // events
    Q_UNUSED(event);
    setWidgetTitle();
    refresh();
    updateTimer->start();
}

void UASInfoWidget::hideEvent(QHideEvent* event)
{
    // React only to internal (pre-display)
    // events
    Q_UNUSED(event);
    updateTimer->stop();
}

void UASInfoWidget::setWidgetTitle()
{
    QString ttl = tr("Status Details");
    QString sys = tr("[not connected]");
    if (activeUAS && !m_uasTimeout) {
        sys = activeUAS->getUASName();
        ttl += " " % tr("for system: %1").arg(sys);
        this->setEnabled(true);
    } else {
        ttl += QString(": %1").arg(sys);
        this->setEnabled(false);
        ui.fixTypeLabel->setStyleSheet("");
    }

    ui.label_uavName->setText(ttl);

//    QDockWidget *pwin = qobject_cast<QDockWidget *>(this->parent());
//    if (pwin)
//        pwin->setWindowTitle(ttl);
//    else
//        this->setWindowTitle(ttl);
}

void UASInfoWidget::addUAS(UASInterface* uas)
{
    if (uas != NULL && uas->getUASID() == UASManager::instance()->getActiveUAS()->getUASID())
        setActiveUAS(uas);
}

void UASInfoWidget::removeUAS(UASInterface *uas)
{
    disconnect(uas, 0, this, 0);
    if (activeUAS && activeUAS == uas)
        setActiveUAS(NULL);
}

void UASInfoWidget::setActiveUAS(UASInterface* uas)
{
    if (uas != NULL && (!activeUAS || activeUAS->getUASID() != uas->getUASID())) {
        if (activeUAS)
            disconnect(activeUAS, 0, this, 0);

        activeUAS = uas;
        m_uasTimeout = false;

        connect(activeUAS, SIGNAL(batteryChanged(UASInterface*,double,double,int)), this, SLOT(updateBattery(UASInterface*,double,double,int)));
        connect(activeUAS, SIGNAL(dropRateChanged(int,float)), this, SLOT(updateReceiveLoss(int,float)));
        connect(activeUAS, SIGNAL(loadChanged(UASInterface*, double)), this, SLOT(updateCPULoad(UASInterface*,double)));
        connect(activeUAS, SIGNAL(errCountChanged(int,QString,QString,int)), this, SLOT(updateErrorCount(int,QString,QString,int)));
        connect(activeUAS, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(updateRSSI(float)));
        connect(activeUAS, SIGNAL(valueChanged(int,QString,QString,QVariant,quint64)), this, SLOT(updateGpsAcc(int,QString,QString,QVariant,quint64)));
        // connect(activeUAS, SIGNAL(gpsLocalizationChanged(UASInterface*,int)), this, SLOT(updateGpsFix(UASInterface*,int)));
        connect(activeUAS, SIGNAL(heartbeatTimeout(bool,unsigned int)), this, SLOT(setUASstatus(bool,unsigned int)));

        updateTimer->start();
    }
    else if (!uas && activeUAS) {
        updateTimer->stop();
        activeUAS = NULL;
    }

    setWidgetTitle();
    refresh();
}

void UASInfoWidget::setUASstatus(bool timeout, unsigned int ms)
{
    Q_UNUSED(ms);
    if (activeUAS) {
        bool t = m_uasTimeout;
        m_uasTimeout = timeout;
        if (t != timeout) {
            setWidgetTitle();
            refresh();
        }
        if (timeout)
            updateTimer->stop();
        else
            updateTimer->start();
    }
}

void UASInfoWidget::updateBattery(UASInterface* uas, double voltage, double percent, int seconds)
{
    setVoltage(uas, voltage);
    setChargeLevel(uas, percent);
    setTimeRemaining(uas, seconds);
}

void UASInfoWidget::updateErrorCount(int uasid, QString component, QString device, int count)
{
    //qDebug()  << activeUAS->getUASID() << "=" << uasid;
    if (activeUAS->getUASID() == uasid) {
        errors.remove(component + ":" + device);
        errors.insert(component + ":" + device, count);
    }
}

/**
 *
 */
void UASInfoWidget::updateCPULoad(UASInterface* uas, double load)
{
    if (activeUAS == uas) {
        this->load = load;
    }
}

void UASInfoWidget::updateReceiveLoss(int uasId, float receiveLoss)
{
    Q_UNUSED(uasId);
    this->receiveLoss = this->receiveLoss * 0.8f + receiveLoss * 0.2f;
}

/**
  The send loss is typically calculated on the GCS based on packets
  that were received scrambled from the MAV
 */
void UASInfoWidget::updateSendLoss(int uasId, float sendLoss)
{
    Q_UNUSED(uasId);
    this->sendLoss = this->sendLoss * 0.8f + sendLoss * 0.2f;
}

void UASInfoWidget::updateRSSI(float rssi)
{
    if (rssi >= 99.0f)
        rssi = 100;
    this->rssi = rssi;
}

void UASInfoWidget::updateGpsFix(UASInterface* uas, const int fix) {
    if (activeUAS == uas)
        gpsFixType = fix;
}

void UASInfoWidget::updateGpsAcc(const int uasId, const QString &name, const QString &unit, const QVariant val, const quint64 msec)
{
    Q_UNUSED(unit);
    Q_UNUSED(msec);
    if (activeUAS->getUASID() != uasId)
        return;
    if (name.contains(QString("eph")))
        gpsEph = val.toFloat()/100.0f;
    else if (name.contains(QString("epv")))
        gpsEpv = val.toFloat()/100.0f;
    else if (name.contains(QString("fix_type")))
        gpsFixType = val.toUInt();

}

void UASInfoWidget::setVoltage(UASInterface* uas, double voltage)
{
    if (activeUAS == uas)
        this->voltage = voltage;
}

void UASInfoWidget::setChargeLevel(UASInterface* uas, double chargeLevel)
{
    if (activeUAS == uas) {
        this->chargeLevel = chargeLevel;
    }
}

void UASInfoWidget::setTimeRemaining(UASInterface* uas, double seconds)
{
    if (activeUAS == uas) {
        this->timeRemaining = seconds;
    }
}

void UASInfoWidget::refresh()
{
    QString text;
    QString color;
    float gpsPrct;

    if (activeUAS && activeUAS->getCommunicationStatus() == UASInterface::COMM_CONNECTED) {
        ui.voltageLabel->setText(QString::number(this->voltage, 'f', voltageDecimals));
        ui.batteryBar->setValue(qMax(0,qMin(static_cast<int>(this->chargeLevel), 100)));

        ui.loadLabel->setText(QString::number(this->load, 'f', loadDecimals));
        ui.loadBar->setValue(qMax(0, qMin(static_cast<int>(this->load), 100)));

        ui.receiveLossBar->setValue(qMax(0, qMin(static_cast<int>(receiveLoss), 100)));
        ui.receiveLossLabel->setText(QString::number(receiveLoss, 'f', 2));

        ui.sendLossBar->setValue(sendLoss);
        ui.sendLossLabel->setText(QString::number(sendLoss, 'f', 2));

        ui.rssiLabel->setText(text.sprintf("%6.2f", this->rssi));
        ui.rssiBar->setValue(qMax(0, qMin(static_cast<int>(this->rssi), 100)));

        text = tr("No fix");
        color = QGC::colorTextErr.name(QColor::HexArgb);
        if (gpsFixType == 2) {
            text = tr("2D");
            color = QGC::colorTextWarn.name(QColor::HexArgb);
        }
        else if (gpsFixType == 3) {
            text = tr("3D");
            color = QGC::colorTextOK.name(QColor::HexArgb);
        }
        ui.fixTypeLabel->setText(text);
        if (ui.fixTypeLabel->isEnabled())
            ui.fixTypeLabel->setStyleSheet("QLabel {color: " + color + ";}");

        if (gpsEph > 0.0f) {
            ui.hAcc->setText(QString::number(gpsEph, 'f', 2));
            gpsPrct = qMin((float)ui.hAccBar->maximum(), gpsEph * 10.0f);
            ui.hAccBar->setValue(ui.hAccBar->maximum() - qRound(gpsPrct));
        }
        if (gpsEpv > 0.0f) {
            ui.vAcc->setText(QString::number(gpsEpv, 'f', 2));
            gpsPrct = qMin((float)ui.vAccBar->maximum(), gpsEpv * 10.0f);
            ui.vAccBar->setValue(ui.vAccBar->maximum() - qRound(gpsPrct));
        }

        QString errorString;
        QMapIterator<QString, int> i(errors);
        while (i.hasNext()) {
            i.next();
            errorString += QString(i.key() + ": %1 ").arg(i.value());

            // FIXME
            errorString.replace("IMU:", "");


        }
        ui.errorLabel->setText(errorString);
    }
    else {
        ui.voltageLabel->setText("0");
        ui.batteryBar->setValue(0);

        ui.loadLabel->setText("0");
        ui.loadBar->setValue(0);

        ui.receiveLossLabel->setText("0");
        ui.sendLossBar->setValue(0);

        ui.sendLossLabel->setText("0");
        ui.sendLossBar->setValue(0);

        ui.rssiLabel->setText("0");
        ui.rssiBar->setValue(0);

        ui.fixTypeLabel->setText(tr("No fix"));
        if (ui.fixTypeLabel->isEnabled())
            ui.fixTypeLabel->setStyleSheet(QString("QLabel{ color: %1; }").arg(QGC::colorTextErr.name(QColor::HexArgb)));
        ui.hAcc->setText("0.00");
        ui.vAcc->setText("0.00");
        ui.hAccBar->setValue(0);
        ui.vAccBar->setValue(0);

        if (activeUAS && m_uasTimeout)
            ui.errorLabel->setText(tr("SYSTEM TIMEOUT"));
        else
            ui.errorLabel->setText(tr("No System Connected"));
    }
}
