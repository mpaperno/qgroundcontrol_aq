#include "aq_LogExporter.h"
#include "ui_aq_LogExporter.h"
//#include "qgcautoquad.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStringBuilder>
#include <QDesktopServices>
#include <QUrl>
#include <QDate>

/**
 * @brief GUI launcher for the AutoQuad logDump log exporter CLI utility.
 * @param parent Parent widget.
 */
AQLogExporter::AQLogExporter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AQLogExporter)
{

    ui->setupUi(this);


    // populate class vars

    // all export file types
    flatExpTypes << "CSV" << "TAB" << "TXT";
    // rich text export types (non flat)
    xmlExpTypes << "GPX" << "KML";
    allExpTypes = flatExpTypes << xmlExpTypes;
    // flag to determine if we should prompt about file overwrite
    outFileWasSelectedViaBrowse = false;

    // connections

    // create a formValidRecheck signal for programatic use
    connect(this, SIGNAL(formValidRecheck()), this, SLOT(validateForm()));
    // map an action whenever one of the export values checkboxes are clicked
    connect(ui->buttonGroup_exportValues, SIGNAL(buttonClicked(int)), this, SLOT(validateForm()));
    connect(ui->buttonGroup_gpsExportVals, SIGNAL(buttonClicked(int)), this, SLOT(validateForm()));
    // connect STDIO
    connect(&ps_export, SIGNAL(finished(int)), this, SLOT(extProcessExit(int)));
    connect(&ps_export, SIGNAL(readyReadStandardError()), this, SLOT(extProcessStdErr()));
    connect(&ps_export, SIGNAL(error(QProcess::ProcessError)), this, SLOT(extProcessError(QProcess::ProcessError)));


    // populate form field values

    // populate export types form field
    for (int i=0; i < allExpTypes.size(); i++)
        ui->comboBox_exportFormat->addItem(allExpTypes.at(i));

    // set IDs for the channel value radio buttons
    foreach (QAbstractButton *curBtn, ui->buttonGroup_trigChanValues->buttons())
        ui->buttonGroup_trigChanValues->setId(curBtn, curBtn->objectName().replace(QRegExp("[^\\d]*"), "").toInt());

    // set IDs for the altitude options radio buttons
    foreach (QAbstractButton *curBtn, ui->buttonGroup_altOptions->buttons())
        ui->buttonGroup_altOptions->setId(curBtn, curBtn->objectName().replace(QRegExp("[^\\d]*"), "").toInt());

    // find QGCAutoquad class to see if a log file is already selected and use its log file selection dialog
//    aq = qobject_cast<QGCAutoquad *>(parent);
//    if (aq)
//        ui->lineEdit_inputFile->insert(aq->LogFile);
//    else
//        aq = new QGCAutoquad();

    // restore settings
    readSettings();

}

AQLogExporter::~AQLogExporter()
{
    delete ui;

    //disconnect STDIO
    disconnect(&ps_export, SIGNAL(finished(int)), this, SLOT(extProcessExit(int)));
    disconnect(&ps_export, SIGNAL(readyReadStandardError()), this, SLOT(extProcessStdErr()));
    disconnect(&ps_export, SIGNAL(error(QProcess::ProcessError)), this, SLOT(extProcessError(QProcess::ProcessError)));
}

/**
 * @brief Scrolls the status window to make sure newly added text is visible
 */
void AQLogExporter::scrollStatusWindow() {
    QTextEdit *te = ui->textEdit_logdumpOutput;
    QTextCursor c = te->textCursor();
    c.movePosition(QTextCursor::End);
    te->setTextCursor(c);
}

/**
 * @brief Write a message to the status window
 * @param msg The message to output
 * @param typ One of statusMsgTypes
 */
void AQLogExporter::writeMsgToStatusWindow(QString msg, statusMsgTypes typ) {
    QStringList colors;
//    QPalette qp;
//    QString winTxtColor = qp.color(QPalette::Active, QPalette::Text).name();
//    qDebug() << winTxtColor;
    colors << "" << "palette(text)" << "green" << "orange" << "red";

    if (typ > MSG_INFO)
        msg = "<b style='color: " % colors.at(typ) % ";'>" % msg % "</b><br/><br/><br/>";
    else if (typ > MSG_PLAIN)
        msg = "<b>" % msg % "</b><br/><br/><br/>";
    else
        msg = msg % "<br/>";

    scrollStatusWindow();
    ui->textEdit_logdumpOutput->insertHtml(msg);
    scrollStatusWindow();
}

