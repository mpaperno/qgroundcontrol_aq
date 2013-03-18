#ifndef AQ_PWMPORTSCONFIG_H
#define AQ_PWMPORTSCONFIG_H

#include <QWidget>

namespace Ui {
class AQPWMPortsConfig;
}

class AQPWMPortsConfig : public QWidget
{
    Q_OBJECT
    
public:
    explicit AQPWMPortsConfig(QWidget *parent = 0);
    ~AQPWMPortsConfig();
    
private:
    Ui::AQPWMPortsConfig *ui;
};

#endif // AQ_PWMPORTSCONFIG_H
