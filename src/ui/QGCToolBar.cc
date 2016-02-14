/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009 - 2011 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

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

#include "QGCToolBar.h"
#include "MG.h"
#include "UASManager.h"
#include "MainWindow.h"
#include "LinkManager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>

QGCToolBar::QGCToolBar(QWidget *parent) :
    QToolBar(parent),
    toggleLoggingAction(NULL),
    logReplayAction(NULL),
    mav(NULL),
    player(NULL),
    changed(true),
    batteryPercent(0),
    batteryVoltage(0),
    wpId(0),
    wpDistance(0),
    systemArmed(false),
    currentLink(NULL)
{
    setObjectName("QGC_TOOLBAR");

}

void QGCToolBar::createUI() {

//    toggleLoggingAction = new QAction(QIcon(":"), "Logging", this);
//    toggleLoggingAction->setCheckable(true);
//    logReplayAction = new QAction(QIcon(":"), "Replay", this);
//    logReplayAction->setCheckable(false);

//    addAction(toggleLoggingAction);
//    addAction(logReplayAction);

    // CREATE TOOLBAR ITEMS
    // Add internal actions
    // Add MAV widget
    symbolLabel = new QLabel(this);
    //symbolButton->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; }");
    symbolLabel->setObjectName("mavSymbolLabel");
    addWidget(symbolLabel);

    toolBarNameLabel = new QLabel("------", this);
    toolBarNameLabel->setToolTip(tr("Currently controlled vehicle"));
    toolBarNameLabel->setObjectName("toolBarNameLabel");
    addWidget(toolBarNameLabel);

    toolBarTimeoutLabel = new QLabel(tr("NOT CONNECTED"), this);
    toolBarTimeoutLabel->setToolTip(tr("System connection status, interval since last message if timed out."));
    toolBarTimeoutLabel->setObjectName("toolBarTimeoutLabel");
    toolBarTimeoutLabel->setStyleSheet(QString("QLabel { background-color: %2; padding: 0 .3em; }").arg(QGC::colorMagenta.dark(250).name()));
    addWidget(toolBarTimeoutLabel);

    toolBarSafetyLabel = new QLabel(tr("SAFE"), this);
    toolBarSafetyLabel->setToolTip(tr("Vehicle safety state"));
    toolBarSafetyLabel->setObjectName("toolBarSafetyLabel");
    addWidget(toolBarSafetyLabel);

    toolBarStateLabel = new QLabel("------", this);
    toolBarStateLabel->setToolTip(tr("Vehicle state"));
    toolBarStateLabel->setObjectName("toolBarStateLabel");
    addWidget(toolBarStateLabel);

    toolBarModeLabel = new QLabel("------", this);
    toolBarModeLabel->setToolTip(tr("Vehicle flight mode"));
    toolBarModeLabel->setObjectName("toolBarModeLabel");
    addWidget(toolBarModeLabel);

    toolBarAuxModeLabel = new QLabel("", this);
    toolBarAuxModeLabel->setToolTip(tr("Flight sub-mode, if any"));
    toolBarAuxModeLabel->setObjectName("toolBarAuxModeLabel");
    addWidget(toolBarAuxModeLabel);

    toolBarBatteryVoltageLabel = new QLabel("---- V");
    //toolBarBatteryVoltageLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(QColor(Qt::green).name()));
    toolBarBatteryVoltageLabel->setToolTip(tr("Battery voltage"));
    toolBarBatteryVoltageLabel->setObjectName("toolBarBatteryVoltageLabel");
    //addWidget(toolBarBatteryVoltageLabel);

    toolBarGpsFixLabel = new QLabel("No GPS");
    toolBarGpsFixLabel->setToolTip(tr("GPS Fix Type"));
    toolBarGpsFixLabel->setObjectName("toolBarGpsFixLabel");
    //addWidget(toolBarGpsFixLabel);


    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    spacer->setDisabled(true);
    addWidget(spacer);

    toolBarMessageLabel = new QLabel(tr("No system messages."), this);
    toolBarMessageLabel->setToolTip(tr("Most recent system message"));
    toolBarMessageLabel->setObjectName("toolBarMessageLabel");
    addWidget(toolBarMessageLabel);

    addSeparator();

    QWidget *labelsStackedWdgt = new QWidget(this);
    labelsStackedWdgt->setObjectName("labelsStackedWdgt");
    labelsStackedWdgt->setProperty("styleType", "toolBarStackedWdgt");
    labelsStackedWdgt->setLayout(new QVBoxLayout());
    labelsStackedWdgt->setContentsMargins(0,0,0,0);
    labelsStackedWdgt->layout()->setContentsMargins(0,0,0,0);
    labelsStackedWdgt->layout()->setSpacing(0);
    labelsStackedWdgt->layout()->addWidget(toolBarBatteryVoltageLabel);
    labelsStackedWdgt->layout()->addWidget(toolBarGpsFixLabel);
    addWidget(labelsStackedWdgt);


//    toolBarWpLabel = new QLabel("WP--", this);
//    toolBarWpLabel->setStyleSheet("QLabel { margin: 0px 2px; font: 18px; color: #3C7B9E; }");
//	toolBarWpLabel->setToolTip(tr("Current mission"));
//    addWidget(toolBarWpLabel);

//    toolBarDistLabel = new QLabel("--- ---- m", this);
//	toolBarDistLabel->setToolTip(tr("Distance to current mission"));
//    addWidget(toolBarDistLabel);

    toolBarBatteryBar = new QProgressBar(this);
    toolBarBatteryBar->setMinimum(0);
    toolBarBatteryBar->setMaximum(100);
    toolBarBatteryBar->setToolTip(tr("Battery charge level"));
    toolBarBatteryBar->setObjectName("toolBarBatteryBar");
    toolBarBatteryBar->setTextVisible(false);
    //addWidget(toolBarBatteryBar);

    toolBarRssiBar = new QProgressBar(this);
    toolBarRssiBar->setMinimum(0);
    toolBarRssiBar->setMaximum(100);
    toolBarRssiBar->setToolTip(tr("Radio reception quality"));
    toolBarRssiBar->setObjectName("toolBarRssiBar");
    toolBarRssiBar->setTextVisible(false);
    //addWidget(toolBarRssiBar);

    toolBarGpsBar = new QProgressBar(this);
    toolBarGpsBar->setMinimum(0);
    toolBarGpsBar->setMaximum(10);
    toolBarGpsBar->setToolTip(tr("GPS Horizontal Accuracy on a scale of 0 to 10m. The more filled the bar is, the better the accuracy."));
    toolBarGpsBar->setObjectName("hAccBar");
    toolBarGpsBar->setTextVisible(false);

    QWidget *progressBarsWdgt = new QWidget(this);
    progressBarsWdgt->setObjectName("toolBarProgressBarsWdgt");
    progressBarsWdgt->setProperty("styleType", "toolBarStackedWdgt");
    progressBarsWdgt->setLayout(new QVBoxLayout());
    progressBarsWdgt->setContentsMargins(0,0,0,0);
    progressBarsWdgt->layout()->setContentsMargins(0,0,6,0);
    progressBarsWdgt->layout()->setSpacing(0);
    progressBarsWdgt->layout()->addWidget(toolBarBatteryBar);
    progressBarsWdgt->layout()->addWidget(toolBarRssiBar);
    progressBarsWdgt->layout()->addWidget(toolBarGpsBar);
    addWidget(progressBarsWdgt);

    // DONE INITIALIZING BUTTONS

    // Configure the toolbar for the current default UAS
    setActiveUAS(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));

    if (LinkManager::instance()->getLinks().count() > 1)
        addLink(LinkManager::instance()->getLinks().last());
    connect(LinkManager::instance(), SIGNAL(newLink(LinkInterface*)), this, SLOT(addLink(LinkInterface*)));
    connect(LinkManager::instance(), SIGNAL(linkRemoved(LinkInterface*)), this, SLOT(removeLink(LinkInterface*)));

    // Set the toolbar to be updated every 2s
    connect(&updateViewTimer, SIGNAL(timeout()), this, SLOT(updateView()));
    updateViewTimer.setInterval(2000);
    updateViewTimer.stop();

    toggleActiveUasView(false);
}

