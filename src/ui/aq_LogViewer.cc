#include "aq_LogViewer.h"
#include "ui_aq_LogViewer.h"
#include "MainWindow.h"
#include "MG.h"

#include <qwt_picker_machine.h>

#include <QFileDialog>
#include <QStandardItemModel>
//#include <QSignalMapper>
#include <QSvgGenerator>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>

AQLogViewer::AQLogViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQLogViewer),
    plot(new IncrementalPlot(parent)),
    LastFilePath(""),
    model(NULL),
    picker(NULL),
    MarkerCut1(NULL),
    MarkerCut2(NULL),
    MarkerCut3(NULL),
    MarkerCut4(NULL)
{
    ui->setupUi(this);

    ui->pushButton_save_image_plot->hide();

    QHBoxLayout* layout = new QHBoxLayout(ui->plotFrame);
    layout->addWidget(plot);
    ui->plotFrame->setLayout(layout);

    ui->comboBox_marker->clear();
    ui->comboBox_marker->addItem("Start & End 1s", 0);
    ui->comboBox_marker->addItem("Start & End 2s", 1);
    ui->comboBox_marker->addItem("Start & End 3s", 2);
    ui->comboBox_marker->addItem("Start & End 5s", 3);
    ui->comboBox_marker->addItem("Start & End 10s", 4);
    ui->comboBox_marker->addItem("Start & End 15s", 5);
    ui->comboBox_marker->addItem("manual", 6);

    connect(ui->pushButton_Export_Log, SIGNAL(clicked()),this,SLOT(openExportOptionsDlg()));
    connect(ui->pushButton_Open_Log_file, SIGNAL(clicked()),this,SLOT(OpenLogFile()));
    connect(ui->pushButton_set_marker, SIGNAL(clicked()),this,SLOT(startSetMarker()));
    connect(ui->pushButton_cut, SIGNAL(clicked()),this,SLOT(startCutting()));
    connect(ui->pushButton_remove_marker, SIGNAL(clicked()),this,SLOT(removeMarker()));
    connect(ui->pushButton_clearCurves, SIGNAL(clicked()),this,SLOT(deselectAllCurves()));
//    connect(ui->pushButton_save_image_plot, SIGNAL(clicked()),this,SLOT(save_plot_image()));
    connect(ui->pushButtonshow_cahnnels, SIGNAL(clicked()),this,SLOT(showChannels()));
    connect(ui->comboBox_marker, SIGNAL(currentIndexChanged(int)),this,SLOT(CuttingItemChanged(int)));

    loadSettings();
}

AQLogViewer::~AQLogViewer()
{
    writeSettings();
    delete ui;
}


void AQLogViewer::loadSettings()
{
    // Load defaults from settings
    // QSettings settings("Aq.ini", QSettings::IniFormat);

    settings.beginGroup("AUTOQUAD_SETTINGS");

    if (settings.contains("LAST_LOG_FILE_PATH"))
        LastFilePath = settings.value("LAST_LOG_FILE_PATH", "").toString();
    else
        LastFilePath = settings.value("AUTOQUAD_LAST_PATH", "").toString();
    if (settings.contains("LOGVIEW_SPLITTER_SIZES"))
        ui->splitter->restoreState(settings.value("LOGVIEW_SPLITTER_SIZES").toByteArray());

    settings.endGroup();
    settings.sync();
}

void AQLogViewer::writeSettings()
{
    //QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

    settings.setValue("LAST_LOG_FILE_PATH", LastFilePath);
    settings.setValue("LOGVIEW_SPLITTER_SIZES", ui->splitter->saveState());

    settings.sync();
    settings.endGroup();
}

