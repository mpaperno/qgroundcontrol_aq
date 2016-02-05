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
 *   @brief Definition of class MainWindow
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QStatusBar>
#include <QStackedWidget>
#include <QSettings>
#include <QSplashScreen>
#include <qlist.h>

#include "configuration.h"
#include "ui_MainWindow.h"
#include "LinkInterface.h"
#include "UASInterface.h"
#if (defined MOUSE_ENABLED_WIN) | (defined MOUSE_ENABLED_LINUX)
#include "Mouse6dofInput.h"
#endif // MOUSE_ENABLED_WIN
#include "LogCompressor.h"

#include <cstring>

class QGCToolBar;
class MAVLinkProtocol;
class MAVLinkSimulationLink;
class QGCMAVLinkLogPlayer;
class MAVLinkDecoder;
class QGCMapTool;
class QGCAutoquad;
class HUD;
class QGCDataViewWidget;
#ifdef USE_GOOGLE_EARTH_PLUGIN
class QGCGoogleEarthView;
#endif
//class QGCMAVLinkMessageSender;
//class QGCFirmwareUpdate;
//class XMLCommProtocolWidget;

/**
 * @brief Main Application Window
 *
 **/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    enum QGC_MAINWINDOW_STYLE
    {
        QGC_MAINWINDOW_STYLE_NONE,
        QGC_MAINWINDOW_STYLE_NATIVE,
        QGC_MAINWINDOW_STYLE_DARK,
        QGC_MAINWINDOW_STYLE_LIGHT,
        QGC_MAINWINDOW_STYLE_PLASTIQUE,
        QGC_MAINWINDOW_STYLE_GTK,
        QGC_MAINWINDOW_STYLE_CLEANLOOKS,
        QGC_MAINWINDOW_STYLE_MAC,
        QGC_MAINWINDOW_STYLE_WIN,
        QGC_MAINWINDOW_STYLE_WINXP,
        QGC_MAINWINDOW_STYLE_WINVISTA,
        QGC_MAINWINDOW_STYLE_MOTIF,
        QGC_MAINWINDOW_STYLE_CDE,
        QGC_MAINWINDOW_STYLE_CUSTOM
    };

    /// @brief Returns the MainWindow singleton. Will not create the MainWindow if it has not already
    ///         been created.
    static MainWindow* instance(void);

    /// @brief Deletes the MainWindow singleton
    void deleteInstance(void);

    /// @brief Creates the MainWindow singleton. Should only be called once by QGCApplication.
    static MainWindow* _create(QSplashScreen* splashScreen);

    /// @brief Called to indicate that splash screen is no longer being displayed.
    //void splashScreenFinished(void) {  }

    ~MainWindow();

    /** @brief Get current visual style */
    int getStyle() { return (int)currentStyle; }
    /** @brief Get current visual style by name */
    QString getStyleName() { return m_windowStyleNames.value(currentStyle); }
    /** @brief Get a list of available style names */
    QStringList getAvailableStyles() { return m_windowStyleNames.values(); }
    /** @brief Get id of visual style by name */
    int getStyleIdByName(const QString &style) { return m_windowStyleNames.key(style); }

    /** @brief Get auto link reconnect setting */
    bool autoReconnectEnabled() { return autoReconnect; }

    /** @brief Get low power mode setting */
    bool lowPowerModeEnabled() { return lowPowerMode; }

    QString getCurrentLanguage() { return currentLanguage; }

    QGCMAVLinkLogPlayer* getLogPlayer() { return logPlayer; }

    MAVLinkProtocol* getMAVLink() { return mavlink; }

    QList<QAction*> listLinkMenuActions(void);
    QAction *getActionByLink(LinkInterface *link);

    bool getLoadDefaultStyles() const { return loadDefaultStyles; }