void QGCToolBar::toggleActiveUasView(bool on)
{
    toolBarNameLabel->setEnabled(on);
    toolBarSafetyLabel->setEnabled(on);
    toolBarStateLabel->setEnabled(on);
    toolBarModeLabel->setEnabled(on);
    toolBarBatteryBar->setEnabled(on);
    toolBarRssiBar->setEnabled(on);
    toolBarGpsBar->setEnabled(on);
    toolBarBatteryVoltageLabel->setEnabled(on);
    toolBarGpsFixLabel->setEnabled(on);
    if (on) {
        toolBarTimeoutLabel->setStyleSheet(QString("QLabel {padding: 0; margin: 0;}"));
        toolBarTimeoutLabel->setText("");
        if (mav)
            toolBarNameLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(mav->getColor().name()));
        changed = true;
    } else {
        toolBarSafetyLabel->setStyleSheet("");
        toolBarNameLabel->setStyleSheet("");
        toolBarBatteryVoltageLabel->setStyleSheet("");
        toolBarGpsFixLabel->setStyleSheet("");
        toolBarStateLabel->setStyleSheet("");
    }
}

void QGCToolBar::setPerspectiveChangeActions(const QList<QAction*> &actions)
{
    if (actions.count())
    {
        group = new QButtonGroup(this);
        group->setExclusive(true);

        for (int i = 0; i < actions.count(); i++)
        {
            // Add last button
            QToolButton *btn = new QToolButton(this);
            // Add first button
            btn->setIcon(actions.at(i)->icon());
            btn->setText(actions.at(i)->text());
            btn->setToolTip(actions.at(i)->toolTip());
            btn->setObjectName(actions.at(i)->objectName());
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            btn->setCheckable(true);
            connect(btn, SIGNAL(clicked(bool)), actions.at(i), SIGNAL(triggered(bool)));
            connect(actions.at(i),SIGNAL(triggered(bool)),btn,SLOT(setChecked(bool)));
            connect(actions.at(i),SIGNAL(toggled(bool)),btn,SLOT(setChecked(bool)));
            addWidget(btn);
            group->addButton(btn);
        }

    } else {
        qDebug() << __FILE__ << __LINE__ << "Not enough perspective change actions provided";
    }

    // Add the "rest"
    createUI();
}

