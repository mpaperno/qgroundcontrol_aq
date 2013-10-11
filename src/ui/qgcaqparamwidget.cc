/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2010 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/
/**
 * @file
 *   @brief Implementation of class QGCParamWidget
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 */
#include <cmath>
#include <float.h>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QList>
#include <QTime>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>

#include "qgcaqparamwidget.h"
#include "UASInterface.h"
#include "MainWindow.h"
#include <QDebug>
#include "QGC.h"

/**
 * @param uas MAV to set the parameters on
 * @param parent Parent widget
 */
QGCAQParamWidget::QGCAQParamWidget(UASInterface* uas_ext, QWidget *parent) :
    QGCUASParamManager(uas_ext, parent),
    components(new QMap<int, QTreeWidgetItem*>())
{
    OverrideCheckValue = 0;
    uas = uas_ext;
    // Load settings
    loadSettings();

    loadParameterInfoCSV(uas_ext->getAutopilotTypeName(), uas_ext->getSystemTypeName());

    // Create tree widget
    tree = new QTreeWidget(this);
    statusLabel = new QLabel();
    statusLabel->setAutoFillBackground(true);
    tree->header()->resizeSection(0, 175);
    tree->setItemDelegateForColumn(0, new NoEditDelegate(this));

    // Set tree widget as widget onto this component
    QGridLayout* horizontalLayout;
    //form->setAutoFillBackground(false);
    horizontalLayout = new QGridLayout(this);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setMargin(0);
    horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // Parameter tree
    horizontalLayout->addWidget(tree, 0, 0, 1, 4);

    // Status line
    statusLabel->setText(tr("Click refresh to download parameters"));
    horizontalLayout->addWidget(statusLabel, 1, 0, 1, 4);


    // BUTTONS
    QPushButton* refreshButton = new QPushButton(tr("Refresh"));
    refreshButton->setToolTip(tr("Load parameters currently in non-permanent memory of aircraft."));
    refreshButton->setWhatsThis(tr("Load parameters currently in non-permanent memory of aircraft."));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(requestParameterList()));
    horizontalLayout->addWidget(refreshButton, 2, 0);

    QPushButton* setButton = new QPushButton(tr("Transmit"));
    setButton->setToolTip(tr("Set current parameters in non-permanent onboard memory"));
    setButton->setWhatsThis(tr("Set current parameters in non-permanent onboard memory"));
    connect(setButton, SIGNAL(clicked()), this, SLOT(setParameters()));
    horizontalLayout->addWidget(setButton, 2, 1, 1, 2);

    QPushButton* writeButton = new QPushButton(tr("Write (ROM)"));
    writeButton->setToolTip(tr("Copy current parameters in non-permanent memory of the aircraft to permanent memory. Transmit your parameters first to write these."));
    writeButton->setWhatsThis(tr("Copy current parameters in non-permanent memory of the aircraft to permanent memory. Transmit your parameters first to write these."));
    connect(writeButton, SIGNAL(clicked()), this, SLOT(writeParameters()));
    horizontalLayout->addWidget(writeButton, 2, 3);


    QPushButton* loadFileButton = new QPushButton(tr("Load File"));
    loadFileButton->setToolTip(tr("Load parameters from a file on this computer in the view. To write them to the aircraft, use transmit after loading them."));
    loadFileButton->setWhatsThis(tr("Load parameters from a file on this computer in the view. To write them to the aircraft, use transmit after loading them."));
    connect(loadFileButton, SIGNAL(clicked()), this, SLOT(loadParameters()));
    horizontalLayout->addWidget(loadFileButton, 3, 0);

    QPushButton* saveFileButton = new QPushButton(tr("Save File"));
    saveFileButton->setToolTip(tr("Save parameters in this view to a file on this computer."));
    saveFileButton->setWhatsThis(tr("Save parameters in this view to a file on this computer."));
    connect(saveFileButton, SIGNAL(clicked()), this, SLOT(saveParameters()));
    horizontalLayout->addWidget(saveFileButton, 3, 1, 1, 2);
    QAction* action;
    action =  saveFileMenu.addAction(tr("AQ params.txt format (can also load via QGC v1.3+)"), this, SLOT(saveParamFile()));
    action->setData(1);
    action =  saveFileMenu.addAction(tr("QGC format (for loading with older QGC versions)"), this, SLOT(saveParamFile()));
    action->setData(0);
    saveFileButton->setMenu(&saveFileMenu);

    QPushButton* readButton = new QPushButton(tr("Read (ROM)"));
    readButton->setToolTip(tr("Copy parameters from permanent memory to non-permanent current memory of aircraft. DOES NOT update the parameters in this view, click refresh after copying them to get them."));
    readButton->setWhatsThis(tr("Copy parameters from permanent memory to non-permanent current memory of aircraft. DOES NOT update the parameters in this view, click refresh after copying them to get them."));
    connect(readButton, SIGNAL(clicked()), this, SLOT(readParameters()));
    horizontalLayout->addWidget(readButton, 3, 3);

    QPushButton* loadParaFromSDButton = new QPushButton(tr("Load from SD"));
    loadParaFromSDButton->setToolTip(tr("Load parameters from a file on the SD card. These parameters will be in the aircraft non-permanent memory."));
    loadParaFromSDButton->setWhatsThis(tr("Load parameters from a file on the SD card. These parameters will be in the aircraft non-permanent memory."));
    connect(loadParaFromSDButton, SIGNAL(clicked()), this, SLOT(loadParaFromSD()));
    horizontalLayout->addWidget(loadParaFromSDButton, 4, 0, 1, 2);

    QPushButton* saveParaToSDButton = new QPushButton(tr("Save to SD"));
    saveParaToSDButton->setToolTip(tr("Save parameters in this view to a file on the on-board SD card."));
    saveParaToSDButton->setWhatsThis(tr("Save parameters in this view to a file on the on-board SD card."));
    connect(saveParaToSDButton, SIGNAL(clicked()), this, SLOT(saveParaToSD()));
    horizontalLayout->addWidget(saveParaToSDButton, 4, 2, 1, 2);

