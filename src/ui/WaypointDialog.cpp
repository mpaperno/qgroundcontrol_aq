#include "WaypointDialog.h"
#include "ui_WaypointDialog.h"

#include <QSettings>

WaypointDialog::WaypointDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaypointDialog),
    m_settingsPrefix("WPT_")
{
    ui->setupUi(this);
}

WaypointDialog::~WaypointDialog()
{
    if (ui)
        saveDefaults();
    delete ui;
}

void WaypointDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    loadDefaults();
}

void WaypointDialog::loadDefaults()
{
    QSettings settings;
    settings.beginGroup("QGC_WAYPOINT_DIALOG");
    ui->fld_alt->setValue(settings.value(settingsPrefix() % "ALT", ui->fld_alt->value()).toDouble());
    ui->fld_hdg->setValue(settings.value(settingsPrefix() % "HDG", ui->fld_hdg->value()).toDouble());
    ui->fld_hvel->setValue(settings.value(settingsPrefix() % "HVEL", ui->fld_hvel->value()).toDouble());
    ui->fld_vvel->setValue(settings.value(settingsPrefix() % "VVEL", ui->fld_vvel->value()).toDouble());
    ui->opt_alt_rel->setChecked(settings.value(settingsPrefix() % "ALT_REL", ui->opt_alt_rel->isChecked()).toBool());
    ui->opt_saveDefaults->setChecked(settings.value(settingsPrefix() % "SAVE_DFLT", ui->opt_saveDefaults->isChecked()).toBool());
    ui->sel_lat->setChecked(settings.value(settingsPrefix() % "SEL_LAT", ui->sel_lat->isChecked()).toBool());
    ui->sel_lon->setChecked(settings.value(settingsPrefix() % "SEL_LON", ui->sel_lon->isChecked()).toBool());
    ui->sel_alt->setChecked(settings.value(settingsPrefix() % "SEL_ALT", ui->sel_alt->isChecked()).toBool());
    ui->sel_hdg->setChecked(settings.value(settingsPrefix() % "SEL_HDG", ui->sel_hdg->isChecked()).toBool());
    ui->sel_hvel->setChecked(settings.value(settingsPrefix() % "SEL_HVEL", ui->sel_hvel->isChecked()).toBool());
    ui->sel_vvel->setChecked(settings.value(settingsPrefix() % "SEL_VVEL", ui->sel_vvel->isChecked()).toBool());

}

void WaypointDialog::saveDefaults()
{
    QSettings settings;
    settings.beginGroup("QGC_WAYPOINT_DIALOG");
    settings.setValue(settingsPrefix() % "SAVE_DFLT", ui->opt_saveDefaults->isChecked());
    if (ui->opt_saveDefaults->isChecked()) {
        settings.setValue(settingsPrefix() % "ALT", ui->fld_alt->value());
        settings.setValue(settingsPrefix() % "HDG", ui->fld_hdg->value());
        settings.setValue(settingsPrefix() % "HVEL", ui->fld_hvel->value());
        settings.setValue(settingsPrefix() % "VVEL", ui->fld_vvel->value());
        settings.setValue(settingsPrefix() % "ALT_REL", ui->opt_alt_rel->isChecked());
        settings.setValue(settingsPrefix() % "SEL_LAT", ui->sel_lat->isChecked());
        settings.setValue(settingsPrefix() % "SEL_LON", ui->sel_lon->isChecked());
        settings.setValue(settingsPrefix() % "SEL_ALT", ui->sel_alt->isChecked());
        settings.setValue(settingsPrefix() % "SEL_HDG", ui->sel_hdg->isChecked());
        settings.setValue(settingsPrefix() % "SEL_HVEL", ui->sel_hvel->isChecked());
        settings.setValue(settingsPrefix() % "SEL_VVEL", ui->sel_vvel->isChecked());
    }
    settings.endGroup();
    settings.sync();
}

void WaypointDialog::setLabel(const QString txt)
{
    ui->label->setText(txt);
}

void WaypointDialog::setLatLon(const QPointF ll)
{
    ui->fld_lat->setValue(ll.x());
    ui->fld_lon->setValue(ll.y());
}

void WaypointDialog::toggleLatLon(bool on)
{
    ui->fld_lat->setVisible(on);
    ui->sel_lat->setVisible(on);
    ui->fld_lon->setVisible(on);
    ui->sel_lon->setVisible(on);
}

void WaypointDialog::toggleAlt(bool on)
{
    ui->fld_alt->setVisible(on);
    ui->sel_alt->setVisible(on);
    ui->opt_alt_rel->setVisible(on);
}

void WaypointDialog::toggleHdg(bool on)
{
    ui->fld_hdg->setVisible(on);
    ui->sel_hdg->setVisible(on);
}

void WaypointDialog::toggleHVel(bool on)
{
    ui->fld_hvel->setVisible(on);
    ui->sel_hvel->setVisible(on);
}

void WaypointDialog::toggleVVel(bool on)
{
    ui->fld_vvel->setVisible(on);
    ui->sel_vvel->setVisible(on);
}

void WaypointDialog::toggleHRadius(bool on)
{
    ui->fld_hradius->setVisible(on);
}

QPointF WaypointDialog::getLatLon()
{
    return QPointF(ui->fld_lat->value(), ui->fld_lon->value());
}

QPair<double, bool> WaypointDialog::getAlt()
{
    return QPair<double, bool>(ui->fld_alt->value(), ui->opt_alt_rel->isChecked());
}

double WaypointDialog::getHdg()
{
    return ui->fld_hdg->value();
}

double WaypointDialog::getHVel()
{
    return ui->fld_hvel->value();
}

double WaypointDialog::getVVel()
{
    return ui->fld_vvel->value();
}

double WaypointDialog::getHRadius()
{
    return ui->fld_hradius->value();
}

QStringList WaypointDialog::getSelFields()
{
    QStringList ret;
    foreach (QCheckBox *sel, ui->groupBox->findChildren<QCheckBox *>(QRegExp("sel_.+"))) {
        if (sel->isChecked())
            ret << sel->objectName().replace("sel_", "");
    }
    return ret;
}



void WaypointDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString WaypointDialog::settingsPrefix() const
{
    return m_settingsPrefix;
}

void WaypointDialog::setSettingsPrefix(const QString &settingsPrefix)
{
    m_settingsPrefix = settingsPrefix;
}

