#ifndef SELECTADJUSTABLEPARAMDIALOG_H
#define SELECTADJUSTABLEPARAMDIALOG_H

#include <stdint.h>
#include <QDialog>
#include <QMap>
#include <QString>

class QGCUASParamManager;
class QTreeWidgetItem;
class QAbstractButton;

namespace Ui {
class SelectAdjustableParamDialog;
}

class SelectAdjustableParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectAdjustableParamDialog(QWidget *parent = 0, QGCUASParamManager *paramManager = 0);
    ~SelectAdjustableParamDialog();

    void setParamManager(QGCUASParamManager *paramManager);

    void setAqBuildNum(int aqBuildNum);

    uint16_t selParamId() const;
    void setSelParamId(const uint16_t &selParamId);

    QString paramName() const;
    void setParamName(const QString &paramName);

public slots:
    int selectParam(const QString &pName, const uint16_t selParamId, const int aqBuild = 0);

protected:
//    void showEvent(QShowEvent *event);
    void changeEvent(QEvent *e);

protected slots:
    void requestParamsList();
    void onParamsLoaded(uint8_t component);
    void drawTree();
    void deselectAll() ;
    void onTreeItemActivated(QTreeWidgetItem *item, int col);
    void onTreeItemDblClick(QTreeWidgetItem *item, int col);

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::SelectAdjustableParamDialog *ui;
    QGCUASParamManager *m_paramManager;
    QMap <QString, int> *m_paramsMap;
    QMap<QString, QTreeWidgetItem *> m_paramGroups;
    QMap<uint16_t, QTreeWidgetItem *> m_paramWidgetsMap;
    QString m_paramName;
    int m_aqBuildNum;
    int m_paramsLoadedForBuildNum;
    uint16_t m_selParamId;
    bool m_paramsLoading;

signals:
    void selParamIdChanged(uint16_t id);
};

#endif // SELECTADJUSTABLEPARAMDIALOG_H
