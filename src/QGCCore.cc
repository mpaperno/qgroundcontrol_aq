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
 *   @brief Implementation of class QGCCore
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QFile>
#include <QFlags>
#include <QThread>
#include <QSplashScreen>
#include <QPixmap>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleFactory>
#include <QAction>
#include <QTranslator>
#include <QDir>
#include <QFileInfo>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSettings>
#include <QString>
#include <QFontDatabase>
#include <QPluginLoader>
#include <QStringBuilder>

#include <QDebug>
#include <cstdio>

#include "configuration.h"
#include "QGC.h"
#include "QGCCore.h"
#include "MainWindow.h"
#include "GAudioOutput.h"
#include "UDPLink.h"
#include "MAVLinkSimulationLink.h"
#ifdef OPAL_RT
#include "OpalLink.h"
#endif


#ifdef _MSC_VER
#include "Windows.h"
#endif

QTranslator* QGCCore::current = 0;
Translators QGCCore::translators;
QString QGCCore::langPath = "/files/lang";

/**
 * @brief Constructor for the main application.
 *
 * This constructor initializes and starts the whole application. It takes standard
 * command-line parameters
 *
 * @param argc The number of command-line parameters
 * @param argv The string array of parameters
 **/


QGCCore::QGCCore(int &argc, char* argv[]) :
    QApplication(argc, argv),
    m_exit(false)
{
    // Set application name
    this->setApplicationName(QGCAUTOQUAD::APP_NAME);
    this->setApplicationVersion(QGCAUTOQUAD::APP_VERSION_TXT);
    this->setOrganizationName(QGCAUTOQUAD::APP_ORG);
    this->setOrganizationDomain(QGCAUTOQUAD::APP_DOMAIN);

    // Set up CLI
    QCommandLineParser parser;
    parser.setApplicationDescription("Ground control station for AutoQuad flight controller and other MAVLink compatible robots.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption opt_clearSettings(
                "fresh",
                QCoreApplication::translate("main", "Back up then delete all program settings and start fresh (also see -swap). NOTE that only ONE BACKUP VERSION IS KEPT!"));
    parser.addOption(opt_clearSettings);

    QCommandLineOption opt_swapSettings(
                "swap",
                QCoreApplication::translate("main", "Back up current settings, then restore and use previosly backed-up settings. If no backup exists, acts same as -fresh."));
    parser.addOption(opt_swapSettings);

    QCommandLineOption opt_nobakSettings(
                "nobak",
                QCoreApplication::translate("main", "When used with --fresh or --swap, do NOT back up current settings first (this preserves the previous backup, if any, but you will LOOSE ALL CURRENT SETTINGS!)."));
    parser.addOption(opt_nobakSettings);

#ifdef _MSC_VER
    QCommandLineOption opt_keepConsole(
                "console",
                QCoreApplication::translate("main", "Keep the command-line console open/attached after startup (useful for debug, etc).  Redirect output to a file to create a log, like this: `qgroundcontrol_aq --console >>qgclog.txt` "));
    parser.addOption(opt_keepConsole);
#endif

    parser.process(QCoreApplication::arguments());

    // Set settings format
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;

    // Clear or swap out settings, then exit
    if (parser.isSet(opt_clearSettings) || parser.isSet(opt_swapSettings)) {
        bool hasTempFile = false;
        QFile settingsFile(settings.fileName());
        QString settingsBakFileName = settings.fileName().replace(".ini", ".bak.ini");
        QFile settingsBakFile(settingsBakFileName);

        printf("\n    === QGC CLI Utility Mode ===\n\n");

        // rename current settings file to temp name
        if (settingsFile.exists()) {
            if (parser.isSet(opt_nobakSettings)) {
                printf("Deleting settings file: %s\n", settingsFile.fileName().toStdString().c_str());
                settingsFile.remove();
            } else {
                printf("Backing up settings file: %s\n", settingsFile.fileName().toStdString().c_str());
                hasTempFile = true;
                settingsFile.rename(settingsFile.fileName() % ".tmp");
            }
        } else
            printf("No settings found at: %s\n", settingsFile.fileName().toStdString());


        if (parser.isSet(opt_swapSettings) && settingsBakFile.exists()) {
            printf("Restoring backup settings from: %s\n", settingsBakFile.fileName().toStdString().c_str());
            settingsBakFile.copy(settingsBakFile.fileName().remove(".bak"));
            printf("Your active settings are at: %s\n", settingsBakFile.fileName().remove(".bak").toStdString().c_str());
        }

        if (hasTempFile) {
            if (settingsBakFile.exists()) {
                printf("Removing old backup: %s\n", settingsBakFile.fileName().toStdString().c_str());
                settingsBakFile.remove();
            }
            printf("Renaming temp file to new backup %s\n", settingsBakFileName.toStdString().c_str());
            settingsFile.rename(settingsBakFileName);
        }

        printf("\n    Finished.Press the Enter/Return key to quit, then restart QGC normally.\n");
        std::getchar();
        m_exit = true;  // set a flag for main() so we don't enter exec() loop.
        return;  // don't start up the GUI
    }

    // If we have no AQ settings this means first run of this app.
    // Look for settings in older QGC version location.
    if (!settings.contains("AUTOQUAD_SETTINGS/APP_VERSION")) {
        // check for old QGroundControl for AQ settings
        QSettings qsets(QSettings::IniFormat, QSettings::UserScope, QGC_ORGANIZATION_NAME, QGC_APPLICATION_NAME);
        qDebug() << "Looking for old settings in" << qsets.fileName();
        if (qsets.contains("AUTOQUAD_SETTINGS/APP_VERSION") && !qsets.contains("AUTOQUAD_SETTINGS/SETTINGS_MIGRATED_TO_QGCAQ")) {
            qDebug() << "Copying settings from" << qsets.fileName() << "to" << settings.fileName();
            foreach (QString childKey, qsets.allKeys())
                settings.setValue(childKey, qsets.value(childKey));
            settings.sync();
            qDebug() << "Done converting settings.";
            qsets.setValue("AUTOQUAD_SETTINGS/SETTINGS_MIGRATED_TO_QGCAQ", true);
            qsets.sync();
        }
        else if (qsets.contains("AUTOQUAD_SETTINGS/SETTINGS_MIGRATED_TO_QGCAQ"))
            qDebug() << "Settings already converted, assuming current settings cleared on purpose.";
        else
            qDebug() << "No settings found.";
    }

#if defined(_MSC_VER) && !defined(QT_DEBUG)
    // if we're done with the console on Windows, close/detech from it.
    if (!parser.isSet(opt_keepConsole))
        FreeConsole();
#endif

    loadTranslations();

    // Exit main application when last window is closed
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));

    // Show splash screen
    QPixmap splashImage(":/files/images/splash.png");
    QSplashScreen* splashScreen = new QSplashScreen(splashImage);
    // Delete splash screen after mainWindow was displayed
    splashScreen->setAttribute(Qt::WA_DeleteOnClose);
    splashScreen->show();
    processEvents();
    splashScreen->showMessage(tr("Loading application fonts"), Qt::AlignLeft | Qt::AlignBottom, QColor(62, 93, 141));

    // Load application font
    QFontDatabase fontDatabase = QFontDatabase();
    const QString fontFileName = ":/general/vera.ttf"; ///< Font file is part of the QRC file and compiled into the app
    if (QFile::exists(fontFileName))
        fontDatabase.addApplicationFont(fontFileName);
    else
        qWarning() << "ERROR! font file:" << fontFileName << "DOES NOT EXIST!";

    // Start the comm link manager
    splashScreen->showMessage(tr("Starting Communication Links"), Qt::AlignLeft | Qt::AlignBottom, QColor(62, 93, 141));
    startLinkManager();

    // Start the UAS Manager
    splashScreen->showMessage(tr("Starting UAS Manager"), Qt::AlignLeft | Qt::AlignBottom, QColor(62, 93, 141));
    startUASManager();

    // Start the user interface
    splashScreen->showMessage(tr("Starting User Interface"), Qt::AlignLeft | Qt::AlignBottom, QColor(62, 93, 141));
    // Start UI

    // Connect links
    // to make sure that all components are initialized when the
    // first messages arrive
