import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: root
    minimumWidth: 400
        minimumHeight: 300
    width: 800
    height: 400
    visible: true
    title: qsTr("Scale")
    color: "#1e1e1e"


    property bool allowExit : false
    property bool outputting : false

    onClosing: (close) => {
        close.accepted = allowExit;
        if(!allowExit) {
            appLogic.requestedExit();
        }

    }


    property bool deviceConnected: false
    property bool loggedIn: false
    property bool recording: false;
    property string fullName: "";


//Toolbar
    ListModel {
        id: buttonModel
        ListElement{
            imgPath: "/images/play.png"
            action: "start"
            isVisible: true
        }
        ListElement{
            imgPath: "/images/pause.png"
            action: "pause"
            isVisible: true
        }
        ListElement{
            imgPath: "/images/stop.png"
            action: "stop"
            isVisible: true
        }
        ListElement{
            imgPath: "/images/record.png"
            action: "record"
            isVisible: true
        }
        ListElement{
            imgPath: "/images/reset.png"
            action: 'reset'
            isVisible: true
        }
        ListElement{
            imgPath: "/images/calibrate.png"
            action: 'calibrate'
            isVisible: true
        }
    }

    ToolBar {
        id: toolBar
        width:parent.width
        height: 45
        background: Rectangle{
            anchors.fill: parent
            color: "#92a8d1"
        }
        Rectangle{
            id: hamburger
            color: '#7288b1'
            height: parent.height-5
            width: parent.height-5
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            Image{
                anchors.centerIn: parent
                source: "/images/hamburger.png"

            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    drawer.open();
                }
                onHoveredChanged:{
                    if(containsMouse)
                        parent.color = "#a2b8e1"
                    else
                        parent.color = "#8298c1"
                }
            }
        }


        ListView{
            id: buttonList
            interactive: false
            visible: root.deviceConnected
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: hamburger.right
            anchors.right: parent.right
            anchors.leftMargin: 5
            orientation: Qt.Horizontal
            model: buttonModel
            spacing: 5
            delegate: Rectangle{
                color: "#8298c1"
                height: parent.height-5
                width: parent.height-5
                radius: 5
                visible: isVisible
                anchors.verticalCenter: parent.verticalCenter
                Image{
                    anchors.centerIn: parent
                    source: imgPath
                    width: parent.width*0.6
                    height: parent.height*0.6


                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if(!root.deviceConnected)
                            return;
                        switch(action){
                            case 'start':
                                appLogic.startOutput();
                                break;
                            case 'stop':
                                appLogic.stopOutput();
                                appLogic.clearBuffer();
                                graph.clear();
                                break;
                            case 'pause':
                                appLogic.stopOutput();
                                break;
                            case 'reset':
                                appLogic.restartDevice();
                                buttonList.update()
                                parent.color = "#8298c1"
                                break;
                            case 'calibrate':
                                appLogic.calibrateDevice();
                                break;
                            case 'record':
                                if(!root.recording){
                                    root.recording = true
                                    appLogic.setRecording(true);
                                    buttonModel.get(4).isVisible = false;
                                    buttonModel.get(5).isVisible = false;
                                }
                                else{
                                    root.recording = false
                                    appLogic.setRecording(false);
                                    buttonModel.get(4).isVisible = true;
                                    buttonModel.get(5).isVisible = true;
                                }
                                break;
                        }
                    }
                    onHoveredChanged:{
                        if(!root.deviceConnected)
                            return;

                        if(containsMouse)
                            parent.color = "#a2b8e1"
                        else
                            parent.color = "#8298c1"
                    }
                }
            }
        }
        Text{
            anchors.left: hamburger.right
            visible: !root.deviceConnected
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            font.pixelSize: parent.height*0.5

            text: root.deviceConnected?"Connected":"Not connected"
        }

        Text{
            id: loginInfo
            anchors.right: webButton.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            font.pixelSize: parent.height*0.5

            text: root.loggedIn?"Welcome, "+root.fullName:"Logged out"
        }

        Rectangle{
            id: webButton
            color: '#7288b1'
            height: parent.height-5
            width: parent.height-5
            radius: 5
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            Image{
                id: webIconText
                anchors.centerIn: parent
                source: "/images/cloud.png"
                width: parent.width*0.6
                height: parent.height*0.6


            }


            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    webDrawer.open();
                }
                onHoveredChanged:{
                    if(containsMouse)
                        parent.color = "#a2b8e1"
                    else
                        parent.color = "#8298c1"
                }
            }
        }


    }

