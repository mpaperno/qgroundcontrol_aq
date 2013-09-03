#ifndef AQ_LOGEXPORTER_H
#define AQ_LOGEXPORTER_H

#include <QDialog>
#include <QProcess>
#include <QCheckBox>
#include <QSettings>
#include <QCloseEvent>

//class QGCAutoquad;

namespace Ui {
class AQLogExporter;
}

namespace AQLOGEXPORTER {
    const QString APP_NAME = "AQLogExporter";
    const QString APP_ORG = "AutoQuad";
    const int APP_VERSION = 100; // 1.0.0
}

class AQLogExporter : public QDialog
{
    Q_OBJECT
    
public:
    explicit AQLogExporter(QWidget *parent = 0);
    ~AQLogExporter();
    void setLogFile(QString &logFile);

protected:
    enum statusMsgTypes { MSG_PLAIN, MSG_INFO, MSG_SUCCESS, MSG_WARNING, MSG_ERROR};
    void closeEvent(QCloseEvent *event);

private:
    void newLogFile();
    void newOutputFile();
    void toggleGPSTrackOpts(bool enable);
    void setExportTypeOptions(QString typ);
    void writeMsgToStatusWindow(QString msg, statusMsgTypes typ = MSG_INFO);
    void scrollStatusWindow();
    void readSettings();
    void writeSettings();

signals:
    void formValidRecheck();

private slots:
    void extProcessExit(int exitcode);
    void extProcessStdErr();
    void extProcessError(QProcess::ProcessError err);
    bool validateForm(bool showAlert = false);
    void startExport();

    void on_lineEdit_inputFile_editingFinished();
    void on_lineEdit_outputFile_editingFinished();
    void on_checkBox_triggerUseGmblTrigger_toggled(bool checked);
    void on_spinBox_triggerChannel_valueChanged(int arg1);
    void on_comboBox_exportFormat_activated(const QString &arg1);
    void on_checkBox_gpsTrack_toggled(bool checked);
    void on_checkBox_gpsWaypoints_toggled(bool checked);
    void on_checkBox_allValues_clicked();
    void on_pushButton_doExport_clicked();
    void on_toolButton_selectLogFile_clicked();
    void on_toolButton_selectOutputFile_clicked();
    void on_toolButton_openOutput_clicked();
    void on_toolButton_browseOutput_clicked();

private:
    Ui::AQLogExporter *ui;
//    QGCAutoquad *aq;
    QSettings settings;
    QProcess ps_export;
    QString savedOutputPath;
    QString savedLogfilePath;
    QString lastOutfilePath;
    QStringList flatExpTypes;
    QStringList xmlExpTypes;
    QStringList allExpTypes;
    bool outFileWasSelectedViaBrowse;

};

#endif // AQ_LOGEXPORTER_H
