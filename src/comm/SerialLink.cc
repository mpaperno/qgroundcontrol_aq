/*=====================================================================
======================================================================*/
/**
 * @file
 *   @brief Cross-platform support for serial ports
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *   @author Maxim Paperno (max@paperno.us)
 *
 */

#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QMutexLocker>
//#include <QSerialPortInfo>

#include "SerialLink.h"
#include "LinkManager.h"
#include "QGC.h"
#include <MG.h>

SerialLink::SerialLink(QString portname, int baudRate, bool hardwareFlowControl, bool parity,
                       int dataBits, int stopBits) :
    port(0),
    portSettings(PortSettings()),
    portOpenMode(QIODevice::ReadWrite),
    portVendorId(0),
    portProductId(0),
    bitsSentTotal(0),
    bitsSentShortTerm(0),
    bitsSentCurrent(0),
    bitsSentMax(0),
    bitsReceivedTotal(0),
    bitsReceivedShortTerm(0),
    bitsReceivedCurrent(0),
    bitsReceivedMax(0),
    connectionStartTime(0),
    ports(new QVector<QString>()),
    waitingToReconnect(0),
    m_reconnectDelayMs(0),
    m_linkLossExpected(false),
    m_stopp(false),
    mode_port(false),
    countRetry(0),
    maxLength(0),
    rows(0),
    cols(0),
    firstRead(0)
{
    portEnumerator = new QextSerialEnumerator();
    portEnumerator->setUpNotifications();
    QObject::connect(portEnumerator, SIGNAL(deviceDiscovered(const QextPortInfo &)), this, SLOT(deviceDiscovered(const QextPortInfo &)));
    QObject::connect(portEnumerator, SIGNAL(deviceRemoved(const QextPortInfo &)), this, SLOT(deviceRemoved(const QextPortInfo &)));

    getCurrentPorts();

    // Setup settings
    this->porthandle = portname.trimmed();
    if (!ports->contains(porthandle))
        porthandle = "";
//    if (this->porthandle == "" && ports->size() > 0)
//        this->porthandle = ports->first();

    // Set unique ID and add link to the list of links
    this->id = getNextLinkId();

    int par = parity ? (int)PAR_EVEN : (int)PAR_NONE;
    int fc = hardwareFlowControl ? (int)FLOW_HARDWARE : (int)FLOW_OFF;

    setBaudRate(baudRate);
    setFlowType(fc);
    setParityType(par);
    setDataBitsType(dataBits);
    setStopBitsType(stopBits);
    setTimeoutMillis(-1);  // -1 means do not block on serial read/write. Do not use zero.
    setReconnectDelayMs(10);  // default 10ms before reconnecting to M4, after detecting that COM port is back

    // Set the port name
    name = this->porthandle.length() ? this->porthandle : tr("Serial Link ") + QString::number(getId());

    QObject::connect(this, SIGNAL(portError()), this, SLOT(disconnect()));

}

SerialLink::~SerialLink()
{
    this->disconnect();
    if(port)
        delete port;
    if (ports)
        delete ports;
    port = NULL;
    ports = NULL;
}

QVector<QString>* SerialLink::getCurrentPorts()
{
    ports->clear();
    foreach (const QextPortInfo &p, portEnumerator->getPorts()) {
        if (p.portName.length())
            ports->append(p.portName);//  + " - " + p.friendName);
//      qDebug() << p.portName  << p.friendName << p.physName << p.enumName << p.vendorID << p.productID;
    }

    return this->ports;
}

/**
 * @brief Runs the thread
 *
 **/
void SerialLink::run()
{
    // Initialize the connection
    if (!hardwareConnect())
        return;

    while (!m_stopp)
    {
        this->readBytes();

        // Serial data isn't arriving that fast normally, this saves the thread
        // from consuming too much processing time
        msleep(SerialLink::poll_interval);
    }
}

bool SerialLink::validateConnection() {
    bool ok = this->isConnected() && (!port->lastError() || (port->lastError() == E_READ_FAILED && SERIAL_IS_BUGGY_CP210x));
    if (ok && (portOpenMode & QIODevice::ReadOnly) && !port->isReadable())
        ok = false;
    if (ok && (portOpenMode & QIODevice::WriteOnly) && !port->isWritable())
        ok = false;
    if(!ok) {
        emit portError();
        if (!m_linkLossExpected)
            emit communicationError(this->getName(), tr("Link %1 unexpectedly disconnected!").arg(this->porthandle));
        qWarning() << ok << port->lastError() << port->errorString();
        //this->disconnect();
        return false;
    }
    return true;
}