void AQLogViewer::SetupListView()
{
    ui->listView_Curves->setAutoFillBackground(true);
    QPalette p =  ui->listView_Curves->palette();
    DefaultColorMeasureChannels = p.color(QPalette::Window);
    model = new QStandardItemModel(this); //listView_curves
    QPixmap pix(12, 16);
    pix.fill(Qt::transparent);
    QIcon icn(pix);
    for ( int i=0; i<parser.LogChannelsStruct.count(); i++ ) {
        QPair<QString, AQLogParser::loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
        QStandardItem *item = new QStandardItem(val_pair.second.fieldName);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        item->setData(false, Qt::UserRole);
        item->setIcon(icn);
        model->appendRow(item);
    }
    ui->listView_Curves->setModel(model);
    connect(ui->listView_Curves, SIGNAL(clicked(QModelIndex)), this, SLOT(CurveItemClicked(QModelIndex)));
}

void AQLogViewer::OpenLogFile()
{
    QString dirPath;
    QString fileName;

    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);

    // use native file dialog
    fileName = QFileDialog::getOpenFileName(this, tr("Select AQ Log File"), dir.absoluteFilePath(),
                                            tr("AQ Log File") + " (*.LOG);;" + tr("All File Types") + " (*.*)");

//    if (fileNames.size() > 0)
    if (fileName.length())
    {
//        QFile file(fileNames.first());
        QFile file(fileName);
        LogFile = QDir::toNativeSeparators(file.fileName());
        LastFilePath = LogFile;
        if (!file.size()) {
            MainWindow::instance()->showCriticalMessage("Error", "Log file size is zero!");
        } else if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            MainWindow::instance()->showCriticalMessage("Error", "Could not read Log file. Permission denied!");
        } else {
            file.close();
            DecodeLogFile(LogFile);
        }
    }
}

void AQLogViewer::openExportOptionsDlg() {

    QString platformExt = "";
#if defined(Q_OS_WIN)
     platformExt = ".exe";
#endif
    QString app = QCoreApplication::applicationDirPath() + "/AQ_Log_Export" + platformExt;
    QStringList arg;
    if (LogFile.length() && QFileInfo(LogFile).exists())
        arg << LogFile;

    QProcess::startDetached(app, arg, QCoreApplication::applicationDirPath());

//    static QWeakPointer<AQLogExporter> dlg_;

//    if (!dlg_)
//        dlg_ = new AQLogExporter(this);

//    AQLogExporter *dlg = dlg_.data();

//    if (LogFile.length())
//        dlg->setLogFile(LogFile);

//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    dlg->show();
//    dlg->raise();
//    dlg->activateWindow();
}


void AQLogViewer::CurveItemClicked(QModelIndex index) {
    QStandardItem *item = model->itemFromIndex(index);

    if (item->data(Qt::UserRole).toBool() == false)
    {
        item->setCheckState(Qt::Checked);
        item->setData(true, Qt::UserRole);
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString, AQLogParser::loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 1;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }
    else
    {
        item->setCheckState(Qt::Unchecked);
        item->setData(false, Qt::UserRole);
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString, AQLogParser::loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 0;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }
}

void AQLogViewer::deselectAllCurves(void) {
    if (model) {
        for (int i=0; i < model->rowCount(); ++i){
            if (model->item(i)->data(Qt::UserRole).toBool() == true)
                CurveItemClicked(model->item(i)->index());
        }
    }
}

void AQLogViewer::recolor()
{
    plot->styleChanged(MainWindow::instance()->getStyle());
}

void AQLogViewer::DecodeLogFile(QString fileName)
{
    plot->removeData();
    //plot->detachItems();
    //plot->setStyleText("lines");

    disconnect(ui->listView_Curves, SIGNAL(clicked(QModelIndex)), this, SLOT(CurveItemClicked(QModelIndex)));
    ui->listView_Curves->reset();

    if ( parser.ParseLogHeader(fileName) == 0)
        SetupListView();

}

