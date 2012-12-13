#include "aq_LogExporter.h"
#include "ui_aq_LogExporter.h"
#include "qgcautoquad.h"

AQLogExporter::AQLogExporter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AQLogExporter)
{

    // QGCAutoquad *aq = new QGCAutoquad();

    ui->setupUi(this);

    // populate default form values
    ui->comboBox_exportFormat->addItem("CSV");
    ui->comboBox_exportFormat->addItem("TAB");
    ui->comboBox_exportFormat->addItem("GPX");
    ui->comboBox_exportFormat->addItem("KML");

    aq = qobject_cast<QGCAutoquad *>(parent);
    if (aq)
        ui->lineEdit_inputFile->insert(aq->LogFile);
    else
        aq = new QGCAutoquad();

}

AQLogExporter::~AQLogExporter()
{
    delete ui;
}

//
// UI Event Handlers
//

// Select log file button click
void AQLogExporter::on_toolButton_selectLogFile_clicked()
{
    aq->OpenLogFile(false);
    ui->lineEdit_inputFile->clear();
    ui->lineEdit_inputFile->insert(aq->LogFile);
}

// Trigger channel changed
void AQLogExporter::on_spinBox_triggerChannel_valueChanged(int arg1)
{
    ui->checkBox_triggerOnly->setEnabled((bool)arg1);
    ui->radioButton_trigVal_opt1->setEnabled((bool)arg1);
    ui->radioButton_trigVal_opt2->setEnabled((bool)arg1);
    ui->radioButton_trigVal_opt3->setEnabled((bool)arg1);
    ui->spinBox_trigVal_gt->setEnabled(arg1 && ui->radioButton_trigVal_opt2->isChecked());
    ui->spinBox_trigVal_lt->setEnabled(arg1 && ui->radioButton_trigVal_opt1->isChecked());
}