//    UDPLink* udpLink = new UDPLink(QHostAddress::Any, 14550);
//    MainWindow::instance()->addLink(udpLink);
    // Listen on Multicast-Address 239.255.77.77, Port 14550
    //QHostAddress * multicast_udp = new QHostAddress("239.255.77.77");
    //UDPLink* udpLink = new UDPLink(*multicast_udp, 14550);

#ifdef OPAL_RT
    // Add OpalRT Link, but do not connect
    OpalLink* opalLink = new OpalLink();
    MainWindow::instance()->addLink(opalLink);
#endif

    mainWindow = MainWindow::_create(splashScreen);
    Q_CHECK_PTR(mainWindow);

    // Remove splash screen
    splashScreen->finish(mainWindow);

}

/**
 * @brief Destructor for the groundstation. It destroys all loaded instances.
 *
 **/
QGCCore::~QGCCore()
{
    if (mainWindow) {
        mainWindow->close();
        // Delete singletons
        // First systems
        delete UASManager::instance();
        // then links
        delete LinkManager::instance();
        // Finally the main window
        delete MainWindow::instance();
    }
}

/**
 * @brief Start the link managing component.
 *
 * The link manager keeps track of all communication links and provides the global
 * packet queue. It is the main communication hub
 **/
void QGCCore::startLinkManager()
{
    LinkManager::instance();
}