//    QPushButton* wpFromSDButton = new QPushButton(tr("WP from SD"));
//    wpFromSDButton->setToolTip(tr("Load mission plan from on-board SD card."));
//    wpFromSDButton->setWhatsThis(tr("Load mission plan from on-board SD card."));
//    connect(wpFromSDButton, SIGNAL(clicked()), this, SLOT(wpFromSD()));
//    horizontalLayout->addWidget(wpFromSDButton, 5, 0);

//    QPushButton* wpToSDButton = new QPushButton(tr("WP to SD"));
//    wpToSDButton->setToolTip(tr("Save on-board mission plan to a file on the on-board SD card."));
//    wpToSDButton->setWhatsThis(tr("Save on-board mission plan to a file on the on-board SD card."));
//    connect(wpToSDButton, SIGNAL(clicked()), this, SLOT(wpToSD()));
//    horizontalLayout->addWidget(wpToSDButton, 5, 1);


    horizontalLayout->setColumnStretch(0, 2);
    horizontalLayout->setColumnStretch(1, 1);
    horizontalLayout->setColumnStretch(2, 1);
    horizontalLayout->setColumnStretch(3, 2);

    // Set layout
    this->setLayout(horizontalLayout);

    // Set header
    QStringList headerItems;
    headerItems.append("Parameter");
    headerItems.append("Value");
    tree->setHeaderLabels(headerItems);
    tree->setColumnCount(2);
    tree->setExpandsOnDoubleClick(true);

    // Connect signals/slots
    connect(this, SIGNAL(parameterChanged(int,QString,QVariant)), mav, SLOT(setParameter(int,QString,QVariant)));
    connect(tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(parameterItemChanged(QTreeWidgetItem*,int)));

    // New parameters from UAS
    connect(uas_ext, SIGNAL(parameterChanged(int,int,int,int,QString,QVariant)), this, SLOT(addParameter(int,int,int,int,QString,QVariant)));

    // Connect retransmission guard
    connect(this, SIGNAL(requestParameter(int,QString)), uas_ext, SLOT(requestParameter(int,QString)));
    connect(this, SIGNAL(requestParameter(int,int)), uas_ext, SLOT(requestParameter(int,int)));
    connect(&retransmissionTimer, SIGNAL(timeout()), this, SLOT(retransmissionGuardTick()));

    // Get parameters
    //if (uas_ext) requestParameterList();
}

void QGCAQParamWidget::loadSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_MAVLINK_PROTOCOL");
    bool ok;
    int temp = settings.value("PARAMETER_RETRANSMISSION_TIMEOUT", retransmissionTimeout).toInt(&ok);
    if (ok) retransmissionTimeout = temp;
    temp = settings.value("PARAMETER_REWRITE_TIMEOUT", rewriteTimeout).toInt(&ok);
    if (ok) rewriteTimeout = temp;
    settings.endGroup();
}

void QGCAQParamWidget::loadParameterInfoCSV(const QString& autopilot, const QString& airframe)
{
    Q_UNUSED(airframe);
    QDir appDir = QApplication::applicationDirPath();
    appDir.cd("files");
    QString fileName = QString("%1/%2/parameter_tooltips/tooltips.txt").arg(appDir.canonicalPath()).arg(autopilot.toLower());
    QFile paramMetaFile(fileName);

    qDebug() << "AUTOPILOT:" << autopilot;
    qDebug() << "FILENAME: " << fileName;

    // Load CSV data
    if (!paramMetaFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug() << "COULD NOT OPEN PARAM META INFO FILE:" << fileName;
        return;
    }

    // Extract header

    // Read in values
    // Find all keys
    QTextStream in(&paramMetaFile);

    // First line is header
    // there might be more lines, but the first
    // line is assumed to be at least header
    QString header = in.readLine();

    // Ignore top-level comment lines
    while (header.startsWith('#') || header.startsWith('/')
           || header.startsWith('=') || header.startsWith('^'))
    {
        header = in.readLine();
    }

    bool charRead = false;
    QString separator = "";
    QList<QChar> sepCandidates;
    sepCandidates << '\t';
    sepCandidates << ',';
    sepCandidates << ';';
    //sepCandidates << ' ';
    sepCandidates << '~';
    sepCandidates << '|';

    // Iterate until separator is found
    // or full header is parsed
    for (int i = 0; i < header.length(); i++)
    {
        if (sepCandidates.contains(header.at(i)))
        {
            // Separator found
            if (charRead)
            {
                separator += header[i];
            }
        }
        else
        {
            // Char found
            charRead = true;
            // If the separator is not empty, this char
            // has been read after a separator, so detection
            // is now complete
            if (separator != "") break;
        }
    }

    bool stripFirstSeparator = false;
    bool stripLastSeparator = false;

    // Figure out if the lines start with the separator (e.g. wiki syntax)
    if (header.startsWith(separator)) stripFirstSeparator = true;

    // Figure out if the lines end with the separator (e.g. wiki syntax)
    if (header.endsWith(separator)) stripLastSeparator = true;

    QString out = separator;
    out.replace("\t", "<tab>");
    //qDebug() << " Separator: \"" << out << "\"";
    //qDebug() << "READING CSV:" << header;


    // Read data
    while (!in.atEnd())
    {
        QString line = in.readLine();

        //qDebug() << "LINE PRE-STRIP" << line;

        // Strip separtors if necessary
        if (stripFirstSeparator) line.remove(0, separator.length());
        if (stripLastSeparator) line.remove(line.length()-separator.length(), line.length()-1);

        //qDebug() << "LINE POST-STRIP" << line;

        // Keep empty parts here - we still have to act on them
        QStringList parts = line.split(separator, QString::KeepEmptyParts);

        // Each line is:
        // variable name, Min, Max, Default, Multiplier, Enabled (0 = no, 1 = yes), Comment


        // Fill in min, max and default values
        if (parts.count() > 1)
        {
            // min
            paramMin.insert(parts.at(0).trimmed(), parts.at(1).toDouble());
        }
        if (parts.count() > 2)
        {
            // max
            paramMax.insert(parts.at(0).trimmed(), parts.at(2).toDouble());
        }
        if (parts.count() > 3)
        {
            // default
            paramDefault.insert(parts.at(0).trimmed(), parts.at(3).toDouble());
        }
        // IGNORING 4 and 5 for now
        if (parts.count() > 6)
        {
            // tooltip
            paramToolTips.insert(parts.at(0).trimmed(), parts.at(6).trimmed());
            qDebug() << "PARAM META:" << parts.at(0).trimmed();
        }
    }
}

