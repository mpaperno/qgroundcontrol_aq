#include "aqkmlgpxoptions.h"
#include "ui_aqkmlgpxoptions.h"

AQKMLGPXOptions::AQKMLGPXOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AQKMLGPXOptions)
{
    ui->setupUi(this);
    //csv|tab|gpx|kml
    ui->format->addItem("csv");
    ui->format->addItem("tab");
    ui->format->addItem("gpx");
    ui->format->addItem("kml");
}

AQKMLGPXOptions::~AQKMLGPXOptions()
{
    delete ui;
}
