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
#include "LinkInterface.h"
#include "SerialLinkInterface.h"
#include "LinkManager.h"
#include "SerialConfigurationWindow.h"
#include "SerialLinkInterface.h"

#include <QDir>
#include <QSettings>
#include <QFileInfoList>
#include <QMessageBox>
#include <qextserialenumerator.h>

SerialConfigurationWindow::SerialConfigurationWindow(LinkInterface* link, QWidget *parent, Qt::WindowFlags flags) :
    QWidget(parent, flags),
    userConfigured(false)
{
    SerialLinkInterface* serialLink = dynamic_cast<SerialLinkInterface*>(link);

    if (!serialLink) {
        qDebug() << "Link is NOT a serial link, can't open configuration window";
        return;
    }

    this->link = serialLink;

    // Setup the user interface according to link type
    ui.setupUi(this);

    portEnumerator = new QextSerialEnumerator();
    portEnumerator->setUpNotifications();
    QObject::connect(portEnumerator, SIGNAL(deviceDiscovered(QextPortInfo)), this, SLOT(setupPortList()));
    QObject::connect(portEnumerator, SIGNAL(deviceRemoved(QextPortInfo)), this, SLOT(setupPortList()));

    // Set up baud rates
    QList<int> supportedBaudRates = MG::SERIAL::getBaudRates();
    ui.baudRate->clear();
    for (int i = 0; i < supportedBaudRates.size(); ++i) {
        ui.baudRate->addItem(QString::number(supportedBaudRates.at(i)), supportedBaudRates.at(i));
    }

    setupPortList();
    loadSettings();

    ui.portName->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.baudRate->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.widget_advanced->setVisible(ui.groupBox_advanced->isChecked());

    // connect these before setting the port name since that will trigger a settings load
    connect(ui.baudRate, SIGNAL(currentIndexChanged(QString)), this, SLOT(setBaudRate(QString)));
    connect(ui.comboBox_flowControl, SIGNAL(currentIndexChanged(int)), this, SLOT(setFlowControl(int)));
    connect(ui.comboBox_Parity, SIGNAL(currentIndexChanged(int)), this, SLOT(setParity(int)));
    connect(ui.dataBitsCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDataBits(QString)));
    connect(ui.stopBitsCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setStopBits(QString)));
    connect(ui.spinBox_timeout, SIGNAL(valueChanged(int)), this, SLOT(setTimeoutMs(int)));
    connect(ui.spinBox_reconnectDelay, SIGNAL(valueChanged(int)), this, SLOT(setReconnectDelay(int)));

    // only load default port if this is the first serial link being added
    if (LinkManager::instance()->getLinksForType(LinkInterface::LINK_INTERFACE_TYPE_SERIAL).size() > 1)
        defaultPortName = ui.portName->currentText();

    ui.portName->setCurrentIndex(ui.portName->findText(defaultPortName, Qt::MatchContains));
    setPortName(defaultPortName);

    // Create action to open this config dialog
    action = new QAction(QIcon(":/files/images/devices/network-wireless.svg"), "", this->link);
    connect(action, SIGNAL(triggered()), this, SLOT(configureCommunication()));

    // Make sure that a change in the link name will be reflected in the UI
    connect(this->link, SIGNAL(nameChanged(QString)), this, SLOT(setLinkName(QString)));
    connect(this->link, SIGNAL(connected()), this, SLOT(writePortSettings()));

    // Connect the individual user interface inputs
    connect(ui.portName, SIGNAL(activated(QString)), this, SLOT(setPortName(QString)));
    connect(ui.portName, SIGNAL(editTextChanged(QString)), this, SLOT(setPortName(QString)));

    //connect(this->link, SIGNAL(connected(bool)), this, SLOT());
    //portCheckTimer = new QTimer(this);
    //portCheckTimer->setInterval(5000);
    //connect(portCheckTimer, SIGNAL(timeout()), this, SLOT(setupPortList()));

    // Display the widget
    setLinkName(this->link->getName());
    this->window()->setWindowTitle(tr("Serial Communication Settings"));
}

SerialConfigurationWindow::~SerialConfigurationWindow()
{
}

void SerialConfigurationWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    setupPortList();
    //portCheckTimer->start();
}

void SerialConfigurationWindow::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    writeSettings();
    //portCheckTimer->stop();
}

void SerialConfigurationWindow::loadSettings()
{
    // Load defaults from settings
    QSettings settings;
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    ui.groupBox_advanced->setChecked(settings.value("ADVANCED_VISIBLE", false).toBool());
    defaultPortName = settings.value("DEFAULT_COMM_PORT", "").toString();
}

void SerialConfigurationWindow::writeSettings()
{
    // Store settings
    QSettings settings;
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    settings.setValue("ADVANCED_VISIBLE", ui.groupBox_advanced->isChecked());
    settings.setValue("DEFAULT_COMM_PORT", ui.portName->currentText());
    settings.sync();
}

