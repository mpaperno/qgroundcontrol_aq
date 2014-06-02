#include <QSettings>

#include "QGCCore.h"
#include "QGCSettingsWidget.h"
#include "MainWindow.h"
#include "ui_QGCSettingsWidget.h"

#include "LinkManager.h"
#include "MAVLinkProtocol.h"
#include "MAVLinkSettingsWidget.h"
#include "GAudioOutput.h"

//, Qt::WindowFlags flags

QGCSettingsWidget::QGCSettingsWidget(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::QGCSettingsWidget)
{
    ui->setupUi(this);

    // Add all protocols
    QList<ProtocolInterface*> protocols = LinkManager::instance()->getProtocols();
    foreach (ProtocolInterface* protocol, protocols) {
        MAVLinkProtocol* mavlink = dynamic_cast<MAVLinkProtocol*>(protocol);
        if (mavlink) {
            MAVLinkSettingsWidget* msettings = new MAVLinkSettingsWidget(mavlink, this);
            ui->tabWidget->addTab(msettings, "MAVLink");
        }
    }

    this->window()->setWindowTitle(tr("QGroundControl Settings"));


    QString langPath = QGCCore::getLangFilePath();
    foreach (QString lang, QGCCore::availableLanguages()) {
        QLocale locale(lang);
        QString name = QLocale::languageToString(locale.language());
        QIcon ico(QString("%1/flags/%2.png").arg(langPath).arg(lang));
        ui->comboBox_language->addItem(ico, name, lang);
    }

    // Audio preferences
    ui->audioMuteCheckBox->setChecked(GAudioOutput::instance()->isMuted());
    connect(ui->audioMuteCheckBox, SIGNAL(toggled(bool)), GAudioOutput::instance(), SLOT(mute(bool)));
    connect(GAudioOutput::instance(), SIGNAL(mutedChanged(bool)), ui->audioMuteCheckBox, SLOT(setChecked(bool)));

    // Reconnect
    ui->reconnectCheckBox->setChecked(MainWindow::instance()->autoReconnectEnabled());
    connect(ui->reconnectCheckBox, SIGNAL(clicked(bool)), MainWindow::instance(), SLOT(enableAutoReconnect(bool)));

    // Low power mode
    ui->lowPowerCheckBox->setChecked(MainWindow::instance()->lowPowerModeEnabled());
    connect(ui->lowPowerCheckBox, SIGNAL(clicked(bool)), MainWindow::instance(), SLOT(enableLowPowerMode(bool)));

    // Language
    ui->comboBox_language->setCurrentIndex(ui->comboBox_language->findData(MainWindow::instance()->getCurrentLanguage()));
    connect(ui->comboBox_language, SIGNAL(currentIndexChanged(int)), this, SLOT(loadLanguage(int)));

    // Style
    MainWindow::QGC_MAINWINDOW_STYLE style = (MainWindow::QGC_MAINWINDOW_STYLE)MainWindow::instance()->getStyle();
    switch (style) {
    case MainWindow::QGC_MAINWINDOW_STYLE_NATIVE:
        ui->nativeStyle->setChecked(true);
        break;
    case MainWindow::QGC_MAINWINDOW_STYLE_INDOOR:
        ui->indoorStyle->setChecked(true);
        break;
    case MainWindow::QGC_MAINWINDOW_STYLE_OUTDOOR:
        ui->outdoorStyle->setChecked(true);
        break;
    case MainWindow::QGC_MAINWINDOW_STYLE_PLASTIQUE:
        ui->plastiqueStyle->setChecked(true);
        break;
    }
    connect(ui->nativeStyle, SIGNAL(clicked()), MainWindow::instance(), SLOT(loadNativeStyle()));
    connect(ui->indoorStyle, SIGNAL(clicked()), MainWindow::instance(), SLOT(loadIndoorStyle()));
    connect(ui->outdoorStyle, SIGNAL(clicked()), MainWindow::instance(), SLOT(loadOutdoorStyle()));
    connect(ui->plastiqueStyle, SIGNAL(clicked()), MainWindow::instance(), SLOT(loadPlastiqueStyle()));

    // Close / destroy
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(deleteLater()));

    // Set layout options
    ui->generalPaneGridLayout->setAlignment(Qt::AlignTop);
}

QGCSettingsWidget::~QGCSettingsWidget()
{
    delete ui;
}

void QGCSettingsWidget::loadLanguage(int idx) {
    MainWindow::instance()->loadLanguage(ui->comboBox_language->itemData(idx).toString());
}
