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

#include <QtGlobal>

#include <float.h>
#include <UASInfoWidget.h>
#include <UASManager.h>
#include <MG.h>
#include <QTimer>
#include <QDir>
#include <cstdlib>
#include <cmath>

#include <QDebug>

UASInfoWidget::UASInfoWidget(QWidget *parent, QString name) : QWidget(parent)
{
    ui.setupUi(this);
    this->name = name;

    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));

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
    errors = QMap<QString, int>();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    updateTimer->start(updateInterval);

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
    updateTimer->start(updateInterval);
}

void UASInfoWidget::hideEvent(QHideEvent* event)
{
    // React only to internal (pre-display)
    // events
    Q_UNUSED(event);
    updateTimer->stop();
}

void UASInfoWidget::addUAS(UASInterface* uas)
{
    if (uas != NULL) {
        connect(uas, SIGNAL(batteryChanged(UASInterface*,double,double,int)), this, SLOT(updateBattery(UASInterface*,double,double,int)));
        connect(uas, SIGNAL(dropRateChanged(int,float)), this, SLOT(updateReceiveLoss(int,float)));
        connect(uas, SIGNAL(loadChanged(UASInterface*, double)), this, SLOT(updateCPULoad(UASInterface*,double)));
        connect(uas, SIGNAL(errCountChanged(int,QString,QString,int)), this, SLOT(updateErrorCount(int,QString,QString,int)));
        connect(uas, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(updateRSSI(float)));
        connect(uas, SIGNAL(valueChanged(int,QString,QString,quint16,quint64)), this, SLOT(updateGpsAcc(int,QString,QString,quint16,quint64)));
        connect(uas, SIGNAL(gpsLocalizationChanged(UASInterface*,int)), this, SLOT(updateGpsFix(UASInterface*,int)));

        // Set this UAS as active if it is the first one
        if (activeUAS == 0) activeUAS = uas;
    }
}

void UASInfoWidget::setActiveUAS(UASInterface* uas)
{
    activeUAS = uas;
}

void UASInfoWidget::updateBattery(UASInterface* uas, double voltage, double percent, int seconds)
{
    setVoltage(uas, voltage);
    setChargeLevel(uas, percent);
    setTimeRemaining(uas, seconds);
}

void UASInfoWidget::updateErrorCount(int uasid, QString component, QString device, int count)
{
    //qDebug() << __FILE__ << __LINE__ << activeUAS->getUASID() << "=" << uasid;
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

void UASInfoWidget::updateGpsAcc(const int uasId, const QString &name, const QString &unit, const quint16 val, const quint64 msec)
{
    Q_UNUSED(uasId);
    Q_UNUSED(unit);
    Q_UNUSED(msec);
    if (name.contains(QString("eph")))
        gpsEph = (float)val/100.0f;
    else if (name.contains(QString("epv")))
        gpsEpv = (float)val/100.0f;

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

    ui.voltageLabel->setText(QString::number(this->voltage, 'f', voltageDecimals));
    ui.batteryBar->setValue(qMax(0,qMin(static_cast<int>(this->chargeLevel), 100)));

    ui.loadLabel->setText(QString::number(this->load, 'f', loadDecimals));
    ui.loadBar->setValue(qMax(0, qMin(static_cast<int>(this->load), 100)));

    ui.receiveLossBar->setValue(qMax(0, qMin(static_cast<int>(receiveLoss), 100)));
    ui.receiveLossLabel->setText(QString::number(receiveLoss, 'f', 2));

    ui.sendLossBar->setValue(sendLoss);
    ui.sendLossLabel->setText(QString::number(sendLoss, 'f', 2));

    ui.rssiLabel->setText(text.sprintf("%6.2f", this->rssi));
    ui.rssiBar->setValue(qMax(0, qMin(static_cast<int>(this->rssi), 99)));

    text = "No fix";
    color = "red";
    if (gpsFixType == 2) {
        text = "2D fix";
        color = "yellow";
    }
    else if (gpsFixType == 3) {
        text = "3D fix";
        color = "green";
    }
    ui.fixTypeLabel->setText(text);
    ui.fixTypeLabel->setStyleSheet("font-weight: bold; color: " + color + ";");

    if (gpsEph > 0.0f)
        ui.haccLabel->setText("Hacc: " + QString::number(gpsEph, 'f', 2) + "m");
    if (gpsEpv > 0.0f)
        ui.vaccLabel->setText("Vacc: " + QString::number(gpsEpv, 'f', 2) + "m");

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