void AQLogViewer::showChannels() {

    if (!QFile::exists(LogFile)) {
        MainWindow::instance()->showCriticalMessage("Error", "Could not open log file!");
        return;
    }

    plot->removeData();
    //plot->detachItems();
    plot->resetColor();

    parser.ShowCurves();

    for (int i = 0; i < parser.yValues.count(); i++) {
        plot->appendData(parser.yValues.keys().at(i), parser.xValues.values().at(0)->data(), parser.yValues.values().at(i)->data(), parser.xValues.values().at(0)->count());
    }

    plot->setStyleText("lines");
    plot->updateScale();
    QPixmap pix(12, 16);
    for ( int i = 0; i < model->rowCount(); i++) {
        QStandardItem *item = model->item(i,0);
        if ( item->data(Qt::UserRole).toBool() ) {
            pix.fill(plot->getColorForCurve(item->text()));
            //item->setBackground(plot->getColorForCurve(item->text()));
        } else {
            pix.fill(Qt::transparent);
            //item->setBackground(Qt::NoBrush);
        }
        item->setIcon(QIcon::QIcon(pix));
    }

}


void AQLogViewer::save_plot_image(){
//    QString fileName = "plot.svg";
//    fileName = QFileDialog::getSaveFileName(this, "Export File Name", \
//                                            DEFAULT_STORAGE_PATH, \
//                                            "SVG Images (*.svg)"); // ;;PDF Documents (*.pdf)

//    if (!fileName.contains(".")) {
//        // .svg is default extension
//        fileName.append(".svg");
//    }

//    while(!(fileName.endsWith(".svg") || fileName.endsWith(".pdf"))) {
//        QMessageBox msgBox;
//        msgBox.setIcon(QMessageBox::Critical);
//        msgBox.setText("Unsuitable file extension for PDF or SVG");
//        msgBox.setInformativeText("Please choose .pdf or .svg as file extension. Click OK to change the file extension, cancel to not save the file.");
//        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
//        msgBox.setDefaultButton(QMessageBox::Ok);
//        // Abort if cancelled
//        if(msgBox.exec() == QMessageBox::Cancel) return;
//        save_plot_image();
//    }

//    if (fileName.endsWith(".svg"))
//        exportSVG(fileName);
//    else if (fileName.endsWith(".pdf"))
//        exportPDF(fileName);
}

/*
void AQLogViewer::exportPDF(QString fileName)
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    //printer.setFullPage(true);
    printer.setPageMargins(10.0, 10.0, 10.0, 10.0, QPrinter::Millimeter);
    printer.setPageSize(QPrinter::A4);

    QString docName = plot->title().text();
    if ( !docName.isEmpty() ) {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("QGroundControl");
    printer.setOrientation(QPrinter::Landscape);

    plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
    //        plot->setCanvasBackground(Qt::white);
    //        QwtPlotPrintFilter filter;
    //        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);
    //        if ( printer.colorMode() == QPrinter::GrayScale )
    //        {
    //            int options = QwtPlotPrintFilter::PrintAll;
    //            options &= ~QwtPlotPrintFilter::PrintBackground;
    //            options |= QwtPlotPrintFilter::PrintFrameWithScales;
    //            filter.setOptions(options);
    //        }
    plot->print(printer);//, filter);
    plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
    //plot->setCanvasBackground(QColor(5, 5, 8));

}
*/

void AQLogViewer::exportSVG(QString fileName)
{
//    if ( !fileName.isEmpty() ) {
//        plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
//        //plot->setCanvasBackground(Qt::white);
//        QSvgGenerator generator;
//        generator.setFileName(fileName);
//        generator.setSize(QSize(800, 600));

//        QwtPlotPrintFilter filter;
//        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
//        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
//        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
//        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
//        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);

//        plot->print(generator, filter);
//        plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
//    }
}

void AQLogViewer::newPicker()
{
    if (picker)
        removePicker();

    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOff, plot->canvas());
    picker->setStateMachine(new QwtPickerClickPointMachine);
    //picker->setTrackerPen(QColor(Qt::red));
    connect(picker, SIGNAL (selected(const QPointF &)),  this, SLOT (setPoint1(const QPointF &)) );
}

void AQLogViewer::removePicker() {
    if (picker) {
        disconnect(picker, 0,  this, 0);
        picker->setEnabled(false);
        picker->deleteLater();
        picker = NULL;
    }
}

