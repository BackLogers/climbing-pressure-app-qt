#ifndef SERIALINPUTDEVICE_H
#define SERIALINPUTDEVICE_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "inputdevice.h"


class SerialInputDevice : public InputDevice
{
    Q_OBJECT

    QList<QSerialPortInfo> serial_infos;
    QSerialPort serial_handle;
    bool baudSelected;

    void _dataReady(void);
    void _deviceError(QSerialPort::SerialPortError error);

public:
    explicit SerialInputDevice(QObject *parent = nullptr);
    ~SerialInputDevice();


public slots:
    void startScan(void);
    void startConnection(size_t index);
    void stopConnection(void);
    void writeData(const QByteArray &data);
    void baudSelect(QSerialPort::BaudRate baudRate);



};

#endif // SERIALINPUTDEVICE_H
