#include <WiFi.h>
#include <WebServer.h>


#define TURNON 22
#define TURNOFF 22
#define BRIGHTEN 21
#define DIM 18
#define NIGHTMODE 16
#define TEMP 2
#define WIRD 27
#define SUN 33


#define MAX_BRIGHTNESS 30



const char* ssid = "HomeOne";
const char* password = "57!!akMM";

IPAddress local_IP( 192, 168, 1, 200 );
IPAddress gateway( 192, 168, 1, 1 );
IPAddress subnet( 255, 255, 0, 0 );

int brightness = 0;
int onOffState = 0;

WebServer server(80);

void setup(){
    Serial.begin( 115200 );

    if ( !WiFi.config( local_IP, gateway, subnet ) ){
      Serial.println( "STA Failed to configure" );
    }
    
    Serial.print( "Connecting" );
    WiFi.begin( ssid, password );
    while ( WiFi.status() != WL_CONNECTED ){
        delay( 1000 );
        Serial.print( "." );
    }

    Serial.println( "Ip Address:" );
    Serial.println( WiFi.localIP() );

    server.enableCORS( true ); 
    server.on( "/turnon", turnon );
    server.on( "/turnoff", turnoff );
    server.on( "/brighten", brighten );
    server.on( "/dim", dim );
    server.on( "/nightmode", nightMode );
    server.on( "/temp", temp );
    server.on( "/wird", wird );
    server.on( "/sun", sun );
    
    server.on( "/resetBrightness", resetBrightness );
    server.on( "/resetOnOffState", resetOnOff );

    server.on( "/setbrightness", HTTP_POST, handlePost);

    server.on( "/getstates", getStates );

    server.on( "/", send_html );
    server.begin();

    pinMode( TURNON, OUTPUT );
    pinMode( BRIGHTEN, OUTPUT );
    pinMode( DIM, OUTPUT );
    pinMode( NIGHTMODE, OUTPUT );
    pinMode( TEMP, OUTPUT );
    pinMode( WIRD, OUTPUT );
    pinMode( SUN, OUTPUT );
    

    digitalWrite( TURNON, LOW );
    digitalWrite( BRIGHTEN, LOW );
    digitalWrite( DIM, LOW );
    digitalWrite( NIGHTMODE, LOW );
    digitalWrite( TEMP, LOW );
    digitalWrite( WIRD, LOW );
    digitalWrite( SUN, LOW );

    Serial.println( "Calibrating..." );
    Serial.println( "Turn on" );
    pressButton( TURNON );
    Serial.println( "DIM to 0 brightness" );
    delay( 100 );
    for ( int i = 0; i < 50; i = i + 1 ){
      pressButton( DIM );
      delay( 100 );
    }
    delay( 100 );
    pressButton( TURNOFF );
    brightness = 0;
    Serial.println( "Turning off" );
}

void handlePost(){
  String body = server.arg( "plain" );
  char recvBright[ 2 ];
  strcpy( recvBright, body.c_str() );
  
  int setBright = atoi( recvBright );
  if ( setBright < 0 ){ setBright = 0; }
  if ( setBright > MAX_BRIGHTNESS ){ setBright = MAX_BRIGHTNESS; };

  if ( brightness < setBright ){
    for ( int i = brightness; i < setBright; i++ ){
      pressButton( BRIGHTEN );
      delay( 100 );
      brightness = brightness + 1;
    }
  } else {
    for ( int i = brightness; i > setBright; i-- ){
      pressButton( DIM );
      delay( 100 );
      brightness = brightness - 1;
    }
  }
  server.send( 200 );
}

void loop(){
  server.handleClient();
}


void send_html(){
  server.send( 200, "text/html", getHTML() );
}

void resetOnOff(){
  onOffState = false;
  server.send( 200 );
}

void resetBrightness(){
  brightness = 0;
  for ( int i = 0; i < 50; i = i + 1 ){
      pressButton( DIM );
  }
  server.send( 200 );
}