void AQLogViewer::startSetMarker() {

    if ( parser.xValues.count() <= 0)
        return;
    if ( parser.yValues.count() <= 0)
        return;
    removeMarker();

    if ( picker == NULL ) {
        if ( ui->comboBox_marker->currentIndex() == 6) {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }
            ui->pushButton_cut->setEnabled(false);
            QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
            newPicker();
            StepCuttingPlot = 0;
        }
        else {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }
            ui->pushButton_cut->setEnabled(false);

            double x1,x2,y1,y2 = 0;
            int time_count = 0;
            //200 Hz
            if ( ui->comboBox_marker->currentIndex() == 0)
                time_count = 200 * 1;
            if ( ui->comboBox_marker->currentIndex() == 1)
                time_count = 200 * 2;
            if ( ui->comboBox_marker->currentIndex() == 2)
                time_count = 200 * 3;
            if ( ui->comboBox_marker->currentIndex() == 3)
                time_count = 200 * 5;
            if ( ui->comboBox_marker->currentIndex() == 4)
                time_count = 200 * 10;
            if ( ui->comboBox_marker->currentIndex() == 5)
                time_count = 200 * 15;

            MarkerCut1 = new QwtPlotMarker();
            MarkerCut1->setLabel(QString::fromLatin1("sp1"));
            MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
            MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
            x1 = parser.xValues.value("XVALUES")->value(0);
            y1 = parser.yValues.values().at(0)->value(0);
            MarkerCut1->setValue(x1,y1);
            MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut1->setVisible(true);
            MarkerCut1->attach(plot);

            MarkerCut2 = new QwtPlotMarker();
            MarkerCut2->setLabel(QString::fromLatin1("ep1"));
            MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
            MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x2 = parser.xValues.value("XVALUES")->value(time_count-1);
                y2 = parser.yValues.values().at(0)->value(time_count-1);
            }
            else {
                x2 = parser.xValues.value("XVALUES")->value(time_count);
                y2 = parser.yValues.values().at(0)->value(time_count);
            }
            MarkerCut2->setValue(x2,y2);
            MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut2->setVisible(true);
            MarkerCut2->attach(plot);

            MarkerCut3 = new QwtPlotMarker();
            MarkerCut3->setLabel(QString::fromLatin1("sp2"));
            MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
            MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x1 = (parser.xValues.values().at(0)->count()) - time_count;
                y1 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count())- time_count);
            }
            else {
                x1 = (parser.xValues.values().at(0)->count()-1) - time_count;
                y1 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()-1)- time_count);
            }
            MarkerCut3->setValue(x1,y1);
            MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut3->setVisible(true);
            MarkerCut3->attach(plot);

            MarkerCut4 = new QwtPlotMarker();
            MarkerCut4->setLabel(QString::fromLatin1("ep2"));
            MarkerCut4->setLabelAlignment(Qt::AlignLeft|Qt::AlignBottom);
            MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
            if ( parser.getOldLog()) {
                x2 = (parser.xValues.values().at(0)->count());
                y2 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()));
            }
            else {
                x2 = (parser.xValues.values().at(0)->count()-1);
                y2 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()-1));
            }
            MarkerCut4->setValue(x2,y2);
            MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut4->setVisible(true);
            MarkerCut4->attach(plot);
            plot->replot();
            ui->pushButton_cut->setEnabled(true);
        }
    }
    else {
        removePicker();
    }

}

