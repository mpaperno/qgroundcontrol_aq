#include "AQEsc32.h"
#include "MG.h"
#include <QTimer>


AQEsc32::AQEsc32()
{
    fastSend = false;
    esc32BinaryMode = 0;
    esc32DoCommand = 0;
    StepMessageFromEsc32 = 1;
    TelemetryFrequenzy = 1000;
    esc32state = 0;
    esc32dataLogger = NULL;
    bootModeTimer = new QTimer(this);
}

AQEsc32::~AQEsc32()
{
}

void AQEsc32::Connect(QString port, QString baud)
{
    TimerState = 0;
    ExitCalibration = 0;
    fastSend = false;
    currentError = false;

    seriallinkEsc32 = new SerialLink(port, baud.toInt(), false, false, 8, 1);
    seriallinkEsc32->setEsc32Mode(false);
    connect(seriallinkEsc32, SIGNAL(connected()), this, SLOT(connectedEsc32()));
    connect(seriallinkEsc32, SIGNAL(communicationError(QString,QString)), this, SLOT(communicationErrorEsc32(QString,QString)));
    connect(seriallinkEsc32, SIGNAL(disconnected()), this, SLOT(disconnectedEsc32()));
    connect(seriallinkEsc32, SIGNAL(destroyed()), this, SLOT(destroyedEsc32()));
    connect(seriallinkEsc32, SIGNAL(bytesReceived(LinkInterface*, QByteArray)), this, SLOT(BytesRceivedEsc32(LinkInterface*, QByteArray)));
    seriallinkEsc32->connect();

    checkEsc32State = new QTimer(this);
    checkEsc32State->setInterval(10);
    checkEsc32State->stop();
    connect(checkEsc32State, SIGNAL(timeout()), this, SLOT(checkEsc32StateTimeOut()));

    if ( !esc32dataLogger )
        esc32dataLogger = new AQEsc32Logger();
}

void AQEsc32::Disconnect()
{
        seriallinkEsc32->setEsc32Mode(false);
        if (esc32BinaryMode) {
            SleepThread(500);
            SwitchFromBinaryToAscii();
            SleepThread(500);
        }
        checkEsc32State->stop();
        checkEsc32State = NULL;
        if ( esc32dataLogger )
            esc32dataLogger = NULL;

        seriallinkEsc32->disconnect();
}

void AQEsc32::SavePara(QString ParaName, QVariant ParaValue) {
    if ( !SwitchFromAsciiToBinary() )
        return;

    for ( int i = 0; i<5; i++) {
        StepMessageFromEsc32 = 4;
        ParaNameLastSend = ParaName;
        ParaLastSend = BINARY_COMMAND_SET;
        int paraToWrite =  getEnumByName(ParaName);
        float valueToWrite = ParaValue.toFloat();
        command_ACK_NACK = 0;
        esc32SendCommand(BINARY_COMMAND_SET,paraToWrite,valueToWrite,2);

        TimeOutWaiting = 0;
        while ( command_ACK_NACK != 250) {
            QCoreApplication::processEvents();
            SleepThread(5);
            TimeOutWaiting++;
            if (TimeOutWaiting > 100 ) {
                qDebug() << "Timeout " << i+1 << "of 5";
                break;
            }
        }
        if (command_ACK_NACK == 250 ){
            //qDebug() << "command ACK = 250";
            break;
        }

    }
}

void AQEsc32::sendCommand(int command, float Value1, float Value2, int num, bool withOutCheck ){
    if ( !SwitchFromAsciiToBinary() )
        return;

    command_ACK_NACK = 0;
    for ( int i = 0; i<5; i++) {
        StepMessageFromEsc32 = 4;
        ParaNameLastSend = "";
        ParaLastSend = command;
        LastParaValueSend1 = Value1;
        LastParaValueSend2 = Value2;
        esc32SendCommand(command,Value1,Value2,num);
        /*
        if ( i == 0)
            qDebug() << "Send command";
        else
            qDebug() << "Send command again";
        */
        if ( !withOutCheck ) {
            TimeOutWaiting = 0;
            while ( command_ACK_NACK != 250) {
                SleepThread(1);
                TimeOutWaiting++;
                if (TimeOutWaiting > 500 ) {
                    qDebug() << "Timeout " << i+1 << "of 5";
                    break;
                }
            }
        }
        else {
            command_ACK_NACK = 250;
            break;
        }

        if (command_ACK_NACK == 250 ){
            qDebug() << "command ACK = 250";
            break;
        }
    }
}

int AQEsc32::GetEsc32State() {
    return esc32state;
}