void QGCToolBar::setLogPlayer(QGCMAVLinkLogPlayer* player)
{
    this->player = player;
//    connect(toggleLoggingAction, SIGNAL(triggered(bool)), this, SLOT(logging(bool)));
//    connect(logReplayAction, SIGNAL(triggered(bool)), this, SLOT(playLogFile(bool)));
}

void QGCToolBar::playLogFile(bool checked)
{
    // Check if player exists
    if (player)
    {
        // If a logfile is already replayed, stop the replay
        // and select a new logfile
        if (player->isPlayingLogFile())
        {
            player->playPause(false);
            if (checked)
            {
                if (!player->selectLogFile()) return;
            }
        }
        // If no replaying happens already, start it
        else
        {
            if (!player->selectLogFile()) return;
        }
        player->playPause(checked);
    }
}

void QGCToolBar::logging(bool checked)
{
    // Stop logging in any case
    MainWindow::instance()->getMAVLink()->enableLogging(false);

	// If the user is enabling logging
    if (checked)
    {
		// Prompt the user for a filename/location to save to
        QString fileName = QFileDialog::getSaveFileName(this, tr("Specify MAVLink log file to save to"), DEFAULT_STORAGE_PATH, tr("MAVLink Logfile") + " (*.mavlink *.log *.bin)");

		// Check that they didn't cancel out
		if (fileName.isNull())
		{
			toggleLoggingAction->setChecked(false);
			return;
		}

		// Make sure the file's named properly
        if (!fileName.endsWith(".mavlink"))
        {
            fileName.append(".mavlink");
        }

		// Check that we can save the logfile
        QFileInfo file(fileName);
        if ((file.exists() && !file.isWritable()))
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(tr("The selected logfile is not writable"));
            msgBox.setInformativeText(tr("Please make sure that the file %1 is writable or select a different file").arg(fileName));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        }
		// Otherwise we're off and logging
        else
        {
            MainWindow::instance()->getMAVLink()->setLogfileName(fileName);
            MainWindow::instance()->getMAVLink()->enableLogging(true);
        }
    }
}

void QGCToolBar::addPerspectiveChangeAction(QAction* action)
{
    insertAction(toggleLoggingAction, action);
}