/**
 * @return The MAV of this widget. Unless the MAV object has been destroyed, this
 *         pointer is never zero.
 */
UASInterface* QGCAQParamWidget::getUAS()
{
    return mav;
}

/**
 *
 * @param uas System which has the component
 * @param component id of the component
 * @param componentName human friendly name of the component
 */
void QGCAQParamWidget::addComponent(int uas, int component, QString componentName)
{
    Q_UNUSED(uas);
    if (components->contains(component)) {
        // Update existing
        components->value(component)->setData(0, Qt::DisplayRole, QString("%1 (#%2)").arg(componentName).arg(component));
        //components->value(component)->setData(1, Qt::DisplayRole, QString::number(component));
        components->value(component)->setFirstColumnSpanned(true);
    } else {
        // Add new
        QStringList list(QString("%1 (#%2)").arg(componentName).arg(component));
        QTreeWidgetItem* comp = new QTreeWidgetItem(list);
        comp->setFirstColumnSpanned(true);
        components->insert(component, comp);
        // Create grouping and update maps
        paramGroups.insert(component, new QMap<QString, QTreeWidgetItem*>());
        tree->addTopLevelItem(comp);
        tree->update();
        // Create map in parameters
        if (!parameters.contains(component)) {
            parameters.insert(component, new QMap<QString, QVariant>());
        }
        // Create map in changed parameters
        if (!changedValues.contains(component)) {
            changedValues.insert(component, new QMap<QString, QVariant>());
        }
    }
}

/**
 * @param uas System which has the component
 * @param component id of the component
 * @param parameterName human friendly name of the parameter
 */
void QGCAQParamWidget::addParameter(int uas, int component, int paramCount, int paramId, QString parameterName, QVariant value)
{
    addParameter(uas, component, parameterName, value);

    // Missing packets list has to be instantiated for all components
    if (!transmissionMissingPackets.contains(component)) {
        transmissionMissingPackets.insert(component, new QList<int>());
    }

    // List mode is different from single parameter transfers
    if (transmissionListMode) {
        // Only accept the list size once on the first packet from
        // each component
        if (!transmissionListSizeKnown.contains(component))
        {
            // Mark list size as known
            transmissionListSizeKnown.insert(component, true);

            // Mark all parameters as missing
            for (int i = 0; i < paramCount; ++i)
            {
                if (!transmissionMissingPackets.value(component)->contains(i))
                {
                    transmissionMissingPackets.value(component)->append(i);
                }
            }

            // There is only one transmission timeout for all components
            // since components do not manage their transmission,
            // the longest timeout is safe for all components.
            quint64 thisTransmissionTimeout = QGC::groundTimeMilliseconds() + ((paramCount)*retransmissionTimeout);
            if (thisTransmissionTimeout > transmissionTimeout)
            {
                transmissionTimeout = thisTransmissionTimeout;
            }
        }

        // Start retransmission guard
        // or reset timer
        setRetransmissionGuardEnabled(true);
    }

    // Mark this parameter as received in read list
    int index = transmissionMissingPackets.value(component)->indexOf(paramId);
    // If the MAV sent the parameter without request, it wont be in missing list
    if (index != -1) transmissionMissingPackets.value(component)->removeAt(index);

    if (changedValues.contains(component) && changedValues.value(component)->contains(parameterName))
        changedValues.value(component)->remove(parameterName);

    bool justWritten = false;
    bool writeMismatch = false;
    //bool lastWritten = false;
    // Mark this parameter as received in write ACK list
    QMap<QString, QVariant>* map = transmissionMissingWriteAckPackets.value(component);
    if (map && map->contains(parameterName))
    {
        justWritten = true;
        if (map->value(parameterName) != value)
        {
                writeMismatch = true;
        }
        map->remove(parameterName);
    }

    int missCount = 0;
    foreach (int key, transmissionMissingPackets.keys())
    {
        missCount +=  transmissionMissingPackets.value(key)->count();
    }

    int missWriteCount = 0;
    foreach (int key, transmissionMissingWriteAckPackets.keys())
    {
        missWriteCount += transmissionMissingWriteAckPackets.value(key)->count();
    }

    if (justWritten && !writeMismatch && missWriteCount == 0)
    {
        // Just wrote one and count went to 0 - this was the last missing write parameter
        statusLabel->setText(tr("SUCCESS: WROTE ALL PARAMETERS"));
        QPalette pal = statusLabel->palette();
        pal.setColor(backgroundRole(), QGC::colorGreen);
        statusLabel->setPalette(pal);
    } else if (justWritten && !writeMismatch)
    {
        statusLabel->setText(tr("SUCCESS: Wrote %2 (#%1/%4): %3").arg(paramId+1).arg(parameterName).arg(value.toDouble()).arg(paramCount));
        QPalette pal = statusLabel->palette();
        pal.setColor(backgroundRole(), QGC::colorGreen);
        statusLabel->setPalette(pal);
    } else if (justWritten && writeMismatch)
    {
        // Mismatch, tell user
        QPalette pal = statusLabel->palette();
        pal.setColor(backgroundRole(), QGC::colorRed);
        statusLabel->setPalette(pal);
        statusLabel->setText(tr("FAILURE: Wrote %1: sent %2 != onboard %3").arg(parameterName).arg(map->value(parameterName).toDouble()).arg(value.toDouble()));
    }
    else
    {
        if (missCount > 0)
        {
            QPalette pal = statusLabel->palette();
            pal.setColor(backgroundRole(), QGC::colorOrange);
            statusLabel->setPalette(pal);
        }
        else
        {
            QPalette pal = statusLabel->palette();
            pal.setColor(backgroundRole(), QGC::colorGreen);
            statusLabel->setPalette(pal);
        }
        QString val = QString("%1").arg(value.toFloat(), 5, 'f', 1, QChar(' '));
        //statusLabel->setText(tr("OK: %1 %2 #%3/%4, %5 miss").arg(parameterName).arg(val).arg(paramId+1).arg(paramCount).arg(missCount));
        if (missCount == 0)
        {
            // Transmission done
            QTime time = QTime::currentTime();
            QString timeString = time.toString();
            statusLabel->setText(tr("All received. (updated at %1)").arg(timeString));
        }
        else
        {
            // Transmission in progress
            statusLabel->setText(tr("OK: %1 %2 (%3/%4)").arg(parameterName).arg(val).arg(paramCount-missCount).arg(paramCount));
        }
    }

    // Check if last parameter was received
    if (missCount == 0 && missWriteCount == 0)
    {
        this->transmissionActive = false;
        this->transmissionListMode = false;
        transmissionListSizeKnown.clear();
        foreach (int key, transmissionMissingPackets.keys())
        {
            transmissionMissingPackets.value(key)->clear();
        }
        emit requestParameterRefreshed();

        // Expand visual tree
        tree->expandItem(tree->topLevelItem(0));
    }
}

