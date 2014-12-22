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
 *   @brief Brief Description
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>

#include "../configuration.h"
#include "SerialLinkInterface.h"

#include "qextserialport.h"
#include "qextserialenumerator.h"

#define SERIAL_AQUSB_VENDOR_ID  1155
#define SERIAL_AQUSB_PRODUCT_ID  22352
#define SERIAL_IS_BUGGY_CP210x  (portVendorId == 4292 && portProductId == 60000)

/**
 * @brief The SerialLink class provides cross-platform access to serial links.
 * It takes care of the link management and provides a common API to higher
 * level communication layers. It is implemented as a wrapper class for a thread
 * that handles the serial communication. All methods have therefore to be thread-
 * safe.
 *
 */
class SerialLink : public SerialLinkInterface
{
    Q_OBJECT
    //Q_INTERFACES(SerialLinkInterface:LinkInterface)

public:

    SerialLink(QString portname = "",
               int baudrate=115200,
               bool flow=false,
               bool parity=false,
               int dataBits=8,
               int stopBits=1);
    ~SerialLink();

    static const int poll_interval = SERIAL_POLL_INTERVAL;  ///< ms to sleep between run() loops, defined in configuration.h
    static const int reconnect_wait_timeout = 10 * 1000;  ///< ms to wait for an automatic reconnection attempt

    bool isConnected();
    bool isPortValid(const QString &pname);
    bool isPortHandleValid();
    qint64 bytesAvailable();

    /**
     * @brief The port handle
     */
    QString getPortName();
    /**
     * @brief The human readable port name
     */
    QString getName();
    int getLinkType() { return LINK_INTERFACE_TYPE_SERIAL; }

    int getBaudRate();
    long getTimeoutMillis() { return portSettings.Timeout_Millisec; }

    // ENUM values
    int getBaudRateType();
    int getFlowType();
    int getParityType();
    int getDataBitsType();
    int getStopBitsType();

    /* Extensive statistics for scientific purposes */
    qint64 getNominalDataRate();
    qint64 getTotalUpstream();
    qint64 getCurrentUpstream();
    qint64 getMaxUpstream();
    qint64 getTotalDownstream();
    qint64 getCurrentDownstream();
    qint64 getMaxDownstream();
    qint64 getBitsSent();
    qint64 getBitsReceived();

//    void loadSettings();
//    void writeSettings();

    void run();

    int getLinkQuality();
    bool isFullDuplex();
    int getId();
    //unsigned char read();
    quint16 reconnectDelayMs() const { return m_reconnectDelayMs; }

public slots:
    /** @brief Get a list of the currently available ports */
    QVector<QString>* getCurrentPorts();

    bool setPortName(QString portName);
    bool setBaudRate(int rate);
    bool setTimeoutMillis(const long &ms);

    // Set string rate
    bool setBaudRateString(const QString& rate);

    // Set ENUM values
    bool setFlowType(int flow);
    bool setParityType(int parity);
    bool setDataBitsType(int dataBits);
    bool setStopBitsType(int stopBits);

    void readBytes();

    QextSerialPort *getPort();
    void setEsc32Mode(bool mode);
    bool getEsc32Mode();
    void readEsc32Tele();

    void linkLossExpected(const bool yes);
    void setReconnectDelayMs(const quint16 &ms);

    /**
     * @brief Write a number of bytes to the interface.
     *
     * @param data Pointer to the data byte array
     * @param size The size of the bytes array
     **/
    void writeBytes(const char* data, qint64 length);
    bool connect();
    bool disconnect();

protected slots:
//    void checkForBytes();
    bool validateConnection();
    void deviceRemoved(const QextPortInfo &pi);
    void deviceDiscovered(const QextPortInfo &pi);

protected:
    QextSerialPort * port;
    PortSettings portSettings;
    QIODevice::OpenMode portOpenMode;
    QextSerialEnumerator *portEnumerator;
    QString porthandle;
    QString name;
    int id;
    int portVendorId;
    int portProductId;

    quint64 bitsSentTotal;
    quint64 bitsSentShortTerm;
    quint64 bitsSentCurrent;
    quint64 bitsSentMax;
    quint64 bitsReceivedTotal;
    quint64 bitsReceivedShortTerm;
    quint64 bitsReceivedCurrent;
    quint64 bitsReceivedMax;
    quint64 connectionStartTime;
    QMutex statisticsMutex;
    QMutex dataMutex;
    QVector<QString>* ports;
    quint64 waitingToReconnect;  // msec while waiting to reconnect automatically, zero if not waiting
    quint16 m_reconnectDelayMs;    // msec to wait before reconnecting, after device is discovered
    bool m_linkLossExpected;

private:
	volatile bool m_stopp;
    QMutex m_stoppMutex;

    void setUsbDeviceInfo();
    void setName(QString name);
    bool hardwareConnect();

    bool mode_port;  // esc32 mode
    //char SerialIn[1];
    int countRetry;
    int maxLength;
    char data[4096];
    int rows;
    int cols;
    int firstRead;


signals:
    void aboutToCloseFlag();
    void portError();

};

#endif // SERIALLINK_H