void SerialLink::deviceRemoved(const QextPortInfo &pi)
{
    bool isValid = isPortHandleValid();

    if (!isValid && pi.vendorID == SERIAL_AQUSB_VENDOR_ID && pi.productID == SERIAL_AQUSB_PRODUCT_ID)
        waitingToReconnect = MG::TIME::getGroundTimeNow();

    //qDebug() <<  pi.portName  << pi.friendName << pi.physName << pi.enumName << pi.vendorID << pi.productID << getPortName() << waitingToReconnect;

    if (!port || !port->isOpen() || isValid)
        return;

    emit portError();
    if (!m_linkLossExpected)
        emit communicationError(this->getName(), tr("Link %1 unexpectedly disconnected!").arg(this->porthandle));
    //qWarning() << __FILE__ << __LINE__ << "device removed" << port->lastError() << port->errorString();
}

void SerialLink::deviceDiscovered(const QextPortInfo &pi)
{
//    qDebug() <<  pi.portName  << pi.friendName << pi.physName << pi.enumName << pi.vendorID << pi.productID << getPortName()
//             << waitingToReconnect << MG::TIME::getGroundTimeNow() << MG::TIME::getGroundTimeNow() - waitingToReconnect;
    Q_UNUSED(pi);
    if (waitingToReconnect && !port) {
        if (MG::TIME::getGroundTimeNow() - waitingToReconnect > reconnect_wait_timeout) {
            waitingToReconnect = 0;
            return;
        }
        if (isPortHandleValid()) {
            QTimer::singleShot(m_reconnectDelayMs, this, SLOT(connect()));
            waitingToReconnect = 0;
            //this->connect();
        }
    }
}

void SerialLink::writeBytes(const char* data, qint64 size)
{
    if (!validateConnection())
        return;
    int b = port->write(data, size);

    if (b > 0) {
        // Increase write counter
        bitsSentTotal += b * 8;
        //qDebug() << "Serial link " << this->getName() << "transmitted" << b << "bytes:";
    } else if (b == -1) {
        emit portError();
        if (!m_linkLossExpected)
            emit communicationError(this->getName(), tr("Could not send data - error on link %1").arg(this->porthandle));
    }
}

/**
 * @brief Read a number of bytes from the interface.
 *
 * @param data Pointer to the data byte array to write the bytes to
 * @param maxLength The maximum number of bytes to write
 **/
void SerialLink::readBytes()
{
    if (!validateConnection())
        return;

    if (mode_port)  // ESC32 special data read mode
        return readEsc32Tele();

    const qint64 maxLength = 2048;
    char data[maxLength];
    qint64 numBytes = 0, rBytes = 0; //port->bytesAvailable();

    dataMutex.lock();
    while ((numBytes = port->bytesAvailable())) {
    //if(rBytes) {
        //qDebug() << "numBytes: " << numBytes;
        /* Read as much data in buffer as possible without overflow */
        rBytes = numBytes;
        if(maxLength < rBytes) rBytes = maxLength;

        if (port->read(data, rBytes) == -1) { // -1 result means error
            emit portError();
            if (!m_linkLossExpected)
                emit communicationError(this->getName(), tr("Could not read data - link %1 is disconnected!").arg(this->getName()));
            return;
        }

        QByteArray b(data, rBytes);
        emit bytesReceived(this, b);
        bitsReceivedTotal += rBytes * 8;

//        for (int i=0; i<rBytes; i++){
//            unsigned int v=data[i];
//            fprintf(stderr,"%02x ", v);
//        } fprintf(stderr,"\n");
    }
    dataMutex.unlock();
}

