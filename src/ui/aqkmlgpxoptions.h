#ifndef AQKMLGPXOPTIONS_H
#define AQKMLGPXOPTIONS_H

#include <QDialog>

namespace Ui {
class AQKMLGPXOptions;
}

class AQKMLGPXOptions : public QDialog
{
    Q_OBJECT
    
public:
    explicit AQKMLGPXOptions(QWidget *parent = 0);
    ~AQKMLGPXOptions();
    
private:
    Ui::AQKMLGPXOptions *ui;
};

#endif // AQKMLGPXOPTIONS_H
