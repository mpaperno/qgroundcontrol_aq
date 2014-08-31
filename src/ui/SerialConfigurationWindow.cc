/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2010 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Implementation of SerialConfigurationWindow
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QDebug>

#include "MG.h"
#include <SerialConfigurationWindow.h>
#include <SerialLinkInterface.h>
#include <QDir>
#include <QSettings>
#include <QFileInfoList>
#include <QMessageBox>
#include "qextserialenumerator.h"

SerialConfigurationWindow::SerialConfigurationWindow(LinkInterface* link, QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags),
    userConfigured(false)
{
    SerialLinkInterface* serialLink = dynamic_cast<SerialLinkInterface*>(link);

    if (!serialLink) {
        qDebug() << "Link is NOT a serial link, can't open configuration window";
        return;
    }

    serialLink->loadSettings();
    this->link = serialLink;

    // Setup the user interface according to link type
    ui.setupUi(this);

    // Create action to open this menu
    // Create configuration action for this link
    // Connect the current UAS
    action = new QAction(QIcon(":/files/images/devices/network-wireless.svg"), "", link);
    setLinkName(link->getName());


    portEnumerator = new QextSerialEnumerator();
    portEnumerator->setUpNotifications();
    QObject::connect(portEnumerator, SIGNAL(deviceDiscovered(QextPortInfo)), this, SLOT(setupPortList()));
    QObject::connect(portEnumerator, SIGNAL(deviceRemoved(QextPortInfo)), this, SLOT(setupPortList()));

    setupPortList();

    ui.portName->setCurrentIndex(ui.portName->findText(this->link->getPortName(), Qt::MatchStartsWith));

    // Set up baud rates
    QList<int> supportedBaudRates = MG::SERIAL::getBaudRates();
    ui.baudRate->clear();
    for (int i = 0; i < supportedBaudRates.size(); ++i) {
        ui.baudRate->addItem(QString::number(supportedBaudRates.at(i)), supportedBaudRates.at(i));
    }

    ui.portName->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.baudRate->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    ui.baudRate->setCurrentIndex(ui.baudRate->findText(QString::number(this->link->getBaudRate())));
    ui.comboBox_flowControl->setCurrentIndex(this->link->getFlowType());
    ui.comboBox_Parity->setCurrentIndex(this->link->getParityType());
    ui.dataBitsCombo->setCurrentIndex(ui.dataBitsCombo->findText(QString::number(this->link->getDataBitsType())));
    ui.stopBitsCombo->setCurrentIndex(ui.stopBitsCombo->findText(QString::number(this->link->getStopBitsType())));
    ui.spinBox_timeout->setValue(this->link->getTimeoutMillis());
    ui.spinBox_reconnectDelay->setValue(this->link->reconnectDelayMs());

    ui.widget_advanced->setVisible(ui.groupBox_advanced->isChecked());

    connect(action, SIGNAL(triggered()), this, SLOT(configureCommunication()));

    // Make sure that a change in the link name will be reflected in the UI
    connect(link, SIGNAL(nameChanged(QString)), this, SLOT(setLinkName(QString)));

    // Connect the individual user interface inputs
    connect(ui.portName, SIGNAL(activated(QString)), this, SLOT(setPortName(QString)));
    connect(ui.portName, SIGNAL(editTextChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui.baudRate, SIGNAL(activated(QString)), this->link, SLOT(setBaudRateString(QString)));
    connect(ui.comboBox_flowControl, SIGNAL(currentIndexChanged(int)), this, SLOT(setFlowControl()));
    connect(ui.comboBox_Parity, SIGNAL(currentIndexChanged(int)), this, SLOT(setParity()));
    connect(ui.dataBitsCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDataBits(QString)));
    connect(ui.stopBitsCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setStopBits(QString)));
    connect(ui.spinBox_timeout, SIGNAL(valueChanged(int)), this, SLOT(setTimeoutMs()));
    connect(ui.spinBox_reconnectDelay, SIGNAL(valueChanged(int)), this, SLOT(setReconnectDelay()));

    //connect(this->link, SIGNAL(connected(bool)), this, SLOT());
    //portCheckTimer = new QTimer(this);
    //portCheckTimer->setInterval(5000);
    //connect(portCheckTimer, SIGNAL(timeout()), this, SLOT(setupPortList()));

    // Display the widget
    this->window()->setWindowTitle(tr("Serial Communication Settings"));
}

SerialConfigurationWindow::~SerialConfigurationWindow()
{
    writeSettings();
}

void SerialConfigurationWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    loadSettings();
    //portCheckTimer->start();
}

void SerialConfigurationWindow::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    writeSettings();
    //portCheckTimer->stop();
    if (this->link)
        this->link->writeSettings();
}