void SerialLink::readEsc32Tele(){
    //dataMutex.lock();
    qint64 numBytes = 0;

    if (!validateConnection())
        return;

    // Clear up the input Buffer
    if ( this->firstRead == 1) {
        while(true) {
            port->read(data,1);
            if (data[0] == 'A') {
                break;
            }
        }
        this->firstRead = 2;
    }
    retryA:
    if (!port->isOpen() || !mode_port)
        return;

    while( true) {
        numBytes = port->bytesAvailable();
        if ( numBytes > 0)
            break;
        msleep(1);
        if (!port->isOpen() || !mode_port)
            break;
    }
    if ( this->firstRead == 0) {
        numBytes = port->read(data,1);
        if ( numBytes  >= 1 ) {
            if ( data[0] != 'A') {
                goto retryA;
            }
        }
        else if (numBytes <= 0){
            msleep(5);
            goto retryA;
        }
    }
    else {
         this->firstRead = 0;
    }

    while( true) {
        numBytes = port->bytesAvailable();
        if ( numBytes > 0)
            break;
        msleep(1);
        if (!port->isOpen() || !mode_port)
            break;
    }
    numBytes = port->read(data,1);
    if ( numBytes  == 1 ) {
        if ( data[0] != 'q') {
            goto retryA;
        }
    }
    else if (numBytes <= 0){
        msleep(5);
        goto retryA;
    }

    while( true) {
        numBytes = port->bytesAvailable();
        if ( numBytes > 0)
            break;
        msleep(1);
        if (!port->isOpen() || !mode_port)
            break;
    }
    numBytes = port->read(data,1);
    if ( numBytes  == 1 ) {
        if ( data[0] != 'T') {
            msleep(5);
            goto retryA;
        }
    }
    else if (numBytes <= 0){
        msleep(5);
        goto retryA;
    }

    while( true) {
        numBytes = port->bytesAvailable();
        if ( numBytes >= 2)
            break;
        msleep(1);
        if (!port->isOpen() || !mode_port)
            break;
    }
    port->read(data,2);
    rows = 0;
    cols = 0;
    rows = data[0];
    cols = data[1];
    int length_array = (((cols*rows)*sizeof(float))+2);
    if ( length_array > 300) {
        qDebug() << "bad col " << cols << " rows " << rows;
        goto retryA;
    }
    while( true) {
        numBytes = port->bytesAvailable();
        if ( numBytes >= length_array)
            break;
        msleep(1);
        if (!port->isOpen() || !mode_port)
            break;
    }
    //qDebug() << "avalible" << numBytes;
    numBytes = port->read(data,length_array);
    if (numBytes == length_array ){
        QByteArray b(data, numBytes);
        emit teleReceived(b, rows, cols);
    }

    goto retryA;

    //dataMutex.unlock();
}

/**
 * @brief Disconnect the connection.
 *
 * @return True if connection has been disconnected, false if connection couldn't be disconnected.
 **/
bool SerialLink::disconnect()
{
    if(this->isRunning() && !m_stopp) {
        m_stoppMutex.lock();
        this->m_stopp = true;
        //m_waitCond.wakeOne();
        m_stoppMutex.unlock();
        this->wait();
    }

    if (this->isConnected())
        port->close();

    if (port) {
        port->deleteLater();
        port = NULL;
    }

    portVendorId = 0;
    portProductId = 0;

    emit disconnected();
    emit connected(false);
    return !this->isConnected();
}

/**
 * @brief Connect the connection.
 *
 * @return True if connection has been established, false if connection couldn't be established.
 **/
bool SerialLink::connect()
{
    if (this->isRunning()) {
        this->disconnect();
        this->wait();
    }

    // reset the run stop flag
    m_stoppMutex.lock();
    m_stopp = false;
    m_stoppMutex.unlock();

    waitingToReconnect = 0;
    m_linkLossExpected = false;

    this->start(QThread::LowPriority);
    return this->isRunning();
}

/**
 * @brief This function is called indirectly by the connect() call.
 *
 * The connect() function starts the thread and indirectly calls this method.
 *
 * @return True if the connection could be established, false otherwise
 * @see connect() For the right function to establish the connection.
 **/
