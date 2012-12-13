#ifndef AQKMLGPXOPTIONS_H
#define AQKMLGPXOPTIONS_H

#include <QDialog>

class QGCAutoquad;

namespace Ui {
class AQLogExporter;
}

class AQLogExporter : public QDialog
{
    Q_OBJECT
    
public:
    explicit AQLogExporter(QWidget *parent = 0);
    ~AQLogExporter();

private slots:
    void on_toolButton_selectLogFile_clicked();
    void on_spinBox_triggerChannel_valueChanged(int arg1);

private:
    Ui::AQLogExporter *ui;
    QGCAutoquad *aq;
};

#endif // AQKMLGPXOPTIONS_H
