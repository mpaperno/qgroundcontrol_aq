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
 *   @brief Declaration of QGCRemoteControlView
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 */

#ifndef QGCREMOTECONTROLVIEW_H
#define QGCREMOTECONTROLVIEW_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QMap>
#include <stdint.h>

#include "UASInterface.h"

namespace Ui
{
class QGCRemoteControlView;
}

class QVBoxLayout;
class QLabel;
class QProgressBar;

class QGCRemoteControlView : public QWidget
{
    Q_OBJECT
public:
    QGCRemoteControlView(QWidget *parent = 0);
    ~QGCRemoteControlView();

public slots:
    void setUASId(int id);
    void uasDeleted(UASInterface *mav);
    void setUASstatus(bool timeout, unsigned int ms);
    void dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off);
    void setChannelRaw(int channelId, float raw);
    void setChannelScaled(int channelId, float normalized);
    void setRemoteRSSI(float rssiNormalized);
    void redraw(int channelId);
    void redrawRssi();
    void toggleRadioValuesUpdate(bool enable);
    void toggleRadioStream(const bool enable);
    void onToggleRadioValuesRefresh(const bool on);
    void delayedSendRcRefreshFreq();
    void sendRcRefreshFreq();

protected slots:
    void removeActiveUAS();
    QPair<QLabel*, QProgressBar*> *drawDataDisplay(int min, int max, QString label);
    void appendChannelWidget(int channelId, bool valType);
    void setStatusTitle(bool on);

protected:
    void changeEvent(QEvent *e);
    int uasId;
    float rssi;
    bool updated;
    bool m_uasTimeout;
    QVBoxLayout* channelLayout;
    QMap<int, int> raw;
    QMap<int, float> normalized;
    QMap<int, QLabel*> rawLabels;
    QMap<int, QProgressBar*> progressBars;
    QProgressBar* rssiBar;
    QTimer updateTimer;
    QTimer delayedSendRCTimer;  // for setting radio channel refresh freq.

private:
    Ui::QGCRemoteControlView *ui;
};

#endif // QGCREMOTECONTROLVIEW_H