void QGCToolBar::setActiveUAS(UASInterface* active)
{
    // Do nothing if system is the same or NULL
    if ((active == NULL) || mav == active) return;

    if (mav) {
        // Disconnect old system
        disconnect(mav, 0, this, 0);
        if (mav->getWaypointManager())
            disconnect(mav->getWaypointManager(), 0, this, 0);
    }

    // Connect new system
    mav = active;
    connect(active, SIGNAL(statusChanged(UASInterface*,QString,QString)), this, SLOT(updateState(UASInterface*, QString,QString)));
    connect(active, SIGNAL(modeChanged(int,QString,QString)), this, SLOT(updateMode(int,QString,QString)));
    connect(active, SIGNAL(nameChanged(QString)), this, SLOT(updateName(QString)));
    connect(active, SIGNAL(systemTypeSet(UASInterface*,uint)), this, SLOT(setSystemType(UASInterface*,uint)));
    connect(active, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(receiveTextMessage(int,int,int,QString)));
    connect(active, SIGNAL(batteryChanged(UASInterface*,double,double,int)), this, SLOT(updateBatteryRemaining(UASInterface*,double,double,int)));
    connect(active, SIGNAL(armingChanged(bool)), this, SLOT(updateArmingState(bool)));
    connect(active, SIGNAL(heartbeatTimeout(bool, unsigned int)), this, SLOT(heartbeatTimeout(bool,unsigned int)));
    connect(active, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(updateRSSI(float)));
    connect(active, SIGNAL(valueChanged(int,QString,QString,QVariant,quint64)), this, SLOT(updateGpsAcc(int,QString,QString,QVariant,quint64)));
    if (active->getWaypointManager())
    {
        connect(active->getWaypointManager(), SIGNAL(currentWaypointChanged(quint16)), this, SLOT(updateCurrentWaypoint(quint16)));
        connect(active->getWaypointManager(), SIGNAL(waypointDistanceChanged(double)), this, SLOT(updateWaypointDistance(double)));
    }

    // Update all values once
    toggleActiveUasView(true);
    systemName = mav->getUASName();
    systemArmed = mav->isArmed();
    mode = mav->getShortMode();
    state = mav->getShortState();
    auxMode = mav->getShortAuxMode();
    setSystemType(mav, mav->getSystemType());
    toolBarNameLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(mav->getColor().name()));
    changed = true;
    updateView();
    updateViewTimer.start();
}

//void QGCToolBar::createCustomWidgets() {}

void QGCToolBar::heartbeatTimeout(bool timeout, unsigned int ms)
{
    if (ms > 10000) {
        if (!currentLink || !currentLink->isConnected()) {
            toolBarTimeoutLabel->setText(tr("DISCONNECTED"));
            toolBarTimeoutLabel->setStyleSheet(QString("QLabel { padding: 0 .3em; background-color: %2; }").arg(QGC::colorMagenta.dark(250).name()));
            toggleActiveUasView(false);
            return;
        }
    }

    // set timeout label visible
    if (timeout) {
        // Alternate colors to increase visibility
        QString color = !((ms / 1000) % 2) ? QGC::colorMagenta.name() : QGC::colorMagenta.dark(250).name();
        toolBarTimeoutLabel->setStyleSheet(QString("QLabel { padding: 0 .3em; background-color: %2; }").arg(color));
        toolBarTimeoutLabel->setText(tr("CONNECTION LOST: %1 s").arg((ms / 1000.0f), 2, 'f', 1, ' '));
    }
    else if (!toolBarTimeoutLabel->text().isEmpty())
        toggleActiveUasView(true);
}

