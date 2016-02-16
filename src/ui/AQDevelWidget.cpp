#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include "AQDevelWidget.h"
#include "ui_AQDevelWidget.h"

AQDevelWidget::AQDevelWidget(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::AQDevelWidget),
    uas(NULL)
{
    aqBinFolderPath = QCoreApplication::applicationDirPath() + "/aq/bin/";

    ui->setupUi(this);

    connect(ui->pushButton_dev1, SIGNAL(clicked()),this, SLOT(pushButton_dev1()));
    connect(ui->pushButton_ObjectTracking, SIGNAL(clicked()),this, SLOT(pushButton_tracking()));
    connect(ui->pushButton_ObjectTracking_File, SIGNAL(clicked()),this, SLOT(pushButton_tracking_file()));

    //Process Slots for tracking
    ps_tracking.setProcessChannelMode(QProcess::MergedChannels);
    connect(&ps_tracking, SIGNAL(finished(int)), this, SLOT(prtstexitTR(int)));
    connect(&ps_tracking, SIGNAL(readyReadStandardOutput()), this, SLOT(prtstdoutTR()));
    connect(&ps_tracking, SIGNAL(readyReadStandardError()), this, SLOT(prtstderrTR()));
    TrackingIsrunning = 0;

}

AQDevelWidget::~AQDevelWidget()
{
    delete ui;
}

void AQDevelWidget::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void AQDevelWidget::pushButton_dev1(){
//    QString audiostring = QString("Hello, welcome to AutoQuad");
//    GAudioOutput::instance()->say(audiostring.toLower());
//    float headingInDegree = ui->lineEdit_13->text().toFloat();
//    uas->sendCommmandToAq(7, 1, headingInDegree,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
//    QEventLoop waiter;
//    connect(uas, SIGNAL(textMessageReceived()), &waiter, SLOT(quit()));
//    QTimer::singleShot(5000, &waiter, SLOT(quit()));
//    ui->lineEdit_13->setText("");
//    ui->lineEdit_14->setText("");
//    ui->lineEdit_13->setText(QString::number(aqFirmwareVersion));
//    ui->lineEdit_14->setText(QString::number(aqFirmwareRevision));
//    ui->lineEdit_15->setText(QString::number(aqHardwareRevision));
//    waiter.exec();
}

//
// Tracking
//

void AQDevelWidget::globalPositionChangedAq(UASInterface *, double lat, double lon, double alt, quint64 time)
{
     Q_UNUSED(time);
     if ( !uas)
          return;
     this->lat = lat;
     this->lon = lon;
     this->alt = alt;
}

void AQDevelWidget::pushButton_tracking() {
     if ( TrackingIsrunning == 0) {
          QString AppPath = QDir::toNativeSeparators(aqBinFolderPath + "opentld");
          QStringList Arguments;

          if ( this->uas == NULL) {
                //qDebug() << "no UAS connected";
                //return;
          }
          OldTrackingMoveX = 0;
          OldTrackingMoveY = 0;

          //globalPositionChanged
          // For video cam
          if ( FileNameForTracking == "" ) {
                Arguments.append("-d CAM");
                Arguments.append("-n " + ui->lineEdit_21->text());
                Arguments.append(aqBinFolderPath + "config.cfg");
          }
          // for Files
          else {
                Arguments.append("-d VID");
                Arguments.append("-i " + FileNameForTracking);
          }
          focal_lenght = ui->lineEdit_20->text().toFloat();
          camera_yaw_offset = ui->lineEdit_19->text().toFloat();
          camera_pitch_offset = ui->lineEdit_18->text().toFloat();
          pixel_size = ui->lineEdit_171->text().toFloat();
          pixelFilterX = ui->lineEdit_22->text().toFloat();
          pixelFilterY = ui->lineEdit_23->text().toFloat();
          currentPosN = (float)10.75571;
          currentPosE = (float)48.18003;
          ps_tracking.setWorkingDirectory(aqBinFolderPath);
          ps_tracking.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
          TrackingIsrunning = 1;
      }
      else {
          OldTrackingMoveX = 0;
          OldTrackingMoveY = 0;

          ps_tracking.kill();
          TrackingIsrunning = 0;
      }

}

void AQDevelWidget::pushButton_tracking_file() {
     QString dirPath;
     //QFileInfo dir(dirPath);
     QFileDialog dialog;
     //dialog.setDirectory(dir.absoluteDir());
     dialog.setFileMode(QFileDialog::AnyFile);
     //dialog.setFilter(tr("AQ Parameter-File (*.params)"));
     dialog.setViewMode(QFileDialog::Detail);
     QStringList fileNames;
     if (dialog.exec())
     {
          fileNames = dialog.selectedFiles();
     }

     dirPath = fileNames.at(0);

     FileNameForTracking = QDir::toNativeSeparators(dirPath);
     QFile file( FileNameForTracking );
}

void AQDevelWidget::prtstexitTR(int) {
     qDebug() << ps_tracking.readAll();
}

