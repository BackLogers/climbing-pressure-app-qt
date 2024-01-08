#include "applogic.h"
#include "qqmlcontext.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

AppLogic::AppLogic(QObject *parent)
    : QObject{parent}
{
    currentDevice = NULL;
    plotSeconds = 5;
    dataCapture = false;
    uploading = false;
    exitAfterLogOut = false;
    token = "";
    //ip = "serwer-wspinaczka-adam.onrender.com";
    ip = "http://127.0.0.1/";

    connect(&networkManager, &QNetworkAccessManager::finished, this, &AppLogic::replyReady);
    connect(&bleDevice, &BLEInputDevice::scanComplete, this, &AppLogic::scanCompleteBLE);
    connect(&serialDevice, &SerialInputDevice::scanComplete, this, &AppLogic::scanCompleteSerial);
}

AppLogic::~AppLogic()
{
    if(dataCapture && currentDevice!=NULL)
        stopOutput();
}

void AppLogic::replyReady(QNetworkReply *reply){

    if(reply->request().url() == "http://"+ip+"/api/user" && reply->operation() == QNetworkAccessManager::PostOperation)
        logInRes(reply);

    if(reply->request().url() == "http://"+ip+"/api/user" && reply->operation() == QNetworkAccessManager::DeleteOperation)
        logOutRes(reply);

    if(reply->request().url() == "http://"+ip+"/api/user" && reply->operation() == QNetworkAccessManager::GetOperation)
       refreshTokenRes(reply);

    if(reply->request().url() == "http://"+ip+"/api/files/wall" && reply->operation() == QNetworkAccessManager::PostOperation)
        sendDataRes(reply);
}

void AppLogic::refreshTokenReq()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://"+ip+"/api/user"));
    networkManager.get(request);
}

void AppLogic::sendDataReq(QByteArray data)
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://"+ip+"/api/files/wall"));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "audio/wave");
    request.setRawHeader("Authorization", token);
    uploading = true;
    networkManager.post(request, data);
}

void AppLogic::sendDataRes(QNetworkReply *reply)
{
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200){
        uploading = false;
        emit uploadComplete();
    }
    else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 400) {
        refreshTokenReq();
    }
    else {
        uploading = false;
        emit uploadError();
    }


}

void AppLogic::refreshTokenRes(QNetworkReply *reply)
{   if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200){
        QByteArray arr = reply->readAll();
        arr = arr.sliced(arr.indexOf("\"token\":")+sizeof("\"token\":"));
        arr = arr.sliced(0, arr.indexOf("\""));
        arr = "Bearer " + arr;
        token = arr;
        emit loginComplete("");

        if(uploading){
            sendDataReq(sendBuffer);
        }
    }
    else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 400){
        emit loginError("Session expired, please log in");
    }
    else{
        if(uploading){
            emit uploadError();
            uploading = false;
        }
    }

}

void AppLogic::logInRes(QNetworkReply *reply)
{
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ==200){
        QByteArray arr = reply->readAll();
        QByteArray tokenArr = arr.sliced(arr.indexOf("\"token\":")+sizeof("\"token\":"));
        tokenArr = tokenArr.sliced(0, arr.indexOf("\""));
        tokenArr = "Bearer " + arr;
        token = tokenArr;
        QByteArray nameArr = arr.sliced(arr.indexOf("\"fullName\":")+sizeof("\"fullName\":"));
        qDebug() << nameArr;
        nameArr = nameArr.sliced(0, nameArr.indexOf("\""));
        qDebug() << nameArr;

        emit loginComplete(nameArr);
        if(uploading){
            sendDataReq(sendBuffer);
        }
    }
    else if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 400){
        emit loginError("Name or password is invalid");
    }
    else{
        emit loginError("Connection error");
        if(uploading)
            uploading = false;
    }
}

void AppLogic::logOutRes(QNetworkReply *)
{
    token = "";
    if(exitAfterLogOut)
        emit requestExit();
     else
        emit logoutComplete();

}


void AppLogic::logOut(bool exitAL){

    exitAfterLogOut = exitAL;

    QNetworkRequest request;
    request.setUrl(QUrl("http://"+ip+"/api/user"));
    networkManager.deleteResource(request);

}