int AQEsc32::getEnumByName(QString Name)
{
    if ( Name == "CONFIG_VERSION")
        return CONFIG_VERSION;
    if ( Name == "STARTUP_MODE")
        return STARTUP_MODE;
    if ( Name == "BAUD_RATE")
        return BAUD_RATE;
    if ( Name == "PTERM")
        return PTERM;
    if ( Name == "ITERM")
        return ITERM;
    if ( Name == "FF1TERM")
        return FF1TERM;
    if ( Name == "FF2TERM")
        return FF2TERM;
    if ( Name == "CL1TERM")
        return CL1TERM;
    if ( Name == "CL2TERM")
        return CL2TERM;
    if ( Name == "CL3TERM")
        return CL3TERM;
    if ( Name == "CL4TERM")
        return CL4TERM;
    if ( Name == "CL5TERM")
        return CL5TERM;
    if ( Name == "SHUNT_RESISTANCE")
        return SHUNT_RESISTANCE;
    if ( Name == "MIN_PERIOD")
        return MIN_PERIOD;
    if ( Name == "MAX_PERIOD")
        return MAX_PERIOD;
    if ( Name == "BLANKING_MICROS")
        return BLANKING_MICROS;
    if ( Name == "ADVANCE")
        return ADVANCE;
    if ( Name == "START_VOLTAGE")
        return START_VOLTAGE;
    if ( Name == "GOOD_DETECTS_START")
        return GOOD_DETECTS_START;
    if ( Name == "BAD_DETECTS_DISARM")
        return BAD_DETECTS_DISARM;
    if ( Name == "MAX_CURRENT")
        return MAX_CURRENT;
    if ( Name == "SWITCH_FREQ")
        return SWITCH_FREQ;
    if ( Name == "MOTOR_POLES")
        return MOTOR_POLES;
    if ( Name == "PWM_MIN_PERIOD")
        return PWM_MIN_PERIOD;
    if ( Name == "PWM_MAX_PERIOD")
        return PWM_MAX_PERIOD;
    if ( Name == "PWM_MIN_VALUE")
        return PWM_MIN_VALUE;
    if ( Name == "PWM_LO_VALUE")
        return PWM_LO_VALUE;
    if ( Name == "PWM_HI_VALUE")
        return PWM_HI_VALUE;
    if ( Name == "PWM_MAX_VALUE")
        return PWM_MAX_VALUE;
    if ( Name == "PWM_MIN_START")
        return PWM_MIN_START;
    if ( Name == "PWM_RPM_SCALE")
        return PWM_RPM_SCALE;
    if ( Name == "FET_BRAKING")
        return FET_BRAKING;
    if ( Name == "PNFAC")
        return PNFAC;
    if ( Name == "INFAC")
        return INFAC;
    if ( Name == "THR1TERM")
        return THR1TERM;
    if ( Name == "THR2TERM")
        return THR2TERM;
    if ( Name == "START_ALIGN_TIME")
        return START_ALIGN_TIME;
    if ( Name == "START_ALIGN_VOLTAGE")
        return START_ALIGN_VOLTAGE;
    if ( Name == "START_STEPS_NUM")
        return START_STEPS_NUM;
    if ( Name == "START_STEPS_PERIOD")
        return START_STEPS_PERIOD;
    if ( Name == "START_STEPS_ACCEL")
        return START_STEPS_ACCEL;
    if ( Name == "PWM_LOWPASS")
        return PWM_LOWPASS;
    if ( Name == "RPM_MEAS_LP")
        return RPM_MEAS_LP;
    if ( Name == "SERVO_DUTY")
        return SERVO_DUTY;
    if ( Name == "SERVO_P")
        return SERVO_P;
    if ( Name == "SERVO_D")
        return SERVO_D;
    if ( Name == "SERVO_MAX_RATE")
        return SERVO_MAX_RATE;
    if ( Name == "SERVO_SCALE")
        return SERVO_SCALE;
    if ( Name == "ESC_ID")
        return ESC_ID;
    if ( Name == "DIRECTION")
        return DIRECTION;

    if ( Name == "CONFIG_NUM_PARAMS")
        return CONFIG_NUM_PARAMS;

    return 0;
}

void AQEsc32::connectedEsc32(){
    emit Esc32Connected();
}

void AQEsc32::disconnectedEsc32(){
    disconnect(seriallinkEsc32, 0, this, 0);
    seriallinkEsc32->deleteLater();
    seriallinkEsc32 = NULL;
    emit ESc32Disconnected();
}

void AQEsc32::destroyedEsc32(){
}

void AQEsc32::communicationErrorEsc32(QString err1, QString err2){
    Q_UNUSED(err1);
    Q_UNUSED(err2);
}

