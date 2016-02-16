#ifndef AQDEVELWIDGET_H
#define AQDEVELWIDGET_H

#include <QWidget>
#include <QProcess>

#include "UASInterface.h"

namespace Ui {
class AQDevelWidget;
}

class AQDevelWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AQDevelWidget(QWidget *parent = 0);
	~AQDevelWidget();

    void setUas(UASInterface *value);

protected:
    void changeEvent(QEvent *e);

private slots:

    // Tracking
    void globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time);
    void pushButton_dev1();
    void pushButton_tracking();
    void pushButton_tracking_file();
    void prtstexitTR(int);
    void prtstdoutTR();
    void prtstderrTR();
    void sendTracking();

private:
    Ui::AQDevelWidget *ui;
    UASInterface* uas;
    QString aqBinFolderPath;    // absolute path to AQ supporting utils

    // Tracking
    double lat,lon,alt;
    QProcess ps_tracking;
    int TrackingIsrunning;
    QString FileNameForTracking;
    int TrackingResX;
    int TrackingResY;
    int TrackingMoveX;
    int TrackingMoveY;
    int OldTrackingMoveX;
    int OldTrackingMoveY;
    float focal_lenght;
    float camera_yaw_offset;
    float camera_pitch_offset;
    float pixel_size;
    int pixelFilterX;
    int pixelFilterY;
    float res1,res2;
    QStringList SplitRes;
    float currentPosN, currentPosE;

};

#endif // AQDEVELWIDGET_H