public slots:

    /** @brief Shows a status message on the bottom status bar */
    void showStatusMessage(const QString& status, int timeout);
    /** @brief Shows a status message on the bottom status bar */
    void showStatusMessage(const QString& status);
    /** @brief Shows an info or warning message */
    void showMessage(const QString &title, const QString &message, const QString &details, const QString severity = "info");
    /** @brief Shows a critical message as popup or as widget */
    void showCriticalMessage(const QString& title, const QString& message);
    /** @brief Shows an info message with extra details text */
    void showDetailedCriticalMessage(const QString &title, const QString &message, const QString &details);
    /** @brief Shows an info message as popup or as widget */
    void showInfoMessage(const QString& title, const QString& message);
    /** @brief Shows an info message with extra details text */
    void showDetailedInfoMessage(const QString &title, const QString &message, const QString &details);

    /** @brief Show the application settings */
    void showSettings();
    /** @brief Add a communication link */
    void addLink();
    void addLink(LinkInterface* link);
    /** @brief Set the currently controlled UAS */
    void setActiveUAS(UASInterface* uas);

    /** @brief Start a connection to simlation log */
    void simulate(bool enable);
    /** @brief Add a new UAS */
    void UASCreated(UASInterface* uas);
    /** Delete an UAS */
    void UASDeleted(UASInterface* uas);
    /** @brief Update system specs of a UAS */
    void UASSpecsChanged(int uas);
    void startVideoCapture();
    void stopVideoCapture();
    void saveScreen();

    /** @brief Load view for pilot */
    void loadPilotView();
    /** @brief Load view for config */
    void loadEngineerView();
    /** @brief Load view for mission */
    void loadOperatorView();
    /** @brief Load data/log/telemetry view */
    void loadDataView();


    void openUrlLink(const QString &url);
    void showAQHelp();
    void showAQReleaseNotes();
    /** @brief Show the online help for users */
    void showHelp();
    /** @brief Show the authors / credits */
    void showCredits();
    /** @brief Show the project roadmap */
//    void showRoadMap();

    /** @brief loads a language by the given language shortcur (e.g. de, en, ...) */
    void loadLanguage(const QString& lang);

    /** @brief Reload the CSS style sheet */
    void reloadStylesheet(const QString file = "");
    /** @brief Let the user select the CSS style sheet */
    bool selectStylesheet();
    /** @brief Automatically reconnect last link */
    void enableAutoReconnect(bool enabled);
    /** @brief Save power by reducing update rates */
    void enableLowPowerMode(bool enabled) { lowPowerMode = enabled; }

    /** @brief Switch to named style */
    void loadStyleByName(const QString style);
    /** @brief Load a specific style */
    void loadStyle(QGC_MAINWINDOW_STYLE style);
    /** @brief Set up list of available styles */
    void setAvailableStyles();

    QString getCustomStyleFile() { return customStyleFile; }
    void setCustomStyleFile(QString fileName);
    void setLoadDefaultStyles(bool value);

    /** @brief Add a custom tool widget */
    void createCustomWidget();

    /** @brief Load a custom tool widget from a file chosen by user (QFileDialog) */
    void loadCustomWidget();

    /** @brief Load a custom tool widget from a file */
    void loadCustomWidget(const QString& fileName, bool singleinstance=false);

    /** @brief Load custom widgets from default file */
    void loadCustomWidgetsFromDefaults(const QString& systemType, const QString& autopilotType);

    /** @brief Loads and shows the HIL Configuration Widget for the given UAS*/
//    void showHILConfigurationWidget(UASInterface *uas);

    void closeEvent(QCloseEvent* event);

    /**
     * @brief Shows a Docked Widget based on the action sender
     *
     * This slot is written to be used in conjunction with the addTool() function
     * It shows the QDockedWidget based on the action sender
     *
     */
    void showTool(bool visible);

    /**
     * @brief Shows a Widget from the center stack based on the action sender
     *
     * This slot is written to be used in conjunction with the addCentralWidget() function
     * It shows the Widget based on the action sender
     *
     */
    void showCentralWidget();

    /** @brief Update the window name */
    void configureWindowName();

    void launchExternalTool(QAction *action);