void AQEsc32::BytesRceivedEsc32(LinkInterface* link, QByteArray bytes){
    // Only add data from current link

    if ( link == seriallinkEsc32)
    {
        //unsigned char byte = bytes.at(j);
        switch (StepMessageFromEsc32)
        {
            case 0:
                // Parse all bytes
            break;

            //get ascii values for parameter
            case 1:
                LIST_MessageFromEsc32.append(QString(bytes));
                if ( LIST_MessageFromEsc32.contains("Command not found")) {
                    LIST_MessageFromEsc32 = "";
                    qDebug() << "Command not found";
                    return;
                }

                if ( LIST_MessageFromEsc32.contains("\r\n\r\n")) {
                    //decodeParameterFromEsc32(LIST_MessageFromEsc32);
                    emit ShowConfig(LIST_MessageFromEsc32);
                    //qDebug() << LIST_MessageFromEsc32;
                    LIST_MessageFromEsc32 = "";
                    SwitchFromAsciiToBinary();
                }
            break;

            // Waiting for a commit to ASCII mode
            case 2:
                ParaWriten_MessageFromEsc32.append(QString(bytes));
                ResponseFromEsc32.append(bytes);
                indexOfAqC = ParaWriten_MessageFromEsc32.indexOf("AqC");
                if (indexOfAqC > -1) {
                    checkInA = checkInB = 0;
                    int in = indexOfAqC+3;
                    commandLengthBack = ResponseFromEsc32[in];
                    esc32InChecksum(commandLengthBack);
                    in++;
                    command_ACK_NACK = ResponseFromEsc32[in];
                    esc32InChecksum(command_ACK_NACK);
                    in++;
                    commandSeqIdBack = ResponseFromEsc32[in];
                    esc32InChecksum(commandSeqIdBack);
                    in++;
                    commandBack = ResponseFromEsc32[in];
                    esc32InChecksum(commandBack);
                    in++;
                    unsigned char tmp_A = ResponseFromEsc32[in];
                    unsigned char tmp_B = ResponseFromEsc32[in+1];
                    if ((checkInA == tmp_A ) && (checkInB == tmp_B)) {
                        if ( command_ACK_NACK == 250) {
                            esc32BinaryMode = 0;
                            esc32state = 0;
                        }
                    }
                    ResponseFromEsc32.clear();
                    ParaWriten_MessageFromEsc32="";
                }

            break;

            // Waiting for a commit to binary mode
            case 3:
                ParaWriten_MessageFromEsc32.append(QString(bytes));
                if ( ParaWriten_MessageFromEsc32.contains("command mode...\r\n")) {
                    esc32BinaryMode = 1;
                    ParaWriten_MessageFromEsc32 = "";
                    esc32state = 1;
                }
            break;

            //Waiting for commit of send Parameter
            case 4:
                ParaWriten_MessageFromEsc32.append(QString(bytes));
                ResponseFromEsc32.append(bytes);
                indexOfAqC = ParaWriten_MessageFromEsc32.indexOf("AqC");
                reCheck:
                if (indexOfAqC > -1) {
                    checkInA = checkInB = 0;
                    int in = indexOfAqC+3;
                    commandLengthBack = ResponseFromEsc32[in];
                    esc32InChecksum(commandLengthBack);
                    in++;
                    command_ACK_NACK = ResponseFromEsc32[in];
                    esc32InChecksum(command_ACK_NACK);
                    in++;
                    commandSeqIdBack = ResponseFromEsc32[in];
                    //qDebug() << "commandSeqIdBack=" << commandSeqIdBack;
                    esc32InChecksum(commandSeqIdBack);
                    in++;
                    commandBack = ResponseFromEsc32[in];
                    //qDebug() << "commandBack=" << commandBack;
                    esc32InChecksum(commandBack);
                    in++;
                    unsigned char tmp_A = ResponseFromEsc32[in];
                    unsigned char tmp_B = ResponseFromEsc32[in+1];
                    if ((checkInA == tmp_A ) && (checkInB == tmp_B)) {
                        if ( ParaLastSend == BINARY_COMMAND_SET )
                            emit Esc32ParaWritten(ParaNameLastSend);
                        else
                            emit Esc32CommandWritten(ParaLastSend,LastParaValueSend1,LastParaValueSend2);

                        emit getCommandBack(commandBack);

                        if ( ParaLastSend == BINARY_COMMAND_START)
                            esc32state = 2;
                        if ( ParaLastSend == BINARY_COMMAND_STOP )
                            esc32state = 3;
                        if ( ParaLastSend == BINARY_COMMAND_ARM )
                            esc32state = 4;
                        if ( ParaLastSend == BINARY_COMMAND_DISARM )
                            esc32state = 5;
                    }
                    if ( ( in + 5) < ResponseFromEsc32.count()) {
                        indexOfAqC = ParaWriten_MessageFromEsc32.indexOf("AqC",in);
                        if ( indexOfAqC > -1 )
                            goto reCheck;
                    }
                    ResponseFromEsc32.clear();
                    ParaWriten_MessageFromEsc32="";
                }

            break;

            case 5:
                BootloaderMessage.append(QString(bytes));
                if ( BootloaderMessage.contains("Rebooting in boot loader mode...") ) {
                    bootModeTimer->stop();
                    emit EnteredBootMode();
                }
                else if ( BootloaderMessage.contains("ESC armed, disarm first") || BootloaderMessage.contains("Command not found") ) {
                    bootModeTimer->stop();
                    emit NoBootModeArmed(BootloaderMessage.remove(QRegExp("[^\\w\\s]")));
                }

                //qDebug() << BootloaderMessage;
            break;

            case 6:
                if (bytes.contains("ver")) {
                    firmwareVersion.append(bytes);
                    firmwareVersion.remove(QRegExp("(esc32|[^\\d\\.\\-])", Qt::CaseInsensitive));
                } else
                    firmwareVersion = "[unknown]";

                emit GotFirmwareVersion(firmwareVersion);
            break;

            default:
            break;
        }
    }
}

void AQEsc32::SetToBootMode() {
    QByteArray transmit;
    if (!SwitchFromBinaryToAscii())
        return;
    bootloaderInitReturned = false;
    BootloaderMessage = "";
    SleepThread(500);
    StepMessageFromEsc32 = 5;
    transmit.append("bootloader\n");
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return;
    seriallinkEsc32->writeBytes(transmit,transmit.size());

    bootModeTimer->setSingleShot(true);
    bootModeTimer->start(5000);
    connect(bootModeTimer, SIGNAL(timeout()), this, SLOT(emitBootModeTimeout()));
}

void AQEsc32::emitBootModeTimeout() {
    emit BootModeTimeout();
}

