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
    const QString APP_VERSION_TXT = "1.6.2";
    const unsigned APP_VERSION = 0x01602000; // bytes: maj|min|patch|build
}

#endif // QGC_CONFIGURATION_H