void getStates(){
  String finishedMessage = "brightness: " + String( brightness );
  if ( onOffState == 1 ){
    finishedMessage = finishedMessage + " | onOff: true";
  } else {
    finishedMessage = finishedMessage + " | onOff: false";
  }
  Serial.println( "get states" );
  Serial.println( finishedMessage );
  server.send( 200, "text/plain", finishedMessage );
}

void pressButton( int pinNum ){
  digitalWrite( pinNum, HIGH );
  delay( 100 ); 
  digitalWrite( pinNum, LOW );
}


void turnon(){
  if ( onOffState == 0 ){
    pressButton( TURNON );
    onOffState = 1;
  }
  server.send( 200 );
}


void turnoff(){
  if ( onOffState == 1 ){ 
    pressButton( TURNOFF );
    onOffState = 0;
  }
  server.send( 200 );
}

void brighten(){
  if ( brightness < MAX_BRIGHTNESS  ){
    pressButton( BRIGHTEN );
    brightness = brightness + 1;
  }
  server.send( 200 );
}

void dim(){
  if ( brightness > 0 ){
    brightness = brightness - 1;
    pressButton( DIM );
  }
  server.send( 200 );
}

void nightMode(){
  pressButton( NIGHTMODE );
  brightness = 0;
  server.send( 200 );
}

void temp(){
  pressButton( TEMP );
  brightness = MAX_BRIGHTNESS;
  server.send( 200 );
}

void wird(){
  pressButton( WIRD );
  server.send( 200 );
}

void sun(){
  pressButton( SUN );
  server.send( 200 );
}




String getHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head>\n";
  ptr += "    <title>Light Controll</title>\n";
  ptr += "    <meta name='viewport' content='user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width, height=device-height, target-densitydpi=device-dpi' />\n";
  ptr += "    <style>\n";
  ptr += "        html, body {\n";
  ptr += "            overflow-y: hidden;\n";
  ptr += "            overflow-x: hidden;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        body {\n";
  ptr += "            margin: 0; \n";
  ptr += "            height: 100%;\n";
  ptr += "            width: 100%;\n";
  ptr += "            background-color: red;\n";
  ptr += "            overflow: hidden;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .leftButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            left: 0;\n";
  ptr += "            width: 33.3333%;\n";
  ptr += "            height: 33.3333%;\n";
  ptr += "            z-index: 100;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .rightButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            left: 66.66666%;\n";
  ptr += "            width: 33.3333%;\n";
  ptr += "            height: 33.3333%;\n";
  ptr += "            z-index: 100;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #offButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 0;\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        #stateChange{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #sunButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 33.3333%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #wirdButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 66.66666%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #nightButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 33.3333%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #timerButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 66.66666%;\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        #brightnessRegulator{\n";
  ptr += "            position: absolute;\n";
  ptr += "            left: 33.3333%;\n";
  ptr += "            height: 50%;\n";
  ptr += "            width: 33.3333%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #onButton{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0;\n";
  ptr += "            width: 100%;\n";
  ptr += "            height: 100%;\n";
  ptr += "            z-index: -10;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .image{\n";
  ptr += "            width: 100%;\n";
  ptr += "            height: 100%;\n";
  ptr += "            z-index: 100;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #brightnessRegulator_lowerBound{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0%;\n";
  ptr += "            width: 100%;\n";
  ptr += "            height: 100%;\n";
  ptr += "            background-color: whitesmoke;\n";
  ptr += "            z-index: 10;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #brightnessRegulator_upperBound{\n";
  ptr += "            position: absolute;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0%;\n";
  ptr += "            width: 100%;\n";
  ptr += "            height: 90%;\n";
  ptr += "            background-color: navy;\n";
  ptr += "            z-index: 20;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #brightnessRegulator{\n";
  ptr += "            position: relative;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 33.333333%;\n";
  ptr += "            height: 100%;\n";
  ptr += "            background-color: yellow;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .curtainRegulator{\n";
  ptr += "            position: absolute;\n";
  ptr += "            background-color: gray;\n";
  ptr += "            border: 1px solid rgb(85, 85, 85);\n";
  ptr += "            width: 100%;\n";
  ptr += "            height: 33.333%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #curtain_1{\n";
  ptr += "            position: relative;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #curtain_2{\n";
  ptr += "            position: relative;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #curtain_3{\n";
  ptr += "            position: relative;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .curtainRegulator_lower{\n";
  ptr += "            position: absolute;\n";
  ptr += "            height: 100%;\n";
  ptr += "            width: 100%;\n";
  ptr += "            background-color: lightslategray;\n";
  ptr += "            z-index: 10;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .curtainRegulator_upper{\n";
  ptr += "            position: absolute;\n";
  ptr += "            height: 100%;\n";
  ptr += "            width: 90%;\n";
  ptr += "            background-color: black;\n";
  ptr += "            z-index: 20;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "    </style>\n";
  ptr += "    \n";
  ptr += "</head>\n";
  ptr += "\n";
  ptr += "<body>\n";
  ptr += "    <div id = 'container' style = 'position: absolute; width: 100%; height: 70%;'>\n";
  ptr += "        <div id = 'offButton' class = 'leftButton' style = 'z-index: 100;'>\n";
  ptr += "            <img src = '/static/offButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'nightButton' class = 'leftButton'>\n";
  ptr += "            <img src = '/static/nightButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'timerButton' class = 'leftButton'>\n";
  ptr += "            <img src = '/static/timerButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'stateChange' class = 'rightButton'>\n";
  ptr += "            <img src = '/static/stateButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'sunButton' class = 'rightButton'>\n";
  ptr += "            <img src = '/static/sunButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'wirdButton' class = 'rightButton'>\n";
  ptr += "            <img src = '/static/wirdButton.jpg' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    \n";
  ptr += "        <div id = 'brightnessRegulator'>\n";
  ptr += "            <div id = 'brightnessRegulator_lowerBound'></div>\n";
  ptr += "            <div id = 'brightnessRegulator_upperBound'></div>\n";
  ptr += "        </div>\n";
  ptr += "\n";
  ptr += "        <div id = 'onButton'>\n";
  ptr += "            <img src = '/static/onButton.jpg' id = 'onButtonImage' class = 'image'>\n";
  ptr += "        </div>\n";
  ptr += "    </div>\n";
  ptr += "\n";
  ptr += "    <div id = 'curtains' style = 'position: absolute; top: 70%; width: 100%; height: 30%; background-color: white;'>\n";
  ptr += "        <div id = 'curtain_1' class = 'curtainRegulator' >\n";
  ptr += "            <div id = 'curtain_1_lower' class = 'curtainRegulator_lower' ></div>\n";
  ptr += "            <div id = 'curtain_1_upper' class = 'curtainRegulator_upper'></div>\n";
  ptr += "        </div>\n";
  ptr += "\n";
  ptr += "        <div id = 'curtain_2' class = 'curtainRegulator' >\n";
  ptr += "            <div id = 'curtain_2_lower' class = 'curtainRegulator_lower' ></div>\n";
  ptr += "            <div id = 'curtain_2_upper' class = 'curtainRegulator_upper' ></div>\n";
  ptr += "        </div>\n";
  ptr += "\n";
  ptr += "        <div id = 'curtain_3' class = 'curtainRegulator' >\n";
  ptr += "            <div id = 'curtain_3_lower' class = 'curtainRegulator_lower' ></div>\n";
  ptr += "            <div id = 'curtain_3_upper' class = 'curtainRegulator_upper'></div>\n";
  ptr += "        </div>\n";
  ptr += "    </div>\n";
  ptr += "\n";
  ptr += "    <script>\n";
  ptr += "        var localIp = 'http://192.168.1.200:80/';\n";
  ptr += "        var lightOn = false;\n";
  ptr += "        var brightness = 0;\n";
  ptr += "        var curtain_1 = 0;\n";
  ptr += "        var curtain_2 = 0;\n";
  ptr += "        var curtain_3 = 0;\n";
  ptr += "        var prevBrightness = 0;\n";
  ptr += "        var mouseDown = false;\n";
  ptr += "        var moved = false;\n";
  ptr += "\n";
  ptr += "        class Regulator{\n";
  ptr += "            constructor( barId, lowerBarId, upperBarId, horizontal, numSteps, sliderWidth, startPos, endPos ){\n";
  ptr += "                this.barId = barId;\n";
  ptr += "                this.lowerBarId = lowerBarId;\n";
  ptr += "                this.upperBarId = upperBarId;\n";
  ptr += "                this.horizontal = horizontal;\n";
  ptr += "                this.numSteps = numSteps;\n";
  ptr += "                this.sliderWidth = sliderWidth;\n";
  ptr += "\n";
  ptr += "                this.startPos = startPos;\n";
  ptr += "                this.endPos = endPos;\n";
  ptr += "\n";
  ptr += "                this.maxValue = numSteps - sliderWidth;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            drawBar( value ){\n";
  ptr += "                let lowerBar = document.getElementById( this.lowerBarId );\n";
  ptr += "                let upperBar = document.getElementById( this.upperBarId );\n";
  ptr += "\n";
  ptr += "                let lowerBarY =  ( this.numSteps - value ) / this.numSteps;\n";
  ptr += "                let upperBarY = ( this.numSteps - this.sliderWidth - value ) / this.numSteps;\n";
  ptr += "                \n";
  ptr += "                if ( lowerBarY > 1 ){ lowerBarY = 1; }\n";
  ptr += "\n";
  ptr += "                if ( !this.horizontal ){\n";
  ptr += "                    lowerBar.style.height = lowerBarY * 100 + '%';\n";
  ptr += "                    upperBar.style.height = upperBarY * 100 + '%';\n";
  ptr += "                } else {\n";
  ptr += "                    lowerBar.style.width = lowerBarY * 100 + '%';\n";
  ptr += "                    upperBar.style.width = upperBarY * 100 + '%';\n";
  ptr += "                }\n";
  ptr += "            }\n";
  ptr += "            \n";
  ptr += "            update( x, y, value ){\n";
  ptr += "                if ( !this.horizontal ){\n";
  ptr += "                    const scrollbar = document.getElementById( this.barId );\n";
  ptr += "                    var screenHeight = scrollbar.clientHeight;\n";
  ptr += "                    \n";
  ptr += "                    if ( x > this.startPos && x < this.endPos ){\n";
  ptr += "\n";
  ptr += "                        let lowerBar = document.getElementById( this.lowerBarId );\n";
  ptr += "                        let upperBar = document.getElementById( this.upperBarId );\n";
  ptr += "\n";
  ptr += "                        let startY = screenHeight / this.numSteps * ( this.numSteps - this.sliderWidth * 3 - value );\n";
  ptr += "                        let endY = screenHeight / this.numSteps * ( this.numSteps + this.sliderWidth * 3 - value );\n";
  ptr += "\n";
  ptr += "                        if ( endY > screenHeight ){\n";
  ptr += "                            endY = screenHeight;\n";
  ptr += "                        }\n";
  ptr += "                        \n";
  ptr += "                        if ( y > startY && y < endY ){\n";
  ptr += "                            let middle = y / screenHeight * this.numSteps;\n";
  ptr += "                            value = this.numSteps - middle - this.sliderWidth / 2;\n";
  ptr += "\n";
  ptr += "                            value = parseInt( value );\n";
  ptr += "\n";
  ptr += "                            if ( value < 0 ){ value = 0; };\n";
  ptr += "                            if ( value > this.maxValue ){ value = this.maxValue; };\n";
  ptr += "                        }\n";
  ptr += "                    }\n";
  ptr += "                    return value;\n";
  ptr += "                    \n";
  ptr += "                } else {\n";
  ptr += "                    const scrollbar = document.getElementById( this.barId );\n";
  ptr += "                    var screenWidth = scrollbar.clientWidth;\n";
  ptr += "                    if ( y > this.startPos && y < this.endPos ){\n";
  ptr += "                       \n";
  ptr += "                        let lowerBar = document.getElementById( this.lowerBarId );\n";
  ptr += "                        let upperBar = document.getElementById( this.upperBarId );\n";
  ptr += "\n";
  ptr += "                        let startY = screenWidth / this.numSteps * ( this.numSteps - this.sliderWidth * 3 - value );\n";
  ptr += "                        let endY = screenWidth / this.numSteps * ( this.numSteps + this.sliderWidth * 3 - value );\n";
  ptr += "\n";
  ptr += "                        if ( endY > screenWidth ){\n";
  ptr += "                            endY = screenWidth;\n";
  ptr += "                        }\n";
  ptr += "\n";
  ptr += "                        if ( x > startY && x < endY ){\n";
  ptr += "                            let middle = x / screenWidth * this.numSteps;\n";
  ptr += "                            value = this.numSteps - middle - this.sliderWidth / 2;\n";
  ptr += "                            value = parseInt( value );\n";
  ptr += "\n";
  ptr += "                            if ( value < 0 ){ value = 0; };\n";
  ptr += "                            if ( value > this.maxValue ){ value = this.maxValue; };\n";
  ptr += "                            \n";
  ptr += "                        }\n";
  ptr += "                    }\n";
  ptr += "                    return value;\n";
  ptr += "                }\n";
  ptr += "                \n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        \n";
  ptr += "        let screenWidth = window.innerWidth;\n";
  ptr += "        let screenHeight = window.innerHeight;\n";
  ptr += "\n";
  ptr += "        var brightnessBar = new Regulator( 'brightnessRegulator', 'brightnessRegulator_lowerBound', 'brightnessRegulator_upperBound', false, 34, 4, screenWidth * 0.33, screenWidth * 0.66 )\n";
  ptr += "        var curtain_1_bar = new Regulator( 'curtain_1', 'curtain_1_lower', 'curtain_1_upper', true, 110, 10, screenHeight * 0.70, screenHeight * 0.80 )\n";
  ptr += "        var curtain_2_bar = new Regulator( 'curtain_2', 'curtain_2_lower', 'curtain_2_upper', true, 110, 10, screenHeight * 0.80, screenHeight * 0.90 )\n";
  ptr += "        var curtain_3_bar = new Regulator( 'curtain_3', 'curtain_3_lower', 'curtain_3_upper', true, 110, 10, screenHeight * 0.90, screenHeight * 1 )\n";
  ptr += "        \n";
  ptr += "        function getInfo(){ \n";
  ptr += "            //get info data\n";
  ptr += "            let data = fetch( localIp + '/command?action=getStates' ).then( ( response ) => { return response.text() } ).then( ( data ) => {\n";
  ptr += "                const splitted = data.split( ' ' );\n";
  ptr += "                console.log( splitted );\n";
  ptr += "                brightness = parseInt( splitted[ 1 ] );\n";
  ptr += "                lightOn = splitted[ 3 ];\n";
  ptr += "\n";
  ptr += "                if ( lightOn == 'false' ){ lightOn = false; } else { lightOn = true; }\n";
  ptr += "\n";
  ptr += "                //Check if light is on \n";
  ptr += "                //if it is on show page else show on button\n";
  ptr += "                let onButton = document.getElementById( 'onButton' );\n";
  ptr += "                if ( lightOn == false ){ onButton.style.zIndex = 999; } else { onButton.style.zIndex = -999; }\n";
  ptr += "                \n";
  ptr += "                let onButtonImage = document.getElementById( 'onButtonImage' );\n";
  ptr += "                if ( lightOn == false ){ onButtonImage.style.zIndex = 999; } else { onButtonImage.style.zIndex = -999; }\n";
  ptr += "                \n";
  ptr += "                brightnessBar.drawBar( brightness );\n";
  ptr += "                let brightnessRegulator = document.getElementById( 'brightnessRegulator' );\n";
  ptr += "                color = parseInt( ( 255 / 30 ) * brightness );\n";
  ptr += "                brightnessRegulator.style.backgroundColor = 'rgb(' + color + ',' + color + ',' + 0 + ')';\n";
  ptr += "            });\n";
  ptr += "            \n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        function send( action ){\n";
  ptr += "            fetch( localIp + '/command?action=' + action ).then( ( response ) => { console.log( response ); } );\n";
  ptr += "            setTimeout(function(){\n";
  ptr += "                getInfo();\n";
  ptr += "            }, 250);\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function turnLightOn(){\n";
  ptr += "            if ( lightOn == false ){\n";
  ptr += "                send( 'turnon' );\n";
  ptr += "            }\n";
  ptr += "            setTimeout(function(){\n";
  ptr += "                getInfo();\n";
  ptr += "            }, 250);\n";
  ptr += "            console.log( 'Gotten info' );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function turnLightOff(){\n";
  ptr += "            if ( lightOn == true ){\n";
  ptr += "                send( 'turnoff' );\n";
  ptr += "            }\n";
  ptr += "            setTimeout(function(){\n";
  ptr += "                getInfo();\n";
  ptr += "            }, 250);\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        function setMouseDown(){ \n";
  ptr += "            prevBrightness = brightness; \n";
  ptr += "            mouseDown = true;\n";
  ptr += "            moved = false;\n";
  ptr += "            console.log( 'down', moved );\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        function setMouseUp(){ \n";
  ptr += "            mouseDown = false; \n";
  ptr += "            console.log( 'up', moved );\n";
  ptr += "            if ( moved ){\n";
  ptr += "                let background = document.getElementById( 'brightnessRegulator_lowerBound' );\n";
  ptr += "                background.style.backgroundColor = 'red';\n";
  ptr += "                fetch( localIp + '/command?action=brighten&brightness=' + String( brightness ) ).then( ( response ) => { console.log( response ); } ); \n";
  ptr += "                setTimeout(function(){\n";
  ptr += "                    getInfo();\n";
  ptr += "                    background.style.backgroundColor = 'whitesmoke';\n";
  ptr += "                }, 250 * 30 );\n";
  ptr += "            }\n";
  ptr += "            moved = false;           \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        \n";
  ptr += "        \n";
  ptr += "        function handleMouse( event ){\n";
  ptr += "            if ( mouseDown ){\n";
  ptr += "                if ( event.type == 'touchmove' ){\n";
  ptr += "                    var x = event.touches[ 0 ].clientX;\n";
  ptr += "                    var y = event.touches[ 0 ].clientY;\n";
  ptr += "                } else {\n";
  ptr += "                    var x = event.x;\n";
  ptr += "                    var y = event.y;    \n";
  ptr += "                }\n";
  ptr += "                moved = true;\n";
  ptr += "\n";
  ptr += "                console.log( 'Handle', moved );\n";
  ptr += "\n";
  ptr += "                brightness = brightnessBar.update( x, y, brightness );\n";
  ptr += "                brightnessBar.drawBar( brightness );\n";
  ptr += "\n";
  ptr += "                let brightnessRegulator = document.getElementById( 'brightnessRegulator' );\n";
  ptr += "                color = parseInt( ( 255 / 30 ) * brightness );\n";
  ptr += "                brightnessRegulator.style.backgroundColor = 'rgb(' + color + ',' + color + ',' + 0 + ')';\n";
  ptr += "                \n";
  ptr += "\n";
  ptr += "                curtain_1 = curtain_1_bar.update( x, y, curtain_1 );\n";
  ptr += "                curtain_1_bar.drawBar( curtain_1 );\n";
  ptr += "\n";
  ptr += "                let curtain_1_background = document.getElementById( 'curtain_1' );\n";
  ptr += "                color = parseInt( ( 255 / 100 ) * curtain_1 );\n";
  ptr += "                curtain_1_background.style.backgroundColor = 'rgb(' + color + ',' + color + ',' + 0 + ')';\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "                curtain_2 = curtain_2_bar.update( x, y, curtain_2 );\n";
  ptr += "                curtain_2_bar.drawBar( curtain_2 );\n";
  ptr += "\n";
  ptr += "                let curtain_2_background = document.getElementById( 'curtain_2' );\n";
  ptr += "                color = parseInt( ( 255 / 100 ) * curtain_2 );\n";
  ptr += "                curtain_2_background.style.backgroundColor = 'rgb(' + color + ',' + color + ',' + 0 + ')';\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "                curtain_3 = curtain_3_bar.update( x, y, curtain_3 );\n";
  ptr += "                curtain_3_bar.drawBar( curtain_3 );\n";
  ptr += "\n";
  ptr += "                let curtain_3_background = document.getElementById( 'curtain_3' );\n";
  ptr += "                color = parseInt( ( 255 / 100 ) * curtain_3 );\n";
  ptr += "                curtain_3_background.style.backgroundColor = 'rgb(' + color + ',' + color + ',' + 0 + ')';\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function nightMode(){\n";
  ptr += "            send( 'nightmode' );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function update(){\n";
  ptr += "            setTimeout( function(){\n";
  ptr += "                getInfo();\n";
  ptr += "                update();\n";
  ptr += "            }, 60_000 );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function timer(){\n";
  ptr += "            if ( lightOn == false ){\n";
  ptr += "                setTimeout(function(){\n";
  ptr += "                    send( 'turnoff' );\n";
  ptr += "                }, 60_000);\n";
  ptr += "            } \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function stateChange(){\n";
  ptr += "            send( 'temp' );\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        function setSun(){\n";
  ptr += "            send( 'sun' );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function pressWird(){\n";
  ptr += "            send( 'wird' );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        update();\n";
  ptr += "\n";
  ptr += "        document.addEventListener( 'mousemove', handleMouse );\n";
  ptr += "        document.addEventListener( 'mousedown', setMouseDown );\n";
  ptr += "        document.addEventListener( 'mouseup', setMouseUp );\n";
  ptr += "\n";
  ptr += "        document.addEventListener( 'touchmove', handleMouse );\n";
  ptr += "        document.addEventListener( 'touchstart', setMouseDown );\n";
  ptr += "        document.addEventListener( 'touchend', setMouseUp );\n";
  ptr += "\n";
  ptr += "        const onButton = document.getElementById( 'onButton' );\n";
  ptr += "        onButton.addEventListener( 'click', turnLightOn );\n";
  ptr += "\n";
  ptr += "        const nightButton = document.getElementById( 'nightButton' );\n";
  ptr += "        nightButton.addEventListener( 'click', nightMode );\n";
  ptr += "\n";
  ptr += "        const timerButton = document.getElementById( 'timerButton' );\n";
  ptr += "        timerButton.addEventListener( 'click', timer );\n";
  ptr += "\n";
  ptr += "        const tempButton = document.getElementById( 'stateChange' );\n";
  ptr += "        tempButton.addEventListener( 'click', stateChange );\n";
  ptr += "\n";
  ptr += "        const sunButton = document.getElementById( 'sunButton' );\n";
  ptr += "        sunButton.addEventListener( 'click', setSun );\n";
  ptr += "\n";
  ptr += "        const wirdButton = document.getElementById( 'wirdButton' );\n";
  ptr += "        wirdButton.addEventListener( 'click', pressWird );\n";
  ptr += "\n";
  ptr += "        const offButton = document.getElementById( 'offButton' );\n";
  ptr += "        offButton.addEventListener( 'click', turnLightOff );\n";
  ptr += "    </script>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}