void AQEsc32::CheckVersion() {
    QByteArray transmit;
    if (!SwitchFromBinaryToAscii())
        return;
    SleepThread(500);
    StepMessageFromEsc32 = 6;
    transmit.append("version\n");
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return;
    seriallinkEsc32->writeBytes(transmit,transmit.size());
}

void AQEsc32::ReadConfigEsc32() {
    QByteArray transmit;
    if (!SwitchFromBinaryToAscii())
        return;
    SleepThread(500);
    StepMessageFromEsc32 = 1;
    transmit.append("set list\n");
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return;
    seriallinkEsc32->writeBytes(transmit,transmit.size());
}

int AQEsc32::SwitchFromAsciiToBinary()
{
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return 0;
    if (esc32BinaryMode)
        return 1;

    StepMessageFromEsc32 = 3;
    QByteArray transmit;
    transmit.append("binary\n");
    seriallinkEsc32->writeBytes(transmit,transmit.size());
    TimeOutWaiting = 0;
    while ( esc32BinaryMode == 0) {
        QCoreApplication::processEvents();
        TimeOutWaiting++;
        if (TimeOutWaiting > 100000 ) {
            qDebug() << "Error switch to binary mode!";
            return 0;
        }
    }
    return 1;
}

int AQEsc32::SwitchFromBinaryToAscii()
{
    if (!esc32BinaryMode)
        return 1;

    StepMessageFromEsc32 = 2;
    commandSeqIdBack = esc32SendCommand(BINARY_COMMAND_CLI, 0.0, 0.0, 0);
    TimeOutWaiting = 0;
    while ( esc32BinaryMode == 1) {
        QCoreApplication::processEvents();
        TimeOutWaiting++;
        if (TimeOutWaiting > 100000 ) {
            qDebug() << "Error switch to ascii mode!";
            return 0;
        }
    }
    return 1;
}

int AQEsc32::esc32SendCommand(unsigned char command, float param1, float param2, int n)
{
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return 0;
    QByteArray transmit;
    ResponseFromEsc32.clear();
    ParaWriten_MessageFromEsc32 = "";
    transmit.append("A");
    transmit.append("q");
    //################################
    seriallinkEsc32->writeBytes(transmit,transmit.size());
    checkOutA = 0;
    checkOutB = 0;
    command_ACK_NACK = 0;
    esc32SendChar(1 + 2 + n*sizeof(float));
    esc32SendChar(command);
    esc32SendShort(commandSeqId++);
    if (n > 0)
        esc32SendFloat(param1);
    if (n > 1)
        esc32SendFloat(param2);
    transmit.clear();
    transmit.append(checkOutA);
    transmit.append(checkOutB);
    seriallinkEsc32->writeBytes(transmit,transmit.size());

    return (commandSeqId - 1);
}

void AQEsc32::esc32SendChar(unsigned char c) {
    if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
        return;
    QByteArray transmit;
    transmit.append(c);
    seriallinkEsc32->writeBytes(transmit,transmit.size());
    esc32OutChecksum(c);
}

void AQEsc32::esc32SendShort(unsigned short i) {
    unsigned char j;
    unsigned char *c = (unsigned char *)&i;

    for (j = 0; j < sizeof(short); j++)
        esc32SendChar(*c++);
}

void AQEsc32::esc32SendFloat(float f) {
    unsigned char j;
    unsigned char *c = (unsigned char *)&f;

    for (j = 0; j < sizeof(float); j++)
        esc32SendChar(*c++);
}

void AQEsc32::esc32OutChecksum(unsigned char c) {
    checkOutA += c;
    checkOutB += checkOutA;
}

void AQEsc32::esc32InChecksum(unsigned char c) {
    checkInA += c;
    checkInB += checkInA;
}

SerialLink *AQEsc32::getSerialLink(){
    return seriallinkEsc32;
}

void AQEsc32::StartCalibration(float MaxCurrent, QString LogFile, QString ResFile) {
    Q_UNUSED(LogFile);
    LoggingFile = ""; //LogFile
    ResultFile = ResFile;
    maximum_Current = MaxCurrent;
    CommandBack = -1;
    TimerState = -2;
    ExitCalibration = 0;


    if ( ResultFile != "" )     {
        ResultFile = QDir::toNativeSeparators(ResultFile);
        #ifdef Q_OS_WIN
            calResultFile = fopen(ResultFile.toLocal8Bit().constData(),"w+");
        #else
            calResultFile = fopen(ResultFile.toLocal8Bit().constData(),"w+");
        #endif
    }

    //Timer starten
    checkEsc32State->start();
}

void AQEsc32::StopCalibration(bool withEmergencyExit) {
    if ( withEmergencyExit)
        ExitCalibration = 98;
    sendCommand(BINARY_COMMAND_STOP, 0.0, 0.0, 0, true);
    sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0, 0.0, 1, true);
    sendCommand(BINARY_COMMAND_DISARM, 0.0, 0.0, 0, true);

}