/**
 * @param uas System which has the component
 * @param component id of the component
 * @param parameterName human friendly name of the parameter
 */
void QGCAQParamWidget::addParameter(int uas, int component, QString parameterName, QVariant value)
{
    //qDebug() << "PARAM WIDGET GOT PARAM AQ:" << parameterName << value;
    Q_UNUSED(uas);
    // Reference to item in tree
    QTreeWidgetItem* parameterItem = NULL;
    QTreeWidgetItem* parentItem = NULL;

    // Get component
    if (!components->contains(component))
    {
//        QString componentName;
//        switch (component)
//        {
//        case MAV_COMP_ID_CAMERA:
//            componentName = tr("Camera (#%1)").arg(component);
//            break;
//        case MAV_COMP_ID_IMU:
//            componentName = tr("IMU (#%1)").arg(component);
//            break;
//        default:
//            componentName = tr("Component #").arg(component);
//            break;
//        }
        QString componentName = tr("Component #%1").arg(component);
        addComponent(uas, component, componentName);
    }

    // Replace value in map

    // FIXME
    if (parameters.value(component)->contains(parameterName))
        parameters.value(component)->remove(parameterName);

    parameters.value(component)->insert(parameterName, value);


    QString splitToken = "_";
    // Check if auto-grouping can work
    if (parameterName.contains(splitToken))
    {
        QString parent = parameterName.section(splitToken, 0, 0, QString::SectionSkipEmpty);
        QMap<QString, QTreeWidgetItem*>* compParamGroups = paramGroups.value(component);
        if (!compParamGroups->contains(parent))
        {
            // Insert group item
            QStringList glist;
            glist.append(parent);
            QTreeWidgetItem* item = new QTreeWidgetItem(glist);
            item->setFirstColumnSpanned(true);
            compParamGroups->insert(parent, item);
            components->value(component)->addChild(item);
        }

        // Append child to group
        bool found = false;
        parentItem = compParamGroups->value(parent);
        for (int i = 0; i < parentItem->childCount(); i++) {
            QTreeWidgetItem* child = parentItem->child(i);
            QString key = child->data(0, Qt::DisplayRole).toString();
            if (key == parameterName)
            {
                //qDebug() << "UPDATED CHILD";
                parameterItem = child;
                parameterItem->setData(1, Qt::DisplayRole, value);
                found = true;
            }
        }

        if (!found)
        {
            // Insert parameter into map
            QStringList plist;
            plist.append(parameterName);
            // CREATE PARAMETER ITEM
            parameterItem = new QTreeWidgetItem(plist);
            // CONFIGURE PARAMETER ITEM
            parameterItem->setData(1, Qt::DisplayRole, value);
//          if ( !parent.contains("IMU"))
            parameterItem->setFlags(parameterItem->flags() | Qt::ItemIsEditable);

            compParamGroups->value(parent)->addChild(parameterItem);
        }
    }
    else
    {
        bool found = false;
        parentItem = components->value(component);
        for (int i = 0; i < parentItem->childCount(); i++)
        {
            QTreeWidgetItem* child = parentItem->child(i);
            QString key = child->data(0, Qt::DisplayRole).toString();
            if (key == parameterName)
            {
                //qDebug() << "UPDATED CHILD";
                parameterItem = child;
                parameterItem->setData(1, Qt::DisplayRole, value);
                found = true;
            }
        }

        if (!found)
        {
            // Insert parameter into map
            QStringList plist;
            plist.append(parameterName);
            // CREATE PARAMETER ITEM
            parameterItem = new QTreeWidgetItem(plist);
            // CONFIGURE PARAMETER ITEM
            parameterItem->setData(1, Qt::DisplayRole, value);
//          if ( !parameterName.contains("IMU_"))
            parameterItem->setFlags(parameterItem->flags() | Qt::ItemIsEditable);

            components->value(component)->addChild(parameterItem);

        }
        //tree->expandAll();
    }
    // Reset background color
    if (OverrideCheckValue && changedValues.contains(component) && changedValues.value(component)->contains(parameterName)) {
        parameterItem->setBackground(0, QBrush(QColor(QGC::colorOrange)));
        parameterItem->setBackground(1, QBrush(QColor(QGC::colorOrange)));
        tree->expandItem(parentItem);
    }
    else {
        parameterItem->setBackground(0, Qt::NoBrush);
        parameterItem->setBackground(1, Qt::NoBrush);
    }

    // Add tooltip
    QString tooltipFormat;
    if (paramDefault.contains(parameterName))
    {
        tooltipFormat = tr("Default: %1, %2");
        tooltipFormat = tooltipFormat.arg(paramDefault.value(parameterName, 0.0f)).arg(paramToolTips.value(parameterName, ""));
    }
    else
    {
        tooltipFormat = paramToolTips.value(parameterName, "");
    }
    parameterItem->setToolTip(0, tooltipFormat);
    parameterItem->setToolTip(1, tooltipFormat);

    //tree->update();
}