void AppLogic::logIn(QByteArray login, QByteArray password, QByteArray adress){

    QByteArray body = "{\
                       \"name\":\""+login+"\",\
                       \"password\":\""+password+"\"\
                       }";

    if (adress.length() > 0) ip = adress;

    QNetworkRequest request;
    request.setUrl(QUrl("http://"+ip+"/api/user"));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");

    networkManager.post(request, body);
}




void AppLogic::refreshBLE()
{
    bleDevice.startScan();
}

void AppLogic::refreshSerial()
{
    serialDevice.startScan();
}

void AppLogic::scanCompleteBLE()
{
    emit refreshedBLE(bleDevice.getNames());
}

void AppLogic::scanCompleteSerial()
{
    emit refreshedSerial(serialDevice.getNames());
}

void AppLogic::connectDevice(QString type, int index)
{
    if(currentDevice!=NULL)
        currentDevice->stopConnection();

    if(type == "Bluetooth devices")
        currentDevice = &bleDevice;
    else if(type == "Serial devices")
        currentDevice = &serialDevice;

    if(currentDevice !=NULL){
        connect(currentDevice, &InputDevice::deviceConnected, this, &AppLogic::deviceConnected);
        connect(currentDevice, &InputDevice::infoUpdated, this, &AppLogic::infoUpdated);
        connect(currentDevice, &SerialInputDevice::deviceDisconnected, this, &AppLogic::deviceDisconnected);
        connect(currentDevice, &SerialInputDevice::deviceError, this, &AppLogic::deviceError);
        connect(currentDevice, &SerialInputDevice::dataReady, this, &AppLogic::dataReceieved);
        currentDevice->startConnection(index);
    }


}

void AppLogic::disconnectDevice()
{
    if(currentDevice!= NULL){
        currentDevice->stopConnection();
    }
}



void AppLogic::restartDevice()
{
    if(currentDevice!= NULL)
        currentDevice->restartDevice();
}

void AppLogic::calibrateDevice()
{
    if(currentDevice!= NULL)
        currentDevice->calibrateDevice();
}

void AppLogic::startOutput()
{
    if(currentDevice == NULL)
        return;
    if((typeid(*currentDevice) == typeid(SerialInputDevice)))
        currentDevice->startOutput(0);
    else if((typeid(*currentDevice) == typeid(BLEInputDevice)))
        currentDevice->startOutput(1);
    dataCapture = true;
}

void AppLogic::stopOutput()
{
    if(currentDevice == NULL)
        return;
    if((typeid(*currentDevice) == typeid(SerialInputDevice)))
        currentDevice->stopOutput(0);
    else if((typeid(*currentDevice) == typeid(BLEInputDevice)))
        currentDevice->stopOutput(1);
    dataCapture = false;
}

void AppLogic::setRecording(bool state)
{
    if(state){
        recording = true;
    }
    else{
        recording = false;

        struct WavHeader header;
               header.chunkID[0] = 'R';
               header.chunkID[1] = 'I';
               header.chunkID[2] = 'F';
               header.chunkID[3] = 'F';
               header.chunkSize = (sizeof(struct WavHeader) + recordingBuffer.size() - 8);
               header.format[0] = 'W';
               header.format[1] = 'A';
               header.format[2] = 'V';
               header.format[3] = 'E';
               header.subChunk1ID[0] = 'f';
               header.subChunk1ID[1] = 'm';
               header.subChunk1ID[2] = 't';
               header.subChunk1ID[3] = ' ';
               header.subChunk1Size = 18;
               header.audioFormat = WAVE_FORMAT_IEEE_FLOAT;
               header.numChannels = dataInfo->numChannels;
               header.sampleRate = dataInfo->samplingFrequency;
               header.byteRate = dataInfo->samplingFrequency * sizeof(float) * header.numChannels;
               header.blockAlign = header.numChannels * sizeof(float);
               header.bitsPerSample = 32;
               header.extensionSize = 0;
               header.subChunk2ID[0] = 'f';
               header.subChunk2ID[1] = 'a';
               header.subChunk2ID[2] = 'c';
               header.subChunk2ID[3] = 't';
               header.subChunk2Size = 4;
               header.sampleLength = (header.numChannels * recordingBuffer.size());
               header.subChunk3ID[0] = 'd';
               header.subChunk3ID[1] = 'a';
               header.subChunk3ID[2] = 't';
               header.subChunk3ID[3] = 'a';
               header.subChunk3Size = (header.sampleLength * sizeof(float));

        sendBuffer.clear();
        sendBuffer.append((char*)&header, sizeof(struct WavHeader));
        sendBuffer.append((char*)recordingBuffer.data(), (recordingBuffer.size()*sizeof(float)));
        sendDataReq(sendBuffer);
        recordingBuffer.clear();
        emit uploadStarted();
    }
}

