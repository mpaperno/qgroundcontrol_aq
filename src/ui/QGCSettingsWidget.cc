#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>

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
        QIcon ico(QString("%1flags/%2.png").arg(langPath).arg(lang));
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

    // Toolbar
    ui->showPerspectiveBtns->setChecked(MainWindow::instance()->getShowPerspectiveChangeButtons());
    connect(ui->showPerspectiveBtns, SIGNAL(toggled(bool)), MainWindow::instance(), SLOT(setShowPerspectiveChangeButtons(bool)));

    // Style
    ui->comboBox_style->addItems(MainWindow::instance()->getAvailableStyles());
    ui->comboBox_style->setCurrentIndex(ui->comboBox_style->findText(MainWindow::instance()->getStyleName()));
    ui->lineEdit_customStylePath->setText(MainWindow::instance()->getCustomStyleFile());
    ui->checkBox_loadDefaultStyle->setChecked(MainWindow::instance()->getLoadDefaultStyles());
    customStyle();
    connect(ui->comboBox_style, SIGNAL(currentIndexChanged(QString)), this, SLOT(onStyleChange(QString)));
    connect(ui->toolButton_selectCustomStyle, SIGNAL(clicked()), this, SLOT(selectCustomStyle()));
    connect(ui->checkBox_loadDefaultStyle, SIGNAL(toggled(bool)), MainWindow::instance(), SLOT(setLoadDefaultStyles(bool)));
#ifndef STYLES_DEFAULT_FILE
    ui->checkBox_loadDefaultStyle->hide();
#endif

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

bool QGCSettingsWidget::customStyle() {
    bool isCustom = ui->comboBox_style->currentText() == "Custom";
    ui->label_style_custom->setVisible(isCustom);
    ui->lineEdit_customStylePath->setVisible(isCustom);
    ui->toolButton_selectCustomStyle->setVisible(isCustom);
    ui->checkBox_loadDefaultStyle->setVisible(isCustom);

    return isCustom;
}

void QGCSettingsWidget::onStyleChange(QString style)
{
    if (!customStyle() || MainWindow::instance()->getCustomStyleFile().length())
        loadStyle(style);
}

void QGCSettingsWidget::loadStyle(QString style) {

    ui->label_message->setText(tr("Please wait..."));
    ui->label_message->repaint();
    MainWindow::instance()->loadStyleByName(style);
    if (MainWindow::instance()->getStyleIdByName(style) == MainWindow::QGC_MAINWINDOW_STYLE_NATIVE)
        ui->label_message->setText(tr("You may need to restart QGroundControl to switch to a fully native look and feel."));
    else
        ui->label_message->setText("");
}

void QGCSettingsWidget::selectCustomStyle()
{
    ui->label_message->setText(tr("Please wait..."));
    if (MainWindow::instance()->selectStylesheet())
        ui->lineEdit_customStylePath->setText(MainWindow::instance()->getCustomStyleFile());
    ui->label_message->setText("");
}