/**
 * Send a request to deliver the list of onboard parameters
 * to the MAV.
 */
void QGCAQParamWidget::requestParameterList()
{
    if (!mav) return;
    // FIXME This call does not belong here
    // Once the comm handling is moved to a new
    // Param manager class the settings can be directly
    // loaded from MAVLink protocol
    loadSettings();
    // End of FIXME

    emit parameterListRequested();

    // Clear view and request param list
    clear();
    parameters.clear();
    received.clear();
    // Clear transmission state
    transmissionListMode = true;
    transmissionListSizeKnown.clear();
    foreach (int key, transmissionMissingPackets.keys())
    {
        transmissionMissingPackets.value(key)->clear();
    }
    transmissionActive = true;

    // Set status text
    statusLabel->setText(tr("Requested param list.. waiting"));

    mav->requestParameters();
}

void QGCAQParamWidget::parameterItemChanged(QTreeWidgetItem* current, int column)
{
    if (current && column > 0) {
        QTreeWidgetItem* parent = current->parent();
        while (parent->parent() != NULL) {
            parent = parent->parent();
        }
        // Parent is now top-level component
        int key = components->key(parent);
        if (!changedValues.contains(key)) {
            changedValues.insert(key, new QMap<QString, QVariant>());
        }
        QMap<QString, QVariant>* map = changedValues.value(key, NULL);
        if (map) {
            QString str = current->data(0, Qt::DisplayRole).toString();
            QVariant value = current->data(1, Qt::DisplayRole);
            // Set parameter on changed list to be transmitted to MAV
            statusLabel->setText(tr("Changed Param %1:%2: %3").arg(key).arg(str).arg(value.toDouble()));
            //qDebug() << "PARAM CHANGED: COMP:" << key << "KEY:" << str << "VALUE:" << value;
            // Changed values list
            if (map->contains(str))
                map->remove(str);
            map->insert(str, value);

            // Check if the value was numerically changed
            if (!parameters.value(key)->contains(str) || fabs(static_cast<float>(parameters.value(key)->value(str).toDouble()) - value.toDouble()) > 2.0f * FLT_EPSILON) {
                current->setBackground(0, QBrush(QColor(QGC::colorOrange)));
                current->setBackground(1, QBrush(QColor(QGC::colorOrange)));
                // qDebug() << "marking changed from: " << parameters.value(key)->value(str) << "to:" << value;
            }

            switch (parameters.value(key)->value(str).type())
            {
            case QVariant::Int:
                {
                    QVariant fixedValue(value.toInt());
                    parameters.value(key)->insert(str, fixedValue);
                }
                break;
            case QVariant::UInt:
                {
                    QVariant fixedValue(value.toUInt());
                    parameters.value(key)->insert(str, fixedValue);
                }
                break;
            case QMetaType::Float:
                {
                    QVariant fixedValue(value.toFloat());
                    parameters.value(key)->insert(str, fixedValue);
                }
                break;
            case QMetaType::Double:
                {
                    QVariant fixedValue(value.toDouble());
                    parameters.value(key)->insert(str, fixedValue);
                }
                break;
            default:
                qCritical() << "ABORTED PARAM UPDATE, NO VALID QVARIANT TYPE";
                return;
            }
        }
    }
}

void QGCAQParamWidget::saveParamFile()
{
    QObject* sender = QObject::sender();
    QAction* action = qobject_cast<QAction*>(sender);

    if (action)
    {
        bool ok;
        int fileFormat = action->data().toInt(&ok);
        if (ok)
            saveParameters(fileFormat);
    }
}

void QGCAQParamWidget::saveParameters(int fileFormat)
{
    if (!mav) return;
    QString suggestPath = (fileFormat == 0) ? "./parameters.txt" : "./PARAMS.txt";
    if (fileNameFromMaster.length()) {
        QFileInfo fi(fileNameFromMaster);
        suggestPath = fi.absolutePath() + "/" + suggestPath;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), suggestPath, tr("Parameter File (*.txt)"));
    if (!fileName.length())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage("Error!", tr("Could not open params file. %1").arg(file.errorString()));
        return;
    }

    fileNameFromMaster = fileName;
    QTextStream in(&file);

    in << "# Onboard parameters for system " << mav->getUASName() << "\n";
    if (fileFormat == 0) {
        in << "#\n";
        in << "# MAV ID  COMPONENT ID  PARAM NAME  VALUE (FLOAT)\n";
    }
    in << "\n";

    // Iterate through all components, through all parameters and emit them
    QMap<int, QMap<QString, QVariant>*>::iterator i;
    for (i = parameters.begin(); i != parameters.end(); ++i) {
        // Iterate through the parameters of the component
        int compid = i.key();
        QMap<QString, QVariant>* comp = i.value();
        {
            QMap<QString, QVariant>::iterator j;
            for (j = comp->begin(); j != comp->end(); ++j)
            {
                QString paramValue("%1");
                QString paramType("%1");
                switch (j.value().type())
                {
                case QVariant::Int:
                    paramValue = paramValue.arg(j.value().toInt());
                    paramType = paramType.arg(MAV_PARAM_TYPE_INT32);
                    break;
                case QVariant::UInt:
                    paramValue = paramValue.arg(j.value().toUInt());
                    paramType = paramType.arg(MAV_PARAM_TYPE_UINT32);
                    break;
                case QMetaType::Float:
                    paramValue = paramValue.arg(j.value().toDouble(), 25, 'g', 12);
                    paramType = paramType.arg(MAV_PARAM_TYPE_REAL32);
                    break;
                case QMetaType::Double:
                    paramValue = paramValue.arg(j.value().toDouble(), 25, 'g', 12);
                    paramType = paramType.arg(MAV_PARAM_TYPE_REAL64);
                    break;
                default:
                    qCritical() << "ABORTED PARAM WRITE TO FILE, NO VALID QVARIANT TYPE" << j.value();
                    return;
                }

                switch (fileFormat) {
                case 0 :  // QGC format
                    in << mav->getUASID() << "\t" << compid << "\t" << j.key() << "\t" << paramValue << "\t" << paramType << "\n";
                    break;
                case 1 : // AQ param.txt format
                    in << j.key() << "\t" << paramValue << "\n";
                    break;
                default:
                    qCritical() << "ABORTED PARAM WRITE TO FILE, NO VALID FILE FORMAT" << fileFormat;
                    return;
                }

                in.flush();
            }
        }
    }
    file.close();
}

