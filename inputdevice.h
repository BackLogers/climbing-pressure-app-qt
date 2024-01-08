#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QString>
#include <QVector>
#include <QObject>
#include <QTimer>



/*
 *
 * Klasa InputDevice implementuje protokół niezależny od fizycznego interfejsu
 * Dziedziczące po niej klasy implementują jedynie funkcje związane z fizyczną transmisją danych
 *
 */



struct __attribute__((__packed__)) DataInfo {
    uint8_t bitsPerSample;
    uint8_t bytesPerSample;
    uint8_t sampleFormat;
    uint8_t numChannels;
    uint32_t samplingFrequency;
};

struct __attribute__((__packed__)) Header {
    uint8_t commandID;
    uint8_t status;
    uint16_t dataSize;
};

#define SERIAL_ID 0x00
#define BLE_ID 0x01

#define CMD_ECHO 0x00
#define CMD_DEVICE_STRING 0x01
#define CMD_DEVICE_INFO 0x02
#define CMD_RESET 0x10
#define CMD_CALIBRATE 0x11
#define CMD_OUTPUT 0x12


class InputDevice:public QObject
{
Q_OBJECT

public:
    InputDevice(QObject *parent);
    QStringList deviceNames;
    bool connected;
    struct DataInfo dataInfo;
    QString deviceString;
    QByteArray RxBuffer;
    uint16_t bytesExpected;
    QTimer timeoutTimer;

public slots:

    virtual void startScan(void) = 0;
    QStringList getNames(void);
    const struct DataInfo *getInfo();
    virtual void startConnection(size_t index) = 0;
    virtual void stopConnection(void) = 0;
    virtual void writeData(const QByteArray &data) = 0;
    void parseData(void);
    void onConnection(void);
    bool isConnected(void);
    void timeoutDetected(void);
    void restartDevice(void);
    void calibrateDevice(void);
    void startOutput(uint8_t type);
    void stopOutput(uint8_t type);

signals:
    void scanComplete(void);
    void deviceConnected(void);
    void deviceDisconnected(void);
    void dataReady(const QByteArray &data);
    void deviceError();
    void infoUpdated();


};

#endif // INPUTDEVICE_H
