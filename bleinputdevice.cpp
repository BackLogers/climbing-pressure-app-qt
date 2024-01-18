#include "bleinputdevice.h"

BLEInputDevice::BLEInputDevice(QObject *parent):
    InputDevice(parent),
    currentDevice(QBluetoothDeviceInfo()),
    controller(0),
    service(0)
{
    DiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    DiscoveryAgent->setLowEnergyDiscoveryTimeout(3000);
    connect(DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLEInputDevice::addDevice);
    connect(DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &BLEInputDevice::deviceScanError);
    connect(DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLEInputDevice::scanFinished);
    connect(DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BLEInputDevice::scanFinished);


}



BLEInputDevice::~BLEInputDevice()
{
    delete DiscoveryAgent;
    delete controller;
}


void BLEInputDevice::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {

        if(!deviceNames.contains(device.name(), Qt::CaseSensitive) && device.name().size()) {
            deviceNames.append(device.name());

            DeviceInfo *dev = new DeviceInfo(device);
            qlDevices.append(dev);
        }
    }
}

void BLEInputDevice::scanFinished()
{
    emit scanComplete();
}

void BLEInputDevice::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "BLE::deviceScanError: " << error;
    connected = false;
    emit deviceError();
}

void BLEInputDevice::startScan()
{
#if QT_CONFIG(permissions)
    //! [permissions]
    QBluetoothPermission permission{};
    permission.setCommunicationModes(QBluetoothPermission::Access);
    switch (qApp->checkPermission(permission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(permission, this, &BLEInputDevice::startScan);
        return;
    case Qt::PermissionStatus::Denied:
        qDebug()<< "Bluetooth permissions not granted!" ;
        return;
    case Qt::PermissionStatus::Granted:
        break; // proceed to search
    }
    //! [permissions]
#endif // QT_CONFIG(permissions)


    if(connected){
        stopConnection();
        connected=false;
    }
    qDeleteAll(qlDevices);
    qlDevices.clear();
    deviceNames.clear();
    DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}



void BLEInputDevice::startConnection(size_t i)
{
    currentDevice.setDevice(((DeviceInfo*)qlDevices.at(i))->getDevice());
    if (controller) {
        controller->disconnectFromDevice();
        delete controller;
        controller = 0;

    }


    controller = QLowEnergyController::createCentral(currentDevice.getDevice());
    connect(controller, &QLowEnergyController::serviceDiscovered, this, &BLEInputDevice::serviceDiscovered);
    connect(controller, &QLowEnergyController::discoveryFinished, this, &BLEInputDevice::serviceScanDone);
    connect(controller, &QLowEnergyController::errorOccurred,  this, &BLEInputDevice::controllerError);
    connect(controller, &QLowEnergyController::connected, this, &BLEInputDevice::_deviceConnected);
    connect(controller, &QLowEnergyController::disconnected, this, &BLEInputDevice::_deviceDisconnected);
    controller->connectToDevice();

}

void BLEInputDevice::stopConnection()
{
    if(controller != NULL && controller->state() != QLowEnergyController::UnconnectedState)
        controller->disconnectFromDevice();
    connected = false;
    emit deviceDisconnected();

}

void BLEInputDevice::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if(gatt==QBluetoothUuid(QUuid(QPP_SERVICE_UUID))) {
        bFoundSensorService = true;
    }
}

void BLEInputDevice::serviceScanDone()
{
    delete service;
    service=0;

    if(bFoundSensorService) {
        service = controller->createServiceObject(QBluetoothUuid(QUuid(QPP_SERVICE_UUID)),this);
    }

    if(!service) {
        stopConnection();
        connected = false;
        emit deviceError();
        return;
    }
    connect(service, &QLowEnergyService::stateChanged,this, &BLEInputDevice::serviceStateChanged);
    connect(service, &QLowEnergyService::characteristicChanged,this, &BLEInputDevice::updateData);
    connect(service, &QLowEnergyService::descriptorWritten,this, &BLEInputDevice::confirmedDescriptorWrite);
    service->discoverDetails();
}

void BLEInputDevice::_deviceDisconnected()
{
    connected = false;
    emit deviceDisconnected();
}




void BLEInputDevice::_deviceConnected()
{

    controller->discoverServices();
}

void BLEInputDevice::controllerError(QLowEnergyController::Error error)
{
    connected = false;
    emit deviceError();
}

void BLEInputDevice::serviceStateChanged(QLowEnergyService::ServiceState s)
{

    switch (s) {
    case QLowEnergyService::RemoteServiceDiscovered:
    {
        //QppTx characteristic
        const QLowEnergyCharacteristic qppTxChar = service->characteristic(QBluetoothUuid(QUuid(QPP_TX_UUID)));
        if (!qppTxChar.isValid()) {
            break;
        }
        const QLowEnergyCharacteristic qppRxChar = service->characteristic(QBluetoothUuid(QUuid(QPP_RX_UUID)));
        if (!qppRxChar.isValid()) {
            break;
        }
        // QppRx notify enabled
        const QLowEnergyDescriptor m_notificationDescQppRx =
            qppRxChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
        if (m_notificationDescQppRx.isValid()) {
            // Enable notification
            service->writeDescriptor(m_notificationDescQppRx, QByteArray::fromHex("0100"));

        }
        break;
    }
    default:

        break;
    }
}

void BLEInputDevice::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{

    if (d.isValid() && d == notificationDesc && value == QByteArray("0000")) {
        controller->disconnectFromDevice();
        connected = false;
        delete service;
        service = nullptr;
        emit deviceError();
        return;
    }
    onConnection();
}

void BLEInputDevice::writeData(const QByteArray &v)
{
    const QLowEnergyCharacteristic qppTxChar = service->characteristic(QBluetoothUuid(QUuid(QPP_TX_UUID)));
    service->writeCharacteristic(qppTxChar, v, QLowEnergyService::WriteWithoutResponse);
}

void BLEInputDevice::updateData(const QLowEnergyCharacteristic &c, const QByteArray &v)
{
    if (c.uuid() == QBluetoothUuid(QUuid(QPP_RX_UUID))) {
        RxBuffer.append(v);
        parseData();
    }
}