void QGCAQParamWidget::loadParameters()
{
    if (!mav) return;

    int fileFormat, component, uasId;
    float paramValueFloat;
    bool changed, ok;
    QString paramName, paramValue;
    QStringList wpParams;

    QString dirPath = QDir::toNativeSeparators(fileNameFromMaster);
    QFileInfo dir(dirPath);

    // use native file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Saved Parameters File"), dir.absoluteFilePath(),
                                            tr("Parameter Files (*.params *.txt)"));

    // use Qt file dialog (sometimes very slow! at least on Win)
//    QFileDialog dialog;
//    dialog.setDirectory(dir.absoluteDir());
//    dialog.setFileMode(QFileDialog::AnyFile);
//    dialog.setFilter(tr("Parameter Files (*.params *.txt)"));
//    dialog.setViewMode(QFileDialog::Detail);
//    QStringList fileNames;
//    if (dialog.exec())
//    {
//        fileNames = dialog.selectedFiles();
//    }

//    if (fileNames.size() > 0)
//    {
//        fileName = QDir::toNativeSeparators(fileNames.at(0));
//        fileNameFromMaster = fileName;
//    }
    if (!fileName.length())
        return;

    fileNameFromMaster = fileName;
    OverrideCheckValue = 1;

    QFile file(fileName);

    if (!file.size() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MainWindow::instance()->showCriticalMessage("Error", "Could not open saved parameters file.");
        return;
    }

    // bool userWarned = false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if ( line.contains("DEFAULT_"))
            line.replace("DEFAULT_", "");
        
        wpParams = line.split(QRegExp("[\\t ]"), QString::SkipEmptyParts);

        changed = false;
        uasId = mav->getUASID();
        component = 190;
        
        if (line.contains(QRegExp("^#define[\\t ]+[A-Z]{2,}"))){
            fileFormat = 2; // AQ .params format
            paramName = wpParams.at(1);
            paramValue = wpParams.at(2);
        }
        else if (line.contains(QRegExp("^[\\d]+\\t")) && wpParams.size() == 5){
            fileFormat = 0; // QGC format
            paramName = wpParams.at(2);
            paramValue = wpParams.at(3);
            // uasId = wpParams.at(0).toInt();
            component = wpParams.at(1).toInt();
        }
        else if (line.contains(QRegExp("^[A-Z][A-Z\\d_]{2,}[\\t ]")) && wpParams.size() == 2){
            fileFormat = 1; // AQ params.txt format
            paramName = wpParams.at(0);
            paramValue = wpParams.at(1);
        }
        else {
            qDebug() << "Could not parse line: " << line;
            continue;
        }

//        qDebug() << line << fileFormat << paramName << paramValue;

        if ( paramName.contains("IMU_ACC_ALN_"))
            paramName = paramName.replace("IMU_ACC_ALN_", "IMU_ACC_ALGN_");
        if ( paramName.contains("L1_ATT_PWM_SCALE"))
            paramName = paramName.replace("L1_ATT_PWM_SCALE", "L1_ATT_PWM_SCAL");
        if ( paramName.contains("NAV_ATL_SPED_") && paramExistsAQ("NAV_ALT_SPED_P"))
            paramName = paramName.replace(QRegExp("NAV_ATL_SPED_(.+)"), "NAV_ALT_SPED_\\1");

        if (!paramExistsAQ(paramName)) {
            //qDebug() << "Param does not exist: " << paramName;
            continue;
        }

        paramValueFloat = paramValue.replace("f", "", Qt::CaseInsensitive).toFloat(&ok);
        if (!ok || isnan(paramValueFloat)) {
            //qDebug() << "Could not convert value: " << paramValue;
            continue;
        }

        // Only load parameters for right mav
//        if (fileFormat == 0 && !userWarned && (mav->getUASID() != uasId)) {
//            MainWindow::instance()->showCriticalMessage(tr("Parameter loading warning"),
//                                                        tr("The parameters from the file %1 have been saved from system %2, but the currently selected system has the ID %3. If this is unintentional, please click on <READ> to revert to the parameters that are currently onboard").arg(fileName).arg(uasId).arg(mav->getUASID()));
//            userWarned = true;
//        }

        // check for change of value
        if (fabs(static_cast<float>(parameters.value(component)->value(paramName).toDouble()) - paramValue.toDouble()) > 2.0f * FLT_EPSILON) {
            changed = true;
            // qDebug() << "Changed" << paramName << "From:" << parameters.value(component)->value(paramName) << "To:" << paramValue;
        }

        // Create changed values data structure if necessary
        if (changed) {
            if (!changedValues.contains(component))
                changedValues.insert(component, new QMap<QString, QVariant>());

            // Remove from changed values if already present
            if (changedValues.value(component)->contains(paramName))
                changedValues.value(component)->remove(paramName);
        }

        if (fileFormat == 0) {  // QGC file format specifies value type
            switch (wpParams.at(4).toUInt())
            {
            case (int)MAV_PARAM_TYPE_REAL32:
                addParameter(uasId, component, paramName, paramValueFloat);
                if (changed) {
                    changedValues.value(component)->insert(paramName, paramValueFloat);
                    // setParameter(wpParams.at(1).toInt(), paramName, paramValueFloat);
                }
                break;
            case (int)MAV_PARAM_TYPE_UINT32:
                addParameter(uasId, component, paramName, paramValue.toUInt());
                if (changed) {
                    changedValues.value(component)->insert(paramName, paramValue.toUInt());
                    // setParameter(wpParams.at(1).toInt(), paramName, QVariant(paramValue.toUInt()));
                }
                break;
            case (int)MAV_PARAM_TYPE_INT32:
                addParameter(uasId, component, paramName, paramValue.toInt());
                if (changed) {
                    changedValues.value(component)->insert(paramName, paramValue.toInt());
                    // setParameter(wpParams.at(1).toInt(), paramName, QVariant(paramValue.toInt()));
                }
                break;
            default:
                qDebug() << "FAILED LOADING PARAM" << paramName << "NO KNOWN DATA TYPE";
            }
        }
        // AQ params.txt and .params file formats
        else {

            addParameter(uasId, component, paramName, paramValueFloat);
            if (changed)
                changedValues.value(component)->insert(paramName, paramValueFloat);

        }

    }
    file.close();
    OverrideCheckValue = 0;
}


