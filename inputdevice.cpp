#include "inputdevice.h"
#include <QDebug>

InputDevice::InputDevice(QObject* parent):QObject(parent){
    connected = false;
    connect(&timeoutTimer, &QTimer::timeout, this, &InputDevice::timeoutDetected);
}

QStringList InputDevice::getNames()
{
    return deviceNames;
}

const DataInfo *InputDevice::getInfo()
{
    return &dataInfo;
}

void InputDevice::parseData()
{
    if(RxBuffer.size() >=4){
        struct Header *header=(Header *)RxBuffer.data();

        while(RxBuffer.size()>=(qsizetype)(header->dataSize+sizeof(struct Header))){

            if(header->status !=0x00){
                emit deviceError();
                RxBuffer.clear();
                return;
            }
            switch(header->commandID){
            case CMD_OUTPUT:
                if(header->dataSize>0){
                    QByteArray samples = RxBuffer.sliced(sizeof(struct Header), header->dataSize);
                    emit dataReady(samples);
                }
                break;
            case CMD_DEVICE_INFO:
                if(header->dataSize==sizeof(struct DataInfo)){
                    memcpy(&dataInfo, (RxBuffer.data()+sizeof(struct Header)), sizeof(struct DataInfo));
                    if(!connected){
                        connected = true;
                        emit deviceConnected();
                    }
                    else
                        emit infoUpdated();

                }
                break;
            case CMD_DEVICE_STRING:
                if(header->dataSize==sizeof(struct DataInfo)){
                    deviceString = (RxBuffer.data()+sizeof(struct Header));
                }
                break;
            case CMD_RESET:
                stopConnection();
                RxBuffer.clear();
                return;
                break;
            case CMD_CALIBRATE:{
                RxBuffer.clear();
                QByteArray command;
                command.append((uint8_t)CMD_DEVICE_INFO);
                command.append((uint8_t)0x00);
                command.append((uint8_t)0x00);
                command.append((uint8_t)0x00);
                writeData(command);
                return;
            }


                break;


            default:
                RxBuffer.clear();
                return;
                break;

            }


            RxBuffer = RxBuffer.sliced((header->dataSize+sizeof(struct Header)));
        }
    }
}

void InputDevice::onConnection()
{
    RxBuffer.clear();
    timeoutTimer.setSingleShot(true);
    timeoutTimer.setInterval(3000);
    timeoutTimer.start();
    QByteArray command;
    command.append((uint8_t)CMD_DEVICE_INFO);
    command.append((uint8_t)0x00);
    command.append((uint8_t)0x00);
    command.append((uint8_t)0x00);
    writeData(command);

}


bool InputDevice::isConnected()
{
    return connected;
}

void InputDevice::timeoutDetected()
{
    if(!connected)
        stopConnection();
}

void InputDevice::restartDevice()
{
    if(connected){
        QByteArray command;
        command.append((uint8_t)CMD_RESET);
        command.append((uint8_t)0x00);
        command.append((uint8_t)0x00);
        command.append((uint8_t)0x00);
        writeData(command);
    }
}

void InputDevice::calibrateDevice()
{
    if(connected){
        QByteArray command;
        command.append((uint8_t)CMD_CALIBRATE);
        command.append((uint8_t)0x00);
        command.append((uint8_t)0x00);
        command.append((uint8_t)0x00);
        writeData(command);
    }
}

void InputDevice::startOutput(uint8_t type)
{
    if(connected){
        QByteArray command;
        command.append((uint8_t)CMD_OUTPUT);
        command.append((uint8_t)type);
        command.append((uint8_t)0x01);
        command.append((uint8_t)0x00);
        writeData(command);
    }
}

void InputDevice::stopOutput(uint8_t type)
{
    if(connected){
        QByteArray command;
        command.append((uint8_t)CMD_OUTPUT);
        command.append((uint8_t)type);
        command.append((uint8_t)0x00);
        command.append((uint8_t)0x00);
        writeData(command);
    }
}
