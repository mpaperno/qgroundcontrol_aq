#ifndef QGCUASPARAMMANAGER_H
#define QGCUASPARAMMANAGER_H

#include <stdint.h>
#include <QWidget>
#include <QMap>
#include <QTimer>
#include <QVariant>
#include "QGCMAVLink.h"

class UASInterface;

class QGCUASParamManager : public QWidget
{
    Q_OBJECT
public:
    QGCUASParamManager(UASInterface* uas, QWidget *parent = 0);

    QList<QString> getParameterNames(int component) const {
        if (parameterIDs.contains(component))
            return parameters.value(component)->keys();
        else
            return QList<QString>();
    }
    QList<QVariant> getParameterValues(int component) const {
        if (parameterIDs.contains(component))
            return parameters.value(component)->values();
        else
            return QList<QVariant>();
    }
    int getParameterId(int component, const QString& parameter) {
        if (parameterIDs.contains(component))
            return parameterIDs.value(component)->value(parameter);
        else
            return -1;
    }
    QMap<QString, int> *getParameterIdMap(int component) {
        if (parameterIDs.contains(component))
            return parameterIDs.value(component);
        else
            return NULL;
    }

    QString getParameterNameById(int component, const int pId) {
        if (parameterIDs.contains(component))
            return parameterIDs.value(component)->key(pId, "");
        else
            return "";
    }

    bool checkParamExists(int component, const QString& param) {  return parameters.contains(component) && parameters.value(component)->contains(param); }

    bool getParameterValue(int component, const QString& parameter, QVariant& value) const {
        if (!parameters.contains(component) || !parameters.value(component)->contains(parameter))
            return false;
        value = parameters.value(component)->value(parameter);
        return true;
    }

    virtual bool isParamMinKnown(const QString& param) = 0;
    virtual bool isParamMaxKnown(const QString& param) = 0;
    virtual bool isParamDefaultKnown(const QString& param) = 0;
    virtual double getParamMin(const QString& param) = 0;
    virtual double getParamMax(const QString& param) = 0;
    virtual double getParamDefault(const QString& param) = 0;
    virtual QString getParamInfo(const QString& param) = 0;

    /** @brief Request an update for the parameter list */
    void requestParameterListUpdate(int component = 0);
    /** @brief Request an update for this specific parameter */
    virtual void requestParameterUpdate(int component, const QString& parameter) = 0;

signals:
    void parameterChanged(int component, QString parameter, QVariant value);
    void parameterChanged(int component, int parameterIndex, QVariant value);
    void parameterListUpToDate(uint8_t component);

public slots:
    /** @brief Write one parameter to the MAV */
    virtual void setParameter(int component, QString parameterName, QVariant value) = 0;
    /** @brief Request list of parameters from MAV */
    virtual void requestParameterList(uint8_t compId = MAV_COMP_ID_ALL) = 0;

protected:
    UASInterface* mav;   ///< The MAV this widget is controlling
    QMap<int, QMap<QString, QVariant>* > changedValues; ///< Changed values
    QMap<int, QMap<QString, QVariant>* > parameters; ///< All parameters
    QMap<int, QMap<QString, int>* > parameterIDs; ///< Parameter IDs by name
    QMap<int, QMap<QString, uint8_t>* > parameterTypes; ///< Parameter data types
    //QVector<bool> received; ///< Successfully received parameters
    QMap<int, QList<int>* > transmissionMissingPackets; ///< Missing packets
    QMap<int, QMap<QString, QVariant>* > transmissionMissingWriteAckPackets; ///< Missing write ACK packets
    bool transmissionListMode;       ///< Currently requesting list
    QMap<int, uint16_t> transmissionListSizeKnown;  ///< List size initialized?
    QMap<int, uint16_t> transmissionParamsReceived;  ///< List size initialized?
    bool transmissionActive;         ///< Missing packets, working on list?
    quint64 transmissionTimeout;     ///< Timeout
    QTimer retransmissionTimer;      ///< Timer handling parameter retransmission
    int retransmissionTimeout; ///< Retransmission request timeout, in milliseconds
    int rewriteTimeout; ///< Write request timeout, in milliseconds
    int retransmissionBurstRequestSize; ///< Number of packets requested for retransmission per burst

};

#endif // QGCUASPARAMMANAGER_H
