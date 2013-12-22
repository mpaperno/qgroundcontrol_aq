/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009, 2010 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of main class
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */


#ifndef QGC_CORE_H
#define QGC_CORE_H

#include <QApplication>

#include "MainWindow.h"
#include "UASManager.h"
#include "LinkManager.h"
/*#include "ViconTarsusProtocol.h" */
#ifdef OPAL_RT
    #include "OpalLink.h"
#endif

class QDir;
class QTranslator;

typedef QHash<QString, QTranslator*> Translators;

/**
 * @brief The main application and management class.
 *
 * This class is started by the main method and provides
 * the central management unit of the groundstation application.
 *
 **/
class QGCCore : public QApplication
{
    Q_OBJECT

public:
    QGCCore(int &argc, char* argv[]);
    ~QGCCore();

    static void loadTranslations();
    static void loadTranslations(const QDir& dir);
    static const QStringList availableLanguages();
    static const QString getLangFilePath();

    static QString langPath;     /**< Path of language files.  */

public slots:
    static void setLanguage(const QString& locale);

protected:
    void startLinkManager();

    /**
     * @brief Start the robot managing system
     *
     * The robot manager keeps track of the configured robots.
     **/
    void startUASManager();

private:
    MainWindow* mainWindow;
    static QTranslator* current;
    static Translators translators;
};

#endif /* _CORE_H_ */