/**
 * @brief Start the Unmanned Air System Manager
 *
 **/
void QGCCore::startUASManager()
{
    // Load UAS plugins
    QDir pluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("plugins");

    UASManager::instance();

    // Load plugins

    QStringList pluginFileNames;

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            //populateMenus(plugin);
            pluginFileNames += fileName;
            //printf(QString("Loaded plugin from " + fileName + "\n").toStdString().c_str());
        }
    }
}

// translation engine

void QGCCore::loadTranslations()
{
    loadTranslations(getLangFilePath());
}

void QGCCore::loadTranslations(const QDir& dir)
{
    // <language>_<country>.qm or <language.qm>
    QString filter = "*.qm";
    QDir::Filters filters = QDir::Files | QDir::Readable;
    QDir::SortFlags sort = QDir::Name;
    QFileInfoList entries = dir.entryInfoList(QStringList() << filter, filters, sort);
    QString language, country;

    foreach (QFileInfo file, entries)
    {
        // pick country and language out of the file name
        QStringList parts = file.baseName().split("_");
        if (parts.count() > 1) {
            language = parts.at(parts.count() - 2).toLower();
            country  = parts.at(parts.count() - 1).toUpper();
        } else {
            language = parts.at(parts.count() - 1).toLower();
            country = "";
        }

        qDebug() << __FILE__ << __LINE__ << "Loaded language file:" << file.absoluteFilePath() << language << country;

        // construct and load translator
        QTranslator* translator = new QTranslator(instance());
        if (translator->load(file.absoluteFilePath()))
        {
            QString locale = language;
            if (country.length())
                locale = locale % "_" % country;
            translators.insert(locale, translator);
        }
    }
}

const QStringList QGCCore::availableLanguages()
{
    // the content won't get copied thanks to implicit sharing and constness
    return QStringList(translators.keys());
}

void QGCCore::setLanguage(const QString& locale)
{
    // remove previous
    if (current)
        removeTranslator(current);

    // install new
    current = translators.value(locale, 0);
    if (current)
        installTranslator(current);
}

const QString QGCCore::getLangFilePath()
{
    QString ret = QApplication::applicationDirPath();
    ret.append(langPath);
    return ret;
}