void AppLogic::requestedExit()
{
    stopOutput();
    if(token != "")
        logOut(true);
    else
        emit requestExit();
}


void AppLogic::dataReceieved(const QByteArray &array)
{
    if(!dataCapture)
        return;

    unsigned int samples = (array.size()/dataInfo->numChannels)/dataInfo->bytesPerSample;
    unsigned int bufferSize = dataInfo->samplingFrequency*plotSeconds;

    //Konwertuje 24 bitowe próbki typu int zapisane na 3 bajtach na 32 bitowe float
    QVector<QVector<float>> parsedSamples;
    parsedSamples.resize(dataInfo->numChannels);
    for(int i=0; i<dataInfo->numChannels; i++){
        parsedSamples[i].resize(samples);
        for(unsigned int j=0; j< samples; j++){
            int rawVal = (int)(*((uint32_t*)(array.data()+(i*3)+(j*dataInfo->numChannels*dataInfo->bytesPerSample))) << 8) >> 8;
            float value =(int)(*((uint32_t*)(array.data()+(i*3)+(j*dataInfo->numChannels*dataInfo->bytesPerSample))) << 8) >> 8;

            value/= 0x00400000;
            parsedSamples[i][j] = value;
        }
    }

    //Jeśli nowe dane miałyby wyjść poza wybraną wielkość bufora, odrzucane są najstarsze próbki poprzez przesunięcie wartości w lewo
    if((dataBuffer[0].size()+samples) > bufferSize){
                \
        for(int k=0; k<dataInfo->numChannels; k++){
            for(unsigned int i=0, j=(dataBuffer[k].size()+samples)-bufferSize; j<dataBuffer[k].size(); i++, j++)
                dataBuffer[k][i] = dataBuffer[k][j];
            dataBuffer[k].resize(bufferSize-samples);
        }
    }
    //Do bufora dodawane są nowe dane
    for(int i=0; i< dataInfo->numChannels; i++)
        dataBuffer[i].append(parsedSamples[i]);

    //W przypadku aktywnego zapisu próbek do późniejszej transmisji na serwer, są one dodawane do osobnego bufora
    if(recording){
        for(unsigned int i=0; i<samples; i++)
            for(int j = 0; j<dataInfo->numChannels; j++)
                recordingBuffer.append(parsedSamples[j][i]);
    }
    //Aktualne dane są przekazywane warstwie QML do wyświetlenia
    emit dataReady(dataBuffer);
}

void AppLogic::deviceConnected()
{
    dataInfo = currentDevice->getInfo();
    dataBuffer.clear();
    dataBuffer.resize(dataInfo->numChannels);
    emit connectionStarted();
}

void AppLogic::deviceDisconnected()
{
    dataCapture = 0;
    if(currentDevice !=NULL){
        disconnect(currentDevice, &InputDevice::deviceConnected, this, &AppLogic::deviceConnected);
        disconnect(currentDevice, &InputDevice::infoUpdated, this, &AppLogic::infoUpdated);
        disconnect(currentDevice, &SerialInputDevice::deviceDisconnected, this, &AppLogic::deviceDisconnected);
        disconnect(currentDevice, &SerialInputDevice::deviceError, this, &AppLogic::deviceError);
        disconnect(currentDevice, &SerialInputDevice::dataReady, this, &AppLogic::dataReceieved);
        currentDevice=NULL;
    }
    emit connectionEnded();
}

void AppLogic::deviceError()
{
    emit connectionError();
    disconnectDevice();
}

void AppLogic::infoUpdated()
{
    dataInfo = currentDevice->getInfo();
    dataBuffer.resize(dataInfo->numChannels);
    emit devinfoUpdated();
}

void AppLogic::clearBuffer()
{
    for(int i =0; i<dataBuffer.size(); i++)
        dataBuffer[i].clear();
}

int AppLogic::getPlotSeconds()
{
    return plotSeconds;
}

int AppLogic::getSamplingFreq()
{
    if(currentDevice !=NULL){
        return dataInfo->samplingFrequency;
    }
    return 80;
}

int AppLogic::getNumChannels()
{
    if(currentDevice !=NULL){
        return dataInfo->numChannels;
    }
    return 0;
}
