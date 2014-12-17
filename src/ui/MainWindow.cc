/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009 - 2011 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

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
 *   @brief Implementation of class MainWindow
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 */

#include <QSettings>
#include <QDockWidget>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QHostInfo>
#include <QDesktopWidget>
#include <QStyleFactory>
//#include <QGCHilLink.h>
//#include <QGCHilConfiguration.h>
//#include <QGCHilFlightGearConfiguration.h>

#include "QGC.h"
#include "MG.h"
#include "QGCCore.h"
#include "MAVLinkSimulationLink.h"
#include "SerialLink.h"
#include "UDPLink.h"
#include "MAVLinkProtocol.h"
#include "CommConfigurationWindow.h"
#include "QGCWaypointListMulti.h"
#include "MainWindow.h"
//#include "JoystickWidget.h"
#include "GAudioOutput.h"
#include "QGCToolWidget.h"
#include "QGCMAVLinkLogPlayer.h"
#include "QGCSettingsWidget.h"
#include "QGCMapTool.h"
#include "MAVLinkDecoder.h"
#include "QGCMAVLinkMessageSender.h"
#include "QGCRGBDView.h"
#include "QGCFirmwareUpdate.h"
#include "ESCtelemetryWidget.h"

#ifdef QGC_OSG_ENABLED
#include "Q3DWidgetFactory.h"
#endif

// FIXME Move
#include "PxQuadMAV.h"
#include "SlugsMAV.h"


#include "LogCompressor.h"
#include "qgcautoquad.h"

static MainWindow* _instance = NULL;   ///< @brief MainWindow singleton

MainWindow* MainWindow::_create(QSplashScreen* splashScreen)
{
    Q_ASSERT(_instance == NULL);

    new MainWindow(splashScreen);

    // _instance is set in constructor
    Q_ASSERT(_instance);

    return _instance;
}

MainWindow* MainWindow::instance(void)
{
    return _instance;
}

//MainWindow* MainWindow::instance(QSplashScreen* screen)
//{
//    static MainWindow* _instance = 0;
//    if(_instance == 0)
//    {
//        _instance = new MainWindow();
//        if (screen) connect(_instance, SIGNAL(initStatusChanged(QString)), screen, SLOT(showMessage(QString)));

//        /* Set the application as parent to ensure that this object
//                 * will be destroyed when the main application exits */
//        //_instance->setParent(qApp);
//    }
//    return _instance;
//}

/**
* Create new mainwindow. The constructor instantiates all parts of the user
* interface. It does NOT show the mainwindow. To display it, call the show()
* method.
*
* @see QMainWindow::show()
**/
MainWindow::MainWindow(QSplashScreen *splashScreen):
    QMainWindow(),
    currentView(VIEW_OPERATOR),
    currentStyle(QGC_MAINWINDOW_STYLE_NONE),
    aboutToCloseFlag(false),
    changingViewsFlag(false),
    centerStackActionGroup(new QActionGroup(this)),
    autoReconnect(false),
    lowPowerMode(false)
{
    Q_ASSERT(_instance == NULL);
    _instance = this;

    if (splashScreen) {
        connect(_instance, SIGNAL(initStatusChanged(QString)), splashScreen, SLOT(showMessage(QString)));
    }

    hide();

    // format systems language
    defaultLanguage = QLocale::system().name();       // e.g. "de_DE"
    defaultLanguage.truncate(defaultLanguage.lastIndexOf('_')); // e.g. "de"

    emit initStatusChanged("Loading UI Settings..");
    loadSettings();
    if (!settings.contains("QGC_MAINWINDOW/CURRENT_VIEW")) {
        // Set this view as default view
        settings.setValue("QGC_MAINWINDOW/CURRENT_VIEW", currentView);
    } else {
        // LOAD THE LAST VIEW
        VIEW_SECTIONS currentViewCandidate = (VIEW_SECTIONS) settings.value("QGC_MAINWINDOW/CURRENT_VIEW", currentView).toInt();
        if (currentViewCandidate >= VIEW_ENGINEER && currentViewCandidate <= VIEW_DATA)
            currentView = currentViewCandidate;
    }

    settings.sync();

    emit initStatusChanged("Loading Style.");
    setAvailableStyles();
    loadStyle(currentStyle);

    emit initStatusChanged("Setting up user interface.");

    QGCCore::setLanguage(defaultLanguage);
    currentLanguage = defaultLanguage;

    // Setup user interface
    ui.setupUi(this);
    hide();

    // Set dock options
    setDockOptions(AnimatedDocks | AllowTabbedDocks | AllowNestedDocks);
    statusBar()->setSizeGripEnabled(true);

    // load language choices
    createLanguageMenu();
    // load external tool choices
    createExternalToolsMenu();

    // Setup corners
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Setup UI state machines
	centerStackActionGroup->setExclusive(true);

    centerStack = new QStackedWidget(this);
    setCentralWidget(centerStack);
    centerStack->setMinimumSize(QSize(200,150)); // this allows smoother resizing of docks on small screens

    // Load Toolbar
    toolBar = new QGCToolBar(this);
    toolBar->setWindowTitle("Tool Bar Visible");
    this->addToolBar(toolBar);

    ui.actionOperatorsView->setObjectName("actionOperatorsView");
    ui.actionEngineersView->setObjectName("actionEngineersView");
    ui.actionPilotsView->setObjectName("actionPilotsView");
    ui.actionDataView->setObjectName("actionDataView");

    QList<QAction*> actions;
    actions << ui.actionPilotsView;
    actions << ui.actionOperatorsView;
    actions << ui.actionEngineersView;
    actions << ui.actionDataView;
    toolBar->setPerspectiveChangeActions(actions);

    // hide system menu for now since it's not useful
    ui.menuUnmanned_System->menuAction()->setVisible(false);

    emit initStatusChanged("Building common widgets.");

    buildCommonWidgets();
    connectCommonWidgets();

    emit initStatusChanged("Building common actions.");

    // Create actions
    connectCommonActions();

    // Populate link menu
    emit initStatusChanged("Populating link menu");
    QList<LinkInterface*> links = LinkManager::instance()->getLinks();
    foreach(LinkInterface* link, links)
    {
        this->addLink(link);
    }

    connect(LinkManager::instance(), SIGNAL(newLink(LinkInterface*)), this, SLOT(addLink(LinkInterface*)));

    // Connect user interface devices
//    emit initStatusChanged("Initializing joystick interface.");
//    joystickWidget = 0;
//    joystick = new JoystickInput();

#ifdef MOUSE_ENABLED_WIN
    emit initStatusChanged("Initializing 3D mouse interface.");

    mouseInput = new Mouse3DInput(this);
    mouse = new Mouse6dofInput(mouseInput);
#endif //MOUSE_ENABLED_WIN

#if MOUSE_ENABLED_LINUX
    emit initStatusChanged("Initializing 3D mouse interface.");

    mouse = new Mouse6dofInput(this);
    connect(this, SIGNAL(x11EventOccured(XEvent*)), mouse, SLOT(handleX11Event(XEvent*)));
#endif //MOUSE_ENABLED_LINUX

    // Connect link
    if (autoReconnect)
    {
        SerialLink* link = new SerialLink();
        // Add to registry
        LinkManager::instance()->add(link);
        LinkManager::instance()->addProtocol(link, mavlink);
        link->connect();
    }

    // Set low power mode
    enableLowPowerMode(lowPowerMode);

    // Initialize window state
    windowStateVal = windowState();

    emit initStatusChanged("Restoring last view state.");

    // Restore the window setup
    loadViewState();

    emit initStatusChanged("Restoring last window size.");
    // Restore the window position and size
    if (settings.contains(getWindowGeometryKey()))
    {
        // Restore the window geometry
        restoreGeometry(settings.value(getWindowGeometryKey()).toByteArray());
        show();
    }
    else
    {
        // Adjust the size
        const int screenWidth = QApplication::desktop()->width();
        const int screenHeight = QApplication::desktop()->height();

        if (screenWidth < 1200)
        {
            showFullScreen();
        }
        else
        {
            resize(screenWidth*0.67f, qMin(screenHeight, (int)(screenWidth*0.67f*0.67f)));
            show();
        }

    }

    retranslateUi();

    connect(&windowNameUpdateTimer, SIGNAL(timeout()), this, SLOT(configureWindowName()));
    windowNameUpdateTimer.start(15000);
    emit initStatusChanged("Done.");
    show();

    loadStyle(currentStyle);
}