/**
 * @brief Validate form for export
 * @param showAlert bool Show alert with error(s) or not
 * @return True if form validates, false otherwise
 */
bool AQLogExporter::validateForm(bool showAlert) {
    QStringList msg;

    if (!ui->lineEdit_inputFile->text().length())
        msg << tr("Please specify a log file to export.");
    else {
        QFileInfo fi(ui->lineEdit_inputFile->text());
        if (!fi.exists()) {
            msg << QString(tr("Log file not found, please verify your entry: %1")).arg(QDir::toNativeSeparators(fi.absoluteFilePath()));
            writeMsgToStatusWindow(msg.at(0), MSG_ERROR);
        }
    }

    if (!ui->lineEdit_outputFile->text().length())
        msg << tr("Please specify a file for the output.");

    if ( !ui->buttonGroup_exportValues->checkedButton() && !ui->buttonGroup_gpsExportVals->checkedButton() ) {
        msg << tr("Please choose at least one value to export.");
        ui->checkBox_allValues->setCheckState(Qt::Unchecked);
    } else if (ui->checkBox_allValues->checkState() == Qt::Unchecked)
        ui->checkBox_allValues->setCheckState(Qt::PartiallyChecked);

    if (!msg.length()) {
        ui->pushButton_doExport->setEnabled(true);
        return true;
    } else {
        ui->pushButton_doExport->setEnabled(false);
        if (showAlert)
            QMessageBox::warning(this, tr("AQ Log Export"), msg.join("\n") ,QMessageBox::Ok, 0 );
        return false;
    }
}

/**
 * @brief Takes action when a new log file has been specified, either via the file
 *          selector button or by editing the form field directly.
 */
void AQLogExporter::newLogFile() {
    if (!ui->lineEdit_inputFile->text().length()) {
        emit formValidRecheck();
        return;
    }

    QString fileName = QDir::toNativeSeparators(ui->lineEdit_inputFile->text());
    ui->lineEdit_inputFile->setText(fileName);
    ui->lineEdit_inputFile->setToolTip(ui->lineEdit_inputFile->text());

    QFileInfo fi(fileName);

    savedLogfilePath = fi.absolutePath();

    // if output file field is blank, populate it
    if (!ui->lineEdit_outputFile->text().length()) {
        QString expName = fi.fileName().replace(fi.suffix(), ui->comboBox_exportFormat->currentText().toLower());
        QString expDir = fi.absolutePath();
        if (savedOutputPath.length())
            expDir = savedOutputPath;

        ui->lineEdit_outputFile->setText(QDir::toNativeSeparators(expDir + "/" + expName));
    }

    // reset date of flight field to log modification date
    // but only if the log date is not the default of 1/1/2012 (which means it's not correct anyway)
    if (fi.exists()) {
        if ( fi.lastModified().toUTC().date() != QDateTime(QDate(2012, 1, 1)).toUTC().date() ) {
            ui->dateEdit_logDate->setDate(fi.lastModified().toUTC().date());
            if (ui->dateEdit_logDate->isEnabled())
                writeMsgToStatusWindow("Date of Flight field has been reset to log file date.", MSG_WARNING);
        } else if (ui->dateEdit_logDate->isEnabled())
           writeMsgToStatusWindow("Log file modification date appears to be incorrect, please set the Date of Flight manually.", MSG_WARNING);
    }

    emit formValidRecheck();
}

/**
 * @brief Takes action when a new output file has been specified either
 *          via file selector or by editing the form field.
 */
void AQLogExporter::newOutputFile() {
    if (!ui->lineEdit_outputFile->text().length()) {
        emit formValidRecheck();
        return;
    }

    QString fileName = QDir::toNativeSeparators(ui->lineEdit_outputFile->text());
    ui->lineEdit_outputFile->setText(fileName);
    ui->lineEdit_outputFile->setToolTip(fileName);

    QFileInfo fi(fileName);
    QString suffx = fi.suffix();

    savedOutputPath = fi.absolutePath();

    int sufIdx = allExpTypes.indexOf(suffx.toUpper());
    if (sufIdx > -1) {
        setExportTypeOptions(suffx);
        ui->comboBox_exportFormat->setCurrentIndex(sufIdx);
    }

    if (!fi.dir().exists()) {
        QString msg = QString(tr("The directory you specified for output (%1) does not exist. The program will attempt to create it during export."))
                .arg(QDir::toNativeSeparators(savedOutputPath));
        writeMsgToStatusWindow(msg, MSG_WARNING);
    }

    emit formValidRecheck();
}

