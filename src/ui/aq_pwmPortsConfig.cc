#include "aq_pwmPortsConfig.h"
#include "ui_aq_pwmPortsConfig.h"

AQPWMPortsConfig::AQPWMPortsConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AQPWMPortsConfig)
{
    ui->setupUi(this);
}

AQPWMPortsConfig::~AQPWMPortsConfig()
{
    delete ui;
}