void AQEsc32::StartLogging(){

	if (!seriallinkEsc32 || !seriallinkEsc32->isConnected())
		return;

    if (!seriallinkEsc32->getEsc32Mode()) {
        seriallinkEsc32->setEsc32Mode(false);
        sendCommand(BINARY_COMMAND_NOP, 0.0f, 0.0f, 0, false);

        sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0, 0.0, 1, false);
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 0.0f, BINARY_VALUE_VOLTS_BAT, 2, false);
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 1.0f, BINARY_VALUE_VOLTS_BAT, 2, false);
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 2.0f, BINARY_VALUE_VOLTS_BAT, 2, false);

        if ( esc32dataLogger ) {
            disconnect(seriallinkEsc32, SIGNAL(bytesReceived(LinkInterface*, QByteArray)), this, SLOT(BytesRceivedEsc32(LinkInterface*, QByteArray)));
            float freq = 1000;
            esc32dataLogger->startLoggingTelemetry(seriallinkEsc32,"");
            seriallinkEsc32->setEsc32Mode(true);
            sendCommand(BINARY_COMMAND_TELEM_RATE, freq, 0.0f, 1, true);
            esc32dataLogger->StartStopDecoding(true);
        }

    }
    else {
        sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0, 0.0, 1, true);
        SleepThread(1000);
        esc32dataLogger->StartStopDecoding(false);
        seriallinkEsc32->setEsc32Mode(false);
        esc32dataLogger->stopLoggingTelemetry();
        SleepThread(1000);
        connect(this->seriallinkEsc32, SIGNAL(bytesReceived(LinkInterface*, QByteArray)), this, SLOT(BytesRceivedEsc32(LinkInterface*, QByteArray)));
    }

}

float AQEsc32::getFF1Term() {
    return FF1Term;
}

float AQEsc32::getFF2Term() {
    return FF2Term;
}

float AQEsc32::getCL1() {
    return CurrentLimiter1;
}

float AQEsc32::getCL2(){
    return CurrentLimiter2;
}

float AQEsc32::getCL3() {
    return CurrentLimiter3;
}

float AQEsc32::getCL4() {
    return CurrentLimiter4;
}

float AQEsc32::getCL5() {
    return CurrentLimiter5;
}

void AQEsc32::SetCalibrationMode(int mode) {
    calibrationMode = mode;
}

bool AQEsc32::RpmToVoltage(float maxAmps) {
    Eigen::MatrixXd A(2,2);
    Eigen::MatrixXd c(2,1);
    Eigen::MatrixXd ab(2,1);
    Eigen::MatrixXd data(100, 3);
    float f;
    int j;
    int i;
    FF1Term = 0;
    FF2Term = 0;
    f = 0;
    j = 0;
    i = 0;
    // reset max current
    SleepThread(100);
    if ( ExitCalibration != 0)
        return true;
    // reset Max Current
    esc32dataLogger->setTelemValueMaxs(0, 0.0f);
    esc32dataLogger->setTelemValueMaxs(1, 0.0f);
    esc32dataLogger->setTelemValueMaxs(2, 0.0f);
    currentError = false;

    printf("\n%5s %5s %5s\n", "RPM", "VOLTS", "AMPS");
    fprintf(calResultFile, "\n%5s %5s %5s\n", "RPM", "VOLTS", "AMPS");
    esc32dataLogger->StartStopDecoding(true);
    for (f = 4; f <= 100.0; f += 2.0) {
        sendCommand(BINARY_COMMAND_DUTY, f, 0.0f, 1, true);

        SleepThread(((100.0f - f) / 3.0f * 1e6 * 0.15)/1000);
        if ( ExitCalibration != 0)
            break;
        data(j,0) = esc32dataLogger->getTelemValueAvgs(0);
        data(j,1) = esc32dataLogger->getTelemValueAvgs(1);
        data(j,2) = esc32dataLogger->getTelemValueAvgs(2);
        printf("%5.0f %5.2f %5.2f\n", data(j,0), data(j,1), data(j,2));
        fprintf(calResultFile, "%5.0f %5.2f %5.2f\n", data(j,0), data(j,1), data(j,2));

        j++;

        if (esc32dataLogger->getTelemValueMaxs(2) > maxAmps){
            qDebug() << "max current = " << esc32dataLogger->getTelemValueMaxs(2);
            currentError = false;
            break;
        }
    }

    sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0, 0.0, 1, true);
    SleepThread(1000);
    esc32dataLogger->StartStopDecoding(false);
    sendCommand(BINARY_COMMAND_STOP, 0.0, 0.0, 0, true);
    SleepThread(1000);
    sendCommand(BINARY_COMMAND_DISARM, 0.0, 0.0, 0, true);
    qDebug() << "Stopping";

    if ( esc32dataLogger->getTelemStorageNum() <= 0) {
        qDebug() << "No new Value in TelemStorage";
        return true;
    }

    A.setZero();
    c.setZero();
    for (i = 0; i < j; i++) {
        A(0, 0) += data(i,0)*data(i,0)*data(i,0)*data(i,0);
        A(0, 1) += data(i,0)*data(i,0)*data(i,0);
        A(1, 0) += data(i,0)*data(i,0)*data(i,0);
        A(1, 1) += data(i,0)*data(i,0);

        c(0) += data(i,0)*data(i,0)*data(i,1);
        c(1) += data(i,0)*data(i,1);
    }

    ab = A.inverse() * c;

    FF1Term = ab(0,0);
    FF2Term = ab(1,0);

    A.setZero();
    c.setZero();
    ab.setZero();
    data.setZero();
    qDebug() << "FF1Term =" << FF1Term;
    qDebug() << "FF2Term =" << FF2Term;

    fprintf(calResultFile, "FF1TERM\t%f\n", FF1Term);
    fprintf(calResultFile, "FF2TERM\t%f\n", FF2Term);
    fflush(calResultFile);
    fclose(calResultFile);
    calResultFile = NULL;
    return false;
}

