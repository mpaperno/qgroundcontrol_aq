#include "QGCDataViewWidget.h"

QGCDataViewWidget::QGCDataViewWidget(QWidget *parent) :
    QWidget(parent)
{
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    tabWidget->setEnabled(true);

    logViewer = new AQLogViewer(this);
    tabWidget->addTab(logViewer, tr("AutoQuad Log Viewer"));

    telemetryView = new AQTelemetryView(this);
    tabWidget->addTab(telemetryView, tr("AQ Diagnostic Telemetry"));

    linechartWidget = new Linecharts(this);
    tabWidget->addTab(linechartWidget, tr("MAVLink Data Plot"));

    layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);

    this->show();
}

QGCDataViewWidget::~QGCDataViewWidget(){
    tabWidget->deleteLater();
}

void QGCDataViewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit visibilityChanged(true);
}

void QGCDataViewWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    emit visibilityChanged(false);
}

void QGCDataViewWidget::addSource(MAVLinkDecoder *decoder)
{
    linechartWidget->addSource(decoder);
}
