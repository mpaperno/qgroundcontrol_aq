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
 *   @brief Main executable
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QApplication>
#include "QGCCore.h"
#include "MainWindow.h"
#include "configuration.h"
#ifdef Q_OS_WIN
#include <crtdbg.h>
#endif

/* SDL does ugly things to main() */
#ifdef main
#undef main
#endif


#ifdef Q_OS_WIN

/// @brief Message handler which is installed using qInstallMsgHandler so you do not need
/// the MSFT debug tools installed to see qDebug(), qWarning(), qCritical and qAbort
#if QT_VERSION >= 0x050000
void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void msgHandler( QtMsgType type, const char* msg )
#endif
{
    const char symbols[] = { 'I', 'E', '!', 'X' };
#if QT_VERSION >= 0x050000
    QString output = QString("[%1] at %2:%3 - \"%4\"").arg(symbols[type]).arg(context.file).arg(context.line).arg(msg);
#else
    QString output = QString("[%1] %2").arg( symbols[type] ).arg( msg );
#endif
    std::cerr << output.toStdString() << std::endl;
    if( type == QtFatalMsg ) abort();

}

/// @brief CRT Report Hook installed using _CrtSetReportHook. We install this hook when
/// we don't want asserts to pop a dialog on windows.
int WindowsCrtReportHook(int reportType, char* message, int* returnValue)
{
    Q_UNUSED(reportType);
    
    std::cerr << message << std::endl;  // Output message to stderr
    *returnValue = 0;                   // Don't break into debugger
    return true;                        // We handled this fully ourselves
}

#endif

/**
 * @brief Starts the application
 *
 * @param argc Number of commandline arguments
 * @param argv Commandline arguments
 * @return exit code, 0 for normal exit and !=0 for error cases
 */

int main(int argc, char *argv[])
{

#ifdef Q_OS_MAC
    // Prevent Apple's app nap from screwing us over
    // tip: the domain can be cross-checked on the command line with <defaults domains>
    QProcess::execute("defaults write " % QGCAUTOQUAD::APP_DOMAIN % ".qgroundcontrol NSAppSleepDisabled -bool YES");
#endif

    // install the message handler
#ifdef Q_OS_WIN
    #if QT_VERSION >= 0x050000
        qInstallMessageHandler(msgHandler);
    #else
        qInstallMsgHandler( msgHandler );
    #endif
#endif

    QGCCore core(argc, argv);
    return core.exec();
}