bool AQEsc32::CurrentLimiter(float maxAmps) {
Eigen::MatrixXd A;
Eigen::MatrixXd c;
Eigen::MatrixXd ab;
Eigen::MatrixXd X;
int m,n;
int i, j, k, z;


    z = 0;
    qDebug() << "Starting...\n";
    sendCommand(BINARY_COMMAND_START, 0.0, 0.0, 0, true);
    SleepThread(1);
    currentError = false;
    if ( ExitCalibration != 0)
        return true;

    for (i = 10; i <= 90; i += 5) {
        qDebug() << "reset maxCurrent";
        esc32dataLogger->setTelemValueMaxs(2, 0.0f);
        for (j = i+5; j <= 100; j += 5) {
            esc32dataLogger->setTelemValueMaxs(2, 0.0f);
            stepUp((float)i, (float)j);
            printf("Duty %d to %d, MAX current = %f\n", i, j, esc32dataLogger->getTelemValueMaxs(2));
            fprintf(calResultFile, "Duty %d to %d, MAX current = %f\n", i, j, esc32dataLogger->getTelemValueMaxs(2));
            if ( ExitCalibration != 0)
                break;
            if (esc32dataLogger->getTelemValueMaxs(2) > maxAmps) {
                break;
            }
        }
        if ( ExitCalibration != 0)
            break;

        if (esc32dataLogger->getTelemValueMaxs(2) > maxAmps && j == i+5) {
            break;
        }
    }

    sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0, 0.0, 1, true);
    SleepThread(1);
    sendCommand(BINARY_COMMAND_STOP, 0.0, 0.0, 0, true);
    SleepThread(1);
    sendCommand(BINARY_COMMAND_DISARM, 0.0, 0.0, 0, true);
    qDebug() << "Stopping";

    if ( esc32dataLogger->getTelemStorageNum() <= 0) {
        qDebug() << "Aborted esc32dataLogger->getTelemStorageNum()";
        return true;
    }

    n = esc32dataLogger->getTelemStorageNum();
    m = 5;
    X.setZero(n, m);
    A.setZero(m, m);
    c.setZero(m, 1);

    for (k = 0; k < n; k++) {
        X(k, 0) = 1.0;
        X(k, 1) = esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*0 + k);
        X(k, 2) = esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*2 + k);
        X(k, 3) = esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*0 + k)*sqrt(fabs(esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*2 + k)));
        X(k, 4) = sqrt(fabs(esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*2 + k)));
    }

    for (i = 0; i < m; i++) {
        for (j = 0; j < m; j++)
            for (k = 0; k < n; k++)
                A(i, j) += X(k, i) * X(k, j);

        for (k = 0; k < n; k++)
            c(i, 0) += X(k, i) * esc32dataLogger->getTelemStorage(MAX_TELEM_STORAGE*1 + k);
    }

    ab = A.inverse() * c;

    CurrentLimiter1 = ab(0,0);
    CurrentLimiter2 = ab(1,0);
    CurrentLimiter3 = ab(2,0);
    CurrentLimiter4 = ab(3,0);
    CurrentLimiter5 = ab(4,0);

    fprintf(calResultFile, "CL1\t%f\n", CurrentLimiter1);
    fprintf(calResultFile, "CL2\t%f\n", CurrentLimiter2);
    fprintf(calResultFile, "CL3\t%f\n", CurrentLimiter3);
    fprintf(calResultFile, "CL4\t%f\n", CurrentLimiter4);
    fprintf(calResultFile, "CL5\t%f\n", CurrentLimiter5);
    fflush(calResultFile);
    fclose(calResultFile);
    calResultFile = NULL;
    A.setZero();
    c.setZero();
    ab.setZero();
    X.setZero();
    return false;
}

void AQEsc32::stepUp(float start, float end) {

    sendCommand(BINARY_COMMAND_DUTY, start, 0.0f, 1.0f, true);
    SleepThread(1500);
    if ( ExitCalibration != 0)
        return;
    sendCommand(BINARY_COMMAND_TELEM_RATE, TelemetryFrequenzy, 0.0f, 1, true);
    SleepThread(500);
    if ( ExitCalibration != 0)
        return;
    sendCommand(BINARY_COMMAND_DUTY, end, 0.0f, 1.0f, true);
    esc32dataLogger->StartStopDecoding(true);
    SleepThread(200);
    if ( ExitCalibration != 0)
        return;
    sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0f, 0.0f, 1, true);
    SleepThread(500);
    esc32dataLogger->StartStopDecoding(false);
    if ( ExitCalibration != 0)
        return;
}

