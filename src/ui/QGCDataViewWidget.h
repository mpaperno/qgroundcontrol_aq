#ifndef QGCDATAVIEWWIDGET_H
#define QGCDATAVIEWWIDGET_H

#include "aq_LogViewer.h"
#include "aq_telemetryView.h"
#include "Linecharts.h"
#include "MAVLinkDecoder.h"

#include <QWidget>
#include <QTabWidget>
#include <QHBoxLayout>

class QGCDataViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QGCDataViewWidget(QWidget *parent = 0);
    ~QGCDataViewWidget();
    void addSource(MAVLinkDecoder *decoder);

signals:
    void visibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);
    void changeEvent(QEvent* event);
    void retranslateUi();

private:
    QTabWidget* tabWidget;
    AQLogViewer* logViewer;
    AQTelemetryView* telemetryView;
    Linecharts* linechartWidget;
    QHBoxLayout* layout;
};

#endif // QGCDATAVIEWWIDGET_H