void QGCToolBar::updateView()
{
    if (!changed)
        return;
    QString text;
    QString color;
    float gpsPrct;

    changed = false;

//    toolBarDistLabel->setText(tr("%1 m").arg(wpDistance, 6, 'f', 2, '0'));
//    toolBarWpLabel->setText(tr("WP%1").arg(wpId));
    toolBarBatteryBar->setValue(batteryPercent);
    toolBarRssiBar->setValue(rssi);

    if (gpsEph > 0.0f) {
        gpsPrct = qMin((float)toolBarGpsBar->maximum(), gpsEph);
        toolBarGpsBar->setValue(toolBarGpsBar->maximum() - qRound(gpsPrct));
    } else
        toolBarGpsBar->setValue(0);

    text = tr("No GPS");
    color = QGC::colorTextErr.name(QColor::HexArgb);
    if (gpsFixType == 2) {
        text = tr("GPS 2D");
        color = QGC::colorTextWarn.name(QColor::HexArgb);
    }
    else if (gpsFixType == 3) {
        text = tr("GPS 3D");
        color = QGC::colorTextOK.name(QColor::HexArgb);
    }
    toolBarGpsFixLabel->setText(text);
    if (toolBarGpsFixLabel->isEnabled())
        toolBarGpsFixLabel->setStyleSheet("color: " + color + ";");

    color = QGC::colorTextOK.name(QColor::HexArgb);
    if (batteryPercent < 5)
        color = QGC::colorTextErr.name(QColor::HexArgb);
    else if ((mav->getBatteryRemainingEstimateEnabled() && batteryVoltage < mav->getBatteryWarnVoltage()) || (!mav->getBatteryRemainingEstimateEnabled() && batteryPercent < mav->getBatteryWarnPercent()))
        color = QGC::colorTextWarn.name(QColor::HexArgb);
    toolBarBatteryVoltageLabel->setText(tr("%1 V").arg(batteryVoltage, 4, 'f', 1, ' '));
    if (toolBarBatteryVoltageLabel->isEnabled())
        toolBarBatteryVoltageLabel->setStyleSheet("color: " + color + ";");

    color = QGC::colorTextOK.name(QColor::HexArgb);
    if (systemArmed)
        color = QGC::colorTextWarn.name(QColor::HexArgb);
    color.append("; ");
    if (state.contains(QRegExp("(CRITICAL|EMERGENCY|FAIL)")))
        color.append("background-color: " % QGC::colorMagenta.name() % ";");

    toolBarStateLabel->setText(state);
    if (toolBarStateLabel->isEnabled())
        toolBarStateLabel->setStyleSheet("color: " + color + ";");

    toolBarModeLabel->setText(mode);
    toolBarAuxModeLabel->setText(auxMode);
    toolBarNameLabel->setText(systemName);
    toolBarMessageLabel->setText(lastSystemMessage);

    if (systemArmed)
    {
        if (toolBarSafetyLabel->isEnabled())
            toolBarSafetyLabel->setStyleSheet(QString("QLabel { color: %1; background-color: %2; }").arg(QGC::colorRed.name()).arg(QGC::colorYellow.name()));
        toolBarSafetyLabel->setText(tr("ARMED"));
    }
    else
    {
        if (toolBarSafetyLabel->isEnabled())
            toolBarSafetyLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(QGC::colorTextOK.name(QColor::HexArgb)));
        else
            toolBarSafetyLabel->setStyleSheet("");
        toolBarSafetyLabel->setText(tr("SAFE"));
    }

}

void QGCToolBar::updateArmingState(bool armed)
{
    if (systemArmed != armed) {
        systemArmed = armed;
        changed = true;
        /* important, immediately update */
        updateView();
    }
}

void QGCToolBar::updateRSSI(float rssiNormalized)
{
    if (rssiNormalized != rssi) {
        rssi = rssiNormalized;
        changed = true;
        updateView();
    }
}

void QGCToolBar::updateWaypointDistance(double distance)
{
    if (wpDistance != distance)
        changed = true;
    wpDistance = distance;
}

void QGCToolBar::updateCurrentWaypoint(quint16 id)
{
    if (wpId != id)
        changed = true;
    wpId = id;
}

void QGCToolBar::updateBatteryRemaining(UASInterface* uas, double voltage, double percent, int seconds)
{
    Q_UNUSED(seconds);
    if (mav != uas)
        return;
    if (batteryPercent != percent || batteryVoltage != voltage)
        changed = true;
    batteryPercent = percent;
    batteryVoltage = voltage;
}

void QGCToolBar::updateState(UASInterface* system, QString name, QString description)
{
    Q_UNUSED(description);
    if (mav != system)
        return;
    if (state != name) {
        changed = true;
        state = name;
        /* important, immediately update */
        updateView();
    }
}

void QGCToolBar::updateMode(int system, QString name, QString description)
{
    if (mav->getUASID() != system)
        return;
    if (mode != name || auxMode != description) {
        changed = true;
        mode = name;
        auxMode = description;
        /* important, immediately update */
        updateView();
    }
}

void QGCToolBar::updateGpsAcc(const int uasId, const QString &name, const QString &unit, const QVariant val, const quint64 msec)
{
    Q_UNUSED(unit);
    Q_UNUSED(msec);
    float acc;
    if (!mav || mav->getUASID() != uasId || !val.isValid())
        return;
    if (name.contains(QString("eph"))) {
        acc = val.toFloat() / 100.0f;
        if (gpsEph != acc) {
            changed = true;
            gpsEph = acc;
        }
    }
    else if (name.contains(QString("epv"))) {
        acc = val.toFloat() / 100.0f;
        if (gpsEpv != acc) {
            changed = true;
            gpsEpv = acc;
        }
    }
    else if (name.contains(QString("fix_type"))) {
        if (gpsEpv != val.toUInt()) {
            changed = true;
            gpsFixType = val.toUInt();
        }
    }
}