void AQEsc32::checkEsc32StateTimeOut() {
    if ( TimerState == -2) {
        qDebug() << "Send a Nop for testing com";
        sendCommand(BINARY_COMMAND_NOP, 0.0f, 0.0f, 0, false);
        TimerState = -1;
    }

    //Wait for binary nop
    if ( TimerState == -1 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            sendCommand(BINARY_COMMAND_ARM, 0.0f, 0.0f, 0, false);
            TimerState = 0;
            qDebug() << "send BINARY_COMMAND_ARM";
        }
    }

    //Wait for Arming
    if ( TimerState == 0 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            TimerState = 1;
            qDebug() << "get BINARY_COMMAND_ARM";
        }
    }
    // Send Stop to eSC32
    else if ( TimerState == 1 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_STOP, 0.0f, 0.0f, 0, false);
        TimerState = 2;
        qDebug() << "send BINARY_COMMAND_STOP";
    }

    //Wait for Stop
    else if ( TimerState == 2 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            TimerState = 3;
            qDebug() << "get BINARY_COMMAND_STOP";
        }
    }
    // Send Tele rate to eSC32
    else if ( TimerState == 3 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_TELEM_RATE, 0.0f, 0.0f, 1, false);
        qDebug() << "send BINARY_COMMAND_TELEM_RATE";
        TimerState = 4;
    }
    //Wait for Tele rate
    else if ( TimerState == 4 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_TELEM_RATE";
            TimerState = 5;
        }
    }

    // Send Tele values to eSC32
    else if ( TimerState == 5 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 0.0f, BINARY_VALUE_RPM, 2, false);
        qDebug() << "send BINARY_COMMAND_TELEM_VALUE";
        TimerState = 6;
    }
    //Wait for Tele value
    else if ( TimerState == 6 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_TELEM_VALUE";
            TimerState = 7;
        }
    }

    // Send Tele values to eSC32
    else if ( TimerState == 7 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 1.0f, BINARY_VALUE_VOLTS_MOTOR, 2, false);
        qDebug() << "send BINARY_COMMAND_TELEM_VALUE";
        TimerState = 8;
    }
    //Wait for Tele value
    else if ( TimerState == 8 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_TELEM_VALUE";
            TimerState = 9;
        }
    }


    // Send Tele values to eSC32
    else if ( TimerState == 9 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_TELEM_VALUE, 2.0f, BINARY_VALUE_AMPS, 2, false);
        qDebug() << "send BINARY_COMMAND_TELEM_VALUE";
        TimerState = 10;
    }
    //Wait for Tele value
    else if ( TimerState == 10 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_TELEM_VALUE";
            TimerState = 11;
        }
    }


    // Send Tele values to eSC32
    else if ( TimerState == 11 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_SET, MAX_CURRENT, 0.0f, 2, false);
        qDebug() << "send BINARY_COMMAND_SET";
        TimerState = 12;
    }
    //Wait for Max Currrent
    else if ( TimerState == 12 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_SET";
            TimerState = 13;
        }
    }


    // Send Tele start to eSC32
    else if ( TimerState == 13 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_START, 0.0f, 0.0f, 0, false);
        qDebug() << "send BINARY_COMMAND_START";
        TimerState = 14;
    }
    //Wait for start
    else if ( TimerState == 14 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_START";
            TimerState = 15;
        }
    }

    // Send Tele duty to eSC32
    else if ( TimerState == 15 ) {
        CommandBack = -1;
        sendCommand(BINARY_COMMAND_DUTY, 4.0f, 0.0f, 1, false);
        qDebug() << "send BINARY_COMMAND_DUTY";
        TimerState = 16;
    }
    //Wait for start
    else if ( TimerState == 16 ) {
        if ((CommandBack == 0) || (command_ACK_NACK == 250)){
            CommandBack = -1;
            qDebug() << "get BINARY_COMMAND_DUTY";
            TimerState = 17;
        }
    }

    // Send Tele start to eSC32
    else if ( TimerState == 17 ) {
        CommandBack = -1;
        disconnect(this->seriallinkEsc32, SIGNAL(bytesReceived(LinkInterface*, QByteArray)), this, SLOT(BytesRceivedEsc32(LinkInterface*, QByteArray)));
        esc32dataLogger->startLoggingTelemetry(this->seriallinkEsc32, "");
        seriallinkEsc32->setEsc32Mode(true);
        if ( calibrationMode == 1) {
            sendCommand(BINARY_COMMAND_TELEM_RATE, TelemetryFrequenzy, 0.0, 1, true);
            TimerState = 18;
        }
        else if ( calibrationMode == 2) {
            TimerState = 19;
        }
        else {
            TimerState = 20;
        }
    }
    //Wait for start
    else if ( TimerState == 18 ) {
        qDebug() << "Start RPMToVoltag";
        RpmToVoltage(maximum_Current);
        TimerState = 20;
    }
    else if ( TimerState == 19 ) {
        qDebug() << "Start Current Limiter";
        CurrentLimiter(maximum_Current);
        TimerState = 20;
    }
    else if ( TimerState == 20 ) {
        CommandBack = -1;
        TimerState = -2;
        //Timer starten
        checkEsc32State->stop();
        seriallinkEsc32->setEsc32Mode(false);
        SleepThread(1000);
        esc32dataLogger->stopLoggingTelemetry();
        SleepThread(1000);
        connect(this->seriallinkEsc32, SIGNAL(bytesReceived(LinkInterface*, QByteArray)), this, SLOT(BytesRceivedEsc32(LinkInterface*, QByteArray)));
        if ( ExitCalibration != 0)
            emit finishedCalibration(ExitCalibration);
        else
            emit finishedCalibration(calibrationMode);
        ExitCalibration = 0;
    }
}

void AQEsc32::SetCommandBack(int Command) {
    CommandBack = Command;
}

