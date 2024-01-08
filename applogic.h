#ifndef APPLOGIC_H
#define APPLOGIC_H

#include <QObject>
#include "bleinputdevice.h"
#include "serialinputdevice.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define WAVE_FORMAT_IEEE_FLOAT 3


//Nagłówek pliku .wav
struct __attribute__((packed)) WavHeader{
    uint8_t chunkID[4];
    uint32_t chunkSize;
    uint8_t format[4];
    uint8_t subChunk1ID[4];
    uint32_t subChunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint16_t extensionSize;
    uint8_t subChunk2ID[4];
    uint32_t subChunk2Size;
    uint32_t sampleLength;
    uint8_t subChunk3ID[4];
    uint32_t subChunk3Size;
};




class AppLogic : public QObject
{
    Q_OBJECT
public:
    explicit AppLogic(QObject *parent = nullptr);
    ~AppLogic();

    //Obiekty używane do skanowania oraz połączenia z wybranym urządzeniem
    //Dla każdego typu połączeń istnieje osobny obiekt
    BLEInputDevice bleDevice;
    SerialInputDevice serialDevice;
    //Po wybraniu urządzenia currentDevice wskazuje na jeden z powyższych obiektów
    //Klasy tych obiektów dziedziczą po InputDevice
    InputDevice *currentDevice;
    //Struktura przechowująca informacje otrzymane z urządzenia
    const DataInfo *dataInfo;
    //Bufor przechowujący dane otrzymywane z urządzenia
    QVector<QVector<float>> dataBuffer;
    //Bufor przechowujący fragment danych, przesłany później na serwer
    QVector<float> recordingBuffer;
    //Tymczasowy bufor przechowujący pełny plik .wav wysyłany na serwer
    QByteArray sendBuffer;

    //Informuje o aktualnie trwającym procesie odbioru danych z urządzenia
    bool dataCapture;
    //Długość osi X wykresu w sekundach
    int plotSeconds;

    //Adres IP serwera danych
    QString ip;
    //Zmienna przechowująca token dający dostęp do chronionych endpointów serwera
    QByteArray token;
    QNetworkAccessManager networkManager;
    //Zmienna informująca o trwającym transferze danych do serwera
    volatile bool uploading;
    //Zmienna informująca o trwającym zbieraniu próbek przeznaczonych do zapisu
    volatile bool recording;
    //Wylogowanie z serwera może zostać zainicjowane przez użytkownika lub przy wyjściu z programu
    //W drugim przypadku po wylogowaniu następuje zakończenie działania
    volatile bool exitAfterLogOut;

    //Odpowiedź na każde zapytanie skierowane do serwera zostaje przekazane tej funkcji
    //W zależności od konkretnego zapytania, wywołuje ona funkcje pomocnicze
    void replyReady(QNetworkReply *);

    //Funkcja realizująca zapytanie odświeżenia tokenu
    void refreshTokenReq();
    //Funkcja realizująca wysłanie zapisanego pliku .wav do serwera
    void sendDataReq(QByteArray data);

    //Funkcje obsługujące odpowiedzi serwera na poszczególne zapytania
    void refreshTokenRes(QNetworkReply *);
    void logInRes(QNetworkReply *);
    void logOutRes(QNetworkReply *);
    void sendDataRes(QNetworkReply *);



public slots:


        //Qml -> C++
    //Funkcje udostępnione przez warstwę C++ dla warstwy QML
    //Większość z nich odpowiada poszczególnym interakcjom użytkownika z GUI
    void refreshBLE();
    void refreshSerial();
    void connectDevice(QString type, int index);
    void disconnectDevice();
    void restartDevice();
    void calibrateDevice();
    void startOutput();
    void stopOutput();
    void clearBuffer();
    int getPlotSeconds();
    int getSamplingFreq();
    int getNumChannels();
    void logIn(QByteArray login, QByteArray password, QByteArray adress);
    void logOut(bool);
    void setRecording(bool);
    void requestedExit();

        //device -> c++
    //Funkcje obsługujące wydarzenia sprzętowe
    void scanCompleteBLE();
    void scanCompleteSerial();
    void dataReceieved(const QByteArray &array);
    void deviceConnected();
    void deviceDisconnected();
    void deviceError();
    void infoUpdated();


signals:
        //C++ -> Qml
    //Funkcje informujące warstwę QML o wydarzeniach warstwy C++
    void refreshedBLE(QStringList list);
    void refreshedSerial(QStringList list);
    void connectionStarted();
    void devinfoUpdated();
    void connectionEnded();
    void connectionError();
    void dataReady(QVector<QVector<float>> data);
    void loginError(QString error);
    void loginComplete(QString fullName);
    void logoutComplete();
    void requestExit();
    void uploadStarted();
    void uploadComplete();
    void uploadError();
};

#endif // APPLOGIC_H
