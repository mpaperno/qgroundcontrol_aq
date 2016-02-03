#include "SelectAdjustableParamDialog.h"
#include "ui_SelectAdjustableParamDialog.h"
#include "autoquadMAV.h"
#include "QGCUASParamManager.h"

#include <QAbstractButton>
#include <QDebug>

SelectAdjustableParamDialog::SelectAdjustableParamDialog(QWidget *parent, QGCUASParamManager *paramManager) :
    QDialog(parent),
    ui(new Ui::SelectAdjustableParamDialog),
    m_paramManager(NULL),
    m_paramsMap(NULL),
    m_aqBuildNum(0),
    m_paramsLoadedForBuildNum(0),
    m_selParamId(0),
    m_paramsLoading(false)
{
    ui->setupUi(this);
    setParamManager(paramManager);
    m_paramsMap = new QMap<QString, int>();

    connect(ui->treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(onTreeItemActivated(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onTreeItemActivated(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onTreeItemDblClick(QTreeWidgetItem*,int)));
}

SelectAdjustableParamDialog::~SelectAdjustableParamDialog()
{
    delete ui;
}

//void SelectAdjustableParamDialog::showEvent(QShowEvent *event)
//{
//}

void SelectAdjustableParamDialog::changeEvent(QEvent *e)
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

int SelectAdjustableParamDialog::selectParam(const QString &pName, const uint16_t selParamId, const int aqBuild)
{
    if (aqBuild)
        setAqBuildNum(aqBuild);
    setParamName(pName);
    setSelParamId(selParamId);
    return this->exec();
}

void SelectAdjustableParamDialog::requestParamsList()
{
    if (m_paramManager) {
        m_paramsLoading = true;
        ui->treeWidget->clear();
        ui->waitLabel->setText(tr("Paremeters loading, please wait..."));
        ui->waitLabel->show();
        m_paramManager->requestParameterList(MAV_ADJUSTABLE_PARAMS_LIST_COMPONENT);
    }
}

void SelectAdjustableParamDialog::onParamsLoaded(uint8_t component)
{
    if (component == MAV_ADJUSTABLE_PARAMS_LIST_COMPONENT) {
        m_paramsLoading = false;
        m_paramsMap = m_paramManager->getParameterIdMap(MAV_ADJUSTABLE_PARAMS_LIST_COMPONENT);
        if (!m_paramsMap->isEmpty()) {
            m_paramsLoadedForBuildNum = m_aqBuildNum;
            ui->waitLabel->hide();
            drawTree();
        } else
            ui->waitLabel->setText(tr("Sorry, there was an error while loading the list of adjustable parameters."));
    }
}

void SelectAdjustableParamDialog::drawTree()
{
    if (!m_paramsMap)
        return;

    QTreeWidgetItem *paramItem = NULL;
    QTreeWidgetItem *parentItem = NULL;
    QString parent;

    m_paramGroups.clear();
    m_paramWidgetsMap.clear();
    ui->treeWidget->blockSignals(true);

    QMapIterator<QString, int> i(*m_paramsMap);
    while (i.hasNext()) {
        i.next();
        parent = i.key().section("_", 0, 0, QString::SectionSkipEmpty);
        if (!m_paramGroups.contains(parent)) {
            parentItem = new QTreeWidgetItem(QStringList(parent));
            parentItem->setFlags(Qt::ItemIsEnabled);
            m_paramGroups.insert(parent, parentItem);
            ui->treeWidget->addTopLevelItem(parentItem);
        }
        else
            parentItem = m_paramGroups.value(parent);

        paramItem = new QTreeWidgetItem(parentItem, QStringList(i.key()));
        paramItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable /*| Qt::ItemIsUserCheckable*/);
        paramItem->setData(0, Qt::UserRole, i.value());
        if (m_selParamId == i.value()) {
            paramItem->setCheckState(0, Qt::Checked);
            paramItem->setSelected(true);
            parentItem->setExpanded(true);
            ui->treeWidget->scrollToItem(paramItem);
        }
        else
            paramItem->setCheckState(0, Qt::Unchecked);
        m_paramWidgetsMap.insert(i.value(), paramItem);
        parentItem->addChild(paramItem);
    }
    ui->treeWidget->blockSignals(false);
}

void SelectAdjustableParamDialog::deselectAll()
{
    QMapIterator<uint16_t, QTreeWidgetItem *> i(m_paramWidgetsMap);
    while (i.hasNext()) {
        i.next();
        i.value()->setCheckState(0, Qt::Unchecked);
    }
}

void SelectAdjustableParamDialog::onTreeItemActivated(QTreeWidgetItem *item, int col)
{
    if (!item->isSelected())
        return;

    bool ok;
    uint16_t t = item->data(0, Qt::UserRole).toUInt(&ok);
    if (ok && m_selParamId != t) {
        deselectAll();
        emit selParamIdChanged(t);
        item->setCheckState(col, Qt::Checked);
        m_selParamId = t;
    }
}

void SelectAdjustableParamDialog::onTreeItemDblClick(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col);
    if (item->isSelected())
        this->accept();
}

QString SelectAdjustableParamDialog::paramName() const
{
    return m_paramName;
}

void SelectAdjustableParamDialog::setParamName(const QString &paramName)
{
    m_paramName = paramName;
}

uint16_t SelectAdjustableParamDialog::selParamId() const
{
    return m_selParamId;
}

void SelectAdjustableParamDialog::setSelParamId(const uint16_t &selParamId)
{
    if (m_selParamId != selParamId)
        emit selParamIdChanged(selParamId);
    m_selParamId = selParamId;

    if (m_paramsLoading)
        return;

    deselectAll();
    ui->treeWidget->clearSelection();
    ui->treeWidget->collapseAll();
    if (m_selParamId && m_paramWidgetsMap.contains(m_selParamId)) {
        QTreeWidgetItem *wi = m_paramWidgetsMap.value(m_selParamId);
        ui->treeWidget->setCurrentItem(wi);
        wi->parent()->setExpanded(true);
        wi->setSelected(true);
        wi->setCheckState(0, Qt::Checked);
        ui->treeWidget->scrollToItem(wi);
    }
}

void SelectAdjustableParamDialog::setAqBuildNum(int aqBuildNum)
{
    if (aqBuildNum != m_aqBuildNum && aqBuildNum != m_paramsLoadedForBuildNum)
        requestParamsList();

    m_aqBuildNum = aqBuildNum;
}

void SelectAdjustableParamDialog::setParamManager(QGCUASParamManager *paramManager)
{
    if (m_paramManager)
        disconnect(m_paramManager, 0, this, 0);

    m_paramManager = paramManager;
    connect(m_paramManager, SIGNAL(requestParameterRefreshed(uint8_t)), this, SLOT(onParamsLoaded(uint8_t)));
}

void SelectAdjustableParamDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Reset) {
        setSelParamId(0);
        this->accept();
    }
}