void QGCToolBar::updateName(const QString& name)
{
    if (systemName != name)
        changed = true;
    systemName = name;
}

/**
 * The current system type is represented through the system icon.
 *
 * @param uas Source system, has to be the same as this->uas
 * @param systemType type ID, following the MAVLink system type conventions
 * @see http://pixhawk.ethz.ch/software/mavlink
 */
void QGCToolBar::setSystemType(UASInterface* uas, unsigned int systemType)
{
    Q_UNUSED(uas);
    QPixmap newPixmap;
    // Set matching icon
    switch (systemType) {
    case MAV_TYPE_GENERIC:
        newPixmap = QPixmap(":/files/images/mavs/generic.svg");
        break;
    case MAV_TYPE_FIXED_WING:
        newPixmap = QPixmap(":/files/images/mavs/fixed-wing.svg");
        break;
    case MAV_TYPE_QUADROTOR:
        newPixmap = QPixmap(":/files/images/mavs/quadrotor.svg");
        break;
    case MAV_TYPE_COAXIAL:
        newPixmap = QPixmap(":/files/images/mavs/coaxial.svg");
        break;
    case MAV_TYPE_HELICOPTER:
        newPixmap = QPixmap(":/files/images/mavs/helicopter.svg");
        break;
    case MAV_TYPE_ANTENNA_TRACKER:
        newPixmap = QPixmap(":/files/images/mavs/antenna-tracker.svg");
        break;
    case MAV_TYPE_GCS:
        newPixmap = QPixmap(":files/images/mavs/groundstation.svg");
        break;
    case MAV_TYPE_AIRSHIP:
        newPixmap = QPixmap(":files/images/mavs/airship.svg");
        break;
    case MAV_TYPE_FREE_BALLOON:
        newPixmap = QPixmap(":files/images/mavs/free-balloon.svg");
        break;
    case MAV_TYPE_ROCKET:
        newPixmap = QPixmap(":files/images/mavs/rocket.svg");
        break;
    case MAV_TYPE_GROUND_ROVER:
        newPixmap = QPixmap(":files/images/mavs/ground-rover.svg");
        break;
    case MAV_TYPE_SURFACE_BOAT:
        newPixmap = QPixmap(":files/images/mavs/surface-boat.svg");
        break;
    case MAV_TYPE_SUBMARINE:
        newPixmap = QPixmap(":files/images/mavs/submarine.svg");
        break;
    case MAV_TYPE_HEXAROTOR:
        newPixmap = QPixmap(":files/images/mavs/hexarotor.svg");
        break;
    case MAV_TYPE_OCTOROTOR:
        newPixmap = QPixmap(":files/images/mavs/octorotor.svg");
        break;
    case MAV_TYPE_TRICOPTER:
        newPixmap = QPixmap(":files/images/mavs/tricopter.svg");
        break;
    case MAV_TYPE_FLAPPING_WING:
        newPixmap = QPixmap(":files/images/mavs/flapping-wing.svg");
        break;
    case MAV_TYPE_KITE:
        newPixmap = QPixmap(":files/images/mavs/kite.svg");
        break;
    default:
        newPixmap = QPixmap(":/files/images/mavs/unknown.svg");
        break;
    }
    symbolLabel->setPixmap(newPixmap.scaledToHeight(34));
}

void QGCToolBar::receiveTextMessage(int uasid, int componentid, int severity, QString text)
{
    Q_UNUSED(uasid);
    Q_UNUSED(componentid);
    Q_UNUSED(severity);
    text = text.trimmed();
    changed = true;
    lastSystemMessage = text;
}

void QGCToolBar::addLink(LinkInterface* link)
{
    // XXX magic number
    if (LinkManager::instance()->getLinks().count() > 1) {
        currentLink = link;
//        connect(currentLink, SIGNAL(connected(bool)), this, SLOT(updateLinkState(bool)));
//        updateLinkState(link->isConnected());
    }
}

void QGCToolBar::removeLink(LinkInterface* link)
{
    if (link == currentLink) {
        currentLink = NULL;
        // XXX magic number
        if (LinkManager::instance()->getLinks().count() > 1) {
            currentLink = LinkManager::instance()->getLinks().last();
//            updateLinkState(currentLink->isConnected());
        }
//        else {
//            connectButton->setText(tr("New Link"));
//        }
    }
}

QGCToolBar::~QGCToolBar()
{
    if (toggleLoggingAction) toggleLoggingAction->deleteLater();
    if (logReplayAction) logReplayAction->deleteLater();
}
