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
 *   @brief Implementation of QGCRemoteControlView
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 *   @author Bryan Godbolt <godbolt@ece.ualberta.ca>
 */

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "QGCRemoteControlView.h"
#include "ui_QGCRemoteControlView.h"
#include "UASManager.h"

QGCRemoteControlView::QGCRemoteControlView(QWidget *parent) :
    QWidget(parent),
    uasId(-1),
    rssi(0.0f),
    updated(false),
    channelLayout(new QVBoxLayout()),
    rssiBar(NULL),
    ui(NULL)
{
    ui->setupUi(this);
    QGridLayout* layout = new QGridLayout(this);
    layout->addLayout(channelLayout, 1, 0, 1, 2, Qt::AlignTop);
    nameLabel = new QLabel(this);
    layout->addWidget(nameLabel, 0, 0, 1, 2, Qt::AlignTop);
    QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Fixed, QSizePolicy::Expanding);
    layout->addItem(spacer, 2, 0, 1, 2, Qt::AlignTop);

    this->setVisible(false);
    //setVisible(false);

//    calibrate = new QPushButton(tr("Calibrate"), this);
//    QHBoxLayout *calibrateButtonLayout = new QHBoxLayout();
//    calibrateButtonLayout->addWidget(calibrate, 0, Qt::AlignHCenter);
//    layout->addItem(calibrateButtonLayout, 3, 0, 1, 2);

//    calibrationWindow = new RadioCalibrationWindow(this);
//    connect(calibrate, SIGNAL(clicked()), calibrationWindow, SLOT(show()));

    connect(UASManager::instance(), SIGNAL(activeUASSet(int)), this, SLOT(setUASId(int)));

    //connect(&updateTimer, SIGNAL(timeout()), this, SLOT(redraw()));
    //updateTimer.start(1500);
}

QGCRemoteControlView::~QGCRemoteControlView()
{
	if(this->ui)
	{
		delete ui;
	}
	if(this->channelLayout)
	{
		delete channelLayout;
	}
}

void QGCRemoteControlView::setUASId(int id)
{
    if (uasId != -1)
    {
        UASInterface* uas = UASManager::instance()->getUASForId(uasId);
        if (uas)
        {
            // The UAS exists, disconnect any existing connections
            disconnect(uas, 0, this, 0);
            //disconnect(uas, SIGNAL(radioCalibrationRawReceived(const QPointer<RadioCalibrationData>&)), calibrationWindow, SLOT(receive(const QPointer<RadioCalibrationData>&)));
            //disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), calibrationWindow, SLOT(setChannel(int,float)));
        }
    }

    // Clear channel count
    raw.clear();
    raw.resize(0);
    normalized.clear();
    normalized.resize(0);

    foreach (QLabel* label, rawLabels)
    {
        label->deleteLater();
    }

    foreach(QProgressBar* bar, progressBars)
    {
        bar->deleteLater();
    }

    rawLabels.clear();
    rawLabels.resize(0);
    progressBars.clear();
    progressBars.resize(0);

    // Connect the new UAS
    UASInterface* newUAS = UASManager::instance()->getUASForId(id);
    if (newUAS)
    {
        // New UAS exists, connect
        nameLabel->setText(QString("RC Input of %1").arg(newUAS->getUASName()));
        //calibrationWindow->setUASId(id);
        //connect(newUAS, SIGNAL(radioCalibrationReceived(const QPointer<RadioCalibrationData>&)), calibrationWindow, SLOT(receive(const QPointer<RadioCalibrationData>&)));

        connect(newUAS, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(setRemoteRSSI(float)));
        connect(newUAS, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));
        connect(newUAS, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));

        // only connect raw channels to calibration window widget
        //connect(newUAS, SIGNAL(remoteControlChannelRawChanged(int,float)), calibrationWindow, SLOT(setChannel(int,float)));
    }
}

void QGCRemoteControlView::setChannelRaw(int channelId, float raw)
{

    raw -= 1024;
    if (this->raw.size() <= channelId) {
        // This is a new channel, append it
        this->raw.append(raw);
        appendChannelWidget(channelId, 0);
    }
    if (this->raw.size() > channelId) {
        this->raw[channelId] = raw;
        redraw(channelId);
    }

}

void QGCRemoteControlView::setChannelScaled(int channelId, float normalized)
{
    normalized = (normalized * 10000.0f) / 13;
    if (this->normalized.size() <= channelId) // using raw vector as size indicator
    {
        // This is a new channel, append it
        this->normalized.append(normalized);
        appendChannelWidget(channelId, 1);
    }

    this->normalized[channelId] = normalized;
    redraw(channelId);
}

void QGCRemoteControlView::setRemoteRSSI(float rssiNormalized)
{
    if (!rssiBar) {
        rssiBar = drawDataDisplay(0, 99, tr("Radio Quality"))->values().at(0);
    }
    rssi = rssiNormalized;
    redrawRssi();
}

QMap<QLabel*, QProgressBar*> *QGCRemoteControlView::drawDataDisplay(int min, int max, QString label)
{
    // Create new layout
    QHBoxLayout* layout = new QHBoxLayout();
    // Add content
    layout->addWidget(new QLabel(label, this));
    QLabel* lbl_val = new QLabel(this);

    // Append raw label
    layout->addWidget(lbl_val);
    // Append progress bar
    QProgressBar* pb = new QProgressBar(this);
    pb->setMinimum(min);
    pb->setMaximum(max);
    pb->setFormat("%p%");
    pb->setValue(min);
    layout->addWidget(pb);
    channelLayout->addLayout(layout);

    QMap<QLabel*, QProgressBar*> *ret = new(QMap<QLabel*, QProgressBar*>);
    ret->insert(lbl_val, pb);
    return ret;
}

void QGCRemoteControlView::appendChannelWidget(int channelId, bool valType)
{
    QString label = QString("Channel %1").arg(channelId + 1);
    int min = -1024;
    int max = 1024;
    if (channelId == 0) {
        min = -100;
        max = 1500;
    }
    if (valType) { // scaled
        min = -1500;
        max = 1500;
    }

    QMap<QLabel*, QProgressBar*> *obj = drawDataDisplay(min, max, label);
    rawLabels.append(obj->keys().at(0));
    progressBars.append(obj->values().at(0));
}

void QGCRemoteControlView::redraw(int channelId)
{
    if(!isVisible())
        return;

    // Update percent bars
    if (channelId <= rawLabels.size())
        rawLabels.at(channelId)->setText(QString("%1 us").arg(raw.at(channelId), 4, 10, QChar('0')));

    if (channelId <= progressBars.size()) {
        int vv = raw.at(channelId)*1.0f;
        if (vv > progressBars.at(channelId)->maximum())
            vv = progressBars.at(channelId)->maximum();
        if (vv < progressBars.at(channelId)->minimum())
            vv = progressBars.at(channelId)->minimum();

        progressBars.at(channelId)->setValue(vv);
    }
}

void QGCRemoteControlView::redrawRssi()
{
    if(!isVisible() || !rssiBar || rssi < 0.0f || rssi > 99.0f)
        return;

    rssiBar->setValue(rssi);
}

void QGCRemoteControlView::changeEvent(QEvent *e)
{
    Q_UNUSED(e);
    // FIXME If the lines below are commented in
    // runtime errors can occur on x64 systems.
//    QWidget::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        //ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
}