void AQEsc32::SleepThread(int msec) {
    //MG::SLEEP::msleep(msec);

    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dieTime){
        if ( ExitCalibration != 0)
            return;
        QCoreApplication::processEvents();
    }

}


//#######################################################################################################


AQEsc32Logger::AQEsc32Logger() {
}

AQEsc32Logger::~AQEsc32Logger() {
    free(TelemStorage);
}

void AQEsc32Logger::startLoggingTelemetry(SerialLink *link, QString FileName) {
    TelemStorage = (float *)calloc(MAX_TELEM_STORAGE, sizeof(float)*3);
    seriallinkEsc32 = link;
    TelemStorageNum = 0;
    bitsReceivedTotal = 0;
    StartStop = false;
    TelemOutFile = NULL;
    if ( FileName != "" )     {
        FileName = QDir::toNativeSeparators(FileName);
        #ifdef Q_OS_WIN
            TelemOutFile = fopen(FileName.toLocal8Bit().constData(),"w+");
        #else
            TelemOutFile = fopen(FileName.toLocal8Bit().constData(),"w+");
        #endif
    }
    connect(seriallinkEsc32, SIGNAL(teleReceived(QByteArray,int,int)), this, SLOT(teleDataReceived(QByteArray,int,int)));
}

void AQEsc32Logger::StartStopDecoding(bool Start) {
    dataMutex.lock();
    this->StartStop = Start;
    dataMutex.unlock();
}

void AQEsc32Logger::teleDataReceived(QByteArray data, int rows, int cols){
    checkInA =  0;
    checkInB = 0;
    int i,j;
    esc32InChecksum(rows);
    esc32InChecksum(cols);
    int count_For_measuring = 0;
    for (i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            TelemData[i][j] = esc32GetFloat(data,count_For_measuring);
            count_For_measuring += sizeof(float);
        }
    }
    unsigned char tmp_A  = data[count_For_measuring+0];
    unsigned char tmp_B  = data[count_For_measuring+1];

    if ((checkInA == tmp_A ) && (checkInB == tmp_B)) {
        bitsReceivedTotal++;
        if (( MG::TIME::getGroundTimeNow() - connectionStartTime) > 1000) {
            //qDebug() << "get " << bitsReceivedTotal << "Messages " << rows << " " << cols;
            bitsReceivedTotal = 0;
            connectionStartTime = MG::TIME::getGroundTimeNow();
        }
        if (!StartStop)
            return;

        // update averages
        for (i = 0; i < rows; i++){
            for (j = 0; j < cols; j++){
                //Average
                TelemValueAvgs[j] -= (TelemValueAvgs[j] - TelemData[i][j]) * 0.01;
            }
        }
        // update Max
        for (i = 0; i < rows; i++){
            for (j = 0; j < cols; j++){
                //Max
                if (TelemValueMaxs[j] < TelemData[i][j]){
                    TelemValueMaxs[j] = TelemData[i][j];
                }
            }
        }
        // save to memory
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                TelemStorage[MAX_TELEM_STORAGE*j + TelemStorageNum] = TelemData[i][j];
            }
            if ( TelemStorageNum < MAX_TELEM_STORAGE)
                TelemStorageNum++;
            else
                TelemStorageNum = MAX_TELEM_STORAGE;
        }

    }
}

void AQEsc32Logger::stopLoggingTelemetry() {
    disconnect(seriallinkEsc32, SIGNAL(teleReceived(QByteArray,int,int)), this, SLOT(teleDataReceived(QByteArray,int,int)));
    if (TelemOutFile) {
        fclose(TelemOutFile);
    }
}

void AQEsc32Logger::run() {
}

unsigned short AQEsc32Logger::esc32GetShort(QByteArray data, int startIndex) {
    unsigned short s;
    for ( unsigned i =0; i< sizeof(unsigned short); i++) {
        esc32InChecksum(data[startIndex+i]);
    }
    QByteArray b = data.mid(startIndex,sizeof(unsigned short));
    unsigned short * buf = (unsigned short *) b.data();
    s = *buf;
    return s;
}

float AQEsc32Logger::esc32GetFloat(QByteArray data, int startIndex) {

    float f;
    unsigned char *c = (unsigned char *)&f;
    unsigned int i;

    for (i = 0; i < sizeof(float); i++) {
        esc32InChecksum(data[startIndex+i]);
        *c++ = data[startIndex+i];
    }
    return f;
}

void AQEsc32Logger::esc32InChecksum(unsigned char c) {
    checkInA += c;
    checkInB += checkInA;
}

float AQEsc32Logger::getTelemValueAvgs(int index){
    dataMutex.lock();
    float ret;
    if ( TelemValueAvgs)
        ret = TelemValueAvgs[index];
    else
        ret = 0.0;
    dataMutex.unlock();
    return ret;
}

void AQEsc32Logger::setTelemValueMaxs(int index ,float value) {
    dataMutex.lock();
    TelemValueMaxs[index] = value;
    dataMutex.unlock();
}

float AQEsc32Logger::getTelemValueMaxs(int index){
    dataMutex.lock();
    float ret = TelemValueMaxs[index];
    dataMutex.unlock();
    return ret;
}

float AQEsc32Logger::getTelemStorage(int index) {
    dataMutex.lock();
    float ret = TelemStorage[index];
    dataMutex.unlock();
    return ret;
}

int AQEsc32Logger::getTelemStorageNum() {
    return TelemStorageNum;
}