void AQDevelWidget::prtstdoutTR() {
     QString stdout_TR = ps_tracking.readAllStandardOutput();
     QStringList stdout_TR_Array = stdout_TR.split("\r\n",QString::SkipEmptyParts);
     for (int i=0; i < stdout_TR_Array.length(); i++) {
          //qDebug() << stdout_TR_Array.at(i);
          if (stdout_TR_Array.at(i).contains("RESOLUTION=")) {
                int posResolution = stdout_TR_Array.at(i).indexOf("RESOLUTION",0,Qt::CaseSensitive);
                if ( posResolution >= 0) {
                     posResolution += 11;
                     int posEndOfResolution = stdout_TR_Array.at(i).indexOf("\"",0,Qt::CaseSensitive);
                     QString resString = stdout_TR_Array.at(i).mid(posResolution, posEndOfResolution-posResolution);
                     SplitRes = resString.split(' ',QString::SkipEmptyParts);
                     TrackingResX = SplitRes[0].toInt();
                     TrackingResY = SplitRes[1].toInt();
                }
          }
          else if (stdout_TR_Array.at(i).contains("POS=")) {
                int posXMove = stdout_TR_Array.at(i).indexOf("POS=",0,Qt::CaseSensitive);
                if ( posXMove >= 0) {
                     posXMove += 4;
                     int posEndOfPOS = stdout_TR_Array.at(i).indexOf("\"",0,Qt::CaseSensitive);
                     QString resString = stdout_TR_Array.at(i).mid(posXMove, posEndOfPOS-posXMove);
                     SplitRes = resString.split(' ',QString::SkipEmptyParts);
                     TrackingMoveX = SplitRes[0].toInt();
                     TrackingMoveY = SplitRes[1].toInt();
                     int DiffX = abs(OldTrackingMoveX - TrackingMoveX);
                     int DiffY = abs(OldTrackingMoveY - TrackingMoveY);
                     if ((  DiffX > pixelFilterX) || ( DiffY > pixelFilterY)) {
                          OldTrackingMoveX = TrackingMoveX;
                          OldTrackingMoveY = TrackingMoveY;
                          sendTracking();
                          QString output_tracking;
                          output_tracking.append("Xdiff=");
                          output_tracking.append(QString::number(DiffX));
                          output_tracking.append("Ydiff=");
                          output_tracking.append(QString::number(DiffY));
                          //qDebug() << output_tracking;
                          ui->lineEdit_24->setText(output_tracking);
                     }

                }
          }
     }
}

void AQDevelWidget::sendTracking(){
     if ( uas != NULL) {
          uas->sendCommmandToAq(9, 1, TrackingMoveX,TrackingMoveY,focal_lenght ,camera_yaw_offset,camera_pitch_offset,pixel_size,0.0f);
     }
     else {
          float yaw, pitch, l1, l2, h, sinp = 0.1f, cotp = 0.1f;
          float Ndev, Edev;
          float PIXEL_SIZE = 7e-6f;
          float FOCAL_LENGTH = 0.02f;
          QString Info;
          h = (float)0.78;//-UKF_POSD; //Check sign
          Info.append(QString::number(h) + "\t");
          yaw = (float)0 + 0; //AQ_YAW + CAMERA_YAW_OFFSET;
          Info.append(QString::number(yaw) + "\t");
          pitch = (float)0 + (float)1.5707963267949;//0.7854; //AQ_PITCH + CAMERA_PITCH_OFFSET;
          Info.append(QString::number(pitch) + "\t");
          //sinp = std::max(sin(pitch), 0.001f);//safety
          Info.append(QString::number(sinp) + "\t");
          //cotp = std::min(1/tan(pitch), 100.0f);//safety
          Info.append(QString::number(cotp) + "\t");
          l1 = h/sinp;
          Info.append(QString::number(l1) + "\t");
          l2 = h*cotp;
          Info.append(QString::number(l2) + "\t");

          Ndev = TrackingMoveX*PIXEL_SIZE*l1/FOCAL_LENGTH;
          Info.append(QString::number(Ndev) + "\t");
          Info.append(QString::number(TrackingMoveX) + "\t");
          Edev = TrackingMoveY*PIXEL_SIZE*l1/FOCAL_LENGTH;
          Info.append(QString::number(TrackingMoveY) + "\t");
          Info.append(QString::number(Edev) + "\t");

          res1 = l2*cos(yaw) + Ndev;
          Info.append(QString::number(res1) + "\t");
          res2 = l2*sin(yaw) + Edev;
          Info.append(QString::number(res2) + "\t");
          qDebug() << Info;
     }
}

void AQDevelWidget::setUas(UASInterface *value)
{
    if (uas)
        disconnect(uas, 0, this, 0);
    uas = value;
    if (uas)
        connect(uas, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(globalPositionChangedAq(UASInterface*,double,double,double,quint64)) );
}

void AQDevelWidget::prtstderrTR() {
    qDebug() << ps_tracking.readAllStandardError();
}