void AQLogViewer::setPoint1(const QPointF &pos) {

    if ( StepCuttingPlot == 0) {
        ui->pushButton_cut->setEnabled(false);

        picker->setEnabled(false);
        MarkerCut1 = new QwtPlotMarker();
        MarkerCut1->setLabel(QString::fromLatin1("sp1"));
        MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut1->setValue((int)pos.x(),pos.y());
        MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut1->setVisible(true);
        MarkerCut1->attach(plot);
        StepCuttingPlot = 1;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
        picker->setEnabled(true);
    }
    else if ( StepCuttingPlot == 1 ) {
        picker->setEnabled(false);
        MarkerCut2 = new QwtPlotMarker();
        MarkerCut2->setLabel(QString::fromLatin1("ep1"));
        MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut2->setValue((int)pos.x(),pos.y());
        MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut2->setVisible(true);
        MarkerCut2->attach(plot);
        StepCuttingPlot = 2;
        plot->replot();
        ui->pushButton_cut->setEnabled(true);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("Select one more cutting area?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                StepCuttingPlot = 3;
                QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
                picker->setEnabled(true);
            }
            break;
            case QMessageBox::No:
            {
                if ( MarkerCut3 != NULL) {
                    MarkerCut3->setVisible(false);
                    MarkerCut3->detach();
                    MarkerCut3 = NULL;
                }
                if ( MarkerCut4 != NULL) {
                    MarkerCut4->setVisible(false);
                    MarkerCut4->detach();
                    MarkerCut4 = NULL;
                }
                removePicker();
            }
            break;

            default:
            break;
        }

    }
    else if ( StepCuttingPlot == 3 ) {
        picker->setEnabled(false);
        MarkerCut3 = new QwtPlotMarker();
        MarkerCut3->setLabel(QString::fromLatin1("sp2"));
        MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
        MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut3->setValue((int)pos.x(),pos.y());
        MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut3->setVisible(true);
        MarkerCut3->attach(plot);
        StepCuttingPlot = 4;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
        picker->setEnabled(true);
    }
    else if ( StepCuttingPlot == 4 ) {
        removePicker();
        MarkerCut4 = new QwtPlotMarker();
        MarkerCut4->setLabel(QString::fromLatin1("ep2"));
        MarkerCut4->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut4->setValue((int)pos.x(),pos.y());
        MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut4->setVisible(true);
        MarkerCut4->attach(plot);
        StepCuttingPlot = 5;
        plot->replot();
        ui->pushButton_cut->setEnabled(true);
    }
}

void AQLogViewer::startCutting() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("Delete the selected frames from the file?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            //LogFile
            QString newFileName = LogFile+".orig";
            if (!QFile::exists(newFileName)) {
                QFile::copy(LogFile,newFileName);
            }
            else
            {
                if (QFile::exists(newFileName))
                    QFile::remove(newFileName);
                QFile::copy(LogFile,newFileName);
            }

            if (QFile::exists(LogFile))
                QFile::remove(LogFile);

            QFile file(LogFile);
            file.write("");
            file.close();


            if ( ui->comboBox_marker->currentIndex() == 6) {
                if ( !MarkerCut3 )
                    parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),-1,-1);
                else
                    parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
            } else {
                parser.ReWriteFile(newFileName,LogFile,MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
            }
            removeMarker();
            plot->removeData();
            plot->detachItems();
            plot->updateScale();
            DecodeLogFile(LogFile);
            ui->pushButton_cut->setEnabled(false);
        }
        break;
        case QMessageBox::No:
            removeMarker();
        break;

        default:
        break;
    }
}

void AQLogViewer::removeMarker() {
    bool needRedraw = false;
    if ( MarkerCut1 != NULL) {
        MarkerCut1->setVisible(false);
        MarkerCut1->detach();
        MarkerCut1 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut2 != NULL) {
        MarkerCut2->setVisible(false);
        MarkerCut2->detach();
        MarkerCut2 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut3 != NULL) {
        MarkerCut3->setVisible(false);
        MarkerCut3->detach();
        MarkerCut3 = NULL;
        needRedraw = true;
    }
    if ( MarkerCut4 != NULL) {
        MarkerCut4->setVisible(false);
        MarkerCut4->detach();
        MarkerCut4 = NULL;
        needRedraw = true;
    }
    if ( needRedraw)
        plot->replot();

    ui->pushButton_cut->setEnabled(false);
}

void AQLogViewer::CuttingItemChanged(int itemIndex) {
    Q_UNUSED(itemIndex);
    removeMarker();
}