bool SerialLink::hardwareConnect()
{
    if (!isPortHandleValid()) {
        emit communicationError(this->getName(), tr("Failed to open serial port %1 because it no longer exists in the system.").arg(porthandle));
        return false;
    }

    if (!port) {
        port = new QextSerialPort(QextSerialPort::Polling);
        if (!port) {
            emit communicationError(this->getName(), tr("Could not create serial port object."));
            return false;
        }
        QObject::connect(port, SIGNAL(aboutToClose()), this, SIGNAL(disconnected()));
        //QObject::connect(port, SIGNAL(readyRead()), this, SLOT(readBytes()), Qt::DirectConnection);
    }

    if (port->isOpen())
        port->close();

    QString err;

    port->setPortName(porthandle);
    port->setBaudRate(portSettings.BaudRate);
    port->setDataBits(portSettings.DataBits);
    port->setParity(portSettings.Parity);
    port->setStopBits(portSettings.StopBits);
    port->setFlowControl(portSettings.FlowControl);
    port->setTimeout(portSettings.Timeout_Millisec);

    if (!port->open(portOpenMode)) {
        err = tr("Failed to open serial port %1").arg(this->porthandle);
        if (port->lastError() != E_NO_ERROR)
            err = err % tr(" with error: %1 (%2)").arg(port->errorString()).arg(port->lastError());
        else
            err = err % tr(". It may already be in use, please check your connections.");
    }

    if (err.length()) {
        emit communicationError(this->getName(), err);
        qWarning() << err << port->portName() << port->baudRate() << "db:" << port->dataBits() \
                   << "p:" << port->parity() << "sb:" << port->stopBits() << "fc:" << port->flowControl();

        if (port->isOpen())
            port->close();
        port->deleteLater();
        port = NULL;

        return false;
    }

    connectionStartTime = MG::TIME::getGroundTimeNow();
    setUsbDeviceInfo();

    if(isConnected()) {
        emit connected();
        emit connected(true);
//        writeSettings();
        qDebug() << "Connected Serial" << porthandle  << "with settings" \
                 << port->portName() << port->baudRate() << "db:" << port->dataBits() \
                 << "p:" << port->parity() << "sb:" << port->stopBits() << "fc:" << port->flowControl();
    } else
        return false;

    return true;
}

void SerialLink::setUsbDeviceInfo()
{
    foreach (const QextPortInfo &p, portEnumerator->getPorts()) {
        if (p.portName == port->portName()) {
            portVendorId = p.vendorID;
            portProductId = p.productID;
            return;
        }
    }

}


// verify that a port still exists on the system
bool SerialLink::isPortValid(const QString &pname) {
//    QSerialPortInfo pi(pname);
//    return pi.isValid();
    return getCurrentPorts()->contains(pname);
}

// verify that the current port still exists on the system
bool SerialLink::isPortHandleValid() {
    return isPortValid(porthandle);
}

bool SerialLink::isConnected()
{
    if (port)
        return port->isOpen(); //isPortHandleValid() &&
    else
        return false;
}


// currently unused
qint64 SerialLink::bytesAvailable()
{
    if (port) {
        return port->bytesAvailable();
    } else {
        return 0;
    }
}

// not used at the moment, doesn't detect disconnect with "native" USB connection (eg. AQ M4)
//void SerialLink::handleError(QSerialPort::SerialPortError error)
//{
//    //qDebug() << __FILE__ << __LINE__ << error << port->error() << port->errorString();
//    if (error == QSerialPort::ResourceError) {
//        emit communicationError(this->getName(), tr("Link %1 unexpectedly disconnected with error: %2").arg(this->getName()).arg(port->errorString()));
//        this->disconnect();
//    }
//}


//
// Setter methods
//

bool SerialLink::setPortName(QString portName)
{
    portName = portName.trimmed();
    if(this->porthandle != portName && this->isPortValid(portName)) {
        if (isConnected())
            this->disconnect();
        if (isRunning())
            this->wait();

        this->porthandle = portName;
//        loadSettings();
        setName(tr("serial port ") + portName);
    }
    return true;
}


void SerialLink::setName(QString name)
{
    if (this->name != name) {
        this->name = name;
        emit nameChanged(this->name);
    }
}

// doesn't seem to be used anywhere
bool SerialLink::setBaudRateString(const QString& rate)
{
    bool ok;
    int intrate = rate.toInt(&ok);
    if (!ok) return false;
    return setBaudRate(intrate);
}

bool SerialLink::setBaudRate(int rate)
{
    bool accepted = false;
    BaudRateType br = (BaudRateType)(rate);
    if (br) {
        portSettings.BaudRate = br;
        if (port)
            port->setBaudRate(portSettings.BaudRate);
        accepted = true;
    }
    return accepted;
}

bool SerialLink::setTimeoutMillis(const long &ms)
{
    portSettings.Timeout_Millisec = ms;
    if (port)
        port->setTimeout(portSettings.Timeout_Millisec);
    return true;
}