QString SerialConfigurationWindow::getSettingsKey(bool checkExists)
{
    QString key;
    QSettings settings;
    bool found = false;
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    int idx = ui.portName->currentIndex();
    if (idx > -1 && ui.portName->itemData(idx).toString() != "[no ports]") {
        key = QString("SERIALLINK_COMM_" % ui.portName->itemData(idx).toString().replace(QRegExp("[^a-zA-Z0-9_]"), "") % "_%1");
        if (settings.contains(key.arg("BAUD")) || !checkExists)
            found = true;
    }
    if (!found)
        key = QString("SERIALLINK_COMM_%1");

    return key;
}

void SerialConfigurationWindow::loadPortSettings()
{
    // Load defaults from settings
    QString tmp;
    int itmp;
    bool ok;
    QSettings settings;
    QString key = getSettingsKey(true);
    settings.beginGroup("SERIAL_CONFIG_WINDOW");

    tmp = settings.value(key.arg("BAUD"),  "115200").toString();
    if (ui.baudRate->findText(tmp) == -1)
        tmp = "115200";
    ui.baudRate->setCurrentIndex(ui.baudRate->findText(tmp));

    tmp = settings.value(key.arg("FLOW_CONTROL"), "None").toString();
    if (ui.comboBox_flowControl->findText(tmp) == -1)
        tmp = "None";
    ui.comboBox_flowControl->setCurrentIndex(ui.comboBox_flowControl->findText(tmp));

    tmp = settings.value(key.arg("PARITY"), "None").toString();
    if (ui.comboBox_Parity->findText(tmp) == -1)
        tmp = "None";
    ui.comboBox_Parity->setCurrentIndex(ui.comboBox_Parity->findText(tmp));

    tmp = settings.value(key.arg("DATABITS"), "8").toString();
    if (ui.dataBitsCombo->findText(tmp) == -1)
        tmp = "8";
    ui.dataBitsCombo->setCurrentIndex(ui.dataBitsCombo->findText(tmp));

    tmp = settings.value(key.arg("STOPBITS"), "1").toString();
    if (ui.stopBitsCombo->findText(tmp) == -1)
        tmp = "1";
    ui.stopBitsCombo->setCurrentIndex(ui.stopBitsCombo->findText(tmp));

    itmp = settings.value(key.arg("TIMEOUT"), -1).toInt(&ok);
    if (ok)
        ui.spinBox_timeout->setValue(itmp);
    itmp = settings.value(key.arg("RECONDELAY"), 10).toInt(&ok);
    if (ok)
        ui.spinBox_reconnectDelay->setValue(itmp);
}

void SerialConfigurationWindow::writePortSettings()
{
    // Store settings
    QSettings settings;
    QString key = getSettingsKey(false);
    settings.beginGroup("SERIAL_CONFIG_WINDOW");
    settings.setValue(key.arg("BAUD"), ui.baudRate->currentText());
    settings.setValue(key.arg("PARITY"), ui.comboBox_Parity->currentText());
    settings.setValue(key.arg("STOPBITS"), ui.stopBitsCombo->currentText());
    settings.setValue(key.arg("DATABITS"), ui.dataBitsCombo->currentText());
    settings.setValue(key.arg("FLOW_CONTROL"), ui.comboBox_flowControl->currentText());
    settings.setValue(key.arg("TIMEOUT"), ui.spinBox_timeout->value());
    settings.setValue(key.arg("RECONDELAY"), ui.spinBox_reconnectDelay->value());
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

    QStringList usedPorts;
    foreach (LinkInterface *li, LinkManager::instance()->getLinksForType(LinkInterface::LINK_INTERFACE_TYPE_SERIAL)) {
        if (li->getId() != link->getId())
            usedPorts.append(li->getPortName());
    }

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

        if (ui.portName->findData(txt) > -1) {
            if (usedPorts.contains(p.portName))
                ui.portName->removeItem(ui.portName->findData(txt));
            continue;
        } else if (usedPorts.contains(p.portName))
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

    if (!ui.portName->count())
        ui.portName->addItem(tr("No ports are available"), "[no ports]");

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

void SerialConfigurationWindow::setFlowControl(int fc)
{
    if (!link->setFlowType(fc))
        portError(tr("Flow Control"));
}

void SerialConfigurationWindow::setParity(int parity)
{
    if (!link->setParityType(parity))
        portError(tr("Parity Type"));
}

void SerialConfigurationWindow::setBaudRate(QString rate)
{
    if (!this->link->setBaudRateString(rate))
        portError(tr("Baud rate to %1").arg(rate));
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
    if (ui.portName->currentText() == ui.portName->itemText(ui.portName->currentIndex())) {
        if (ui.portName->itemData(ui.portName->currentIndex()).toString() == "[no ports]")
            return;
        port = ui.portName->itemData(ui.portName->currentIndex()).toString();

    } else
        port = port.split(" - ").first().remove(" ");

    if (link->isPortValid(port) && link->getPortName() != port) {
        if (link->setPortName(port)){
            userConfigured = true;
            loadPortSettings();
        } else
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

void SerialConfigurationWindow::setTimeoutMs(int to)
{
    if (to)  // zero is bad, will hang whole program
        link->setTimeoutMillis(to);
}

void SerialConfigurationWindow::setReconnectDelay(int dly)
{
    link->setReconnectDelayMs(dly);
}

void SerialConfigurationWindow::on_groupBox_advanced_clicked(bool arg1)
{
    ui.widget_advanced->setVisible(arg1);
    writeSettings();
}

