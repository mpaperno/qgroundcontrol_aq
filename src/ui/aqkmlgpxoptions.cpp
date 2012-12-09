#include "aqkmlgpxoptions.h"
#include "ui_aqkmlgpxoptions.h"

AQKMLGPXOptions::AQKMLGPXOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AQKMLGPXOptions)
{
    ui->setupUi(this);
}

AQKMLGPXOptions::~AQKMLGPXOptions()
{
    delete ui;
}
