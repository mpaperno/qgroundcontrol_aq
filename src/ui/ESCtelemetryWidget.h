/*
 * ESC Teletry Widget
 * Author: Maxim Paperno
 * Date: Dec-2014
 * Copyright: (c)2014 Maxim Paperno
 *
 */

#ifndef ESCTELEMETRYWIDGET_H
#define ESCTELEMETRYWIDGET_H

#include "autoquadMAV.h"

#include "UASManager.h"

#include <stdint.h>
#include <QMap>
#include <QWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QVariantList>

class QDialog;
class QLabel;
class QDoubleSpinBox;
class QCheckBox;
class QLCDNumber;
class QGridLayout;

namespace Ui {
class ESCtelemetryWidget;
}

class ESCtelemetryWidget : public QWidget
{
    Q_OBJECT

public:

    enum layoutColumns {
        COL_ESC_ID = 0,
        COL_VERSION,
        COL_VOLTS,
        COL_AMPS,
        COL_WATTS,
        COL_RPM,
        COL_DUTY,
        COL_TEMP,
        COL_ERR_COUNT,
        COL_STATE,
        COL_DISARM_CODE,
        COL_DISARM_REASON,
        COL_AGE,
        COL_STAT,
        COL_ENUM_COUNT
    };

    enum escStates {
        ESC_STATE_DISARMED = 0,
        ESC_STATE_STOPPED,
        ESC_STATE_NOCOMM,
        ESC_STATE_STARTING,
        ESC_STATE_RUNNING
    };

    enum escDisarmReasons {
        REASON_STARTUP = 0,
        REASON_BAD_DETECTS,
        REASON_CROSSING_TIMEOUT,
        REASON_PWM_TIMEOUT,
        REASON_LOW_VOLTAGE,
        REASON_CLI_USER,
        REASON_BINARY_USER,
        REASON_CAN_USER,
        REASON_CAN_TIMEOUT
    };

    enum statusTypes {
        STATUS_NODATA = 0,
        STATUS_OK = 0x1,
        STATUS_WARN = 0x2,
        STATUS_ERR = 0x4,
        STATUS_HIGH = 0x8,
        STATUS_LOW = 0x10,
        STATUS_WARN_L = STATUS_WARN | STATUS_LOW,
        STATUS_WARN_H = STATUS_WARN | STATUS_HIGH,
        STATUS_ERR_L = STATUS_ERR | STATUS_LOW,
        STATUS_ERR_H = STATUS_ERR | STATUS_HIGH,
    };

    enum alertTypes {
        ALERT_TOT_A = 0,
        ALERT_AVG_V,
        ALERT_AVG_W,
        ALERT_ESC_A,
        ALERT_ESC_V,
        ALERT_ESC_W,
        ALERT_ESC_R,
        ALERT_ESC_T,
        ALERT_ESC_E,
        ALERT_ENUM_COUNT
    };

    struct AlertLevels {
        float warnL;
        float warnH;
        float errL;
        float errH;
        uint8_t enableFlags;  // bitfield errH<<3 errL<<2 warnH<<1 warnL<<0

        AlertLevels() : warnL(0.0), warnH(0.0), errL(0.0), errH(0.0), enableFlags(0) {}
        float getValueByIndex(const unsigned idx) {
            switch (idx) {
                case 0:
                    return this->warnL;
                case 1:
                    return this->warnH;
                case 2:
                    return this->errL;
                case 3:
                    return this->errH;
                default :
                    return 0.0;
            }
        }
        void setValueByIndex(const unsigned idx, const float val) {
            switch (idx) {
                case 0:
                    this->warnL = val;
                    break;
                case 1:
                    this->warnH = val;
                    break;
                case 2:
                    this->errL = val;
                    break;
                case 3:
                    this->errH = val;
                    break;
            }
        }
        QVariantList toVariantList() {
            QVariantList vlist;
            vlist << this->warnL << this->warnH << this->errL << this->errH << this->enableFlags;
            return vlist;
        }
        void fromVariantList(const QVariantList &vlist) {
            this->warnL = vlist.at(0).toFloat();
            this->warnH = vlist.at(1).toFloat();
            this->errL = vlist.at(2).toFloat();
            this->errH = vlist.at(3).toFloat();
            this->enableFlags = vlist.at(4).toUInt();
        }
        bool operator==(const AlertLevels &rhs) const { return this->warnL == rhs.warnL && this->warnH == rhs.warnH && this->errL == rhs.errL && this->errH == rhs.errH && this->enableFlags == rhs.enableFlags; }
        bool operator!=(const AlertLevels &rhs) const { return !(*this == rhs); }
    };

