#include "QGCDataViewWidget.h"

QGCDataViewWidget::QGCDataViewWidget(QWidget *parent) :
    QWidget(parent)
{
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName(QString::fromUtf8("dataViewTabWidget"));
    tabWidget->setEnabled(true);

    logViewer = new AQLogViewer(this);
    logViewer->setObjectName(QString::fromUtf8("tab_logViewer"));
    tabWidget->addTab(logViewer, QString());

    telemetryView = new AQTelemetryView(this);
    telemetryView->setObjectName(QString::fromUtf8("tab_telemetryView"));
    tabWidget->addTab(telemetryView, QString());

    linechartWidget = new Linecharts(this);
    linechartWidget->setObjectName(QString::fromUtf8("tab_linechartWidget"));
    tabWidget->addTab(linechartWidget, QString());

    layout = new QHBoxLayout(this);
    layout->addWidget(tabWidget);

    retranslateUi();

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

void QGCDataViewWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QWidget::changeEvent(event);
}

void QGCDataViewWidget::retranslateUi(/*QWidget *QGCDataViewWidget*/)
{
    tabWidget->setTabText(tabWidget->indexOf(logViewer), tr("AutoQuad Log Viewer"));
    tabWidget->setTabText(tabWidget->indexOf(telemetryView), tr("AQ Diagnostic Telemetry"));
    tabWidget->setTabText(tabWidget->indexOf(linechartWidget), tr("MAVLink Data Plot"));
}

void QGCDataViewWidget::addSource(MAVLinkDecoder *decoder)
{
    linechartWidget->addSource(decoder);
}