/**
 * @brief Assemble command line and call logDump to perform the export.
 */
void AQLogExporter::startExport() {
    QString appPath;
    QStringList appArgs;
    QString appName = "logDump";
    QString platformExt = "";
    QString appWrkDir = QCoreApplication::applicationDirPath() % "/aq/bin";
    QString outfile;
    QString msg;
    QDir outDir;
    bool gpstrk = false;
    int tmp_i;

    #if defined(Q_OS_WIN)
        platformExt = ".exe";
    #endif

    if (!validateForm(true)) return;

    outfile = ui->lineEdit_outputFile->text();

    // check if output directory exists and try to create if it doesn't
    outDir = QFileInfo(outfile).dir();
    if (!outDir.exists()) {
        msg = QString("Attempting to create output directory: %1").arg(QDir::toNativeSeparators(outDir.absolutePath()));
        writeMsgToStatusWindow(msg, MSG_WARNING);
        if (outDir.mkpath(outDir.absolutePath())) {
            writeMsgToStatusWindow("Directory created!", MSG_SUCCESS);
        } else {
            writeMsgToStatusWindow("Directory creation failed! Please check the path.", MSG_ERROR);
            return;
        }
    }
    // otherwise confirm if we're going to overwrite an existing file, but only if it was typed manually
    // (system file select dialog already prompts to overwrite)
    else if (!outFileWasSelectedViaBrowse && QFileInfo(outfile).exists()) {
        QMessageBox::StandardButton qrply;
        msg = QString("Output file (%1) already exists! Overwrite?").arg(QDir::toNativeSeparators(outfile));
        qrply = QMessageBox::question(this, tr("Confirm Overwrite Existing File"), msg, QMessageBox::Yes | QMessageBox::Cancel);
        if (qrply == QMessageBox::Cancel)
            return;
    }

    lastOutfilePath = outfile;
    savedOutputPath = outDir.absolutePath();

    // export format
    appArgs += QString("-e%1").arg(ui->comboBox_exportFormat->currentText().toLower());
    // frequency
    appArgs += QString("-f%1").arg(ui->spinBox_outputFreq->value());
    // log date (might not be necessary but won't hurt)
    appArgs += QString("-d%1").arg(ui->dateEdit_logDate->date().toString("ddMMyy"));
    // column headers
    if (ui->checkBox_colHeaders->isChecked())
        appArgs += "-c";
    // local time
    if (ui->checkBox_localtime->isChecked())
        appArgs += "-l";
    // trigger channel
    if (ui->spinBox_triggerChannel->value()) {
        appArgs += QString("-t%1").arg(ui->spinBox_triggerChannel->value());
        // channel value (depends on radio btn selected)
        switch (ui->buttonGroup_trigChanValues->checkedId()) {
        case 1 : // > value
            tmp_i = ui->spinBox_trigVal_gt->value();
            break;
        case 2 : // < value
            tmp_i = ui->spinBox_trigVal_lt->value();
            break;
        case 3 : // = 0
            tmp_i = 0;
            break;
        }
        appArgs += QString("-r%1").arg(tmp_i);
    }
    // trigger delay
    if (ui->spinBox_triggerDelay->value())
        appArgs += QString("-i%1").arg(ui->spinBox_triggerDelay->value());
    // triggered only
    if (ui->checkBox_triggerOnly->isChecked())
        appArgs += "-y";
    // gps track
    if (ui->checkBox_gpsTrack->isChecked() || ui->checkBox_gpsWaypoints->isChecked()) {
        appArgs += "-g";
        gpstrk = true;
        // waypoints (include/only/none)
        if (ui->checkBox_gpsWaypoints->isChecked()) {
            if (ui->checkBox_gpsTrack->isChecked())
                appArgs += "-wi";
            else
                appArgs += "-wo";
        }
        // H/V accuracy
        appArgs += QString("-a%1").arg(ui->doubleSpinBox_minHAcc->value());
        appArgs += QString("-v%1").arg(ui->doubleSpinBox_minVAcc->value());

        // altitude type
        if (ui->radioButton_altType_opt02->isChecked())
            appArgs += QString("-Au");
        else if (ui->radioButton_altType_opt03->isChecked())
            appArgs += QString("-Ap");
        // altitude offset
        appArgs += QString("-O%1").arg(ui->doubleSpinBox_altOffset->value());
    }

    // values to export (don't bother if XML output format)
    if (xmlExpTypes.indexOf(ui->comboBox_exportFormat->currentText()) == -1){
        QString valName;
        QList<QAbstractButton *> valBtns = ui->buttonGroup_exportValues->buttons();

        // if gps track is not selected, check gps values also
        if (!gpstrk)
            valBtns += ui->buttonGroup_gpsExportVals->buttons();

        foreach (QAbstractButton *curBtn, valBtns) {
            if (curBtn->isChecked()) {
                valName = "--" % curBtn->objectName().replace("val_", "").replace("_", "-");
                appArgs += valName;
            }
        }
    }
    // gbml_trigger record
    if (ui->checkBox_triggerUseGmblTrigger->isChecked())
        appArgs += "--gmbl-trig";

    // log file
    appArgs += ui->lineEdit_inputFile->text();
    // output file
    //appArgs += QString(">\"%1\"").arg(outfile);

    // End setting all arguments //

    appPath = QDir::toNativeSeparators(appWrkDir % "/" % appName % platformExt);

    msg = "Starting logDump...<br/><br/><i>" % appPath % "</i> " % appArgs.join(" ");
    writeMsgToStatusWindow(msg);
    //writeMsgToStatusWindow(QString(""), MSG_PLAIN);

    ps_export.setProcessChannelMode(QProcess::SeparateChannels);
    ps_export.setStandardOutputFile(outfile);
    ps_export.setWorkingDirectory(QDir::toNativeSeparators(appWrkDir));
    ps_export.start(appPath , appArgs);
}