    struct alerts_t {
        AlertLevels *data;
        QString labelTxt;
        QString settingsName;
    };

    explicit ESCtelemetryWidget(QWidget *parent = 0);
    ~ESCtelemetryWidget();

protected slots:
    void changeEvent(QEvent *e);
    void loadSettings();
    void writeSettings();
    void loadAlertSettings(const int uasid = 0);
    void writeAlertSettings(const int uasid = 0);
    void loadUASSettings();
    void writeUASSettings();
    void setWidgetTitle();
    void initAlertsMap();
    void setUAS(int id);
    void uasDeleted(UASInterface *mav);
    void setUASstatus(bool timeout, unsigned int ms);
    void toggleUpdate(bool enable);
    void toggleTelemetryMessage(const bool enable);
    void toggleAlertTimer();
    void dataStreamUpdate(const int uasId, const uint8_t stream_id, const uint16_t rate, const bool on_off);
    int setPrecision(const float val);
    int getStatusType(const AlertLevels *data, const float &val);
    QString getPropertyForStatus(const int stat);
    QBrush getColorForStatus(const int stat);
    void escTelemetryRcv(uint8_t escId, uint8_t version, uint16_t time, uint8_t state, float volts, float amps, uint16_t rpm, float duty, float temp, uint16_t errCount, uint8_t errCode);
    void updateLCD(QLCDNumber *wdgt, const float &val, const int typ);
    void delayedSendRefreshFreq(int rate);
    void sendRefreshFreq();
    void resetDisplayDataAvailable(const bool enable);
    void setDisplayUiEnabled();
    void setControlsUiEnabled(const bool enable);
    void setTableVisible();
    void setVisibleColumns();
    void setWatchdogInterval();
    void setAlertRowRanges(const int row, const int changedValIdx = -1);
    void watchdogTimeout();
    void audioAlertTimeout();
    void onToggleRefresh(const bool on);
    void onAudioAlertsSelected(const bool on);
    void onColumnSelected(const bool on);
    void onShowTableSelected(const bool on);
    void onAlertCheckboxSelected(const bool on);
    void onAlertValueChanged(const double val);
    QLabel* makeOptionsLabel(const QString &title, const Qt::Alignment align = Qt::AlignCenter);
    QCheckBox* makeAlertCheckbox(AlertLevels *data, const unsigned &typ);
    QDoubleSpinBox* makeAlertSpinbox(AlertLevels *data, const unsigned &typ);
    void optionsDialog();

protected:
    UASInterface* m_uas;
    QStandardItemModel *m_dataModel;
    QSortFilterProxyModel *m_tableProxyModel;
    QMap<uint8_t, int> m_modelMap;
    QMap<uint8_t, QMap<int, QModelIndex> > m_modelIndexMap;
    QMap<int, alerts_t> m_alertsMap;
    QList<int> m_valueErrors;
    QList<int> m_valueWarnings;
    QDialog *m_optionsDialog;
    QGridLayout *m_gridAlerts;
    QVariantList m_visibleColumnsList;
    QStringList m_columnTitles;
    QStringList m_columnTips;
    QStringList m_escStateNames;
    QStringList m_escDisarmReasonNames;
    QStringList m_alertTitles;
    QStringList m_alertSettingNames;
    int m_refreshFreq;
    bool m_dataAvailable;
    bool m_firstDataAvailable;
    bool m_dataTimeout;
    bool m_uasTimeout;
    bool m_showTable;
    bool m_showAlertsOpts;
    bool m_hasV2data;
    bool m_hasV3data;
    bool m_enableAudioAlerts;
    bool m_mutexAlertRowsUpdating;
    unsigned m_maxDataAge;          // ms after which esc data is considered invalid
    float m_ttlA;
    float m_avgV;
    float m_avgW;
    QTimer m_delayedSendFreqTimer;  // for setting refresh freq.
    QTimer m_watchdogTimer;         // check data for staleness
    QTimer m_audioAlertsTimer;      // check for any error or warning conditions in data

private:
    Ui::ESCtelemetryWidget *ui;

};

Q_DECLARE_METATYPE(ESCtelemetryWidget::AlertLevels)
Q_DECLARE_METATYPE(ESCtelemetryWidget::AlertLevels*)
//Q_DECLARE_TYPEINFO(ESCtelemetryWidget::AlertLevels, Q_PRIMITIVE_TYPE)

#endif // ESCTELEMETRYWIDGET_H
