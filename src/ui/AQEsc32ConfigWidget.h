#ifndef AQESC32CONFIGWIDGET_H
#define AQESC32CONFIGWIDGET_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QTextBrowser>
#include "AQEsc32.h"

namespace Ui {
class AQEsc32ConfigWidget;
}

class AQEsc32ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AQEsc32ConfigWidget(QWidget *parent = 0);
    ~AQEsc32ConfigWidget();

    Ui::AQEsc32ConfigWidget *getUi() const { return ui; }

    void setBaudRates(const QStringList &value);
    void setOutputBrowser(QTextBrowser *outputBrowser);
    void setLastFilePath(const QString &value);
    void setAqBinFolderPath(const QString &value);
    void setPortName(const QString &value);

public slots:
    void flashFWEsc32();

protected:
    void changeEvent(QEvent *e);

private slots:
    void loadSettings();
    void writeSettings();
    void setupPortList();
    void setPortNameEsc32(QString port);
    void btnConnectEsc32();
    void Esc32LoadConfig(QString Config);
    void Esc32ShowConfig(QMap<QString, QString> paramPairs, bool disableMissing = 1);
    void btnSaveToEsc32();
    void Esc32SaveParamsToFile();
    void Esc32LoadParamsFromFile();
    void btnArmEsc32();
    void btnStartStopEsc32();
    void btnSetRPM();
    void saveEEpromEsc32();
    void ParaWrittenEsc32(QString ParaName);
    void CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2 );
    void ESc32Disconnected();
    void Esc32Connected();
    void Esc32StartCalibration();
    void btnReadConfigEsc32();
    void Esc32LoadDefaultConf();
    void Esc32ReLoadConf();
    void Esc32CaliGetCommand(int Command);
    void Esc32StartLogging();
    void Esc32CalibrationFinished(int mode);
    void Esc32BootModOk();
    void Esc32BootModFailure(QString err);
    void Esc32BootModeTimeout();
    void Esc32GotFirmwareVersion(QString ver);
    void Esc32UpdateStatusText(QString text);

private:
    Ui::AQEsc32ConfigWidget *ui;
    AQEsc32 *esc32;
    QTextBrowser *m_outputBrowser;
    int WaitForParaWriten;
    int Esc32CalibrationMode;
    bool FlashEsc32Active;
    bool skipParamChangeCheck;
    bool esc32_connected;
    bool esc32_armed;
    bool esc32_running;
    bool esc32_calibrating;
    QString portName;
    QString LastFilePath;
    QString aqBinFolderPath;
    QString portNameEsc32;
    QString FwFileForEsc32;
    QString ParaNameWritten;
    QMap<QString, QString> paramEsc32;
    QMap<QString, QString> paramEsc32Written;
    QStringList baudRates;

signals:
    void flashFwStart();
};

#endif // AQESC32CONFIGWIDGET_H