/**
 * @brief Toggle UI options for GPS track export
 * @param enable Whether gps track options are enabled or not
 */
void AQLogExporter::toggleGPSTrackOpts(bool enable) {
    int i;
    bool isXtyp = xmlExpTypes.indexOf(ui->comboBox_exportFormat->currentText()) > -1;
    QList<QAbstractButton *> gpsTrackFlds = ui->buttonGroup_gpsExportVals->buttons();

    if (enable) {
        if (isXtyp) {
            // if xml export type, de-select all non-gps track values
            for (i=0; i < ui->buttonGroup_exportValues->buttons().size(); i++)
                ui->buttonGroup_exportValues->buttons().at(i)->setChecked(false);
            // de-select column headers
            ui->checkBox_colHeaders->setChecked(false);
        } else // select column headers option
            ui->checkBox_colHeaders->setChecked(true);

        // set default frequency if user did not change it
        if (ui->spinBox_outputFreq->value() == 200) {
            ui->spinBox_outputFreq->setValue(5);
        }

        // enable timestamp option
        ui->checkBox_timestamp->setChecked(true);
    }

    // enable/disable min H/V accuracy fields
    ui->doubleSpinBox_minHAcc->setEnabled(enable);
    ui->doubleSpinBox_minVAcc->setEnabled(enable);
    ui->doubleSpinBox_altOffset->setEnabled(enable && isXtyp);
    ui->label_gpsAccH->setEnabled(enable);
    ui->label_gpsAccV->setEnabled(enable);
    ui->label_altOffset->setEnabled(enable && isXtyp);
    ui->label_altSource->setEnabled(enable && isXtyp);

    // enable/disable column headers option
    ui->checkBox_colHeaders->setEnabled(!enable);
    // enable/disable timestamp option
    ui->checkBox_timestamp->setEnabled(!enable);

    // enable/disable all the gps track values in the list
    for (i=0; i < gpsTrackFlds.size(); i++) {
        gpsTrackFlds.at(i)->setEnabled(!enable);
        if (enable)
            // select all gps values in the list
            gpsTrackFlds.at(i)->setChecked(true);
    }

    // enable/disable altitude option radios
    foreach (QAbstractButton *curBtn, ui->buttonGroup_altOptions->buttons())
        curBtn->setEnabled(enable && isXtyp);

    emit formValidRecheck();
}

