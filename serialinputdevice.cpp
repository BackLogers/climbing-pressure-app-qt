#include "serialinputdevice.h"
#include "QDebug"



void SerialInputDevice::_dataReady()
{
   if(serial_handle.bytesAvailable()>0){
       RxBuffer.append(serial_handle.read(serial_handle.bytesAvailable()));
       parseData();
   }

}
SerialInputDevice::~SerialInputDevice()
{
    serial_handle.close();
}
void SerialInputDevice::_deviceError(QSerialPort::SerialPortError error)
{
    qDebug() << error;
    if(error != QSerialPort::NoError)
        emit deviceError();
}

SerialInputDevice::SerialInputDevice(QObject *parent): InputDevice(parent)
{
    baudSelected=false;
    connect(&serial_handle, &QSerialPort::readyRead, this, &SerialInputDevice::_dataReady);
    connect(&serial_handle, &QSerialPort::errorOccurred, this, &SerialInputDevice::_deviceError);

}

void SerialInputDevice::startScan()
{

    if(connected){
        connected =false;
        serial_handle.close();
        emit deviceDisconnected();
    }
    deviceNames.clear();
    serial_infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : serial_infos)
        deviceNames.append(info.description()+" ("+info.portName()+")");

    emit scanComplete();
}


void SerialInputDevice::startConnection(size_t index)
{
    serial_handle.setPortName(serial_infos[index].portName());
    if(!baudSelected)
        serial_handle.setBaudRate(QSerialPort::BaudRate::Baud115200);
    if(serial_handle.open(QIODevice::ReadWrite)){
        onConnection();
    }

    else
        emit deviceError();
}

void SerialInputDevice::stopConnection()
{
    if(serial_handle.isOpen())
        serial_handle.close();
    connected=false;
    emit deviceDisconnected();
}

void SerialInputDevice::writeData(const QByteArray &data)
{
    serial_handle.write(data);
}



void SerialInputDevice::baudSelect(QSerialPort::BaudRate baudRate)
{
    if(connected){
        serial_handle.read(serial_handle.bytesAvailable());
        serial_handle.close();
    }

    serial_handle.setBaudRate(baudRate);
    baudSelected=true;

    if(connected){
        if(!serial_handle.open(QIODevice::ReadWrite)){
            connected =false;
            emit deviceError();
        }
    }


}
