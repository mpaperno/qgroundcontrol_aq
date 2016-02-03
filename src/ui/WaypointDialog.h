#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H

#include <QDialog>

namespace Ui {
class WaypointDialog;
}

class WaypointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaypointDialog(QWidget *parent = 0);
    ~WaypointDialog();

    void loadDefaults();
    void saveDefaults();

    void setLabel(const QString txt);
    void setLatLon(const QPointF ll);

    void toggleLatLon(bool on = false);
    void toggleAlt(bool on = false);
    void toggleHdg(bool on = false);
    void toggleHVel(bool on = false);
    void toggleVVel(bool on = false);
    void toggleHRadius(bool on = false);

    QPointF getLatLon();
    QPair<double, bool> getAlt();
    double getHdg();
    double getHVel();
    double getVVel();
    double getHRadius();
    QStringList getSelFields();

    QString settingsPrefix() const;
    void setSettingsPrefix(const QString &settingsPrefix);

protected:
    void showEvent(QShowEvent* event);
    void changeEvent(QEvent *e);

private:
    Ui::WaypointDialog *ui;
    QString m_settingsPrefix;
};

#endif // WAYPOINTDIALOG_H