/**
 * @brief Toggle UI options based on export format type
 * @param typ Export format
 */
void AQLogExporter::setExportTypeOptions(QString typ) {

    if (xmlExpTypes.indexOf(typ.toUpper()) > -1) {
        ui->groupBox_values->setEnabled(false);
        ui->checkBox_gpsWaypoints->setEnabled(true);
        if (!ui->checkBox_gpsTrack->isChecked() && !ui->checkBox_gpsWaypoints->isChecked())
            ui->checkBox_gpsTrack->setChecked(true);
        else
            toggleGPSTrackOpts(true);
    } else {
        ui->groupBox_values->setEnabled(true);
        ui->checkBox_gpsWaypoints->setEnabled(false);
        if (ui->checkBox_gpsWaypoints->isChecked())
            ui->checkBox_gpsWaypoints->setChecked(false);
        else
            toggleGPSTrackOpts(ui->checkBox_gpsTrack->isChecked());
    }

    emit formValidRecheck();
}

/**
 * @brief Load saved dialog settings from common app storage
 */
void AQLogExporter::readSettings() {
    if (settings.childGroups().contains(AQLOGEXPORTER::APP_NAME)) {
        settings.beginGroup(AQLOGEXPORTER::APP_NAME);

        QPoint pos = settings.value("WindowPosition", QPoint(0,0)).toPoint();
        QSize size = settings.value("WindowSize", QSize(0,0)).toSize();
        if (size.width() > 200)
            resize(size);
        if (pos.x())
            move(pos);

        QString path = settings.value("OutputPath", "").toString();
        if (path.length() && QDir(path).exists())
            savedOutputPath = path;

        path = settings.value("LogfilePath", "").toString();
        if (path.length() && QDir(path).exists())
            savedLogfilePath = path;

        ui->spinBox_outputFreq->setValue(settings.value("OutputFrequency", 200).toInt());
        ui->checkBox_colHeaders->setChecked(settings.value("IncludeHeaders", true).toBool());
        ui->checkBox_gpsTrack->setChecked(settings.value("GPSTrack", false).toBool());
        ui->checkBox_gpsWaypoints->setChecked(settings.value("GPSWaypoints", false).toBool());
        ui->doubleSpinBox_minHAcc->setValue(settings.value("GPSMinHAcc", 2.5).toFloat());
        ui->doubleSpinBox_minVAcc->setValue(settings.value("GPSMinVAcc", 2.5).toFloat());
        ui->buttonGroup_altOptions->button(settings.value("GPSAltOption", 1).toInt())->setChecked(true);
        ui->doubleSpinBox_altOffset->setValue(settings.value("GPSAltOffset", 0).toFloat());
        ui->checkBox_timestamp->setChecked(settings.value("Timestamp", false).toBool());
        ui->checkBox_localtime->setChecked(settings.value("Localtime", false).toBool());
        ui->checkBox_triggerUseGmblTrigger->setChecked(settings.value("TriggerOnLogRecord", false).toBool());
        ui->spinBox_triggerDelay->setValue(settings.value("TriggerDelay", 0).toInt());
        ui->checkBox_triggerOnly->setChecked(settings.value("TriggeredOnly", false).toBool());
        ui->buttonGroup_trigChanValues->button(settings.value("ChannelValueTypeId", 0).toInt())->setChecked(true);
        ui->spinBox_trigVal_gt->setValue(settings.value("TriggerValueGT", 250).toInt());
        ui->spinBox_trigVal_lt->setValue(settings.value("TriggerValueLT", -250).toInt());
        ui->spinBox_triggerChannel->setValue(18); // hack! do this to force a change event on next step
        ui->spinBox_triggerChannel->setValue(settings.value("TriggerChannel", 0).toInt());

        foreach (QString curBtn, settings.value("SelectedValues").toStringList())
            ui->groupBox_values->findChild<QAbstractButton *>(curBtn)->setChecked(true);

        QString expFmt = settings.value("ExportFormat", "CSV").toString().toUpper();
        if (allExpTypes.indexOf(expFmt) > -1) {
            ui->comboBox_exportFormat->setCurrentIndex(allExpTypes.indexOf(expFmt));
            setExportTypeOptions(expFmt);
        }

        settings.endGroup();
    }
}

/**
 * @brief Save dialog settings to common app storage
 */
