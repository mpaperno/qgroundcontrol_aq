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
#include <QScrollArea>

#include "MG.h"
#include "QGCRemoteControlView.h"
#include "ui_QGCRemoteControlView.h"
#include "UASManager.h"

QGCRemoteControlView::QGCRemoteControlView(QWidget *parent) :
    QWidget(parent),
    uasId(-1),
    rssi(0.0f),
    updated(false),
    rssiBar(NULL),
    ui(new Ui::QGCRemoteControlView)
{
    ui->setupUi(this);

    channelLayout = new QVBoxLayout();
    channelLayout->setContentsMargins(2, 2, 2, 2);
    channelLayout->setSpacing(4);
    ui->scrollLayout->insertLayout(0, channelLayout);

    delayedSendRCTimer.setInterval(800);  // timer for sending radio freq. update value

    this->setVisible(false);
    toggleRadioValuesUpdate(false);

    connect(ui->spinBox_updateFreq, SIGNAL(valueChanged(int)), this, SLOT(delayedSendRcRefreshFreq()));
    connect(&delayedSendRCTimer, SIGNAL(timeout()), this, SLOT(sendRcRefreshFreq()));
    connect(ui->toolButton_toggleRadioGraph, SIGNAL(clicked(bool)), this, SLOT(onToggleRadioValuesRefresh(bool)));
    connect(UASManager::instance(), SIGNAL(activeUASSet(int)), this, SLOT(setUASId(int)));
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(uasDeleted(UASInterface*)));

    setStatusTitle(false);
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

void QGCRemoteControlView::setStatusTitle(bool on)
{
    if (uasId > -1 && on)
        ui->label_uavName->setText(QString("RC Input of %1").arg(UASManager::instance()->getUASForId(uasId)->getUASName()));
    else
        ui->label_uavName->setText(tr("No System Connected"));
}

void QGCRemoteControlView::removeActiveUAS()
{
    if (uasId == -1)
        return;

    UASInterface* uas = UASManager::instance()->getUASForId(uasId);
    if (uas)
        disconnect(uas, 0, this, 0);
    uasId = -1;
    toggleRadioValuesUpdate(false);
    setStatusTitle(false);

    // Clear channel count
    raw.clear();
    normalized.clear();
    rawLabels.clear();
    progressBars.clear();
    // remove all channel layout items (labels and progress bars)
    QLayoutItem *child;
    QLayout *innerLayout;
    QLayoutItem *innerChild;
    while ((child = channelLayout->takeAt(1)) != 0) {
        if ((innerLayout = child->layout())) {
            while ((innerChild = innerLayout->takeAt(0)) != 0) {
                if (innerChild->widget())
                    innerChild->widget()->setParent(NULL);
                delete innerChild;
            }
            child->layout()->setParent(NULL);
        }
        delete child;
    }
}

void QGCRemoteControlView::setUASId(int id)
{
    removeActiveUAS();

    // Connect the new UAS
    UASInterface* newUAS = UASManager::instance()->getUASForId(id);
    if (newUAS) {
        // New UAS exists, connect
        uasId = id;
        setStatusTitle(true);
        connect(newUAS, SIGNAL(remoteControlRSSIChanged(float)), this, SLOT(setRemoteRSSI(float)));
        connect(newUAS, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));
        //connect(newUAS, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
        connect(newUAS, SIGNAL(dataStreamAnnounced(int,uint8_t,uint16_t,bool)), this, SLOT(dataStreamUpdate(int,uint8_t,uint16_t,bool)));
        connect(newUAS, SIGNAL(heartbeatTimeout(bool,unsigned int)), this, SLOT(setUASstatus(bool,unsigned int)));
    }
}

void QGCRemoteControlView::uasDeleted(UASInterface *mav)
{
    if (mav && mav->getUASID() == this->uasId)
        removeActiveUAS();
}

void QGCRemoteControlView::setUASstatus(bool timeout, unsigned int ms)
{
    Q_UNUSED(ms);
    if (uasId != -1 && timeout != m_uasTimeout) {
        if (timeout) {
            toggleRadioValuesUpdate(false);
            setStatusTitle(false);
        } else
            setStatusTitle(true);
        m_uasTimeout = timeout;
    }
}

void QGCRemoteControlView::dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off)
{
    if (this->uasId == uasId && stream_id == MAV_DATA_STREAM_RC_CHANNELS) {
        if (on_off)
            ui->spinBox_updateFreq->setValue(rate);
        toggleRadioValuesUpdate(on_off);
    }
}