//Wykres
    Graph{
        id: graph
        anchors.top: toolBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        mainTitle: "Data"
        titleX: "Time (s)"
        titleY: "Value (Kg)"
        type: "linearGraph"

        minAxisX: 0
        maxAxisX: appLogic.getPlotSeconds()

        minValueX: 0
        maxValueX: appLogic.getPlotSeconds()*appLogic.getSamplingFreq()

        maxAxisY: 100
        minAxisY: -100
    }
//Drawer wyboru urządzeń - lewa strona
    Drawer {
        id: drawer
        width: parent.width*0.5
        height: parent.height
        background: Rectangle{
            color:"#2e2e2e"
        }

        Item {
            anchors.fill: parent

            ListView {
                anchors.fill: parent
                model: nestedModel
                delegate: categoryDelegate
                spacing: 5
            }
            //Typy urządzeń
            ListModel {
                id: nestedModel
                ListElement {
                    categoryName: "Bluetooth devices"
                    refreshable: true
                    refreshing: false
                    collapsed: true
                    subItems: []
                }

                ListElement {
                    categoryName: "Serial devices"
                    refreshable: true
                    refreshing: false
                    collapsed: true
                    subItems: []
                }

            }

            Component {
                id: categoryDelegate
                Column {
                    width: parent.width
                    Rectangle {
                        id: categoryItem
                        color: "#92a8d1"
                        height: 50
                        width: parent.width
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            x: 15
                            font.pixelSize: 24
                            text: categoryName
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                if(root.deviceConnected)
                                    return;
                                if(arrowRotation.to===180)
                                    refreshArea.clicked(null);
                                arrowRotation.start()
                                arrowRotation.to = arrowRotation.to===180?0:180
                                nestedModel.setProperty(index, "collapsed", !collapsed)
                            }
                            onHoveredChanged:{
                                if(root.deviceConnected)
                                    return;

                                if(containsMouse)
                                    parent.color = "#a2b8e1"
                                else
                                    parent.color = "#92a8d1"
                            }
                        }
                        Image{
                            id: arrow
                            source: "/images/arrow.png"
                            width: 30
                            height: 15
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            RotationAnimation on rotation {
                                id:arrowRotation
                                running: false
                                to: 180
                                duration: 200
                            }
                        }
                        Rectangle {
                            id: refresh
                            visible: refreshable
                            color: "#8298c1"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 50
                            width: 40
                            height: 40
                            radius: width*0.5

                            Image{
                                source: "/images/view-refresh.png"
                                width: 30
                                height: 30
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            RotationAnimation on rotation {
                                id: rotation
                                loops: Animation.Infinite
                                running: refreshing
                                alwaysRunToEnd: true
                                from: 0
                                to: 360
                                duration: 500
                            }
                            MouseArea{
                                id: refreshArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if(root.deviceConnected)
                                        return;
                                    refreshing = true;
                                    if(categoryName === "Bluetooth devices")
                                        appLogic.refreshBLE()
                                    if(categoryName === "Serial devices")
                                        appLogic.refreshSerial()

                                }
                                onHoveredChanged: {
                                    if(root.deviceConnected)
                                        return;

                                    if(containsMouse)
                                        refresh.color = "#a2b8e1"
                                    else
                                        parent.color = "#8298c1"

                                }
                            }
                        }
                    }
                    Loader {
                        id: subItemLoader
                        visible: !collapsed
                        property variant subItemModel : subItems
                        sourceComponent: collapsed ? null : subItemColumnDelegate
                        onStatusChanged: {
                            if (status == Loader.Ready)
                                item.model = subItemModel
                        }
                    }
                }

            }

            Component {
                id: subItemColumnDelegate
                Column {
                    property alias model : subItemRepeater.model
                    width: drawer.width
                    anchors.bottomMargin: 5
                    Repeater {
                        id: subItemRepeater
                        delegate: Rectangle {

                            color: "#034f84"
                            height: 40
                            width: parent.width
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if(connecting)
                                        return;
                                    if(connected){
                                        appLogic.disconnectDevice();
                                        connected=false;
                                    }
                                    else{
                                        if(root.deviceConnected)
                                            return;

                                        appLogic.connectDevice(type, index);
                                        connecting=true;
                                    }

                                }
                                onHoveredChanged:{
                                    if(root.deviceConnected && !connected)
                                        return;

                                    if(containsMouse){
                                        parent.color = "#135f94"
                                        if(connected)
                                            statusImage.source = "/images/disconnect.png"
                                    }
                                    else{
                                        parent.color = "#034f84"
                                        if(connected)
                                            statusImage.source = "/images/connected.png"
                                    }

                                }
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 30
                                font.pixelSize: 15
                                text: itemName
                            }
                            Image{
                                source: "/images/view-refresh.png"
                                width: 30
                                height: 30
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 10;
                                visible: connecting

                                RotationAnimation on rotation {
                                    loops: Animation.Infinite
                                    running: true
                                    from: 0
                                    to: 360
                                    duration: 1000
                                }

                            }
                            Image{
                                id: statusImage
                                source: "/images/connected.png"
                                width: 20
                                height: 20
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 10;
                                visible: connected


                            }

                        }
                    }
                }
            }
        }
    }