/**
 * Enabling the retransmission guard enables the parameter widget to track
 * dropped parameters and to re-request them. This works for both individual
 * parameter reads as well for whole list requests.
 *
 * @param enabled True if retransmission checking should be enabled, false else
 */
void QGCAQParamWidget::setRetransmissionGuardEnabled(bool enabled)
{
    if (enabled) {
        retransmissionTimer.start(retransmissionTimeout);
    } else {
        retransmissionTimer.stop();
    }
}

void QGCAQParamWidget::retransmissionGuardTick()
{
    if (transmissionActive) {
        //qDebug() << __FILE__ << __LINE__ << "RETRANSMISSION GUARD ACTIVE, CHECKING FOR DROPS..";

        // Check for timeout
        // stop retransmission attempts on timeout
        if (QGC::groundTimeMilliseconds() > transmissionTimeout) {
            setRetransmissionGuardEnabled(false);
            transmissionActive = false;

            // Empty read retransmission list
            // Empty write retransmission list
            int missingReadCount = 0;
            QList<int> readKeys = transmissionMissingPackets.keys();
            foreach (int component, readKeys) {
                missingReadCount += transmissionMissingPackets.value(component)->count();
                transmissionMissingPackets.value(component)->clear();
            }

            // Empty write retransmission list
            int missingWriteCount = 0;
            QList<int> writeKeys = transmissionMissingWriteAckPackets.keys();
            foreach (int component, writeKeys) {
                missingWriteCount += transmissionMissingWriteAckPackets.value(component)->count();
                transmissionMissingWriteAckPackets.value(component)->clear();
            }
            statusLabel->setText(tr("TIMEOUT! MISSING: %1 read, %2 write.").arg(missingReadCount).arg(missingWriteCount));
            emit paramRequestTimeout(missingReadCount, missingWriteCount);
        }

        // Re-request at maximum retransmissionBurstRequestSize parameters at once
        // to prevent link flooding
        QMap<int, QMap<QString, QVariant>*>::iterator i;
        for (i = parameters.begin(); i != parameters.end(); ++i) {
            // Iterate through the parameters of the component
            int component = i.key();
            // Request n parameters from this component (at maximum)
            QList<int> * paramList = transmissionMissingPackets.value(component, NULL);
            if (paramList) {
                int count = 0;
                foreach (int id, *paramList) {
                    if (count < retransmissionBurstRequestSize) {
                        //qDebug() << __FILE__ << __LINE__ << "RETRANSMISSION GUARD REQUESTS RETRANSMISSION OF PARAM #" << id << "FROM COMPONENT #" << component;
                        emit requestParameter(component, id);
                        statusLabel->setText(tr("Requested retransmission of #%1").arg(id+1));
                        count++;
                    } else {
                        break;
                    }
                }
            }
        }

        // Re-request at maximum retransmissionBurstRequestSize parameters at once
        // to prevent write-request link flooding
        // Empty write retransmission list
        QList<int> writeKeys = transmissionMissingWriteAckPackets.keys();
        foreach (int component, writeKeys) {
            int count = 0;
            QMap <QString, QVariant>* missingParams = transmissionMissingWriteAckPackets.value(component);
            foreach (QString key, missingParams->keys()) {
                if (count < retransmissionBurstRequestSize) {
                    // Re-request write operation
                    QVariant value = missingParams->value(key);
                    switch (parameters.value(component)->value(key).type())
                    {
                    case QVariant::Int:
                        {
                            QVariant fixedValue(value.toInt());
                            emit parameterChanged(component, key, fixedValue);
                        }
                        break;
                    case QVariant::UInt:
                        {
                            QVariant fixedValue(value.toUInt());
                            emit parameterChanged(component, key, fixedValue);
                        }
                        break;
                    case QMetaType::Float:
                        {
                            QVariant fixedValue(value.toFloat());
                            emit parameterChanged(component, key, fixedValue);
                        }
                        break;
                    case QMetaType::Double:
                        {
                            QVariant fixedValue(value.toDouble());
                            emit parameterChanged(component, key, fixedValue);
                        }
                        break;
                    default:
                        //qCritical() << "ABORTED PARAM RETRANSMISSION, NO VALID QVARIANT TYPE";
                        return;
                    }
                    statusLabel->setText(tr("Requested rewrite of: %1: %2").arg(key).arg(missingParams->value(key).toDouble()));
                    count++;
                } else {
                    break;
                }
            }
        }
    } else {
        //qDebug() << __FILE__ << __LINE__ << "STOPPING RETRANSMISSION GUARD GRACEFULLY";
        setRetransmissionGuardEnabled(false);
    }
}


/**
 * The .. signal is emitted
 */
void QGCAQParamWidget::requestParameterUpdate(int component, const QString& parameter)
{
    if (mav) mav->requestParameter(component, parameter);
}


/**
 * @param component the subsystem which has the parameter
 * @param parameterName name of the parameter, as delivered by the system
 * @param value value of the parameter
 */