void QGCRemoteControlView::setChannelRaw(int channelId, float raw)
{
    if(!isVisible())
        return;

    if (!ui->scrollAreaWidget->isEnabled())
        toggleRadioValuesUpdate(true);

    // make sure we have the first channel before adding others, otherwise the sequence might get screwed up
    if (channelId != 0 && !this->raw.contains(0))
        return;
    raw -= 1024;
    if (!this->raw.contains(channelId)) {
        // This is a new channel, append it
        this->raw.insert(channelId, (int)raw);
        appendChannelWidget(channelId, 0);
    }
    this->raw[channelId] = raw;
    redraw(channelId);
}

void QGCRemoteControlView::setChannelScaled(int channelId, float normalized)
{
    if(!isVisible())
        return;

    if (!ui->scrollAreaWidget->isEnabled())
        toggleRadioValuesUpdate(true);

    // make sure we have the first channel before adding others, otherwise the sequence might get screwed up
    if (channelId != 1 && !this->raw.contains(1))
        return;

    normalized = (normalized * 10000.0f) / 13;
    if (!this->normalized.contains(channelId)) {
        // This is a new channel, append it
        this->normalized.insert(channelId, normalized);
        appendChannelWidget(channelId, 1);
    }
    this->normalized[channelId] = normalized;
    redraw(channelId);
}

void QGCRemoteControlView::setRemoteRSSI(float rssiNormalized)
{
    if(!isVisible())
        return;

    if (!rssiBar) {
        rssiBar = drawDataDisplay(0, 99, tr("Radio Quality"))->second;
        rssiBar->setProperty("styleType", "radioControlsRSSI");
        rssiBar->setObjectName("rssiBar");
        MG::UTIL::refreshStyleOnWidget(rssiBar);
    }
    rssi = rssiNormalized;
    redrawRssi();
}

QPair<QLabel *, QProgressBar *> *QGCRemoteControlView::drawDataDisplay(int min, int max, QString label)
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

    QPair<QLabel*, QProgressBar*> *ret = new QPair<QLabel*, QProgressBar*>(lbl_val, pb);
    return ret;
}

void QGCRemoteControlView::appendChannelWidget(int channelId, bool valType)
{
    QString label = QString("Ch. %1: ").arg(channelId + 1);
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

    QPair<QLabel*, QProgressBar*> *obj = drawDataDisplay(min, max, label);
    rawLabels.insert(channelId, obj->first);
    progressBars.insert(channelId, obj->second);
}

void QGCRemoteControlView::redraw(int channelId)
{
    if(!isVisible())
        return;

    int val = raw.value(channelId);
    // Update percent bars
    if (rawLabels.contains(channelId) && rawLabels.value(channelId))
        rawLabels.value(channelId)->setText(QString("%1 us").arg(val, 4, 10, QChar('0')));

    if (progressBars.contains(channelId)) {
        QProgressBar *pb = progressBars.value(channelId);
        if (!pb)
            return;
        //val *= 1;
        pb->setValue(qMax(pb->minimum(), qMin(val, pb->maximum())));
    }
}

void QGCRemoteControlView::redrawRssi()
{
    if(!isVisible() || !rssiBar)
        return;

    rssiBar->setValue(qMax(rssiBar->minimum(), qMin((int)rssi, rssiBar->maximum())));
}

void QGCRemoteControlView::toggleRadioValuesUpdate(bool enable)
{
    if (uasId == -1)
        enable = false;

    ui->toolButton_toggleRadioGraph->setChecked(enable);
    ui->scrollAreaWidget->setEnabled(enable);
}

void QGCRemoteControlView::toggleRadioStream(const bool enable)
{
    if (uasId != -1)
        UASManager::instance()->getUASForId(uasId)->enableRCChannelDataTransmission(enable ? ui->spinBox_updateFreq->value() : 0);
}

void QGCRemoteControlView::onToggleRadioValuesRefresh(const bool on)
{
    if (!on || uasId == -1)
        toggleRadioValuesUpdate(false);
    else if (!ui->spinBox_updateFreq->value())
        ui->spinBox_updateFreq->setValue(1);

    toggleRadioStream(on);
}

void QGCRemoteControlView::delayedSendRcRefreshFreq()
{
    delayedSendRCTimer.start();
}

void QGCRemoteControlView::sendRcRefreshFreq()
{
    delayedSendRCTimer.stop();
    toggleRadioValuesUpdate(ui->spinBox_updateFreq->value());
    toggleRadioStream(ui->spinBox_updateFreq->value());
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