void AQLogExporter::writeSettings() {
    QStringList selectedValues;

    settings.beginGroup(AQLOGEXPORTER::APP_NAME);

    settings.setValue("APP_VERSION", AQLOGEXPORTER::APP_VERSION);
    settings.setValue("WindowPosition", pos());
    settings.setValue("WindowSize", size());
    settings.setValue("OutputPath", savedOutputPath);
    settings.setValue("LogfilePath", savedLogfilePath);
    settings.setValue("ExportFormat", ui->comboBox_exportFormat->currentText());
    settings.setValue("OutputFrequency", ui->spinBox_outputFreq->value());
    settings.setValue("IncludeHeaders", ui->checkBox_colHeaders->isChecked());
    settings.setValue("GPSTrack", ui->checkBox_gpsTrack->isChecked());
    settings.setValue("GPSWaypoints", ui->checkBox_gpsWaypoints->isChecked());
    settings.setValue("GPSMinHAcc", ui->doubleSpinBox_minHAcc->value());
    settings.setValue("GPSMinVAcc", ui->doubleSpinBox_minVAcc->value());
    settings.setValue("GPSAltOption", ui->buttonGroup_altOptions->checkedId());
    settings.setValue("GPSAltOffset", ui->doubleSpinBox_altOffset->value());
    settings.setValue("Timestamp", ui->checkBox_timestamp->isChecked());
    settings.setValue("Localtime", ui->checkBox_localtime->isChecked());
    settings.setValue("TriggerOnLogRecord", ui->checkBox_triggerUseGmblTrigger->isChecked());
    settings.setValue("TriggerChannel", ui->spinBox_triggerChannel->value());
    settings.setValue("TriggerDelay", ui->spinBox_triggerDelay->value());
    settings.setValue("TriggeredOnly", ui->checkBox_triggerOnly->isChecked());
    settings.setValue("ChannelValueTypeId", ui->buttonGroup_trigChanValues->id(ui->buttonGroup_trigChanValues->checkedButton()));
    settings.setValue("TriggerValueGT", ui->spinBox_trigVal_gt->value());
    settings.setValue("TriggerValueLT", ui->spinBox_trigVal_lt->value());

    foreach (QAbstractButton *curBtn, ui->buttonGroup_exportValues->buttons() + ui->buttonGroup_gpsExportVals->buttons()) {
        if (curBtn->isChecked())
            selectedValues << curBtn->objectName();
    }
    settings.setValue("SelectedValues", selectedValues);

    settings.endGroup();
    settings.sync();
}


//
// Public
//
void AQLogExporter::setLogFile(QString &logFile) {
    if (logFile.length()) {
        ui->lineEdit_inputFile->setText(logFile);
        newLogFile();
    }
}


//
// Private Slots
//

/**
 * @brief Read and handle STDERR output from external process
 */
void AQLogExporter::extProcessStdErr() {
    QString out = ps_export.readAllStandardError().replace("\n", "<br>");
    //if (out.trimmed().length()){
        ui->textEdit_logdumpOutput->insertHtml(out);
        scrollStatusWindow();
    //}
}

/**
 * @brief External process exit handler
 * @param exitcode Exit code from external process
 */
void AQLogExporter::extProcessExit(int exitcode) {
    if (!exitcode) {
        writeMsgToStatusWindow("<br/>logDump finished successfully", MSG_SUCCESS);
        ui->toolButton_openOutput->setEnabled(true);
        ui->toolButton_browseOutput->setEnabled(true);
    }
    else {
        writeMsgToStatusWindow("<br/>logDump exited with error", MSG_ERROR);
        lastOutfilePath = "";
        ui->toolButton_openOutput->setEnabled(false);
        ui->toolButton_browseOutput->setEnabled(false);
    }
}

/**
 * @brief Handle external process error code
 * @param err Error code
 */
void AQLogExporter::extProcessError(QProcess::ProcessError err) {
    QString msg;
    switch(err)
	{
	case QProcess::FailedToStart:
		msg = "Failed to start.";
		break;
	case QProcess::Crashed:
        msg = "Process crashed.";
		break;
	case QProcess::Timedout:
        msg = "Timeout waiting for process.";
		break;
	case QProcess::WriteError:
        msg = "Cannot write to process, exiting.";
		break;
	case QProcess::ReadError:
        msg = "Cannot read from process, exiting.";
		break;
    default:
        msg = "Unknown error";
		break;
	}
    writeMsgToStatusWindow(msg, MSG_ERROR);
}


