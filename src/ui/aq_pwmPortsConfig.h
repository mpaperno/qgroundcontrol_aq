#ifndef AQ_PWMPORTSCONFIG_H
#define AQ_PWMPORTSCONFIG_H

#include "qgcaqparamwidget.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStyledItemDelegate>
#include <QStringListModel>

class QGCAutoquad;
class QTableWidgetItem;
class QWidget;
class QComboBox;
class QToolButton;

namespace Ui {
class AQPWMPortsConfig;
}

class AQPWMPortsConfig : public QWidget
{
    Q_OBJECT
    
public:
    explicit AQPWMPortsConfig(QWidget *parent = 0);
    ~AQPWMPortsConfig();
    void portConfigConnections(bool enable);
    QComboBox* makeMotorPortCombo(QWidget *parent);
    QComboBox* makeMotorPortTypeCombo(QWidget *parent);
    QString motorPortTypeName(uint8_t type);
    uint8_t motorPortTypeId(QString type);

    enum motorTableIndex { // these correspond to which column of the table is used for which value
        COL_MOTOR = 0,
        COL_PORT,
        COL_THROT,
        COL_PITCH,
        COL_QPITCH,
        COL_ROLL,
        COL_QROLL,
        COL_YAW,
        COL_QYAW,
        COL_TYPE
    };

    enum motorPortTypes {
        MOT_PORT_TYPE_PWM = 0,
        MOT_PORT_TYPE_CAN,
        MOT_PORT_TYPE_CAN_H,
        MOT_PORT_TYPE_ENUM_END
    };

protected:
    void changeEvent(QEvent *event);

private:
    Ui::AQPWMPortsConfig* ui;

    struct motorPortSettings {
        motorPortSettings(uint16_t port=1, float throt=0, float pitch=0, float roll=0, float yaw=0, uint8_t type=0, float qpitch=0, float qroll=0, float qyaw=0) :
            port(port), throt(throt), pitch(pitch), roll(roll), yaw(yaw), qpitch(qpitch), qroll(qroll), qyaw(qyaw), type(type) {}

        uint16_t port;
        float throt;
        float pitch;
        float roll;
        float yaw;
        float qpitch;
        float qroll;
        float qyaw;
        uint8_t type;
    };

    QList<motorPortSettings> motorPortsConfig;

public slots:
    void changeMixType(void);
    void setFrameImage(QString file = "");
    QStringList getMixFileList(void);
    QString getMixFileByConfigId(int configId);
    bool mixTypeQuatos() { return m_mixTypeQuatos; }
    void setMixTypeQuatos(const bool yes);
    void setAllMotorPortTypes(quint8 type);
    void loadFileConfig(QString file);
    void saveConfigFile(QString file);
    void loadCustomConfig(int numMotors);
    void loadOnboardConfig(void);
    quint8 saveOnboardConfig(QMap<QString, QList<float> > *changeList, QStringList *errors);
    void loadFrameTypes(void);
    bool validateForm(void);
    void portNumbersModel_updated(void);

private slots:
    void loadSettings();
    void writeSettings();
    void motorTableConnections(bool enable);
    void drawMotorsTable(void);
    bool updateMotorSums(void);
    void setVisibleTableColumns();
    void detectQuatos();
    void motorPortsConfig_updated(int row, int col);
    void motorMix_buttonClicked(int btn);
    void mixSelector_currentIndexChanged(int index);
    void numOfMotors_currentIndexChanged(int index);
    void portSelector_currentIndexChanged(int);
    void loadFile_clicked();
    void saveFile_clicked();
    void loadImage_clicked();
    void allToCAN_clicked();
    void allToPWM_clicked();
    void toggleQuatos(bool checked);
    void splitterCollapseToggle(bool on);
    void splitterMoved();
    void firmwareVersion_updated();

public:
    bool motorMixType;    // configuration type selected: 0 = custom; 1 = predefined;
    uint8_t mixConfigId;     // config ID of current setup;
    QString frameImageFile;         // file path of current frame image
    int dataChangeType;     // keep track of which data type is being updated in table model
    QString motMixLastFile;     // last file/path used for saved mix file

protected:
    QGCAutoquad* aq;
    QStringListModel* model_portNumbers;
    QGCAQParamWidget* paramHandler;

private:
    QGraphicsScene* scene_frameLayout;
    QGraphicsPixmapItem* frameLayout_bgItem;
    QGraphicsPixmapItem* frameLayout_fgItem;
    QList<QComboBox *> allPortSelectors;
    QStringList motorConfigErrors;
    QString portOrder2Param;
    QString mixFilesPath;
    QString mixImagesPath;
    bool mtx_portModelIsUpdating;
    bool errorInMotorConfig;        // output numbers out of range
    bool errorInMotorConfigTotals;  // output totals don't add up
    bool errorInPortConfig;         // duplicate pwm or motor ports
    bool errorInTimerConfig;        // timer conflict between motor and gimbal outputs
    bool errorInPortNumberType;     // means selected PWM port is > available PWM output ports on hardware
    bool customFrameImg;
    bool m_mixTypeQuatos;
};


// ----------------------------------------------
// Combo Box Delegate for motor port number and port type selectors
// ----------------------------------------------
class PwmPortsComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PwmPortsComboBoxDelegate(QObject *parent = 0, AQPWMPortsConfig* aqPwmPortConfig = NULL);
    ~PwmPortsComboBoxDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,  const QModelIndex &index) const;

protected:
    AQPWMPortsConfig* aqPwmPortConfig;
};

// ----------------------------------------------
// Line Edit Delegate for float values
// ----------------------------------------------
class PwmPortsLineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PwmPortsLineEditDelegate(QObject *parent = 0);
    ~PwmPortsLineEditDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,  const QModelIndex &index) const;
    QString displayText(const QVariant &value, const QLocale& locale) const;
    QString formatDouble(const QVariant &value) const;

protected:
};

#endif // AQ_PWMPORTSCONFIG_H