bool SerialLink::setFlowType(int flow)
{
    bool accepted = false;
    if (flow >= (int)FLOW_OFF && flow <= (int)FLOW_XONXOFF) {
        portSettings.FlowControl = (FlowType)flow;
        if (port)
            port->setFlowControl(portSettings.FlowControl);
        accepted = true;
    }
    return accepted;
}

bool SerialLink::setParityType(int parity)
{
    bool accepted = false;
    if (parity >= (int)PAR_NONE && parity <= (int)PAR_SPACE) {
        portSettings.Parity = (ParityType)parity;
        if (port)
            port->setParity(portSettings.Parity);
        accepted = true;
    }
    return accepted;
}


bool SerialLink::setDataBitsType(int dataBits)
{
    bool accepted = false;
    if (dataBits >= (int)DATA_5 && dataBits <= (int)DATA_8) {
        portSettings.DataBits = (DataBitsType)dataBits;
        if (port)
            port->setDataBits(portSettings.DataBits);
        accepted = true;
    }
    return accepted;
}

bool SerialLink::setStopBitsType(int stopBits)
{
    bool accepted = false;
    if (stopBits == 1 || stopBits == 2) {
        portSettings.StopBits = stopBits == 1 ? STOP_1 : STOP_2;
        if (port)
            port->setStopBits(portSettings.StopBits);
        accepted = true;
    }
    return accepted;
}

void SerialLink::setEsc32Mode(bool mode) {
    mode_port = mode;
    if ( mode_port ) {
        this->rows = 0;
        this->cols = 0;
        firstRead = 1;
    }
}

void SerialLink::setReconnectDelayMs(const quint16 &ms)
{
    m_reconnectDelayMs = ms;
}

void SerialLink::linkLossExpected(const bool yes)
{
    m_linkLossExpected = yes;
}

//
// Misc. getters
//

QextSerialPort * SerialLink::getPort() {
    return port;
}

int SerialLink::getId()
{
    return id;
}

QString SerialLink::getName()
{
    return name;
}

bool SerialLink::getEsc32Mode() {
    return mode_port;
}

qint64 SerialLink::getNominalDataRate()
{
    return (qint64)portSettings.BaudRate;
}

qint64 SerialLink::getTotalUpstream()
{
    QMutexLocker locker(&statisticsMutex);
    return bitsSentTotal / ((MG::TIME::getGroundTimeNow() - connectionStartTime) / 1000);
}

qint64 SerialLink::getCurrentUpstream()
{
    return 0; // TODO
}

qint64 SerialLink::getMaxUpstream()
{
    return 0; // TODO
}

qint64 SerialLink::getBitsSent()
{
    return bitsSentTotal;
}

qint64 SerialLink::getBitsReceived()
{
    return bitsReceivedTotal;
}

qint64 SerialLink::getTotalDownstream()
{
    QMutexLocker locker(&statisticsMutex);
    return bitsReceivedTotal / ((MG::TIME::getGroundTimeNow() - connectionStartTime) / 1000);
}

qint64 SerialLink::getCurrentDownstream()
{
    return 0; // TODO
}

qint64 SerialLink::getMaxDownstream()
{
    return 0; // TODO
}

bool SerialLink::isFullDuplex()
{
    /* Serial connections are always half duplex */
    return false;
}

int SerialLink::getLinkQuality()
{
    /* This feature is not supported with this interface */
    return -1;
}

QString SerialLink::getPortName()
{
    return porthandle;
}

int SerialLink::getBaudRate()
{
    return getNominalDataRate();
}

int SerialLink::getBaudRateType()
{
    return getNominalDataRate();
}

int SerialLink::getFlowType()
{
    return (int)portSettings.FlowControl;
}

int SerialLink::getParityType()
{
    return (int)portSettings.Parity;
}

int SerialLink::getDataBitsType()
{
    return (int)portSettings.DataBits;
}

int SerialLink::getStopBitsType()
{
    switch (portSettings.StopBits) {
    case STOP_1 :
        return 1;
    case STOP_2 :
        return 2;
    default :
        return -1;
    }
}

//int SerialLink::getDataBits()
//{
//    return (int)portSettings.DataBits;
//}

//int SerialLink::getStopBits()
//{
//    switch (portSettings.StopBits) {
//    case STOP_1 :
//    case STOP_1_5 :
//        return 1;
//    case STOP_2 :
//        return 2;
//    }
//}