//
// UI Event Handlers
//

// log file edited manually
void AQLogExporter::on_lineEdit_inputFile_editingFinished()
{
    if (ui->lineEdit_inputFile->isModified())
        newLogFile();
}

// Select log file button click
/**
 * @brief Action for choosing an input file. Determines default path and shows file selection dialog.
 */
void AQLogExporter::on_toolButton_selectLogFile_clicked()
{
//    aq->OpenLogFile(false);
//    ui->lineEdit_inputFile->setText(aq->LogFile);
    QString dirPath;
    QString fileName;

    if ( ui->lineEdit_inputFile->text().length() &&
              QFileInfo(ui->lineEdit_inputFile->text()).canonicalPath().length() )
        dirPath = ui->lineEdit_inputFile->text();
    else if (savedLogfilePath.length())
        dirPath = savedLogfilePath;
    else
        dirPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

    if (!dirPath.length())
        dirPath = QCoreApplication::applicationDirPath();

    QFileInfo fi(dirPath);

    // use native file dialog
    fileName = QFileDialog::getOpenFileName(this, tr("Select AQ Log File"), fi.absoluteFilePath(),
                                            tr("AQ Log File") + " (*.LOG);;" + tr("All File Types") + " (*.*)");

    if (fileName.length()) {
        ui->lineEdit_inputFile->setText(fileName);
        newLogFile();
    }
}

// output file edited manually
void AQLogExporter::on_lineEdit_outputFile_editingFinished()
{
    if (ui->lineEdit_outputFile->isModified()) {
        outFileWasSelectedViaBrowse = false;
        newOutputFile();
    }
}

// file selector button click
/**
 * @brief Action for choosing an output file. Determines default path and shows file selection dialog.
 */
void AQLogExporter::on_toolButton_selectOutputFile_clicked()
{
    QString dirPath;
    QString fileName;

    if ( ui->lineEdit_outputFile->text().length() &&
              QFileInfo(ui->lineEdit_outputFile->text()).canonicalPath().length() )
        dirPath = ui->lineEdit_outputFile->text();
    else if (savedOutputPath.length())
        dirPath = savedOutputPath;
    else if (savedLogfilePath.length())
        dirPath = savedLogfilePath;
    else
        dirPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

    if (!dirPath.length())
        dirPath = QCoreApplication::applicationDirPath();

    QFileInfo fi(dirPath);

    if (fi.isDir()) {
        QFileInfo fiIn(ui->lineEdit_inputFile->text());
        if (fiIn.isFile()) {
            QString expExt = ui->comboBox_exportFormat->currentText().toLower();
            dirPath = fi.absolutePath() + "/" + fiIn.fileName().replace(fiIn.suffix(), expExt);
            fi = QFileInfo(dirPath);
        }
    }

    // use native file dialog
    fileName = QFileDialog::getSaveFileName(this, tr("Select Output File"), fi.absoluteFilePath(),
                                            tr("Export File Types") + " (*.csv *.tab *.txt *.gpx *.kml);;"  + tr("All File Types") + " (*.*)");

    // use Qt file dialog (slower)
//    QFileDialog dialog;
//    dialog.setDirectory(dir.absoluteDir());
//    dialog.setFileMode(QFileDialog::AnyFile);
//    dialog.setNameFilter(tr("Export File Types (*.csv *.tab *.txt *.gpx *.kml)"));
//    dialog.setViewMode(QFileDialog::Detail);
//    if (dialog.exec())
//        fileName = dialog.selectedFiles()at(0);

    if (fileName.length()) {
        outFileWasSelectedViaBrowse = true;
        ui->lineEdit_outputFile->setText(fileName);
        newOutputFile();
    }
}

// Trigger activated
void AQLogExporter::on_checkBox_triggerUseGmblTrigger_toggled(bool checked)
{
    bool en = (checked || ui->spinBox_triggerChannel->value());
    ui->checkBox_triggerOnly->setEnabled(en);
    ui->label_triggerDelay->setEnabled(en);
    ui->spinBox_triggerDelay->setEnabled(en);
}

