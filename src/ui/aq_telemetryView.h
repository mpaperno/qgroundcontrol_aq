#ifndef AQ_TELEMETRYVIEW_H
#define AQ_TELEMETRYVIEW_H

#include "aqlinechartwidget.h"
#include "autoquadMAV.h"
#include <QTabWidget>

namespace Ui {
class AQTelemetryView;
}

using namespace AUTOQUADMAV;

class AQTelemetryView;
typedef QVariant (AQTelemetryView::*ValueCalcCallback)(float);

class AQTelemetryView : public QWidget
{
    Q_OBJECT
    
public:
    explicit AQTelemetryView(QWidget *parent = 0);
    ~AQTelemetryView();
    
private:
    enum telemDatasets { TELEM_DATASET_DEFAULT, TELEM_DATASET_GROUND, TELEM_DATASET_NUM };
    enum telemValueTypes { TELEM_VALUETYPE_FLOAT, TELEM_VALUETYPE_INT };
    enum telemValueDefs {
        TELEM_VALDEF_ACC_MAGNITUDE = 100,
        TELEM_VALDEF_MAG_MAGNITUDE,
        TELEM_VALDEF_ACC_PITCH,
        TELEM_VALDEF_ACC_ROLL
    };

    enum displaySetTypes {
        DSPSET_NONE = 0,
        DSPSET_IMU,
        DSPSET_UKF,
        DSPSET_NAV,
        DSPSET_GPS,
        DSPSET_MOT,
        DSPSET_MOT_PWM,
        DSPSET_SUPERVISOR,
        DSPSET_GIMBAL,
        DSPSET_RC,
        DSPSET_CONFIG,
        DSPSET_STACKS,
        DSPSET_DEBUG,
        DSPSET_ENUM_END
    };

    struct DisplaySet {
        DisplaySet(displaySetTypes id, QString name, QList<int> ds = QList<int>()) : id(id), name(name), datasets(ds) {}

        displaySetTypes id;
        QString name;
        QList<int> datasets;
    };

    struct TelemFieldsMeta {
        TelemFieldsMeta(QString label, telemValueTypes unit, int valueIndex, int msgValueIndex, displaySetTypes dspSet, ValueCalcCallback cb = NULL, telemDatasets dataSet = TELEM_DATASET_DEFAULT) :
            label(label), unit(unit), valueIndex(valueIndex), msgValueIndex(msgValueIndex), dspSetId(dspSet), cb(cb), dataSet(dataSet) {}

        QString label; // human-readable name of field
        telemValueTypes unit; // value type (float|int)
        int valueIndex; // index of telemtry value in mavlink msg
        int msgValueIndex; // __mavlink_aq_telemetry_[f|i]_t.Index
        displaySetTypes dspSetId;
        ValueCalcCallback cb;
        telemDatasets dataSet;
    };

    Ui::AQTelemetryView *ui;
    int msec;
    int totalDatasetFields[TELEM_DATASET_NUM];
    int datasetFieldsSetup;
    bool telemetryRunning;
    int valGridRow;
    int valGridCol;
    uint8_t aqFwVerMaj;
    uint8_t aqFwVerMin;
    uint16_t aqFwVerBld;

    QGridLayout* valuesGridLayout;
    QGridLayout* linLayoutPlot;
    mavlink_aq_telemetry_f_t *currentValuesF;
    telemValueTypes currentValueType;
    telemDatasets currentDataSet;
    QList<DisplaySet> displaySets;
    QList<TelemFieldsMeta> telemDataFields;
    QButtonGroup* btnsDataSets;

    void setupDisplaySetData();
    void clearValuesGrid();
    void setupDataFields(displaySetTypes dspSet);
    void setupCurves(displaySetTypes dspSet);
    float getTelemValue(const int idx);
    QVariant getParamName(const float id);
    bool mavUsesNewDatasets();

public slots:
    void initChart(UASInterface *uav);
    void uasVersionChanged(int uasId, uint32_t fwVer, uint32_t hwVer, QString fwVerStr, QString hwVerStr);


private slots:
    void getNewTelemetry(int uasId, int valIdx);
    void getNewTelemetryF(int uasId, mavlink_aq_telemetry_f_t values);

    void teleValuesToggle();
    bool teleValuesStart();
    void teleValuesStop();
    void frequencyChanged(int freq);
    void datasetButtonClicked(int btnid);

    void on_checkBox_allDatasets_clicked(bool checked);

protected:
    AQLinechartWidget* AqTeleChart;
    UASInterface* uas;
};

#endif // AQ_TELEMETRYVIEW_H