signals:
    void initStatusChanged(const QString& message);
    void styleChanged(int style);
#ifdef MOUSE_ENABLED_LINUX
    /** @brief Forward X11Event to catch 3DMouse inputs */
    void x11EventOccured(XEvent *event);
#endif //MOUSE_ENABLED_LINUX

protected:

    MainWindow(QSplashScreen* splashScreen = 0);

    typedef enum _VIEW_SECTIONS
    {
        VIEW_ENGINEER,
        VIEW_OPERATOR,
        VIEW_PILOT,
        VIEW_MAVLINK,           // Not used, kept for backward compat of settings
        VIEW_FIRMWAREUPDATE,    // Not used, kept for backward compat of settings
        VIEW_UNCONNECTED,       ///< View in unconnected mode, when no UAS is available
        VIEW_FULL,              ///< All widgets shown at once
        VIEW_AQ,
        VIEW_DATA
    } VIEW_SECTIONS;

    /**
     * @brief Adds an already instantiated QDockedWidget to the Tools Menu
     *
     * This function does all the hosekeeping to have a QDockedWidget added to the
     * tools menu and connects the QMenuAction to a slot that shows the widget and
     * checks/unchecks the tools menu item
     *
     * @param widget    The QDockWidget being added
     * @param title     The entry that will appear in the Menu and in the QDockedWidget title bar
     * @param location  The default location for the QDockedWidget in case there is no previous key in the settings
     */
    void addTool(QDockWidget* widget, const QString& title, Qt::DockWidgetArea location=Qt::RightDockWidgetArea);

    /**
     * @brief Adds an already instantiated QWidget to the center stack
     *
     * This function does all the hosekeeping to have a QWidget added to the tools menu
     * tools menu and connects the QMenuAction to a slot that shows the widget and
     * checks/unchecks the tools menu item. This is used for all the central widgets (those in
     * the center stack.
     *
     * @param widget        The QWidget being added
     * @param title         The entry that will appear in the Menu
     */
    void addCentralWidget(QWidget* widget, const QString& title);

    /** @brief Catch window resize events */
    void resizeEvent(QResizeEvent * event);
    /** @brief this event is called when a new translator is loaded or the system language is changed */
    void changeEvent(QEvent*);

    /** @brief Keeps track of the current view */
    VIEW_SECTIONS currentView;
    QGC_MAINWINDOW_STYLE currentStyle;
    bool aboutToCloseFlag;
    bool changingViewsFlag;

    void storeViewState();
    void loadViewState();

    void buildCustomWidget();
    void buildCommonWidgets();
    void connectCommonWidgets();
    void connectCommonActions();
	void connectSenseSoarActions();

    void loadSettings();
    void storeSettings();

    // TODO Should be moved elsewhere, as the protocol does not belong to the UI
    MAVLinkProtocol* mavlink;

    MAVLinkSimulationLink* simulationLink;
    LinkInterface* udpLink;

    QSettings settings;
    QStackedWidget *centerStack;
    QActionGroup* centerStackActionGroup;

    // Center widgets
//    QPointer<Linecharts> linechartWidget;
    QPointer<HUD> hudWidget;
    QPointer<QGCMapTool> mapWidget;
//    QPointer<XMLCommProtocolWidget> protocolWidget;
//    QPointer<QGCDataPlot2D> dataplotWidget;
	QPointer<QGCAutoquad> autoquadWidget;
    QPointer<QGCDataViewWidget> dataViewWidget;
#ifdef QGC_OSG_ENABLED
    QPointer<QWidget> _3DWidget;
#endif
#ifdef USE_GOOGLE_EARTH_PLUGIN
    QPointer<QGCGoogleEarthView> gEarthWidget;