// Trigger channel changed
void AQLogExporter::on_spinBox_triggerChannel_valueChanged(int arg1)
{
    bool en = (arg1 || ui->checkBox_triggerUseGmblTrigger->isChecked());
    ui->checkBox_triggerOnly->setEnabled(en);
    ui->label_triggerDelay->setEnabled(en);
    ui->spinBox_triggerDelay->setEnabled(en);

    ui->radioButton_trigVal_opt01->setEnabled((bool)arg1);
    ui->radioButton_trigVal_opt02->setEnabled((bool)arg1);
    ui->radioButton_trigVal_opt03->setEnabled((bool)arg1);
    ui->label_trigChVal->setEnabled((bool)arg1);
    ui->spinBox_trigVal_gt->setEnabled(arg1 && ui->radioButton_trigVal_opt01->isChecked());
    ui->spinBox_trigVal_lt->setEnabled(arg1 && ui->radioButton_trigVal_opt02->isChecked());
}

// export format changed
void AQLogExporter::on_comboBox_exportFormat_activated(const QString &arg1)
{
    QString outfile = ui->lineEdit_outputFile->text();

    if (outfile.length()) {
        QFileInfo fi(outfile);
        QString expExt = arg1.toLower();
        ui->lineEdit_outputFile->setText(outfile.replace(fi.suffix(), expExt));
    }

    setExportTypeOptions(arg1);
}

// GPS track checkbox toggled
void AQLogExporter::on_checkBox_gpsTrack_toggled(bool checked)
{
    if (!checked && xmlExpTypes.indexOf(ui->comboBox_exportFormat->currentText()) > -1)
        ui->checkBox_gpsWaypoints->setChecked(true);
    else
        toggleGPSTrackOpts(checked);
}

// GPS waypoint checkbox toggled
void AQLogExporter::on_checkBox_gpsWaypoints_toggled(bool checked)
{
    if (!checked && xmlExpTypes.indexOf(ui->comboBox_exportFormat->currentText()) > -1)
        ui->checkBox_gpsTrack->setChecked(true);
    else
        toggleGPSTrackOpts(checked);
}

// Export button clicked
void AQLogExporter::on_pushButton_doExport_clicked()
{
    startExport();
}

// All Values checkbox clicked
void AQLogExporter::on_checkBox_allValues_clicked()
{
    Qt::CheckState state = ui->checkBox_allValues->checkState();
    QAbstractButton *curBtn;
    QList<QAbstractButton *> valBtns;
    valBtns << ui->buttonGroup_exportValues->buttons() << ui->buttonGroup_gpsExportVals->buttons();

    if (state == Qt::PartiallyChecked) {
        state = Qt::Checked;
        ui->checkBox_allValues->setCheckState(Qt::Checked);
    } else if (state == Qt::Checked) {
        state = Qt::Unchecked;
        ui->checkBox_allValues->setCheckState(Qt::Unchecked);
    }

    for (int i=0; i < valBtns.size(); i++)  {
        curBtn = valBtns.at(i);
        if (curBtn->isEnabled()) {
            curBtn->setChecked(state);
        }
    }

    emit formValidRecheck();
}

// Open Output File button click
void AQLogExporter::on_toolButton_openOutput_clicked()
{
    if (lastOutfilePath.length()) {
        if ( !QDesktopServices::openUrl(QUrl::fromLocalFile(lastOutfilePath)) )
            writeMsgToStatusWindow("Could not launch native application to handle output file.", MSG_ERROR);
    } else
        writeMsgToStatusWindow("Could not determine output file location.", MSG_ERROR);
}

// Browse output folder button click
void AQLogExporter::on_toolButton_browseOutput_clicked()
{
    if (lastOutfilePath.length()) {
        if ( !QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(lastOutfilePath).absolutePath())) )
            writeMsgToStatusWindow("Could not launch native file browser.", MSG_ERROR);
    } else
        writeMsgToStatusWindow("Could not determine output directory.", MSG_ERROR);
}

// Dialog close event
void AQLogExporter::closeEvent(QCloseEvent *event)
{
    if (ps_export.state() == QProcess::Running) {
        QMessageBox::StandardButton qrply;
        QString msg = "LogDump is still running. If you close this window, it will keep running in the background.\n\nAre you sure?";
        qrply = QMessageBox::question(this, tr("Confirm Close Window"), msg, QMessageBox::Yes | QMessageBox::Cancel);
        if (qrply == QMessageBox::Cancel)
            event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}
