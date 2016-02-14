#ifndef QGC_CONFIGURATION_H
#define QGC_CONFIGURATION_H

#include <QString>

/** @brief Polling interval in ms */
#define SERIAL_POLL_INTERVAL 9

/** @brief Heartbeat emission rate, in Hertz (times per second) */
#define MAVLINK_HEARTBEAT_DEFAULT_RATE 1

#define QGC_ORGANIZATION_NAME   "QGroundControl"
#define QGC_DOMAIN_NAME         "org.qgroundcontrol"
#define QGC_APPLICATION_NAME    "QGroundControl"
#define QGC_APPLICATION_VERSION "v. 1.0.2 (beta)"

#ifndef STYLES_PATH
    #define STYLES_PATH  ":/files/styles/"
#endif
#if !defined(STYLES_DEFAULT_FILE) && !defined(STYLES_NO_DEFAULT)
    #define STYLES_DEFAULT_FILE  STYLES_PATH "style-default.css"
#endif

namespace QGC

{
const QString APPNAME = "QGROUNDCONTROL";
const QString COMPANYNAME = "QGROUNDCONTROL";
const int APPLICATIONVERSION = 102; // 1.0.1
}

namespace QGCAUTOQUAD {
    const QString APP_NAME = "QGroundControl for AutoQuad";
    const QString APP_ORG = "AutoQuad";
    const QString APP_DOMAIN = "org.autoquad";
    const QString APP_VERSION_TXT = "1.7 BETA 1";
    const unsigned APP_VERSION = 0x01700004; // bytes: maj|min|patch|build
}

#endif // QGC_CONFIGURATION_H