void SerialConfigurationWindow::loadSettings()
{
    // Load defaults from settings
    QSettings settings;
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    ui.groupBox_advanced->setChecked(settings.value("ADVANCED_VISIBLE", false).toBool());
}

void SerialConfigurationWindow::writeSettings()
{
    // Store settings
    QSettings settings;
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    settings.setValue("ADVANCED_VISIBLE", ui.groupBox_advanced->isChecked());
    settings.sync();
}

QAction* SerialConfigurationWindow::getAction()
{
    return action;
}

void SerialConfigurationWindow::configureCommunication()
{
    QString selected = ui.portName->currentText();
    setupPortList();
    ui.portName->setEditText(selected);
    this->show();
}

void SerialConfigurationWindow::setupPortList()
{
    if (!link)
        return;

//    QString selected = ui.portName->currentText();

    ui.portName->blockSignals(true);
    // Get the ports available on this system
    //QVector<QString>* ports = link->getCurrentPorts();
    QList<QextPortInfo> ports = portEnumerator->getPorts();
    QList<QString> portNames;
    QString txt;

    // add any new ports
    foreach (const QextPortInfo &p, ports) {
        //qDebug() << __FILE__ << __LINE__ << p.portName  << p.friendName << p.physName << p.enumName << p.vendorID << p.productID;
        if (!p.portName.length())
            continue;
        portNames.append(p.portName);
        txt = p.portName;
        if (ui.portName->findData(txt) > -1)
            continue;
        if (p.friendName.length())
            txt += " - " + p.friendName.split(QRegExp(" ?\\(")).first();
        ui.portName->addItem(txt, p.portName);
    }
    // mark any invalid items in the selector (eg. port was disconnected)
    bool isval = true, isinv = false;
    for (int i = 0; i < ui.portName->count(); ++i) {
        isval = portNames.contains(ui.portName->itemData(i).toString());
        isinv = ui.portName->itemText(i).contains("INVALID PORT");
        if (!isval && !isinv)
            ui.portName->setItemText(i, ui.portName->itemText(i) +  " [INVALID PORT]");
        else if (isval && isinv)
            ui.portName->setItemText(i, ui.portName->itemText(i).replace(" [INVALID PORT]", ""));
    }

//    if (!selected.length() && ui.portName->count())
//        selected = ui.portName->itemData(0);

//    selected = selected.split(" - ").first().remove(" ");

//    if (!userConfigured && selected.length() && link->isPortValid(selected))
//        setPortName(selected);

    ui.portName->blockSignals(false);
}

void SerialConfigurationWindow::portError(const QString &err) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("Error in port settings"));
    msgBox.setInformativeText(tr("Can't set %1").arg(err));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void SerialConfigurationWindow::setFlowControl()
{
    if (!link->setFlowType(ui.comboBox_flowControl->currentIndex()))
        portError(tr("Flow Control"));
}

void SerialConfigurationWindow::setParity()
{
    if (!link->setParityType(ui.comboBox_Parity->currentIndex()))
        portError(tr("Parity Type"));
}

void SerialConfigurationWindow::setDataBits(QString bits)
{
    if (!link->setDataBitsType(bits.toInt()))
        portError(tr("Data Bits to %1").arg(bits));
}

void SerialConfigurationWindow::setStopBits(QString bits)
{
    if (!link->setStopBitsType(bits.toInt()))
        portError(tr("Stop Bits to %1").arg(bits));
}

void SerialConfigurationWindow::setPortName(QString port)
{
    // if current text is unedited, then use port name from item data
    if (ui.portName->currentText() == ui.portName->itemText(ui.portName->currentIndex()))
        port = ui.portName->itemData(ui.portName->currentIndex()).toString();
    else
        port = port.split(" - ").first().remove(" ");

    if (link->isPortValid(port) && link->getPortName() != port) {
        if (link->setPortName(port))
            userConfigured = true;
        else
            portError(tr("Port to %1").arg(port));
    }
}

void SerialConfigurationWindow::setLinkName(QString name)
{
    Q_UNUSED(name);
    // FIXME
    action->setText(tr("Configure ") + link->getName());
    action->setStatusTip(tr("Configure ") + link->getName());
    setWindowTitle(tr("Configuration of ") + link->getName());
}

void SerialConfigurationWindow::setTimeoutMs()
{
    if (ui.spinBox_timeout->value())  // zero is bad, will hang whole program
        link->setTimeoutMillis(ui.spinBox_timeout->value());
}

void SerialConfigurationWindow::setReconnectDelay()
{
    link->setReconnectDelayMs(ui.spinBox_reconnectDelay->value());
}

void SerialConfigurationWindow::on_groupBox_advanced_toggled(bool arg1)
{
    ui.widget_advanced->setVisible(arg1);
    writeSettings();
}

