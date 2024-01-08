#ifndef BLEINPUTDEVICE_H
#define BLEINPUTDEVICE_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QStandardPaths>
#include <QFile>
#include <QTimer>
#include <QBluetoothLocalDevice>
#include "inputdevice.h"

#include "deviceinfo.h"

#define QPP_SERVICE_UUID "0000fee9-0000-1000-8000-00805f9b34fb"
#define QPP_RX_UUID "d44bc439-abfd-45a2-b575-925416129601"
#define QPP_TX_UUID "d44bc439-abfd-45a2-b575-925416129600"

class BLEInputDevice : public InputDevice
{
    Q_OBJECT

public:
    explicit BLEInputDevice(QObject *parent = nullptr);
    ~BLEInputDevice();
;

private:
    DeviceInfo currentDevice;
    QBluetoothDeviceDiscoveryAgent *DiscoveryAgent;
    QList<QObject*> qlDevices;
    QLowEnergyController *controller;
    QLowEnergyService *service;
    QLowEnergyDescriptor notificationDesc;
    bool bFoundSensorService;
    bool bFoundBattService;

private slots:
    void addDevice(const QBluetoothDeviceInfo &);
    void scanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);

    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    void controllerError(QLowEnergyController::Error);
    void _deviceConnected();
    void _deviceDisconnected();

    void serviceStateChanged(QLowEnergyService::ServiceState);
    void updateData(const QLowEnergyCharacteristic &, const QByteArray &);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &, const QByteArray &);

public slots:

    void startScan(void);
    void startConnection(size_t index);
    void stopConnection(void);
    void writeData(const QByteArray &data);




};

#endif // BLEINPUTDEVICE_H