void QGCAQParamWidget::setParameter(int component, QString parameterName, QVariant value)
{
    if (paramMin.contains(parameterName) && value.toDouble() < paramMin.value(parameterName))
    {
        statusLabel->setText(tr("REJ. %1 < min").arg(value.toDouble()));
        return;
    }
    if (paramMax.contains(parameterName) && value.toDouble() > paramMax.value(parameterName))
    {
        statusLabel->setText(tr("REJ. %1 > max").arg(value.toDouble()));
        return;
    }

    switch (parameters.value(component)->value(parameterName).type())
    {
        case QVariant::Int:
            {
                QVariant fixedValue(value.toInt());
                emit parameterChanged(component, parameterName, fixedValue);
                //qDebug() << "PARAM WIDGET SENT AQ:" << parameterName << fixedValue;
            }
            break;
        case QVariant::UInt:
            {
                QVariant fixedValue(value.toUInt());
                emit parameterChanged(component, parameterName, fixedValue);
                //qDebug() << "PARAM WIDGET SENT AQ:" << parameterName <<  fixedValue;
            }
            break;
        case QMetaType::Float:
            {
                QVariant fixedValue(value.toFloat());
                emit parameterChanged(component, parameterName, fixedValue);
                //qDebug() << "PARAM WIDGET SENT AQ:" << parameterName << fixedValue;
            }
            break;
        case QMetaType::Double:
            {
                QVariant fixedValue(value.toDouble());
                emit parameterChanged(component, parameterName, fixedValue);
                qDebug() << "PARAM WIDGET SENT AQ:" << parameterName << fixedValue;
            }
            break;
            default:
            qCritical() << "ABORTED PARAM SEND, NO VALID QVARIANT TYPE";
            return;
    }

    // Wait for parameter to be written back
    // mark it therefore as missing
    if (!transmissionMissingWriteAckPackets.contains(component))
    {
        transmissionMissingWriteAckPackets.insert(component, new QMap<QString, QVariant>());
    }

    // Insert it in missing write ACK list
    transmissionMissingWriteAckPackets.value(component)->insert(parameterName, value);

    // Set timeouts
    if (transmissionActive)
    {
        transmissionTimeout += rewriteTimeout;
    }
    else
    {
        quint64 newTransmissionTimeout = QGC::groundTimeMilliseconds() + rewriteTimeout;
        if (newTransmissionTimeout > transmissionTimeout)
        {
            transmissionTimeout = newTransmissionTimeout;
        }
        transmissionActive = true;
    }

    // Enable guard / reset timeouts
    setRetransmissionGuardEnabled(true);
}

/**
 * Set all parameter in the parameter tree on the MAV
 */
void QGCAQParamWidget::setParameters()
{
    // Iterate through all components, through all parameters and emit them
    int parametersSent = 0;
    QMap<int, QMap<QString, QVariant>*>::iterator i;
    for (i = changedValues.begin(); i != changedValues.end(); ++i) {
        // Iterate through the parameters of the component
        int compid = i.key();
        QMap<QString, QVariant>* comp = i.value();
        {
            QMap<QString, QVariant>::iterator j;
            for (j = comp->begin(); j != comp->end(); ++j) {
                setParameter(compid, j.key(), j.value());
                parametersSent++;
            }
        }
    }
    //qDebug() << "Send Parameters out" << QString::number(parametersSent);

    // Change transmission status if necessary
    if (parametersSent == 0) {
        statusLabel->setText(tr("No transmission: No changed values."));
    } else {
        statusLabel->setText(tr("Transmitting %1 parameters.").arg(parametersSent));
        // Set timeouts
        if (transmissionActive)
        {
            transmissionTimeout += parametersSent*rewriteTimeout;
        }
        else
        {
        transmissionActive = true;
        quint64 newTransmissionTimeout = QGC::groundTimeMilliseconds() + parametersSent*rewriteTimeout;
        if (newTransmissionTimeout > transmissionTimeout) {
            transmissionTimeout = newTransmissionTimeout;
        }
        }
        // Enable guard
        setRetransmissionGuardEnabled(true);
    }
}


/**
 * Write the current onboard parameters from RAM into
 * permanent storage, e.g. EEPROM or harddisk
 */
void QGCAQParamWidget::writeParameters()
{
    int changedParamCount = 0;

    QMap<int, QMap<QString, QVariant>*>::iterator i;
    for (i = changedValues.begin(); i != changedValues.end(); ++i)
    {
        // Iterate through the parameters of the component
        QMap<QString, QVariant>* comp = i.value();
        {
            QMap<QString, QVariant>::iterator j;
            for (j = comp->begin(); j != comp->end(); ++j)
            {
                changedParamCount++;
            }
        }
    }

    if (changedParamCount > 0)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("There are locally changed parameters. Please transmit them first (<TRANSMIT>) or update them with the onboard values (<REFRESH>) before storing onboard from RAM to ROM."));
        msgBox.exec();
    }
    else
    {
        if (!mav) return;
        mav->writeParametersToStorageAQ();
    }
}




void QGCAQParamWidget::readParameters()
{
    if (!mav) return;
    mav->readParametersFromStorage();
}

/**
 * Clear all data in the parameter widget
 */
void QGCAQParamWidget::clear()
{
    tree->clear();
    components->clear();
}

QVariant QGCAQParamWidget::getParaAQ(QString parameterName)
{
    if ( !parameters.contains(190) )
        return "";
	if (parameters.value(190)->contains(parameterName)) {
		QVariant var = parameters.value(190)->value(parameterName);
		return var;
	}
    return "";
}

void QGCAQParamWidget::setParaAQ(QString parameterName, QVariant value)
{
    setParameter(190, parameterName, value);
}

void QGCAQParamWidget::loadParaAQ()
{
	requestParameterList();
}


void QGCAQParamWidget::loadParaFromSD()
{
    if (uas != NULL)
    {
        uas->readParametersFromSDAQ();
    }
}

void QGCAQParamWidget::saveParaToSD()
{
    if (uas != NULL)
    {
        uas->writeParametersToSDAQ();
    }
}

void QGCAQParamWidget::wpFromSD()
{
    if (uas != NULL)
    {
        uas->readWaypointsFromSDAQ();
    }
}

void QGCAQParamWidget::wpToSD()
{
    if (uas != NULL)
    {
        uas->writeWaypointsToSDAQ();
    }
}

void QGCAQParamWidget::setFilePath(QString fileName)
{
    fileNameFromMaster = fileName;
}
