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

namespace Ui {
class AQPWMPortsConfig;
}

class AQPWMPortsConfig : public QWidget
{
    Q_OBJECT
    
public:
    explicit AQPWMPortsConfig(QWidget *parent = 0);
    ~AQPWMPortsConfig();
    QComboBox* makeMotorPortCombo(QWidget *parent);
    
private:
    Ui::AQPWMPortsConfig* ui;

    enum motorTableIndex { // these correspond to which column of the table is used for which value
        COL_MOTOR = 0,
        COL_PORT,
        COL_THROT,
        COL_PITCH,
        COL_ROLL,
        COL_YAW
    };

    struct motorPortSettings {
        motorPortSettings(uint16_t port=0, float throt=0, float pitch=0, float roll=0, float yaw=0) :
            port(port), throt(throt), pitch(pitch), roll(roll), yaw(yaw) {}

        uint16_t port;
        float throt;
        float pitch;
        float roll;
        float yaw;
    };

    QList<motorPortSettings> motorPortsConfig;

public slots:
    void changeMixType(void);
    void setFrameImage(QString file = "");
    QStringList getMixFileList(void);
    QString getMixFileByConfigId(int configId);
    void loadFileConfig(QString file);
    void saveConfigFile(QString file);
    void loadCustomConfig(int numMotors);
    void loadOnboardConfig(void);
    quint8 saveOnboardConfig(QMap<QString, QList<float> > *changeList, QStringList *errors);
    void loadFrameTypes(void);
    bool validateForm(void);
    void portNumbersModel_updated(void);

private slots:
    void motorTableConnections(bool enable);
    void portConfigConnections(bool enable);
    void drawMotorsTable(void);
    bool updateMotorSums(void);
    void updatePortsConfigModel(int row, int col);
    void motorPortsConfig_updated(int row, int col);
    void motorMix_buttonClicked(int btn);
    void mixSelector_currentIndexChanged(int index);
    void numOfMotors_currentIndexChanged(int index);
    void portSelector_currentIndexChanged(int);
    void loadFile_clicked();
    void saveFile_clicked();
    void loadImage_clicked();

public:
    bool motorMixType;    // configuration type selected: 0 = custom; 1 = predefined;
    uint8_t mixConfigId;     // config ID of current setup;
    QString frameImageFile;         // file path of current frame image

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
    bool errorInMotorConfig;
    bool errorInMotorConfigTotals;
    bool errorInPortConfig;
    bool errorInTimerConfig;
    bool customFrameImg;
};


// ----------------------------------------------
// Combo Box Delegate
// ----------------------------------------------
class PwmPortsComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PwmPortsComboBoxDelegate(QObject *parent = 0, AQPWMPortsConfig* aqPwmPortConfig = NULL);
    ~PwmPortsComboBoxDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,  const QModelIndex &index) const;

private slots:
//    void commitAndCloseEditor();

protected:
    AQPWMPortsConfig* aqPwmPortConfig;

};

#endif // AQ_PWMPORTSCONFIG_H
