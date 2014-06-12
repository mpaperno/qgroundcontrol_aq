#ifndef QGCAQPARAMWIDGET_H
#define QGCAQPARAMWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QMenu>

#include "QGCUASParamManager.h"
#include "UASInterface.h"

class QPushButton;

class QGCAQParamWidget : public QGCUASParamManager
{
    Q_OBJECT
public:
    QGCAQParamWidget(UASInterface* uas, QWidget *parent = 0);
    ~QGCAQParamWidget();
    /** @brief Get the UAS of this widget */
    UASInterface* getUAS();
	QVariant getParaAQ(QString parameterName);
	void setParaAQ(QString parameterName, QVariant value);
	void loadParaAQ();
    void setFilePath(QString fileName);
    bool paramExistsAQ(const QString& param) { return parameters.contains(190) && parameters.value(190)->contains(param); }
    bool isParamMinKnown(const QString& param) { return paramMin.contains(param); }
    bool isParamMaxKnown(const QString& param) { return paramMax.contains(param); }
    bool isParamDefaultKnown(const QString& param) { return paramDefault.contains(param); }
    double getParamMin(const QString& param) { return paramMin.value(param, 0.0f); }
    double getParamMax(const QString& param) { return paramMax.value(param, 0.0f); }
    double getParamDefault(const QString& param) { return paramDefault.value(param, 0.0f); }
    QString getParamInfo(const QString& param) { return paramToolTips.value(param, ""); }

signals:
    /** @brief A parameter was changed in the widget, NOT onboard */
    //void parameterChanged(int component, QString parametername, float value); // defined in QGCUASParamManager already
    /** @brief Request a single parameter */
    void requestParameter(int component, int parameter);
    /** @brief Request a single parameter by name */
    void requestParameter(int component, const QString& parameter);
    void requestParameterRefreshed();
    void paramRequestTimeout(int readCount, int writeCount);
    void parameterListRequested();

public slots:
    void retranslateUi();
    /** @brief Add a component to the list */
    void addComponent(int uas, int component, QString componentName);
    /** @brief Add a parameter to the list with retransmission / safety checks */
    void addParameter(int uas, int component, int paramCount, int paramId, QString parameterName, QVariant value);
/** @brief Add a parameter to the list */
    void addParameter(int uas, int component, QString parameterName, QVariant value);
    /** @brief Request list of parameters from MAV */
    void requestParameterList();
    /** @brief Request one single parameter */
    void requestParameterUpdate(int component, const QString& parameter);
    /** @brief Set one parameter, changes value in RAM of MAV */
    void setParameter(int component, QString parameterName, QVariant value);
    /** @brief Set all parameters, changes the value in RAM of MAV */
    void setParameters();
    /** @brief Write the current parameters to permanent storage (EEPROM/HDD) */
    void writeParameters();
    /** @brief Read the parameters from permanent storage to RAM */
    void readParameters();
    /** @brief Clear the parameter list */
    void clear();
    /** @brief Update when user changes parameters */
    void parameterItemChanged(QTreeWidgetItem* prev, int column);

    void saveParamFile();
    /** @brief Store parameters to a file */
    void saveParameters(int fileFormat = 1);  // fileFormat: 0 = QGC format, 1 = AQ params.txt format
    /** @brief Load parameters from a file */
    void loadParameters();

    /** @brief Check for missing parameters */
    void retransmissionGuardTick();

    void loadParaFromSD();
    void saveParaToSD();
    void wpFromSD();
    void wpToSD();
    void loadOnboardDefaults();

    void restartUas();
    void restartUasWithPrompt();

    void calibrationAccStart();
    void calibrationMagStart();
    void calibrationSave();
    void calibrationLoad();

    void setRestartBtnEnabled(const bool enable);
    void setCalibBtnsEnabled(const bool enable);
    void resetLinkLossExpected();
    void commandAckReceived(int uasid, int compid, uint16_t command, uint8_t result);

protected:
    QTreeWidget* tree;   ///< The parameter tree
    QLabel* statusLabel; ///< Parameter transmission label
    QMap<int, QTreeWidgetItem*>* components; ///< The list of components
    QMap<int, QMap<QString, QTreeWidgetItem*>* > paramGroups; ///< Parameter groups
    QMenu saveFileMenu;

    // Tooltip data structures
    QMap<QString, QString> paramToolTips; ///< Tooltip values
    // Min / Default / Max data structures
    QMap<QString, double> paramMin; ///< Minimum param values
    QMap<QString, double> paramDefault; ///< Default param values
    QMap<QString, double> paramMax; ///< Minimum param values

    QStringList expandedTreeItems;

    /** @brief Activate / deactivate parameter retransmission */
    void setRetransmissionGuardEnabled(bool enabled);
    /** @brief Load  settings */
    void loadSettings();
    void saveSettings();
    /** @brief Load meta information from CSV */
    void loadParameterInfoCSV(const QString& autopilot, const QString& airframe);
    int OverrideCheckValue;
    UASInterface* uas;
    QString fileNameFromMaster;

protected slots:
    void changeEvent(QEvent *event);
    void trackExpandedItems(QTreeWidgetItem *item);

private:
    QPushButton* refreshButton;
    QPushButton* setButton;
    QPushButton* writeButton;
    QPushButton* loadFileButton;
    QPushButton* saveFileButton;
    QPushButton* readButton;
    QPushButton* loadParaFromSDButton;
    QPushButton* saveParaToSDButton;
    QPushButton* loadDefaultsButton;
    QPushButton* calibrateAccButton;
    QPushButton* calibrateMagButton;
    QPushButton* calibrateSaveButton;
    QPushButton* calibrateReadButton;
    QPushButton* restartButton;

};
class NoEditDelegate: public QStyledItemDelegate {
public:
    NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Q_UNUSED(parent);
        Q_UNUSED(option);
        Q_UNUSED(index);
        return 0;
    }
};

#endif // QGCAQPARAMWIDGET_H