MainWindow::~MainWindow()
{
    if (mavlink)
    {
        delete mavlink;
        mavlink = NULL;
    }

    //qApp->removeTranslator(m_translator);
    //delete m_translator;

//    if (simulationLink)
//    {
//        simulationLink->deleteLater();
//        simulationLink = NULL;
//    }
//    if (joystick)
//    {
//        delete joystick;
//        joystick = NULL;
//    }

    // Get and delete all dockwidgets and contained
    // widgets
    QObjectList childList(this->children());

    QObjectList::iterator i;
    QDockWidget* dockWidget;
    for (i = childList.begin(); i != childList.end(); ++i)
    {
        dockWidget = dynamic_cast<QDockWidget*>(*i);
        if (dockWidget)
        {
            // Remove dock widget from main window
            // removeDockWidget(dockWidget);
            // delete dockWidget->widget();
            delete dockWidget;
            dockWidget = NULL;
        }
        else if (dynamic_cast<QWidget*>(*i))
        {
            delete dynamic_cast<QWidget*>(*i);
            *i = NULL;
        }
    }
    // Delete all UAS objects
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
    if (height() < 800)
    {
        ui.statusBar->setVisible(false);
    }
    else
    {
        ui.statusBar->setVisible(true);
        ui.statusBar->setSizeGripEnabled(true);
    }

    if (width() > 1200)
    {
        toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else
    {
        toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
    if(event)
    {
        switch(event->type())
        {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            ui.retranslateUi(this);
            retranslateUi();
            break;
        // this event is send, if the system, language changes
        case QEvent::LocaleChange:
            {
                QString locale = QLocale::system().name();
                locale.truncate(locale.lastIndexOf('_'));
                loadLanguage(locale);
                //QGCCore::setLanguage(currentLanguage);
            }
            break;
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isVisible()) storeViewState();
    storeSettings();
    aboutToCloseFlag = true;
    mavlink->storeSettings();
    UASManager::instance()->storeSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::retranslateUi(/*QMainWindow *MainWindow*/)
{
    configureWindowName();
//    listDockWidget->setWindowTitle(tr("Unmanned Systems"));
}

QString MainWindow::getWindowStateKey()
{
    return QString::number(currentView)+"_windowstate";
}

QString MainWindow::getWindowGeometryKey()
{
    //return QString::number(currentView)+"_geometry";
    return "_geometry";
}

// Called every time sn entry of the language menu is called
void MainWindow::languageChanged(QAction* action)
{
    if(action)
        loadLanguage(action->data().toString());
}

// we create the language menu entries dynamically, dependant on the existing translations.
void MainWindow::createLanguageMenu(void)
{
    QActionGroup* langGroup = new QActionGroup(ui.menuLanguage);
    langGroup->setExclusive(true);

    connect(langGroup, SIGNAL(triggered(QAction *)), this, SLOT(languageChanged(QAction *)));

    QString langPath = QGCCore::getLangFilePath();

    foreach (QString lang, QGCCore::availableLanguages())
    {
        // figure out nice names for locales
        QLocale locale(lang);
        QString name = QLocale::languageToString(locale.language());
        //QString language = QLocale::languageToString(locale.language());
        //QString country = QLocale::countryToString(locale.country());
        //QString name = language;
        //if (country.length())
        //    name += " (" + country + ")";

        // construct an action
        QIcon ico(QString("%1/flags/%2.png").arg(langPath).arg(lang));

        QAction *action = new QAction(ico, name, this);
        action->setCheckable(true);
        action->setData(lang);

        ui.menuLanguage->addAction(action);
        langGroup->addAction(action);

        // set default translators and language checked
        if (currentLanguage == lang)
            action->setChecked(true);
    }

}

// Called every time, when a menu entry of the language menu is called
void MainWindow::loadLanguage(const QString& lang)
{
    if (currentLanguage != lang) {
        currentLanguage = lang;
        //loadLanguage(locale);
        QGCCore::setLanguage(currentLanguage);

        QLocale l = QLocale(currentLanguage);
        QLocale::setDefault(l);
        QString languageName = QLocale::languageToString(l.language());
        ui.statusBar->showMessage(tr("Current Language changed to %1").arg(languageName));
    }
}

void MainWindow::createExternalToolsMenu()
{
#if defined(Q_OS_WIN)
    QString platformExeExt = ".exe";
#else
    QString platformExeExt = "";
#endif
    QIcon aqIcon(":/files/images/contrib/AQ_logo_quad-only_outlined_112px.png");
    QAction *act;
    QActionGroup* toolGroup = new QActionGroup(ui.menuOtherTools);
    toolGroup->setExclusive(false);
    connect(toolGroup, SIGNAL(triggered(QAction *)), this, SLOT(launchExternalTool(QAction *)));

    QString fpath = QCoreApplication::applicationDirPath() + "/AQ_IMU_Calibration" + platformExeExt;
    if (QFileInfo(fpath).exists()) {
        act = new QAction(aqIcon, tr("AQ IMU Calibration"), this);
        act->setData(fpath);
        ui.menuOtherTools->addAction(act);
        toolGroup->addAction(act);
    }
    fpath = QCoreApplication::applicationDirPath() + "/AQ_Log_Export" + platformExeExt;
    if (QFileInfo(fpath).exists()) {
        act = new QAction(aqIcon, tr("AQ Log Export"), this);
        act->setData(fpath);
        ui.menuOtherTools->addAction(act);
        toolGroup->addAction(act);
    }

    if (!toolGroup->actions().size())
        ui.menuOtherTools->setDisabled(true);
}

// Called to launch an external program from the Other Tools menu
void MainWindow::launchExternalTool(QAction* action)
{
    if(action)
        QProcess::startDetached(action->data().toString());
}

void MainWindow::buildCustomWidget()
{
    // Create custom widgets
    QList<QGCToolWidget*> widgets = QGCToolWidget::createWidgetsFromSettings(this);

    if (widgets.size() > 0)
    {
        ui.menuTools->addSeparator();
    }

    for(int i = 0; i < widgets.size(); ++i)
    {
        // Check if this widget already has a parent, do not create it in this case
        QGCToolWidget* tool = widgets.at(i);
        QDockWidget* dock = dynamic_cast<QDockWidget*>(tool->parentWidget());
        if (!dock)
        {
            QDockWidget* dock = new QDockWidget(tool->windowTitle(), this);
            dock->setObjectName(tool->objectName()+"_DOCK");
            dock->setWidget(tool);
            connect(tool, SIGNAL(destroyed()), dock, SLOT(deleteLater()));
            QAction* showAction = new QAction(widgets.at(i)->windowTitle(), this);
            showAction->setCheckable(true);
            connect(showAction, SIGNAL(triggered(bool)), dock, SLOT(setVisible(bool)));
            connect(dock, SIGNAL(visibilityChanged(bool)), showAction, SLOT(setChecked(bool)));
            widgets.at(i)->setMainMenuAction(showAction);
            ui.menuTools->addAction(showAction);

            // Load dock widget location (default is bottom)
            Qt::DockWidgetArea location = static_cast <Qt::DockWidgetArea>(tool->getDockWidgetArea(currentView));

            addDockWidget(location, dock);
        }
    }
}

void MainWindow::buildCommonWidgets()
{
    //TODO:  move protocol outside UI
    mavlink     = new MAVLinkProtocol();
    connect(mavlink, SIGNAL(protocolStatusMessage(QString,QString)), this, SLOT(showCriticalMessage(QString,QString)), Qt::QueuedConnection);
    // Add generic MAVLink decoder
    mavlinkDecoder = new MAVLinkDecoder(mavlink, this);

    // Dock widgets
//    if (!controlDockWidget)
//    {
//        controlDockWidget = new QDockWidget(tr("Control"), this);
//        controlDockWidget->setObjectName("UNMANNED_SYSTEM_CONTROL_DOCKWIDGET");
//        controlDockWidget->setWidget( new UASControlWidget(this) );
//        addTool(controlDockWidget, tr("Control"), Qt::LeftDockWidgetArea);
//    }

    if (!listDockWidget)
    {
        listDockWidget = new QDockWidget(tr("Unmanned Systems"), this);
        listDockWidget->setWidget( new UASListWidget(this) );
        listDockWidget->setObjectName("UNMANNED_SYSTEMS_LIST_DOCKWIDGET");
        addTool(listDockWidget, tr("Unmanned Systems"), Qt::RightDockWidgetArea);
    }

    if (!waypointsDockWidget)
    {
        waypointsDockWidget = new QDockWidget(tr("Mission Plan"), this);
        waypointsDockWidget->setWidget( new QGCWaypointListMulti(this) );
        waypointsDockWidget->setObjectName("WAYPOINT_LIST_DOCKWIDGET");
        addTool(waypointsDockWidget, tr("Mission Plan"), Qt::BottomDockWidgetArea);
    }

    if (!infoDockWidget)
    {
        infoDockWidget = new QDockWidget(tr("Status Details"), this);
        infoDockWidget->setWidget( new UASInfoWidget(this) );
        infoDockWidget->setObjectName("UAS_STATUS_DETAILS_DOCKWIDGET");
        addTool(infoDockWidget, tr("Status Details"), Qt::RightDockWidgetArea);
    }

    if (!debugConsoleDockWidget)
    {
        debugConsoleDockWidget = new QDockWidget(tr("Communication Console"), this);
        debugConsoleDockWidget->setWidget( new DebugConsole(this) );
        debugConsoleDockWidget->setObjectName("COMMUNICATION_DEBUG_CONSOLE_DOCKWIDGET");

        DebugConsole *debugConsole = dynamic_cast<DebugConsole*>(debugConsoleDockWidget->widget());
        connect(mavlinkDecoder, SIGNAL(textMessageReceived(int, int, int, const QString)), debugConsole, SLOT(receiveTextMessage(int, int, int, const QString)));
        connect(debugConsoleDockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), debugConsole, SLOT(dockEvent(Qt::DockWidgetArea)));

        addTool(debugConsoleDockWidget, tr("Communication Console"), Qt::BottomDockWidgetArea);
    }

    if (!logPlayerDockWidget)
    {
        logPlayerDockWidget = new QDockWidget(tr("MAVLink Log Player"), this);
        logPlayer = new QGCMAVLinkLogPlayer(mavlink, this);
        toolBar->setLogPlayer(logPlayer);
        logPlayerDockWidget->setWidget(logPlayer);
        logPlayerDockWidget->setObjectName("MAVLINK_LOG_PLAYER_DOCKWIDGET");
        addTool(logPlayerDockWidget, tr("MAVLink Log Replay"), Qt::RightDockWidgetArea);
    }

    if (!mavlinkInspectorWidget)
    {
        mavlinkInspectorWidget = new QDockWidget(tr("MAVLink Message Inspector"), this);
        mavlinkInspectorWidget->setWidget( new QGCMAVLinkInspector(mavlink, this) );
        mavlinkInspectorWidget->setObjectName("MAVLINK_INSPECTOR_DOCKWIDGET");
        addTool(mavlinkInspectorWidget, tr("MAVLink Inspector"), Qt::RightDockWidgetArea);
    }

//    if (!mavlinkSenderWidget)
//    {
//        mavlinkSenderWidget = new QDockWidget(tr("MAVLink Message Sender"), this);
//        mavlinkSenderWidget->setWidget( new QGCMAVLinkMessageSender(mavlink, this) );
//        mavlinkSenderWidget->setObjectName("MAVLINK_SENDER_DOCKWIDGET");
//        addTool(mavlinkSenderWidget, tr("MAVLink Sender"), Qt::RightDockWidgetArea);
//    }

//    if (!parametersDockWidget)
//    {
//        parametersDockWidget = new QDockWidget(tr("Onboard Parameters"), this);
//        parametersDockWidget->setWidget( new ParameterInterface(this) );
//        parametersDockWidget->setObjectName("PARAMETER_INTERFACE_DOCKWIDGET");
//        addTool(parametersDockWidget, tr("Onboard Parameters"), Qt::RightDockWidgetArea);
//    }
	
    if (!hsiDockWidget)
    {
        hsiDockWidget = new QDockWidget(tr("Horizontal Situation Indicator"), this);
        hsiDockWidget->setWidget( new HSIDisplay(this) );
        hsiDockWidget->setObjectName("HORIZONTAL_SITUATION_INDICATOR_DOCK_WIDGET");
        addTool(hsiDockWidget, tr("Horizontal Situation"), Qt::RightDockWidgetArea);
    }
	
    if (!headDown1DockWidget)
    {
        headDown1DockWidget = new QDockWidget(tr("Custom Gauges"), this);
        HDDisplay* hdDisplay = new HDDisplay(NULL, "Custom Gauges", this);
        hdDisplay->addSource(mavlinkDecoder);
        headDown1DockWidget->setWidget(hdDisplay);
        headDown1DockWidget->setObjectName("HEAD_DOWN_DISPLAY_1_DOCK_WIDGET");
        addTool(headDown1DockWidget, tr("Custom Gauges"), Qt::RightDockWidgetArea);
    }

    if (!rcViewDockWidget)
    {
        rcViewDockWidget = new QDockWidget(tr("Channels Monitor"), this);
        rcViewDockWidget->setWidget( new QGCRemoteControlView(this) );
        rcViewDockWidget->setObjectName("RADIO_CONTROL_CHANNELS_DOCK_WIDGET");
        addTool(rcViewDockWidget, tr("Channels Monitor"), Qt::RightDockWidgetArea);
    }

    if (!headUpDockWidget)
    {
        headUpDockWidget = new QDockWidget(tr("HUD"), this);
        headUpDockWidget->setWidget( new HUD(320, 240, this));
        headUpDockWidget->setObjectName("HEAD_UP_DISPLAY_DOCK_WIDGET");
        addTool(headUpDockWidget, tr("Head Up Display"), Qt::RightDockWidgetArea);
    }

    if (!pfdDockWidget)
    {
        pfdDockWidget = new QDockWidget(tr("Primary Flight Display"), this);
        pfdDockWidget->setWidget( new PrimaryFlightDisplay(320, 240, this));
        pfdDockWidget->setObjectName("PRIMART_FLIGHT_DISPLAY_DOCK_WIDGET");
        addTool(pfdDockWidget, tr("Primary Flight Display"), Qt::RightDockWidgetArea);
    }

//    if (!video1DockWidget)
//    {
//        video1DockWidget = new QDockWidget(tr("Video Stream 1"), this);
//        QGCRGBDView* video1 =  new QGCRGBDView(160, 120, this);
//        video1->enableHUDInstruments(false);
//        video1->enableVideo(false);
//        // FIXME select video stream as well
//        video1DockWidget->setWidget(video1);
//        video1DockWidget->setObjectName("VIDEO_STREAM_1_DOCK_WIDGET");
//        addTool(video1DockWidget, tr("Video Stream 1"), Qt::LeftDockWidgetArea);
//    }

//    if (!video2DockWidget)
//    {
//        video2DockWidget = new QDockWidget(tr("Video Stream 2"), this);
//        QGCRGBDView* video2 =  new QGCRGBDView(160, 120, this);
//        video2->enableHUDInstruments(false);
//        video2->enableVideo(false);
//        // FIXME select video stream as well
//        video2DockWidget->setWidget(video2);
//        video2DockWidget->setObjectName("VIDEO_STREAM_2_DOCK_WIDGET");
//        addTool(video2DockWidget, tr("Video Stream 2"), Qt::LeftDockWidgetArea);
//    }

//    if (!rgbd1DockWidget) {
//        rgbd1DockWidget = new QDockWidget(tr("Video Stream 1"), this);
//        HUD* video1 =  new HUD(160, 120, this);
//        video1->enableHUDInstruments(false);
//        video1->enableVideo(true);
//        // FIXME select video stream as well
//        video1DockWidget->setWidget(video1);
//        video1DockWidget->setObjectName("VIDEO_STREAM_1_DOCK_WIDGET");
//        addTool(video1DockWidget, tr("Video Stream 1"), Qt::LeftDockWidgetArea);
//    }

//    if (!rgbd2DockWidget) {
//        video2DockWidget = new QDockWidget(tr("Video Stream 2"), this);
//        HUD* video2 =  new HUD(160, 120, this);
//        video2->enableHUDInstruments(false);
//        video2->enableVideo(true);
//        // FIXME select video stream as well
//        video2DockWidget->setWidget(video2);
//        video2DockWidget->setObjectName("VIDEO_STREAM_2_DOCK_WIDGET");
//        addTool(video2DockWidget, tr("Video Stream 2"), Qt::LeftDockWidgetArea);
//    }

    if (!escTelemetryWidget)
    {
        escTelemetryWidget = new QDockWidget(tr("ESC Telemetry"), this);
        escTelemetryWidget->setWidget( new ESCtelemetryWidget(escTelemetryWidget) );
        escTelemetryWidget->setObjectName("ESC_TELEMETRY_WIDGET");
        addTool(escTelemetryWidget, tr("ESC Telemetry"), Qt::RightDockWidgetArea);
    }

    // Custom widgets, added last to all menus and layouts
    buildCustomWidget();

    // Center widgets
    if (!mapWidget)
    {
        mapWidget = new QGCMapTool(this);
        addCentralWidget(mapWidget, "Maps");
    }

    if (!hudWidget)
    {
        hudWidget         = new HUD(320, 240, this);
        addCentralWidget(hudWidget, tr("Head Up Display"));
    }

//    if (!dataplotWidget)
//    {
//        dataplotWidget    = new QGCDataPlot2D(this);
//        addCentralWidget(dataplotWidget, tr("Logfile Plot"));
//    }

	if ( !autoquadWidget){
        autoquadWidget    = new QGCAutoquad(this);
        addCentralWidget(autoquadWidget, tr("AutoQuad"));
	}

    if ( !dataViewWidget){
        dataViewWidget    = new QGCDataViewWidget(this);
        dataViewWidget->addSource(mavlinkDecoder);
        addCentralWidget(dataViewWidget, tr("Log Analysis && Telemetry"));
    }


#ifdef QGC_OSG_ENABLED
    if (!_3DWidget)
    {
        _3DWidget         = Q3DWidgetFactory::get("PIXHAWK", this);
        addCentralWidget(_3DWidget, tr("Local 3D"));
    }
#endif

#ifdef USE_GOOGLE_EARTH_PLUGIN
    if (!gEarthWidget)
    {
        gEarthWidget = new QGCGoogleEarthView(this);
        addCentralWidget(gEarthWidget, tr("Google Earth"));
    }
#endif
}

void MainWindow::addTool(QDockWidget* widget, const QString& title, Qt::DockWidgetArea area)
{
    QAction* tempAction = ui.menuTools->addAction(title);

    tempAction->setCheckable(true);
    QVariant var;
    var.setValue((QWidget*)widget);
    tempAction->setData(var);
    connect(tempAction,SIGNAL(triggered(bool)),this, SLOT(showTool(bool)));
    connect(widget, SIGNAL(visibilityChanged(bool)), tempAction, SLOT(setChecked(bool)));
    tempAction->setChecked(widget->isVisible());
    addDockWidget(area, widget);
}


void MainWindow::showTool(bool show)
{
    QAction* act = qobject_cast<QAction *>(sender());
    QWidget* widget = act->data().value<QWidget *>();
    widget->setVisible(show);
}

void MainWindow::addCentralWidget(QWidget* widget, const QString& title)
{
    // Check if this widget already has been added
    if (centerStack->indexOf(widget) == -1)
    {
        centerStack->addWidget(widget);

        QAction* tempAction = ui.menuMain->addAction(title);

        tempAction->setCheckable(true);
        QVariant var;
        var.setValue((QWidget*)widget);
        tempAction->setData(var);
        centerStackActionGroup->addAction(tempAction);
        connect(tempAction,SIGNAL(triggered()),this, SLOT(showCentralWidget()));
        connect(widget, SIGNAL(visibilityChanged(bool)), tempAction, SLOT(setChecked(bool)));
        tempAction->setChecked(widget->isVisible());
    }
}


void MainWindow::showCentralWidget()
{
    QAction* act = qobject_cast<QAction *>(sender());
    QWidget* widget = act->data().value<QWidget *>();
    centerStack->setCurrentWidget(widget);
}

//void MainWindow::showHILConfigurationWidget(UASInterface* uas)
//{
//    // Add simulation configuration widget
//    UAS* mav = dynamic_cast<UAS*>(uas);

//    if (mav)
//    {
//        QGCHilConfiguration* hconf = new QGCHilConfiguration(mav, this);
//        QString hilDockName = tr("HIL Config (%1)").arg(uas->getUASName());
//        QDockWidget* hilDock = new QDockWidget(hilDockName, this);
//        hilDock->setWidget(hconf);
//        hilDock->setObjectName(QString("HIL_CONFIG_%1").arg(uas->getUASID()));
//        addTool(hilDock, hilDockName, Qt::RightDockWidgetArea);

//    }

//    // Reload view state in case new widgets were added
//    loadViewState();
//}

/**
 * Connect the signals and slots of the common window widgets
 */
void MainWindow::connectCommonWidgets()
{
    if (infoDockWidget && infoDockWidget->widget())
    {
        connect(mavlink, SIGNAL(receiveLossChanged(int, float)),
                infoDockWidget->widget(), SLOT(updateSendLoss(int, float)));
    }
}

void MainWindow::createCustomWidget()
{
    QDockWidget* dock = new QDockWidget("Unnamed Tool", this);
    QGCToolWidget* tool = new QGCToolWidget("Unnamed Tool", dock);

    if (QGCToolWidget::instances()->size() < 2)
    {
        // This is the first widget
        ui.menuTools->addSeparator();
    }

    connect(tool, SIGNAL(destroyed()), dock, SLOT(deleteLater()));
    dock->setWidget(tool);

    QAction* showAction = new QAction(tool->getTitle(), this);
    showAction->setCheckable(true);
    connect(dock, SIGNAL(visibilityChanged(bool)), showAction, SLOT(setChecked(bool)));
    connect(showAction, SIGNAL(triggered(bool)), dock, SLOT(setVisible(bool)));
    tool->setMainMenuAction(showAction);
    ui.menuTools->addAction(showAction);
    this->addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->setVisible(true);
}

void MainWindow::loadCustomWidget()
{
    QString widgetFileExtension(".qgw");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Specify Widget File Name"), DEFAULT_STORAGE_PATH,
                                                    QString("QGroundControl Widget (*%1)").arg(widgetFileExtension));
    if (fileName != "") loadCustomWidget(fileName);
}

void MainWindow::loadCustomWidget(const QString& fileName, bool singleinstance)
{
    QGCToolWidget* tool = new QGCToolWidget("", this);
    if (tool->loadSettings(fileName, true) || !singleinstance)
    {
        // Add widget to UI
        QDockWidget* dock = new QDockWidget(tool->getTitle(), this);
        connect(tool, SIGNAL(destroyed()), dock, SLOT(deleteLater()));
        dock->setWidget(tool);
        tool->setParent(dock);

        QAction* showAction = new QAction(tool->getTitle(), this);
        showAction->setCheckable(true);
        connect(dock, SIGNAL(visibilityChanged(bool)), showAction, SLOT(setChecked(bool)));
        connect(showAction, SIGNAL(triggered(bool)), dock, SLOT(setVisible(bool)));
        tool->setMainMenuAction(showAction);
        ui.menuTools->addAction(showAction);
        this->addDockWidget(Qt::BottomDockWidgetArea, dock);
        dock->setVisible(true);
    }
    else
    {
        return;
    }
}

void MainWindow::loadCustomWidgetsFromDefaults(const QString& systemType, const QString& autopilotType)
{
    QString defaultsDir = qApp->applicationDirPath() + "/files/" + autopilotType.toLower() + "/widgets/";
    QString platformDir = qApp->applicationDirPath() + "/files/" + autopilotType.toLower() + "/" + systemType.toLower() + "/widgets/";

    QDir widgets(defaultsDir);
    QStringList files = widgets.entryList();
    QDir platformWidgets(platformDir);
    files.append(platformWidgets.entryList());

    if (files.count() == 0)
    {
        qDebug() << "No default custom widgets for system " << systemType << "autopilot" << autopilotType << " found";
        qDebug() << "Tried with path: " << defaultsDir;
        showStatusMessage(tr("Did not find any custom widgets in %1").arg(defaultsDir));
    }

    // Load all custom widgets found in the AP folder
    for(int i = 0; i < files.count(); ++i)
    {
        QString file = files[i];
        if (file.endsWith(".qgw"))
        {
            // Will only be loaded if not already a custom widget with
            // the same name is present
            loadCustomWidget(defaultsDir+"/"+file, true);
            showStatusMessage(tr("Loaded custom widget %1").arg(defaultsDir+"/"+file));
        }
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_MAINWINDOW");
    autoReconnect = settings.value("AUTO_RECONNECT", autoReconnect).toBool();
    currentStyle = (QGC_MAINWINDOW_STYLE)settings.value("CURRENT_STYLE", QGC_MAINWINDOW_STYLE_DARK).toInt();
    lowPowerMode = settings.value("LOW_POWER_MODE", lowPowerMode).toBool();
    defaultLanguage = settings.value("UI_LANGUAGE", defaultLanguage).toString();
    settings.endGroup();
}

void MainWindow::storeSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_MAINWINDOW");
    settings.setValue("AUTO_RECONNECT", autoReconnect);
    settings.setValue("CURRENT_STYLE", currentStyle);
    if (!aboutToCloseFlag && isVisible())
    {
        settings.setValue(getWindowGeometryKey(), saveGeometry());
        // Save the last current view in any case
        settings.setValue("CURRENT_VIEW", currentView);
        // Save the current window state, but only if a system is connected (else no number of widgets would be present)
        if (UASManager::instance()->getUASList().length() > 0) {
            settings.setValue(getWindowStateKey(), saveState(QGC::applicationVersion()));
            // Save the current view only if a UAS is connected
            settings.setValue("CURRENT_VIEW_WITH_UAS_CONNECTED", currentView);
        }
    }
    // Save the current power mode
    settings.setValue("LOW_POWER_MODE", lowPowerMode);
    settings.setValue("UI_LANGUAGE", currentLanguage);
    settings.endGroup();
    settings.sync();
}

void MainWindow::configureWindowName()
{
    QList<QHostAddress> hostAddresses = QNetworkInterface::allAddresses();
//    QString windowname = qApp->applicationName() + " " + qApp->applicationVersion();
    QString windowname = QGCAUTOQUAD::APP_NAME + " v" + QGCAUTOQUAD::APP_VERSION_TXT + " (" + QString::number(QGCAUTOQUAD::APP_VERSION, 'f', 2) + ")";
    bool prevAddr = false;

    windowname.append(" (" + QHostInfo::localHostName() + ": ");

    for (int i = 0; i < hostAddresses.size(); i++)
    {
        // Exclude loopback IPv4 and all IPv6 addresses
        if (hostAddresses.at(i) != QHostAddress("127.0.0.1") && !hostAddresses.at(i).toString().contains(":"))
        {
            if(prevAddr) windowname.append("/");
            windowname.append(hostAddresses.at(i).toString());
            prevAddr = true;
        }
    }

    windowname.append(")");

    setWindowTitle(windowname);
}

void MainWindow::startVideoCapture()
{
    QString format = "bmp";
    QString initialPath = QDir::currentPath() + tr("/untitled.") + format;

    QString screenFileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                          initialPath,
                                                          QString("%1 Files (*.%2)")
                                                          .arg(format.toUpper()).arg(format)) + ";;" + tr("All File Types") + " (*.*)";
    delete videoTimer;
    videoTimer = new QTimer(this);
    //videoTimer->setInterval(40);
    //connect(videoTimer, SIGNAL(timeout()), this, SLOT(saveScreen()));
    //videoTimer->stop();
}

void MainWindow::stopVideoCapture()
{
    videoTimer->stop();

    // TODO Convert raw images to PNG
}

void MainWindow::saveScreen()
{
    QPixmap window = QPixmap::grabWindow(this->winId());
    QString format = "bmp";

    if (!screenFileName.isEmpty())
    {
        window.save(screenFileName, format.toLatin1());
    }
}

void MainWindow::enableAutoReconnect(bool enabled)
{
    autoReconnect = enabled;
}

void MainWindow::loadStyleByName(const QString style)
{
    loadStyle((QGC_MAINWINDOW_STYLE)m_windowStyleNames.key(style));
}

void MainWindow::loadStyle(QGC_MAINWINDOW_STYLE style)
{
//    if (style == currentStyle)
//        return;

    qApp->setStyleSheet("");
    QString styleFileName = ":files/styles/style-default.css";

    switch (style) {
        case QGC_MAINWINDOW_STYLE_NATIVE:
            styleFileName = ":files/styles/style-native.css";
            break;
        case QGC_MAINWINDOW_STYLE_DARK:
            qApp->setStyle("plastique");
            styleFileName = ":files/styles/style-dark.css";
            break;
        case QGC_MAINWINDOW_STYLE_LIGHT:
            qApp->setStyle("plastique");
            styleFileName = ":files/styles/style-light.css";
            break;
        default:
            qApp->setStyle(m_windowStyleNames.value(style, "plastique"));
            break;
    }
    reloadStylesheet(styleFileName);
    currentStyle = style;
    emit styleChanged((int)style);
}

void MainWindow::setAvailableStyles()
{
    m_windowStyleNames.insert(QGC_MAINWINDOW_STYLE_NATIVE, "Operating System Native");
    m_windowStyleNames.insert(QGC_MAINWINDOW_STYLE_DARK, "QGC Dark");
    m_windowStyleNames.insert(QGC_MAINWINDOW_STYLE_LIGHT, "QGC Light");

    int sid;
    QString s;
    foreach (const QString style, QStyleFactory::keys()) {
        s = style.toLower();
        if (s == "plastique")
            sid = QGC_MAINWINDOW_STYLE_PLASTIQUE;
        else if (s.startsWith("gtk"))
            sid = QGC_MAINWINDOW_STYLE_GTK;
        else if (s == "cleanlooks")
            sid = QGC_MAINWINDOW_STYLE_CLEANLOOKS;
        else if (s.startsWith("macintosh"))
            sid = QGC_MAINWINDOW_STYLE_MAC;
        else if (s == "windows")
            sid = QGC_MAINWINDOW_STYLE_WIN;
        else if (s == "windowsxp")
            sid = QGC_MAINWINDOW_STYLE_WINXP;
        else if (s == "windowsvista")
            sid = QGC_MAINWINDOW_STYLE_WINVISTA;
        else if (s == "motif")
            sid = QGC_MAINWINDOW_STYLE_MOTIF;
        else if (s == "cde")
            sid = QGC_MAINWINDOW_STYLE_CDE;
        else
            continue;

        m_windowStyleNames.insert(sid, style);
    }
}

void MainWindow::selectStylesheet()
{
    // Let user select style sheet
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a stylesheet"), QFileInfo(styleFileName).absolutePath(), tr("CSS Stylesheet") + " (*.css)");

    if (fileName == "")
        return;

    if (!fileName.endsWith(".css"))
    {
        showInfoMessage(tr("QGroundControl did lot load a new style"), tr("No suitable .css file selected. Please select a valid .css file."));
        return;
    }

    // Load style sheet
    reloadStylesheet(fileName);
}

void MainWindow::reloadStylesheet(const QString file)
{
    QFile *styleSheet;
    if (file.length())
        styleSheet = new QFile(file);
    else
        styleSheet = new QFile(styleFileName);

    if (!styleSheet || !styleSheet->exists()) {
        showInfoMessage(tr("QGroundControl did lot load a new style"), tr("Stylesheet file '%1'' was not found").arg(file));
        return;
    }
    if (styleSheet->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString(styleSheet->readAll());
        style.replace("ICONDIR", QCoreApplication::applicationDirPath()+ "files/styles/");
        qApp->setStyleSheet(style);
        styleFileName = QFileInfo(*styleSheet).absoluteFilePath();
    }
    else
        showInfoMessage(tr("QGroundControl did lot load a new style"), tr("Stylesheet file %1 was not readable").arg(file));

    delete styleSheet;
}

/**
 * The status message will be overwritten if a new message is posted to this function
 *
 * @param status message text
 * @param timeout how long the status should be displayed
 */
void MainWindow::showStatusMessage(const QString& status, int timeout)
{
    statusBar()->showMessage(status, timeout);
}

/**
 * The status message will be overwritten if a new message is posted to this function.
 * it will be automatically hidden after 5 seconds.
 *
 * @param status message text
 */
void MainWindow::showStatusMessage(const QString& status)
{
    statusBar()->showMessage(status, 20000);
}

void MainWindow::showCriticalMessage(const QString& title, const QString& message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(title);
    msgBox.setInformativeText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::showInfoMessage(const QString& title, const QString& message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(title);
    msgBox.setInformativeText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

/**
* @brief Create all actions associated to the main window
*
**/
void MainWindow::connectCommonActions()
{
    // Bind together the perspective actions
    QActionGroup* perspectives = new QActionGroup(ui.menuPerspectives);
//    perspectives->addAction(ui.actionMavlinkView);
    perspectives->addAction(ui.actionPilotsView);
    perspectives->addAction(ui.actionOperatorsView);
    perspectives->addAction(ui.actionEngineersView);
    perspectives->addAction(ui.actionDataView);
//    perspectives->addAction(ui.actionFirmwareUpdateView);
//    perspectives->addAction(ui.actionUnconnectedView);
    perspectives->setExclusive(true);

    // Mark the right one as selected
    switch (currentView)
    {
    case VIEW_ENGINEER:
        ui.actionEngineersView->setChecked(true);
        break;
    case VIEW_PILOT:
        ui.actionPilotsView->setChecked(true);
        break;
    case VIEW_DATA:
        ui.actionDataView->setChecked(true);
        break;
//    case VIEW_MAVLINK:
//        ui.actionMavlinkView->setChecked(true);
//        break;
//    case VIEW_FIRMWAREUPDATE:
//        ui.actionFirmwareUpdateView->setChecked(true);
//        break;
//    case VIEW_UNCONNECTED:
//        ui.actionUnconnectedView->setChecked(true);
//        break;
    case VIEW_OPERATOR:
    case VIEW_UNCONNECTED:
    default:
        ui.actionOperatorsView->setChecked(true);
        break;
    }
    // The UAS actions are not enabled without connection to system
    ui.actionLiftoff->setEnabled(false);
    ui.actionLand->setEnabled(false);
    ui.actionEmergency_Kill->setEnabled(false);
    ui.actionEmergency_Land->setEnabled(false);
    ui.actionShutdownMAV->setEnabled(false);

    // Connect actions from ui
    connect(ui.actionAdd_Link, SIGNAL(triggered()), this, SLOT(addLink()));

    // Connect internal actions
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(UASCreated(UASInterface*)));
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(UASDeleted(UASInterface*)));

    // Unmanned System controls
    connect(ui.actionLiftoff, SIGNAL(triggered()), UASManager::instance(), SLOT(launchActiveUAS()));
    connect(ui.actionLand, SIGNAL(triggered()), UASManager::instance(), SLOT(returnActiveUAS()));
    connect(ui.actionEmergency_Land, SIGNAL(triggered()), UASManager::instance(), SLOT(stopActiveUAS()));
    connect(ui.actionEmergency_Kill, SIGNAL(triggered()), UASManager::instance(), SLOT(killActiveUAS()));
    connect(ui.actionShutdownMAV, SIGNAL(triggered()), UASManager::instance(), SLOT(shutdownActiveUAS()));
    connect(ui.actionConfiguration, SIGNAL(triggered()), UASManager::instance(), SLOT(configureActiveUAS()));

    // Views actions
    connect(ui.actionPilotsView, SIGNAL(triggered()), this, SLOT(loadPilotView()));
    connect(ui.actionEngineersView, SIGNAL(triggered()), this, SLOT(loadEngineerView()));
    connect(ui.actionOperatorsView, SIGNAL(triggered()), this, SLOT(loadOperatorView()));
    connect(ui.actionDataView, SIGNAL(triggered()), this, SLOT(loadDataView()));
//    connect(ui.actionUnconnectedView, SIGNAL(triggered()), this, SLOT(loadUnconnectedView()));

//    connect(ui.actionFirmwareUpdateView, SIGNAL(triggered()), this, SLOT(loadFirmwareUpdateView()));
//    connect(ui.actionMavlinkView, SIGNAL(triggered()), this, SLOT(loadMAVLinkView()));

    connect(ui.actionReloadStylesheet, SIGNAL(triggered()), this, SLOT(reloadStylesheet()));
    connect(ui.actionSelectStylesheet, SIGNAL(triggered()), this, SLOT(selectStylesheet()));

    // Help Actions
    connect(ui.actionAutoQuad_Documentation, SIGNAL(triggered()), this, SLOT(showAQHelp()));
    connect(ui.actionQGC_AQ_Release_Notes, SIGNAL(triggered()), this, SLOT(showAQReleaseNotes()));
    connect(ui.actionOnline_Documentation, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(ui.actionDeveloper_Credits, SIGNAL(triggered()), this, SLOT(showCredits()));
//    connect(ui.actionProject_Roadmap_2, SIGNAL(triggered()), this, SLOT(showRoadMap()));

    // Custom widget actions
    connect(ui.actionNewCustomWidget, SIGNAL(triggered()), this, SLOT(createCustomWidget()));
    connect(ui.actionLoadCustomWidgetFile, SIGNAL(triggered()), this, SLOT(loadCustomWidget()));

    // Audio output
    ui.actionMuteAudioOutput->setChecked(GAudioOutput::instance()->isMuted());
    connect(GAudioOutput::instance(), SIGNAL(mutedChanged(bool)), ui.actionMuteAudioOutput, SLOT(setChecked(bool)));
    connect(ui.actionMuteAudioOutput, SIGNAL(triggered(bool)), GAudioOutput::instance(), SLOT(mute(bool)));

    // User interaction
    // NOTE: Joystick thread is not started and
    // configuration widget is not instantiated
    // unless it is actually used
    // so no ressources spend on this.
//    ui.actionJoystickSettings->setVisible(true);

    // Configuration
    // Joystick
//    connect(ui.actionJoystickSettings, SIGNAL(triggered()), this, SLOT(configure()));
    // Application Settings
    connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
}

void MainWindow::openUrlLink(const QString &url)
{
    if(!QDesktopServices::openUrl(QUrl(url)))
    {
        showInfoMessage(tr("Could not open help in browser"), tr("To get to the online help, please open %1 in a browser.").arg(url));
    }
}

void MainWindow::showAQHelp()
{
    openUrlLink("http://autoquad.org/wiki/wiki/");
}

void MainWindow::showAQReleaseNotes()
{
    openUrlLink("http://autoquad.org/wiki/wiki/whats-new/autoquad-software-changes/");
}

void MainWindow::showHelp()
{
    openUrlLink("http://qgroundcontrol.org/users/start");
}

void MainWindow::showCredits()
{
    openUrlLink("http://qgroundcontrol.org/credits");
}

//void MainWindow::showRoadMap()
//{
//    openUrlLink("http://qgroundcontrol.org/dev/roadmap");
//}

//void MainWindow::configure()
//{
//    if (!joystickWidget)
//    {
//        if (!joystick->isRunning())
//        {
//            joystick->start();
//        }
//        joystickWidget = new JoystickWidget(joystick);
//    }
//    joystickWidget->show();
//}

void MainWindow::showSettings()
{
    QGCSettingsWidget* settings = new QGCSettingsWidget(this);
    settings->show();
}

void MainWindow::addLink()
{
    SerialLink* link = new SerialLink();
    // TODO This should be only done in the dialog itself

    LinkManager::instance()->add(link);
    LinkManager::instance()->addProtocol(link, mavlink);

    // Go fishing for this link's configuration window
    QList<QAction*> actions = ui.menuNetwork->actions();

	const int32_t& linkIndex(LinkManager::instance()->getLinks().indexOf(link));
	const int32_t& linkID(LinkManager::instance()->getLinks()[linkIndex]->getId());

    foreach (QAction* act, actions)
    {
        if (act->data().toInt() == linkID)
        { // LinkManager::instance()->getLinks().indexOf(link)
            act->trigger();
            break;
        }
    }
}

void MainWindow::addLink(LinkInterface *link)
{
    // IMPORTANT! KEEP THESE TWO LINES
    // THEY MAKE SURE THE LINK IS PROPERLY REGISTERED
    // BEFORE LINKING THE UI AGAINST IT
    // Register (does nothing if already registered)
    LinkManager::instance()->add(link);
    LinkManager::instance()->addProtocol(link, mavlink);

    // Go fishing for this link's configuration window
    QList<QAction*> actions = ui.menuNetwork->actions();

    bool found(false);

	const int32_t& linkIndex(LinkManager::instance()->getLinks().indexOf(link));
	const int32_t& linkID(LinkManager::instance()->getLinks()[linkIndex]->getId());

    foreach (QAction* act, actions)
    {
        if (act->data().toInt() == linkID)
        { // LinkManager::instance()->getLinks().indexOf(link)
            found = true;
        }
    }

    //UDPLink* udp = dynamic_cast<UDPLink*>(link);

    if (!found)
    {  //  || udp
        CommConfigurationWindow* commWidget = new CommConfigurationWindow(link, mavlink, this);
        QAction* action = commWidget->getAction();
        ui.menuNetwork->addAction(action);

        // Error handling
        connect(link, SIGNAL(communicationError(QString,QString)), this, SLOT(showCriticalMessage(QString,QString)), Qt::QueuedConnection);
        // Special case for simulationlink
        MAVLinkSimulationLink* sim = dynamic_cast<MAVLinkSimulationLink*>(link);
        if (sim)
        {
            connect(ui.actionSimulate, SIGNAL(triggered(bool)), sim, SLOT(connectLink(bool)));
        }
    }
}

void MainWindow::setActiveUAS(UASInterface* uas)
{
    // Enable and rename menu
    ui.menuUnmanned_System->setTitle(uas->getUASName());
//    if (!ui.menuUnmanned_System->isEnabled()) ui.menuUnmanned_System->setEnabled(true);
}

void MainWindow::UASSpecsChanged(int uas)
{
    UASInterface* activeUAS = UASManager::instance()->getActiveUAS();
    if (activeUAS)
    {
        if (activeUAS->getUASID() == uas)
        {
            ui.menuUnmanned_System->setTitle(activeUAS->getUASName());
        }
    }
    else
    {
        // Last system deleted
        ui.menuUnmanned_System->setTitle(tr("No System"));
        ui.menuUnmanned_System->setEnabled(false);
    }
}

void MainWindow::UASCreated(UASInterface* uas)
{

    // Connect the UAS to the full user interface

    //if (uas != NULL)
    //{
        // The pilot, operator and engineer views were not available on startup, enable them now
        ui.actionPilotsView->setEnabled(true);
        ui.actionOperatorsView->setEnabled(true);
        ui.actionEngineersView->setEnabled(true);
        ui.actionDataView->setEnabled(true);
        // The UAS actions are not enabled without connection to system
        ui.actionLiftoff->setEnabled(true);
        ui.actionLand->setEnabled(true);
        ui.actionEmergency_Kill->setEnabled(true);
        ui.actionEmergency_Land->setEnabled(true);
        ui.actionShutdownMAV->setEnabled(true);

        QIcon icon;
        // Set matching icon
        switch (uas->getSystemType())
		{
        case MAV_TYPE_GENERIC:
            icon = QIcon(":files/images/mavs/generic.svg");
            break;
        case MAV_TYPE_FIXED_WING:
            icon = QIcon(":files/images/mavs/fixed-wing.svg");
            break;
        case MAV_TYPE_QUADROTOR:
            icon = QIcon(":files/images/mavs/quadrotor.svg");
            break;
        case MAV_TYPE_COAXIAL:
            icon = QIcon(":files/images/mavs/coaxial.svg");
            break;
        case MAV_TYPE_HELICOPTER:
            icon = QIcon(":files/images/mavs/helicopter.svg");
            break;
        case MAV_TYPE_ANTENNA_TRACKER:
            icon = QIcon(":files/images/mavs/antenna-tracker.svg");
            break;
        case MAV_TYPE_GCS:
            icon = QIcon(":files/images/mavs/groundstation.svg");
            break;
        case MAV_TYPE_AIRSHIP:
            icon = QIcon(":files/images/mavs/airship.svg");
            break;
        case MAV_TYPE_FREE_BALLOON:
            icon = QIcon(":files/images/mavs/free-balloon.svg");
            break;
        case MAV_TYPE_ROCKET:
            icon = QIcon(":files/images/mavs/rocket.svg");
            break;
        case MAV_TYPE_GROUND_ROVER:
            icon = QIcon(":files/images/mavs/ground-rover.svg");
            break;
        case MAV_TYPE_SURFACE_BOAT:
            icon = QIcon(":files/images/mavs/surface-boat.svg");
            break;
        case MAV_TYPE_SUBMARINE:
            icon = QIcon(":files/images/mavs/submarine.svg");
            break;
        case MAV_TYPE_HEXAROTOR:
            icon = QIcon(":files/images/mavs/hexarotor.svg");
            break;
        case MAV_TYPE_OCTOROTOR:
            icon = QIcon(":files/images/mavs/octorotor.svg");
            break;
        case MAV_TYPE_TRICOPTER:
            icon = QIcon(":files/images/mavs/tricopter.svg");
            break;
        case MAV_TYPE_FLAPPING_WING:
            icon = QIcon(":files/images/mavs/flapping-wing.svg");
            break;
        case MAV_TYPE_KITE:
            icon = QIcon(":files/images/mavs/kite.svg");
            break;
        default:
            icon = QIcon(":files/images/mavs/unknown.svg");
            break;
        }

        QAction* uasAction = new QAction(icon, tr("Select %1 for control").arg(uas->getUASName()), ui.menuConnected_Systems);
        connect(uas, SIGNAL(systemRemoved()), uasAction, SLOT(deleteLater()));
        connect(uasAction, SIGNAL(triggered()), uas, SLOT(setSelected()));
        connect(uas, SIGNAL(systemSpecsChanged(int)), this, SLOT(UASSpecsChanged(int)));

        ui.menuConnected_Systems->addAction(uasAction);

        // FIXME Should be not inside the mainwindow
        if (debugConsoleDockWidget) {
            if (DebugConsole *debugConsole = dynamic_cast<DebugConsole*>(debugConsoleDockWidget->widget()))
                connect(uas, SIGNAL(textMessageReceived(int,int,int,QString)), debugConsole, SLOT(receiveTextMessage(int,int,int,QString)));
        }

        // Health / System status indicator
        if (infoDockWidget) {
            if (UASInfoWidget *infoWidget = dynamic_cast<UASInfoWidget*>(infoDockWidget->widget()))
                infoWidget->addUAS(uas);
        }

        // UAS List
        if (listDockWidget) {
            if (UASListWidget *listWidget = dynamic_cast<UASListWidget*>(listDockWidget->widget()))
                listWidget->addUAS(uas);
        }

        // Load default custom widgets for this autopilot type
        loadCustomWidgetsFromDefaults(uas->getSystemTypeName(), uas->getAutopilotTypeName());

        // Change the view only if this is the first UAS

        // If this is the first connected UAS, it is both created as well as
        // the currently active UAS
        if (UASManager::instance()->getUASList().size() == 1) {
            // Load last view if setting is present
            int view = settings.value("QGC_MAINWINDOW/CURRENT_VIEW_WITH_UAS_CONNECTED", (int)VIEW_OPERATOR).toInt();
            switch (view) {
                case VIEW_ENGINEER:
                    loadEngineerView();
                    break;
                case VIEW_PILOT:
                    loadPilotView();
                    break;
                case VIEW_DATA:
                    loadDataView();
                    break;
                case VIEW_OPERATOR:
                default:
                    loadOperatorView();
                    break;
            }
        }

    //}

    ui.menuConnected_Systems->setEnabled(true);
//  ui.menuUnmanned_System->setEnabled(true);

    // Reload view state in case new widgets were added
    loadViewState();
}

void MainWindow::UASDeleted(UASInterface* uas)
{
    if (UASManager::instance()->getUASList().count() == 0)
    {
        // Last system deleted
        ui.menuUnmanned_System->setTitle(tr("No System"));
        ui.menuUnmanned_System->setEnabled(false);
    }

    // FIXME Should be not inside the mainwindow
    if (debugConsoleDockWidget) {
        if (DebugConsole *debugConsole = dynamic_cast<DebugConsole*>(debugConsoleDockWidget->widget()))
            disconnect(uas, 0, debugConsole, 0);
    }

    // Health / System status indicator
    if (infoDockWidget) {
        if (UASInfoWidget *infoWidget = dynamic_cast<UASInfoWidget*>(infoDockWidget->widget()))
            infoWidget->removeUAS(uas);
    }

    // UAS List
    if (listDockWidget) {
        if (UASListWidget *listWidget = dynamic_cast<UASListWidget*>(listDockWidget->widget()))
            listWidget->removeUAS(uas);
    }

    QAction* act;
    QList<QAction*> actions = ui.menuConnected_Systems->actions();
    foreach (act, actions) {
        if (act->text().contains(uas->getUASName()))
            ui.menuConnected_Systems->removeAction(act);
    }
}

/**
 * Stores the current view state
 */
void MainWindow::storeViewState()
{
    if (!aboutToCloseFlag)
    {
        // Save current state
        settings.setValue(getWindowStateKey(), saveState(QGC::applicationVersion()));
        settings.setValue(getWindowStateKey()+"CENTER_WIDGET", centerStack->currentIndex());
        // Although we want save the state of the window, we do not want to change the top-leve state (minimized, maximized, etc)
        // therefore this state is stored here and restored after applying the rest of the settings in the new
        // perspective.
        windowStateVal = this->windowState();
        settings.setValue(getWindowGeometryKey(), saveGeometry());
    }
}

void MainWindow::loadViewState()
{
    // Restore center stack state
    int index = settings.value(getWindowStateKey()+"CENTER_WIDGET", -1).toInt();
    // The offline plot view is usually the consequence of a logging run, always show the realtime view first
    /*
    if (centerStack->indexOf(dataplotWidget) == index)
    {
        // Rewrite to realtime plot
        index = centerStack->indexOf(linechartWidget);
    }
    */

    if (index != -1)
    {
        centerStack->setCurrentIndex(index);
    }
    else
    {
        // Hide custom widgets
//        if (detectionDockWidget) detectionDockWidget->hide();
//        if (watchdogControlDockWidget) watchdogControlDockWidget->hide();

        // Load defaults
        switch (currentView)
        {
        case VIEW_ENGINEER:
        case VIEW_AQ:
            centerStack->setCurrentWidget(autoquadWidget);
            //controlDockWidget->hide();
            listDockWidget->hide();
            waypointsDockWidget->hide();
            infoDockWidget->hide();
            debugConsoleDockWidget->show();
            logPlayerDockWidget->hide();
            mavlinkInspectorWidget->show();
            //mavlinkSenderWidget->show();
            //parametersDockWidget->hide();
            hsiDockWidget->hide();
            headDown1DockWidget->hide();
            //headDown2DockWidget->hide();
            rcViewDockWidget->hide();
            headUpDockWidget->hide();
            pfdDockWidget->hide();
            //video1DockWidget->hide();
            //video2DockWidget->hide();
            escTelemetryWidget->hide();
            break;
        case VIEW_PILOT:
            centerStack->setCurrentWidget(mapWidget);
            //controlDockWidget->hide();
            listDockWidget->hide();
            waypointsDockWidget->hide();
            infoDockWidget->hide();
            debugConsoleDockWidget->hide();
            logPlayerDockWidget->hide();
            mavlinkInspectorWidget->hide();
            //parametersDockWidget->hide();
            hsiDockWidget->show();
            headDown1DockWidget->hide();
            //headDown2DockWidget->show();
            rcViewDockWidget->hide();
            headUpDockWidget->show();
            pfdDockWidget->hide();
            //video1DockWidget->hide();
            //video2DockWidget->hide();
            escTelemetryWidget->hide();
            break;
        case VIEW_DATA:
            centerStack->setCurrentWidget(dataViewWidget);
            //controlDockWidget->hide();
            listDockWidget->hide();
            waypointsDockWidget->hide();
            infoDockWidget->hide();
            debugConsoleDockWidget->hide();
            logPlayerDockWidget->hide();
            mavlinkInspectorWidget->hide();
            //parametersDockWidget->hide();
            hsiDockWidget->hide();
            headDown1DockWidget->hide();
            //headDown2DockWidget->show();
            rcViewDockWidget->hide();
            headUpDockWidget->hide();
            pfdDockWidget->hide();
            //video1DockWidget->hide();
            //video2DockWidget->hide();
            escTelemetryWidget->hide();
            break;
//        case VIEW_MAVLINK:
//            centerStack->setCurrentWidget(protocolWidget);
//            //controlDockWidget->hide();
//            listDockWidget->hide();
//            waypointsDockWidget->hide();
//            infoDockWidget->hide();
//            debugConsoleDockWidget->hide();
//            logPlayerDockWidget->hide();
//            mavlinkInspectorWidget->show();
//            //mavlinkSenderWidget->show();
//            parametersDockWidget->hide();
//            hsiDockWidget->hide();
//            headDown1DockWidget->hide();
//            //headDown2DockWidget->hide();
//            rcViewDockWidget->hide();
//            headUpDockWidget->hide();
//            //video1DockWidget->hide();
//            //video2DockWidget->hide();
//            break;
//        case VIEW_FIRMWAREUPDATE:
//            centerStack->setCurrentWidget(firmwareUpdateWidget);
//            //controlDockWidget->hide();
//            listDockWidget->hide();
//            waypointsDockWidget->hide();
//            infoDockWidget->hide();
//            debugConsoleDockWidget->hide();
//            logPlayerDockWidget->hide();
//            mavlinkInspectorWidget->hide();
//            //mavlinkSenderWidget->hide();
//            parametersDockWidget->hide();
//            hsiDockWidget->hide();
//            headDown1DockWidget->hide();
//            //headDown2DockWidget->hide();
//            rcViewDockWidget->hide();
//            headUpDockWidget->hide();
//            //video1DockWidget->hide();
//            //video2DockWidget->hide();
//            break;
        case VIEW_OPERATOR:
        case VIEW_UNCONNECTED:
        case VIEW_FULL:
        default:
            centerStack->setCurrentWidget(mapWidget);
            //controlDockWidget->hide();
            listDockWidget->show();
            waypointsDockWidget->show();
            infoDockWidget->hide();
            debugConsoleDockWidget->show();
            logPlayerDockWidget->hide();
            //parametersDockWidget->hide();
            hsiDockWidget->hide();
            headDown1DockWidget->hide();
            //headDown2DockWidget->hide();
            rcViewDockWidget->hide();
            headUpDockWidget->hide();
            pfdDockWidget->show();
            //video1DockWidget->hide();
            //video2DockWidget->hide();
            mavlinkInspectorWidget->hide();
            escTelemetryWidget->hide();
            break;
        }
    }

    // Restore the widget positions and size
    if (settings.contains(getWindowStateKey()))
    {
        restoreState(settings.value(getWindowStateKey()).toByteArray(), QGC::applicationVersion());
    }
}

void MainWindow::loadEngineerView()
{
    if (currentView != VIEW_ENGINEER)
    {
        storeViewState();
        currentView = VIEW_ENGINEER;
        ui.actionEngineersView->setChecked(true);
        ui.actionEngineersView->trigger();
        loadViewState();
    }
}

void MainWindow::loadOperatorView()
{
    if (currentView != VIEW_OPERATOR)
    {
        storeViewState();
        currentView = VIEW_OPERATOR;
        ui.actionOperatorsView->setChecked(true);
        ui.actionOperatorsView->trigger();
        loadViewState();
    }
}

//void MainWindow::loadUnconnectedView()
//{
//    if (currentView != VIEW_UNCONNECTED)
//    {
//        storeViewState();
//        currentView = VIEW_UNCONNECTED;
//        ui.actionUnconnectedView->setChecked(true);
//        loadViewState();
//    }
//}

void MainWindow::loadPilotView()
{
    if (currentView != VIEW_PILOT)
    {
        storeViewState();
        currentView = VIEW_PILOT;
        ui.actionPilotsView->setChecked(true);
        ui.actionPilotsView->trigger();
        loadViewState();
    }
}

//void MainWindow::loadMAVLinkView()
//{
//    if (currentView != VIEW_MAVLINK)
//    {
//        storeViewState();
//        currentView = VIEW_MAVLINK;
//        ui.actionMavlinkView->setChecked(true);
//        loadViewState();
//    }
//}

//void MainWindow::loadFirmwareUpdateView()
//{
//    if (currentView != VIEW_FIRMWAREUPDATE)
//    {
//        storeViewState();
//        currentView = VIEW_FIRMWAREUPDATE;
//        ui.actionFirmwareUpdateView->setChecked(true);
//        loadViewState();
//    }
//}


//void MainWindow::loadAQView()
//{
//    if (currentView != VIEW_AQ)
//    {
//        storeViewState();
//        currentView = VIEW_AQ;
//        ui.actionEngineersView->setChecked(true);
//        loadViewState();
//    }
//}


void MainWindow::loadDataView()
{
    if (currentView != VIEW_DATA)
    {
        storeViewState();
        currentView = VIEW_DATA;
        ui.actionDataView->setChecked(true);
        ui.actionDataView->trigger();
        loadViewState();
    }
}


QList<QAction*> MainWindow::listLinkMenuActions(void)
{
    return ui.menuNetwork->actions();
}

#ifdef MOUSE_ENABLED_LINUX
bool MainWindow::x11Event(XEvent *event)
{
    emit x11EventOccured(event);
    //qDebug("XEvent occured...");
    return false;
}
#endif // MOUSE_ENABLED_LINUX