#endif

    // Dock widgets
//    QPointer<QDockWidget> controlDockWidget;
//    QPointer<QDockWidget> controlParameterWidget;
    QPointer<QDockWidget> infoDockWidget;
//    QPointer<QDockWidget> cameraDockWidget;
    QPointer<QDockWidget> listDockWidget;
    QPointer<QDockWidget> waypointsDockWidget;
//    QPointer<QDockWidget> detectionDockWidget;
    QPointer<QDockWidget> debugConsoleDockWidget;
    QPointer<QDockWidget> parametersDockWidget;
    QPointer<QDockWidget> headDown1DockWidget;
//    QPointer<QDockWidget> headDown2DockWidget;

    QPointer<QDockWidget> headUpDockWidget;
//    QPointer<QDockWidget> video1DockWidget;
//    QPointer<QDockWidget> video2DockWidget;
//    QPointer<QDockWidget> rgbd1DockWidget;
//    QPointer<QDockWidget> rgbd2DockWidget;
    QPointer<QDockWidget> logPlayerDockWidget;

    QPointer<QDockWidget> hsiDockWidget;
    QPointer<QDockWidget> rcViewDockWidget;
    QPointer<QDockWidget> hudDockWidget;
    QPointer<QDockWidget> pfdDockWidget;

    QPointer<QGCToolBar> toolBar;

    QPointer<QDockWidget> mavlinkInspectorWidget;
    QPointer<MAVLinkDecoder> mavlinkDecoder;
    QPointer<QDockWidget> mavlinkSenderWidget;
    QPointer<QDockWidget> escTelemetryWidget;

    QGCMAVLinkLogPlayer* logPlayer;

    // Popup widgets
//    JoystickWidget* joystickWidget;

//    JoystickInput* joystick;

#ifdef MOUSE_ENABLED_WIN
    /** @brief 3d Mouse support (WIN only) */
    Mouse3DInput* mouseInput;               ///< 3dConnexion 3dMouse SDK
    Mouse6dofInput* mouse;                  ///< Implementation for 3dMouse input
#endif // MOUSE_ENABLED_WIN

#ifdef MOUSE_ENABLED_LINUX
    /** @brief Reimplementation of X11Event to handle 3dMouse Events (magellan) */
    bool x11Event(XEvent *event);
    Mouse6dofInput* mouse;                  ///< Implementation for 3dMouse input
#endif // MOUSE_ENABLED_LINUX

    /** User interface actions **/
    QAction* connectUASAct;
    QAction* disconnectUASAct;
    QAction* startUASAct;
    QAction* returnUASAct;
    QAction* stopUASAct;
    QAction* killUASAct;
    QAction* simulateUASAct;


    LogCompressor* comp;
    QString screenFileName;
    QTimer* videoTimer;
    QString styleFileName;
    QString customStyleFile;
    bool autoReconnect;
    Qt::WindowStates windowStateVal;
    bool lowPowerMode; ///< If enabled, QGC reduces the update rates of all widgets
    bool loadDefaultStyles;
//    QGCFlightGearLink* fgLink;
//    QTimer windowNameUpdateTimer;

private:
    Ui::MainWindow ui;

    QString defaultLanguage;     /**< contains the default language */
    QString currentLanguage;     /**< contains the currently loaded language */

    QMap<int, QString> m_windowStyleNames;

    void retranslateUi(/*QMainWindow *MainWindow*/);
    QString getWindowStateKey();
    QString getWindowGeometryKey();

    /** @brief creates the language menu dynamically from the content of m_langPath */
    void createLanguageMenu(void);

    /** @brief creates the external tools menu dynamically */
    void createExternalToolsMenu(void);

    //void switchTranslator(QTranslator &translator, const QString &filename);

private slots:
    /** @brief this slot is called by the language menu actions */
    void languageChanged(QAction* action);


};

#endif /* _MAINWINDOW_H_ */