//Drawer odpowiadający za połączenie z serwerem - prawa strona
    Drawer {
            id: webDrawer
            width: parent.width*0.5
            height: parent.height
            edge: Qt.RightEdge
            background: Rectangle{
                color:"#2e2e2e"
            }

            Item{
                width: parent.width
                height: parent.height*0.7
                anchors.centerIn: parent


                Rectangle{
                    id: loggedInScreen
                    anchors.fill: parent
                    color:"#2e2e2e"
                    visible: false
                    Rectangle{
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 50
                        id: loggedInTitleRect
                        color:"#2e2e2e"
                        Label{
                            anchors.centerIn: parent
                            text: "Hello, "+root.fullName
                            font.pixelSize: 20

                            color:"#EeEeEe"
                        }
                    }
                    Button {
                        anchors.top: loggedInTitleRect.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.8
                        anchors.margins: 10
                        height:40
                        id: logoutButton
                        text: "Log out"

                        contentItem: Text {
                            text: logoutButton.text
                            font: logoutButton.font
                            color: "#E0E0E0"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            opacity: enabled ? 1 : 0.3
                            color: logoutButton.down ? "#1e1e1e" : "#3e3e3e"
                            radius: 2
                        }

                        onClicked: {
                            loginMessage.text = ""
                            loginInput.text = ""
                            passwordInput.text = ""
                            appLogic.logOut(false);

                        }


                    }
                }

                Rectangle{
                    id: loginScreen
                    anchors.fill: parent
                    color:"#2e2e2e"
                    Rectangle{
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 50
                        id: loginTitleRect
                        color:"#2e2e2e"
                        Label{
                            anchors.centerIn: parent
                            text: "Log in to access cloud backup:"
                            font.pixelSize: 20

                            color:"#EeEeEe"
                        }
                    }

                    Rectangle{
                        id: serverAdressBox
                        anchors.top: loginTitleRect.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.8
                        height:20
                        anchors.margins: 10
                        color:"#2e2e2e"

                        Label{
                            id: serverAdressLabel
                            anchors.left: parent.left
                            anchors.top: parent.top
                            height: parent.height
                            text: "Adress: "
                            color: "#E0E0E0"
                        }

                        Rectangle{
                            anchors.right: parent.right
                            height: parent.height
                            width: parent.width*0.7

                            color:"#3e3e3e"

                                TextInput {
                                    id: serverAdressInput
                                    objectName: "serverAdressInput"
                                    height: parent.height
                                    anchors.fill: parent
                                    color: "#E0E0E0"
                                    text: "serwer-wspinaczka-adam.onrender.com"
                                    KeyNavigation.tab: serverAdressInput
                                }
                        }
                    }

                    Rectangle{
                        id: loginBox
                        anchors.top: serverAdressBox.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.8
                        height:20
                        anchors.margins: 10
                        color:"#2e2e2e"

                        Label{
                            id: loginLabel
                            anchors.left: parent.left
                            anchors.top: parent.top
                            height: parent.height
                            text: "Login: "
                            color: "#E0E0E0"
                        }

                        Rectangle{
                            anchors.right: parent.right
                            height: parent.height
                            width: parent.width*0.7

                            color:"#3e3e3e"

                                TextInput {
                                    id: loginInput
                                    height: parent.height
                                    anchors.fill: parent
                                    color: "#E0E0E0"

                                    KeyNavigation.tab: passwordInput
                                }
                        }
                    }
                    Rectangle{
                        id: passwordBox
                        anchors.top: loginBox.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.8
                        height:20
                        anchors.margins: 10
                        color:"#2e2e2e"

                        Label{
                            id: passwordLabel
                            anchors.left: parent.left
                            anchors.top: parent.top
                            height: parent.height
                            text: "Password: "
                            color: "#E0E0E0"
                        }

                        Rectangle{
                            anchors.right: parent.right
                            height: parent.height
                            width: parent.width*0.7

                            color:"#3e3e3e"

                                TextInput {
                                    id: passwordInput
                                    height: parent.height
                                    anchors.fill: parent
                                    color: "#E0E0E0"
                                    echoMode: TextInput.Password
                                    onAccepted: {
                                        loginButton.clicked();
                                    }
                                }
                        }
                    }
                    Button {
                        anchors.top: passwordBox.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*0.8
                        anchors.margins: 10
                        height:40
                        id: loginButton
                        text: "Log in"

                        contentItem: Text {
                            text: loginButton.text
                            font: loginButton.font
                            color: "#E0E0E0"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            opacity: enabled ? 1 : 0.3
                            color: loginButton.down ? "#1e1e1e" : "#3e3e3e"
                            radius: 2
                        }

                        onClicked: {
                            if(loginInput.text.trim() === "" || passwordInput.text.trim() === "" || serverAdressInput.text.trim() === "")
                                loginMessage.text = "Please fill all fields";
                            else
                                appLogic.logIn(loginInput.text, passwordInput.text, serverAdressInput.text);
                        }



                    }
                    Label{
                        id: loginMessage
                        anchors.top: loginButton.bottom
                        width: parent.width*0.8
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#F08080"
                        text: ""
                    }

                }

            }

        }

    Connections {
        target: appLogic
        /*
           Po zakończeniu skanowania urządzeń bluetooth
           Tworzy przyciski umożliwiające nawiązanie połączenia
        */
        function onRefreshedBLE(names){
            for (let i=0; i<nestedModel.count;++i)
            {
                if(nestedModel.get(i).categoryName === "Bluetooth devices"){
                    nestedModel.get(i).refreshing = false
                    nestedModel.get(i).subItems.clear();
                    for(let j=0; j<names.length; j++)
                        nestedModel.get(i).subItems.append({itemName: names[j], connected:false, connecting:false, type: nestedModel.get(i).categoryName})
                }
            }
        }
        /*
            Po zakończeniu skanowania interfejsów UART
           Tworzy przyciski umożliwiające nawiązanie połączenia

        */
        function onRefreshedSerial(names){
            for (let i=0; i<nestedModel.count;++i)
            {
                if(nestedModel.get(i).categoryName === "Serial devices"){
                    nestedModel.get(i).refreshing = false
                    nestedModel.get(i).subItems.clear();
                    for(let j=0; j<names.length; j++)
                        nestedModel.get(i).subItems.append({itemName: names[j], connected:false, connecting:false, type: nestedModel.get(i).categoryName})
                }
            }
        }

        /*
            Po pomyślnym połączeniu i otrzymaniu informacji z urządzenia

            -Wyświetla ukryte wcześniej przyciski
            -Zmienia parametry wykresu zgodnie z informacjami o urządzeniu
        */
        function onConnectionStarted(){
            root.deviceConnected = true
            graph.maxValueX =appLogic.getPlotSeconds()*appLogic.getSamplingFreq()
            buttonList.update()
            drawer.close()
            for (let i=0; i<nestedModel.count;++i){
                for(let j=0; j<nestedModel.get(i).subItems.count; j++){
                    if(nestedModel.get(i).subItems.get(j).connecting===true){
                        nestedModel.get(i).subItems.get(j).connecting = false;
                        nestedModel.get(i).subItems.get(j).connected =true;

                    }
                }
            }
        }
        /*
            Po kalibracji możliwe jest np wykrycie dodatkowych czujników przez urządzenie
            Zaktualizowane informacje są przekazywane do tej funkcji
        */

        function onDevinfoUpdated(){
            graph.maxValueX = appLogic.getPlotSeconds()*appLogic.getSamplingFreq()
            loginInfo.text = appLogic.getNumChannels()+" Channels";

        }

        /*
            Błąd połączenia z urządzeniem
            Z punktu widzenia interfejsu graficznego traktowane jest to jak rozłączenie
        */
        function onConnectionError(){
            root.deviceConnected = false
            for (let i=0; i<nestedModel.count;++i){
                for(let j=0; j<nestedModel.get(i).subItems.count; j++){
                    if(nestedModel.get(i).subItems.get(j).connected===true || nestedModel.get(i).subItems.get(j).connecting===true){
                        nestedModel.get(i).subItems.get(j).connected =false;
                        nestedModel.get(i).subItems.get(j).connecting =false;
                    }
                }
            }
        }

        /*
            Rozłączenie urządzenia
        */
        function onConnectionEnded(){
            onConnectionError();

        }
        /*
            Po otrzymaniu nowych danych z urządzenia aktualizowany jest wykres
        */
        function onDataReady(data){
            graph.dataUpdate(data);
        }

        /*
            Po pomyślnym logowaniu do serwera zmienia się interfejs graficzny
            Ukryte są komunikaty o potrzebie logowania, wyświetlona jest możliwość wylogowania
        */
        function onLoginComplete(fullName){
            if(fullName !== "")
                root.fullName = fullName;
            root.loggedIn = true;
            loginScreen.visible = false;
            loggedInScreen.visible = true;

        }

        /*
            Po pomyślnym wylogowaniu zaprezentowane zostają komunikaty o możliwości logowania
        */
        function onLogoutComplete(){
            loginScreen.visible = true;
            loggedInScreen.visible = false;
            root.loggedIn = false;
        }
        /*
           Po błędzie logowania wyświetlany jest komunikat błędu
        */
        function onLoginError(message){
            if(!webDrawer.opened)
                webDrawer.open();
            loginMessage.text = message;

            root.loggedIn = false;
        }

        /*
            Wywoływane przez warstwę C++ gdy wszystkie połączenia zostały zamknięte i bezpiecznie można zamknąć program
        */
        function onRequestExit() {
            root.allowExit = true;
            root.close();
        }

        /*
            Wywoływane tuż po uruchomieniu procesu wysyłania na serwer
            Zmienia kolor ikony informując o trwającym transferze
        */
        function onUploadStarted() {

            webIconText.source = "/images/cloudb.png"
        }
        /*
            Wywołane tuż po pomyślnym wysłaniu pliku na serwer
            Zmienia kolor ikony informując o sukcesie
        */
        function onUploadComplete() {
            webIconText.source = "/images/cloudg.png"
        }
        /*
            Wywoływane w przypadku błędu wysyłania pliku na serwer

        */
        function onUploadError() {
            webIconText.source = "/images/cloudr.png"
        }



    }



}
