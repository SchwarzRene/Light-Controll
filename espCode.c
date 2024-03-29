#include <WiFi.h>
#include <WebServer.h>
#include "time.h"

#define TURNON 22
#define TURNOFF 22
#define BRIGHTEN 21
#define DIM 18
#define NIGHTMODE 16
#define TEMP 2
#define WIRD 27
#define SUN 33


#define MAX_BRIGHTNESS 30


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


const char* ssid = "...";
const char* password = "...";

IPAddress local_IP( 192, 168, 1, 200 );
IPAddress gateway( 192, 168, 1, 1 );
IPAddress subnet( 255, 255, 0, 0 );
IPAddress dns(8,8,8,8);

int brightness = 0;
int onOffState = 0;

float timerTimes[10] = {30,30,30,30,30,30,30,30,30,30};
bool timerStates[10] = {30,30,30,30,30,30,30,30,30,30};

WebServer server(80);

void setup(){
    Serial.begin( 115200 );

    if ( !WiFi.config( local_IP, gateway, subnet, dns ) ){
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
    server.on( "/getstates", getStates );

    server.on( "/deleteTimer/timerId /{}", [](){
      int Id = server.pathArg( 0 ).toInt();
      timerTimes[ Id ] = 30;
      server.send( 200 );
    });

    server.on( "/timer/hour/{}/minute/{}/state/{}", [](){
      float h = server.pathArg(0).toFloat();
      float m = server.pathArg(1).toFloat();
      bool s;
      if ( server.pathArg(2) == "true" ){ s = true; } else { s = false; };

      float gesTime = h + m / 60;

      int idx = 0;
       
      while ( true ){
        if ( timerTimes[ idx ] == 30 ){
          timerTimes[ idx ] = gesTime;
          timerStates[ idx ] = s;
          break;
        } else {
          idx = idx + 1;
        }
      }

      Serial.println( "\ntimerTimes" );
      for ( int i = 0; i < 10; i++ ){
        Serial.print( timerTimes[ i ] );
        Serial.print( " " );
      }
      
      Serial.println( "\nTimerStates" );
      for ( int i = 0; i < 10; i++ ){
        Serial.print( timerStates[ i ] );
        Serial.print( " " );
      }
      
      server.send( 200, "text/plain", (String)idx );
    });

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

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov");
}

float getCurrentTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  
  float hour = (float)timeinfo.tm_hour;
  float minute = (float)timeinfo.tm_min;

  float gesTime = hour + minute / 60;
  return gesTime;
}

void loop(){
  server.handleClient();
  float cTime = getCurrentTime();

  for ( int i = 0; i < 10; i++ ){
    if ( timerTimes[ i ] != 30 ){
      if ( timerTimes[ i ] == cTime ){
        if ( timerStates[ i ] == true ){
          if ( onOffState == 0 ){ pressButton( TURNON ); onOffState = 1; }
        } else { if ( onOffState == 1 ){ pressButton( TURNOFF ); onOffState = 0; }
        }
        timerTimes[ i ] = 30;
      }
    }
  }
}


void send_html(){
  server.send( 200, "text/html", getHTML() );
}


void resetBrightness(){
  brightness = 0;
  for ( int i = 0; i < 50; i = i + 1 ){
      pressButton( DIM );
  }
  server.send( 200 );
}

void getStates(){
  String finishedMessage = "br ightness: " + String( brightness );
  if ( onOffState == 1 ){
    finishedMessage = finishedMessage + " | onOff: true |";
  } else {
    finishedMessage = finishedMessage + " | onOff: false |";
  }

  for ( int i = 0; i < 10; i++ ){
    finishedMessage = finishedMessage + (String)timerTimes[i] + " ";
  }

  finishedMessage = finishedMessage + "|";
  for ( int i = 0; i < 10; i++ ){
    finishedMessage = finishedMessage + (String)timerStates[i] + " ";
  }
  Serial.println( finishedMessage );
  server.send( 200, "text/plain", finishedMessage ); 
}

void pressButton( int pinNum ){
  digitalWrite( pinNum, HIGH );
  delay( 100 ); 
  digitalWrite( pinNum, LOW );
}


void turnon(){
  Serial.println( "Turning On" );
  if ( onOffState == 0 ){
    pressButton( TURNON );
    onOffState = 1;
  }
  server.send( 200 );
}


void turnoff(){
  Serial.println( "Turing off" );
  if ( onOffState == 1 ){ 
    pressButton( TURNOFF );
    onOffState = 0;
  }
  server.send( 200 );
}

void brighten(){
  if ( brightness < MAX_BRIGHTNESS ){
    pressButton( BRIGHTEN );
    brightness = brightness + 1; 
  }
  server.send( 200 );
}
  
void resetOnOff(){
  if ( onOffState == 0 ){
    onOffState = 1;
  } else {
    onOffState = 0;
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
  if ( onOffState == 1 ){
    pressButton( NIGHTMODE );
    brightness = 0;
  }
  server.send( 200 );
}

void temp(){
  if ( onOffState == 1 ){
    pressButton( TEMP );
    brightness = MAX_BRIGHTNESS;
  }
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
  String ptr = "<!DOCTYPE html>\n";
  ptr += "<html lang='en'>\n";
  ptr += "<head>\n";
  ptr += "    <meta charset='UTF-8'>\n";
  ptr += "    <title>Futuristic Light Control Webpage</title>\n";
  ptr += "    <style>\n";
  ptr += "        @import url(https://fonts.googleapis.com/css?family=Shadows+Into+Light);\n";
  ptr += "        body {\n";
  ptr += "            background-color: #2c3e50;\n";
  ptr += "            color: #ecf0f1;\n";
  ptr += "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n";
  ptr += "            margin: 0 auto;\n";
  ptr += "            padding: 20px;\n";
  ptr += "            background-color: #151843;\n";
  ptr += "            overflow:hidden;\n";
  ptr += "            touch-action: none;\n";
  ptr += "            transition: 2s;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .container {\n";
  ptr += "            background-color: #34495e;\n";
  ptr += "            border-radius: 15px;\n";
  ptr += "            box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);\n";
  ptr += "            margin: auto;\n";
  ptr += "            padding: 17px;\n";
  ptr += "\n";
  ptr += "            height: 80vh;\n";
  ptr += "            margin: 0 auto;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( min-width: 1000px ){\n";
  ptr += "            .container{\n";
  ptr += "                position: relative;\n";
  ptr += "                left: -20%;\n";
  ptr += "                width: 50%;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( max-width: 1000px ){\n";
  ptr += "            .container{\n";
  ptr += "                width: 80%;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( max-width: 1000px ) {\n";
  ptr += "            .slider-container {\n";
  ptr += "                margin: 30px 0;\n";
  ptr += "                top: 20vh;\n";
  ptr += "                height: 20vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .slider {\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 20vh;\n";
  ptr += "                width: 70%;\n";
  ptr += "                height: 20vh;\n";
  ptr += "                left: 15%;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( min-width: 1000px ){\n";
  ptr += "            .slider-container {\n";
  ptr += "                margin: 30px 0;\n";
  ptr += "                top: -10vh;\n";
  ptr += "                height: 20vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .slider {\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 41vh;\n";
  ptr += "                width: 70%;\n";
  ptr += "                height: 20vh;\n";
  ptr += "                left: 15%;\n";
  ptr += "            }\n";
  ptr += "        } \n";
  ptr += "\n";
  ptr += "        \n";
  ptr += "\n";
  ptr += "        .slider::-webkit-slider-thumb{\n";
  ptr += "            width: 10vw;\n";
  ptr += "            height: 10vh;\n";
  ptr += "            background-color: white;\n";
  ptr += "            -webkit-appearance: none;\n";
  ptr += "            appearance: none;\n";
  ptr += "            border-radius: 10%; \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        \n";
  ptr += "        .state-buttons{\n";
  ptr += "            width: 10vh;\n";
  ptr += "            height: 10vh;\n";
  ptr += "            \n";
  ptr += "            border-radius: 20%;\n";
  ptr += "            text-align: center;\n";
  ptr += "            margin: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( max-width: 1000px ){\n";
  ptr += "            .state-buttons{\n";
  ptr += "                font-size: 3vh;\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 40vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #night-mode{\n";
  ptr += "                left: 15vw;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #reset-on-off{\n";
  ptr += "                left: 40vw;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #color-change {\n";
  ptr += "                right: 15vw;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( min-width: 1000px ){\n";
  ptr += "            .state-buttons{\n";
  ptr += "                font-size: 6vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #reset-on-off{\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 5vh;\n";
  ptr += "                left: 6vw;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #color-change{\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 46vh;\n";
  ptr += "                left: 45vw;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #night-mode {\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 46vh;\n";
  ptr += "                left: 1vw;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        \n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .timer-controls {\n";
  ptr += "            margin-top: 10px;\n";
  ptr += "            left: 0;\n";
  ptr += "            height\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        .timer-row label, .timer-row input, .timer-row select, .timer-row {\n";
  ptr += "            margin-right: 10px;\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        .timer-list {\n";
  ptr += "            text-align: center;\n";
  ptr += "            background-color: #16a085;\n";
  ptr += "            border-radius: 5px;\n";
  ptr += "            padding: 10px;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( max-width: 1000px ){\n";
  ptr += "            .timer-list {\n";
  ptr += "                position: absolute;\n";
  ptr += "                top: 75%;\n";
  ptr += "                left: 10%;\n";
  ptr += "                right: 10%;\n";
  ptr += "            }\n";
  ptr += "        } \n";
  ptr += "\n";
  ptr += "        @media ( min-width: 1000px ){\n";
  ptr += "            .timer-list {\n";
  ptr += "                position: absolute;\n";
  ptr += "                width: 40%;\n";
  ptr += "                top: 21px;\n";
  ptr += "                margin: 0 auto;\n";
  ptr += "                left:55.8%;\n";
  ptr += "            }\n";
  ptr += "        } \n";
  ptr += "        \n";
  ptr += "        .timer-item {\n";
  ptr += "            align-items: center;\n";
  ptr += "            border-bottom: 1px solid #ecf0f1;\n";
  ptr += "            display: flex;\n";
  ptr += "            justify-content: space-between;\n";
  ptr += "            padding: 10px 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .timer-item:last-child {\n";
  ptr += "            border-bottom: none;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( max-width: 1000px ){\n";
  ptr += "            .futuristic-input, .futuristic-select {\n";
  ptr += "            position: absolute;\n";
  ptr += "            background-color: #34495e;\n";
  ptr += "            border: 1px solid #ecf0f1;\n";
  ptr += "            border-radius: 5px;\n";
  ptr += "            color: white;\n";
  ptr += "            font-size: 60px;\n";
  ptr += "            padding: 0;\n";
  ptr += "            width: 25%;\n";
  ptr += "            top: 55vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .futuristic-input{\n";
  ptr += "                left: 12%;\n";
  ptr += "                height: 10%;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .futuristic-select{\n";
  ptr += "                left: 38.5%;\n";
  ptr += "                height: 10%;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #addTimer{\n";
  ptr += "                position: absolute;\n";
  ptr += "                right: 12%; \n";
  ptr += "                top: 55vh;\n";
  ptr += "                width: 20%; \n";
  ptr += "                height: 10%; \n";
  ptr += "                margin: 0; \n";
  ptr += "                padding: 0; \n";
  ptr += "                border: 1px solid white; \n";
  ptr += "                color: white;\n";
  ptr += "                font-size: 3vh;\n";
  ptr += "                background-color: #34495e;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @media ( min-width: 1000px ){\n";
  ptr += "            .futuristic-input, .futuristic-select {\n";
  ptr += "            position: absolute;\n";
  ptr += "            background-color: #34495e;\n";
  ptr += "            border: 1px solid #ecf0f1;\n";
  ptr += "            border-radius: 5px;\n";
  ptr += "            color: white;\n";
  ptr += "            font-size: 60px;\n";
  ptr += "            padding: 0;\n";
  ptr += "            top: 65vh;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .futuristic-input{\n";
  ptr += "                width: 30%;\n";
  ptr += "                left: 4%;\n";
  ptr += "                height: 10%;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            .futuristic-select{\n";
  ptr += "                left: 38%;\n";
  ptr += "                width: 30%;\n";
  ptr += "                height: 10.5%;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #addTimer{\n";
  ptr += "                position: absolute;\n";
  ptr += "                right: 4%; \n";
  ptr += "                width: 24%; \n";
  ptr += "                height: 10.5%; \n";
  ptr += "                top: 65vh;\n";
  ptr += "                margin: 0; \n";
  ptr += "                padding: 0; \n";
  ptr += "                border: 1px solid white; \n";
  ptr += "                color: white;\n";
  ptr += "                font-size: 3vh;\n";
  ptr += "                background-color: #34495e;\n";
  ptr += "                border-radius: 5px;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        .delete-timer {\n";
  ptr += "            background-color: #c0392b;\n";
  ptr += "            border-radius: 50%;\n";
  ptr += "            color: white;\n";
  ptr += "            cursor: pointer;\n";
  ptr += "            font-size: 0.8em;\n";
  ptr += "            padding: 5px 8px;\n";
  ptr += "        }\n";
  ptr += "        .delete-timer:hover {\n";
  ptr += "            background-color: #e74c3c;\n";
  ptr += "        }\n";
  ptr += "    \n";
  ptr += "\n";
  ptr += "        #wrapper {\n";
  ptr += "            margin: 0 auto;\n";
  ptr += "            position: relative;\n";
  ptr += "            margin-top: 1%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .planet {\n";
  ptr += "        margin: 0 auto;\n";
  ptr += "        width: 300px;\n";
  ptr += "        height: 300px;\n";
  ptr += "        background-color: #ebf3fe;\n";
  ptr += "        display: block;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        position: relative;\n";
  ptr += "        box-shadow: inset -40px 0px 0px #d8e8f7, inset 20px 0px 0px #ffffff, inset -50px 0px 0px 20px #e2eefa, 0px 0px 0px 20px  rgba(255, 255, 255, 0.05), 0px 0px 0px 40px  rgba(255, 255, 255, 0.025), 0px 0px 0px 60px  rgba(255, 255, 255, 0.0125);\n";
  ptr += "        border: solid 10px black;\n";
  ptr += "        transition: all 0.2s ease-in;\n";
  ptr += "        }\n";
  ptr += "    \n";
  ptr += "\n";
  ptr += "        .planet:after {\n";
  ptr += "        content: '';\n";
  ptr += "        width: 50px;\n";
  ptr += "        height: 50px;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        background-color: #d8e8f7;\n";
  ptr += "        position: absolute;\n";
  ptr += "        top: 20%;\n";
  ptr += "        left: 20%;\n";
  ptr += "        box-shadow: 40px -20px 0px -10px #d8e8f7, 40px 10px 0px -15px #d8e8f7;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .face {\n";
  ptr += "        position: absolute;\n";
  ptr += "        width: 100px;\n";
  ptr += "        top: 50%;\n";
  ptr += "        left: 40%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .zzz1, .zzz2, .zzz3{\n";
  ptr += "        color:white;\n";
  ptr += "        position:absolute;\n";
  ptr += "        top:0;\n";
  ptr += "        right:10%;\n";
  ptr += "        font-size:36px;\n";
  ptr += "        opacity:0;\n";
  ptr += "        font-family: 'Shadows Into Light', cursive;\n";
  ptr += "            animation-name: zzz-sleep;\n";
  ptr += "        animation-duration: 5s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease-in-out;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .zzz1:after, .zzz2:after, .zzz3:after {\n";
  ptr += "        content:'z';\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .zzz1 {\n";
  ptr += "        animation-delay: 0s;\n";
  ptr += "        \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .zzz2 {\n";
  ptr += "        animation-delay: 1.2s;\n";
  ptr += "        right:15%;\n";
  ptr += "        top:3%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .zzz3 {\n";
  ptr += "        animation-delay: 2.4s;\n";
  ptr += "        right:10%;\n";
  ptr += "        top:6%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .day .mouth {\n";
  ptr += "        margin-top: 15px;\n";
  ptr += "        height: 15px;\n";
  ptr += "        width: 15px;\n";
  ptr += "        display: block;\n";
  ptr += "        border-radius: 0px 0px 50% 50%;\n";
  ptr += "        background-color: transparent;\n";
  ptr += "        float: left;\n";
  ptr += "        border: 6px solid #151843;\n";
  ptr += "        border-top: 0;\n";
  ptr += "        margin-right: 10px;\n";
  ptr += "        position: relative;\n";
  ptr += "        margin-left:0px;\n";
  ptr += "        animation-name: none;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .mouth {\n";
  ptr += "        margin-top: 15px;\n";
  ptr += "        height: 25px;\n";
  ptr += "        width: 25px;\n";
  ptr += "        display: block;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        background-color: #151843;\n";
  ptr += "        float: left;\n";
  ptr += "        border: none;\n";
  ptr += "        border-top: 0;\n";
  ptr += "        margin-right: 10px;\n";
  ptr += "        margin-left:2px;\n";
  ptr += "        position: relative;\n";
  ptr += "        \n";
  ptr += "            animation-name: mouth-sleep;\n";
  ptr += "        animation-duration: 5s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease-out;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .mouth:after {\n";
  ptr += "        content: '';\n";
  ptr += "        width: 6px;\n";
  ptr += "        height: 6px;\n";
  ptr += "        position: absolute;\n";
  ptr += "        display: block;\n";
  ptr += "        top: -3px;\n";
  ptr += "        left: -6px;\n";
  ptr += "        background-color: #151843;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .mouth:before {\n";
  ptr += "        content: '';\n";
  ptr += "        width: 6px;\n";
  ptr += "        height: 6px;\n";
  ptr += "        position: absolute;\n";
  ptr += "        display: block;\n";
  ptr += "        top: -3px;\n";
  ptr += "        left: 15px;\n";
  ptr += "        background-color: #151843;\n";
  ptr += "        border-radius: 50%\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .eye {\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        display: block;\n";
  ptr += "        float: left;\n";
  ptr += "        margin-right: 10px;\n";
  ptr += "        animation-name: eye-blink;\n";
  ptr += "        animation-duration: 10s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease-out;\n";
  ptr += "        position: relative;\n";
  ptr += "        overflow: hidden\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .eye {\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        display: block;\n";
  ptr += "        float: left;\n";
  ptr += "        margin-right: 8px;\n";
  ptr += "        position: relative;\n";
  ptr += "\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .eye-in {\n";
  ptr += "        content: '';\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        background-color: #151843;\n";
  ptr += "        display: block;\n";
  ptr += "        position: absolute;\n";
  ptr += "        top: 0px;\n";
  ptr += "        left: 0px;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        transform: scale(1, 1) !important;\n";
  ptr += "        border:none;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .eye-in {\n";
  ptr += "        width: 15px;\n";
  ptr += "        height: 8px;\n";
  ptr += "        display: block;\n";
  ptr += "        position: absolute;\n";
  ptr += "        top: 5px;\n";
  ptr += "        left: -1px;\n";
  ptr += "            border-bottom-left-radius: 15px;\n";
  ptr += "            border-bottom-right-radius: 15px;\n";
  ptr += "            border: 5px solid #151843;\n";
  ptr += "            border-top: 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .pos-star1 {\n";
  ptr += "        top: 50px;\n";
  ptr += "        left: 50px;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .pos-star2 {\n";
  ptr += "        top: 250px;\n";
  ptr += "        left: 450px;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .pos-star3 {\n";
  ptr += "        top: 240px;\n";
  ptr += "        left: 5px;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .star {\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        background-color: white;\n";
  ptr += "        display: block;\n";
  ptr += "        border-radius: 50%;\n";
  ptr += "        float: left;\n";
  ptr += "        position: absolute;\n";
  ptr += "        transform-origin: 50% 50%;\n";
  ptr += "        animation-name: star3;\n";
  ptr += "        animation-duration: 20s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease;\n";
  ptr += "        transform-origin: 50% 50%;\n";
  ptr += "        transition: all 0.3s ease-out;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .star:before {\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        content: '';\n";
  ptr += "        background-color: white;\n";
  ptr += "        position: absolute;\n";
  ptr += "        transform: scale(1, 2) rotate(45deg);\n";
  ptr += "        animation-name: star1;\n";
  ptr += "        animation-duration: 3s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease-out;\n";
  ptr += "        border-radius: 20%;\n";
  ptr += "        transition: all 0.3s ease-out;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .star:after {\n";
  ptr += "        width: 20px;\n";
  ptr += "        height: 20px;\n";
  ptr += "        content: '';\n";
  ptr += "        background-color: white;\n";
  ptr += "        position: absolute;\n";
  ptr += "        transform: scale(2, 1) rotate(45deg);\n";
  ptr += "        animation-name: star2;\n";
  ptr += "        animation-duration: 3s;\n";
  ptr += "        animation-iteration-count: infinite;\n";
  ptr += "        animation-timing-function: ease-out;\n";
  ptr += "        animation-delay: 0.2s;\n";
  ptr += "        border-radius: 20%;\n";
  ptr += "        transition: all 0.3s ease-out;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes star1 {\n";
  ptr += "        0% {\n";
  ptr += "            transform: scale(1, 2) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        50% {\n";
  ptr += "            transform: scale(1, 1) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        100% {\n";
  ptr += "            transform: scale(1, 2) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes star2 {\n";
  ptr += "        0% {\n";
  ptr += "            transform: scale(2, 1) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        50% {\n";
  ptr += "            transform: scale(1, 1) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        100% {\n";
  ptr += "            transform: scale(2, 1) rotate(45deg);\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes star3 {\n";
  ptr += "        0% {\n";
  ptr += "            transform: rotate(0deg);\n";
  ptr += "        }\n";
  ptr += "        100% {\n";
  ptr += "            transform: rotate(360deg);\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes eye-blink {\n";
  ptr += "        0% {\n";
  ptr += "            transform: scale(1, 1)\n";
  ptr += "        }\n";
  ptr += "        2% {\n";
  ptr += "            transform: scale(1, 0)\n";
  ptr += "        }\n";
  ptr += "        4% {\n";
  ptr += "            transform: scale(1, 1)\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes mouth-sleep {\n";
  ptr += "        0% {\n";
  ptr += "            transform: scale(1, 1)\n";
  ptr += "        }\n";
  ptr += "        35% {\n";
  ptr += "            transform: scale(0.7, 0.7)\n";
  ptr += "        }\n";
  ptr += "        40% {\n";
  ptr += "            transform: scale(0.7, 0.7)\n";
  ptr += "        }\n";
  ptr += "        75% {\n";
  ptr += "            transform: scale(1, 1)\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes wiggle {\n";
  ptr += "        0% {\n";
  ptr += "            transform: translate(0px, 0px)\n";
  ptr += "        }\n";
  ptr += "        50% {\n";
  ptr += "            transform: translate(50px, 0px)\n";
  ptr += "        }\n";
  ptr += "        100% {\n";
  ptr += "            transform: translate(0px, 0px)\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        @keyframes zzz-sleep {\n";
  ptr += "        0% {\n";
  ptr += "            transform: translate(0px, 0px) scale(1, 1);\n";
  ptr += "            opacity:0;\n";
  ptr += "\n";
  ptr += "        }\n";
  ptr += "        25% {\n";
  ptr += "            transform: translate(-5px, -10px) scale(1.1, 1.1);\n";
  ptr += "        }\n";
  ptr += "        50% {\n";
  ptr += "            transform: translate(8px, -20px) scale(0.9, 0.9);\n";
  ptr += "            opacity:1;\n";
  ptr += "        }\n";
  ptr += "        75% {\n";
  ptr += "            transform: translate(-15px, -30px) scale(1.1, 1.1);\n";
  ptr += "        }\n";
  ptr += "        100% {\n";
  ptr += "            transform: translate(15px, -40px) scale(0.9, 0.9);\n";
  ptr += "            opacity:0;\n";
  ptr += "        }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        @keyframes zzz-awake {\n";
  ptr += "        0% {\n";
  ptr += "            transform: translate(0px, 0px) scale(1, 1) rotate(0deg);\n";
  ptr += "\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        100% {\n";
  ptr += "            transform: translate(-15px, -30px) scale(2, 2) rotate(10deg);\n";
  ptr += "                opacity:0;\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        /* DAY TIME */\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .day .planet {\n";
  ptr += "        background-color: #ffdb01 !important;\n";
  ptr += "        box-shadow: inset -40px 0px 0px #ff8603, inset 20px 0px 0px #ffffff, inset -50px 0px 0px 20px #ffd201, 0px 0px 0px 20px  rgba(255, 210, 1, 0.2), 0px 0px 0px 40px  rgba(255, 210, 1, 0.1), 0px 0px 0px 60px  rgba(255, 210, 1, 0.05);\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .planet:after {\n";
  ptr += "        background-color: #ffd201;\n";
  ptr += "        box-shadow: 40px -20px 0px -10px #ffd201, 40px 10px 0px -15px #ffd201;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .star:after,\n";
  ptr += "        .day .star:before {\n";
  ptr += "        animation-name: none;\n";
  ptr += "        transform: none;\n";
  ptr += "        width: 50px;\n";
  ptr += "        height: 50px;\n";
  ptr += "        border-radius: 50%\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .star:after {\n";
  ptr += "        left: 150%;\n";
  ptr += "        box-shadow: -50px -25px 0px 10px white, -10px -25px 0px -5px white, inset -7px 0px 0px 3px #ebf3fe;\n";
  ptr += "        } \n";
  ptr += "\n";
  ptr += "        .day .star:before {\n";
  ptr += "        left: 50%;\n";
  ptr += "        top: 0;\n";
  ptr += "        border-radius: 0;\n";
  ptr += "        width: 75px;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        @media (min-width: 1000px ){\n";
  ptr += "            #nightbg {\n";
  ptr += "            background-color: lightblue;\n";
  ptr += "            width:100px;\n";
  ptr += "            height:100px;\n";
  ptr += "            position:absolute;\n";
  ptr += "            top: 100px;\n";
  ptr += "            left: calc(50% - 50px);\n";
  ptr += "            border-radius:50%;\n";
  ptr += "            transition:0.7s;\n";
  ptr += "            z-index: -10;\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            #nightbg:before {\n";
  ptr += "            content:'';\n";
  ptr += "            width:200px;\n";
  ptr += "            height:200px;\n";
  ptr += "            border-radius:50%;\n";
  ptr += "            background-color:rgba(173, 216, 230, 0.1);\n";
  ptr += "            position:absolute;\n";
  ptr += "            left:-50px;\n";
  ptr += "            top:-50px;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .star {\n";
  ptr += "        width: 50px;\n";
  ptr += "        height: 50px;\n";
  ptr += "        animation-name: wiggle;\n";
  ptr += "        animation-duration: 10s;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .zzz1:after, .day .zzz2:after, .day .zzz3:after {\n";
  ptr += "        content:'!';\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .zzz1, .day .zzz2, .day .zzz3{\n";
  ptr += "        color:white;\n";
  ptr += "        top:0;\n";
  ptr += "        right:10%;\n";
  ptr += "            animation-name: zzz-awake;\n";
  ptr += "        animation-duration: 1s;\n";
  ptr += "        animation-iteration-count: 1;\n";
  ptr += "        animation-timing-function: ease-in-out;\n";
  ptr += "        animation-fill-mode: forwards;\n";
  ptr += "        opacity:1;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        .day .zzz1 {\n";
  ptr += "\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .zzz2 {\n";
  ptr += "        animation-delay: 0.2s;\n";
  ptr += "        right:15% !important; \n";
  ptr += "        top:3%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        .day .zzz3 {\n";
  ptr += "        animation-delay: 0.4s;\n";
  ptr += "        right:10% !important;\n";
  ptr += "        top:6%;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        #standby{\n";
  ptr += "            position: absolute;\n";
  ptr += "            width: 100vw;\n";
  ptr += "            height: 100vh;\n";
  ptr += "            top: 0;\n";
  ptr += "            left: 0;\n";
  ptr += "            z-index: -999;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "    </style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "    <div class='container'>\n";
  ptr += "        <div id='wrapper'>\n";
  ptr += "            <div id='nightbg'></div>\n";
  ptr += "            <div class='zzz1'></div>\n";
  ptr += "            <div class='zzz2'></div>\n";
  ptr += "            <div class='zzz3'></div>\n";
  ptr += "            <div class='planet' id = 'button' onclick='toggleLight()'> \n";
  ptr += "            <div class='planet-circle'></div>\n";
  ptr += "            <div class='face'>\n";
  ptr += "                <div class='eye'>\n";
  ptr += "                <div class='eye-in'></div>\n";
  ptr += "                </div>\n";
  ptr += "                <div class='mouth'></div>\n";
  ptr += "                <div class='eye'>\n";
  ptr += "                <div class='eye-in'></div>\n";
  ptr += "                </div>\n";
  ptr += "            </div>\n";
  ptr += "            </div>\n";
  ptr += "            <div class='star pos-star1 '></div>\n";
  ptr += "            <div class='star pos-star2 '></div>\n";
  ptr += "            <div class='star pos-star3 '></div>\n";
  ptr += "        </div>\n";
  ptr += "        <div class='slider-container'>\n";
  ptr += "            <input type='range' class='slider' id='brightnessSlider' min='0' max='30' value='10'>\n";
  ptr += "            <button id = 'night-mode' class = 'state-buttons'>🌛</button>\n";
  ptr += "            <button id = 'color-change' class = 'state-buttons'>🔯</button>\n";
  ptr += "        </div>\n";
  ptr += "        <div class='timer-controls'>\n";
  ptr += "            <div class='timer-row' style = 'height: 20vh'>\n";
  ptr += "                <input type='time' id='timerInput' class='futuristic-input' value = '00:00'>\n";
  ptr += "                <button id='addTimer'>Add Timer</button>\n";
  ptr += "                <select id='timerAction' class='futuristic-select'>\n";
  ptr += "                    <option value='on'>ON</option>\n";
  ptr += "                    <option value='off'>OFF</option>\n";
  ptr += "                </select>\n";
  ptr += "                \n";
  ptr += "            </div>\n";
  ptr += "        </div>\n";
  ptr += "    </div>\n";
  ptr += "\n";
  ptr += "    <button id = 'reset-on-off' class = 'state-buttons'>🔄</button>\n";
  ptr += "\n";
  ptr += "    <div class='timer-list' id='timerList'>\n";
  ptr += "        <h2>Set Timers</h2>\n";
  ptr += "    </div>\n";
  ptr += "\n";
  ptr += "    <div id = 'standby'></div>\n";
  ptr += "\n";
  ptr += "    <script>\n";
  ptr += "        //Defining the esp32 ip address in the network\n";
  ptr += "        const espIp = 'http://192.168.1.200:80/'\n";
  ptr += "        //Defining the maximum timeout until the screen gets black\n";
  ptr += "        const MAXTIMEOUT = 120;\n";
  ptr += "        //Defining the update ratio of the buttons\n";
  ptr += "        var updateTimeOut = 2500;\n";
  ptr += "\n";
  ptr += "        const toggleLightButton = document.getElementById('button');\n";
  ptr += "        const brightnessSlider = document.getElementById('brightnessSlider');\n";
  ptr += "        const timerInput = document.getElementById('timerInput');\n";
  ptr += "        const timerAction = document.getElementById('timerAction');\n";
  ptr += "        const addTimerButton = document.getElementById('addTimer');\n";
  ptr += "        const timerList = document.getElementById('timerList');\n";
  ptr += "        const nightmodeButton = document.getElementById( 'night-mode' );\n";
  ptr += "        const colorChangeButton = document.getElementById( 'color-change' );\n";
  ptr += "        const resetButton = document.getElementById( 'reset-on-off' );\n";
  ptr += "\n";
  ptr += "        const timers = [];\n";
  ptr += "        let lightOn = false;\n";
  ptr += "        var currentBrightValue = 0;\n";
  ptr += "        var toSetBrightValue = 0;\n";
  ptr += "        var changeBrightness = false;\n";
  ptr += "\n";
  ptr += "        function updateStates(){\n";
  ptr += "            //Get the on/off state and the brightness of the light and update the website\n";
  ptr += "            //Also update all timers\n";
  ptr += "            fetch( espIp + 'getstates' ).then( response => response.text() ).then( body => { \n";
  ptr += "                let splitted = body.split( '|' );\n";
  ptr += "\n";
  ptr += "                //set the light state to the correct value\n";
  ptr += "                if ( splitted[ 1 ].includes( 'true' ) ){\n";
  ptr += "                    lightOn = true;\n";
  ptr += "                } else{\n";
  ptr += "                    lightOn = false;\n";
  ptr += "                }\n";
  ptr += "\n";
  ptr += "                //set the light brightness to the correct value\n";
  ptr += "                let value = splitted[ 0 ].split( ':' )[ 1 ];\n";
  ptr += "                let brightness = parseInt( value.trim() );\n";
  ptr += "                \n";
  ptr += "                currentBrightValue = brightness;\n";
  ptr += "                brightnessSlider.value = brightness;\n";
  ptr += "                \n";
  ptr += "                //If the light is switched on by another devise aktivate this device again\n";
  ptr += "                if ( lightOn ){\n";
  ptr += "                    removeStandBy();\n";
  ptr += "                }\n";
  ptr += "                \n";
  ptr += "                //Update the website\n";
  ptr += "                updateLightStatus();\n";
  ptr += "                updateBrightness();\n";
  ptr += "\n";
  ptr += "                //Timers are a list which consit of the time and the on/off action, the list can hold maximum up to 10 elements\n";
  ptr += "                var timerTimes = splitted[ 2 ].split( ' ' ); //Time\n";
  ptr += "                var timerStates = splitted[ 3 ].split( ' ' ); //On/OFF\n";
  ptr += "\n";
  ptr += "                //The timerlist is a list with time entities \n";
  ptr += "                //Check if every entity is displayed and up to date else change the description of the timer\n";
  ptr += "                for ( let i = 0; i < 10; i = i + 1 ){\n";
  ptr += "                    time = timerTimes[ i ];\n";
  ptr += "                    let timerEntry = document.getElementById( 'timerId-' + i.toString() );\n";
  ptr += "                    \n";
  ptr += "                    let displayTime = time.toString();\n";
  ptr += "                    let splittedDisplayTime = displayTime.split( '.' );\n";
  ptr += "                    let hours = splittedDisplayTime[ 0 ];\n";
  ptr += "                    let minutes = splittedDisplayTime[ 1 ];\n";
  ptr += "\n";
  ptr += "                    //convert time as decimal back to hours and minutes\n";
  ptr += "                    minutes = '0.' + minutes;\n";
  ptr += "                    minutes = parseFloat( minutes ) * 60;\n";
  ptr += "                    minutes = minutes.toFixed( 0 );\n";
  ptr += "\n";
  ptr += "                    //convert minutes back to string and fill up the values to get 00:00 and not 0:0\n";
  ptr += "                    minutes = minutes.toString();\n";
  ptr += "                    if ( minutes.length == 1 ){\n";
  ptr += "                        minutes = '0' + minutes;\n";
  ptr += "                    }\n";
  ptr += "                    \n";
  ptr += "                    if ( hours.length == 1 ){\n";
  ptr += "                        hours = '0' + hours;\n";
  ptr += "                    }\n";
  ptr += "                    displayTime = hours + ':' + minutes;\n";
  ptr += "\n";
  ptr += "                    //Convert the action from 0/1 to OFF/ON\n";
  ptr += "                    let action;\n";
  ptr += "                    if ( timerStates[ i ] == 1 ){\n";
  ptr += "                        action = 'ON';\n";
  ptr += "                    } else {\n";
  ptr += "                        action = 'OFF';\n";
  ptr += "                    }\n";
  ptr += "                    \n";
  ptr += "                    //Check if the timer is already displayed\n";
  ptr += "                    if ( timerEntry != null ){\n";
  ptr += "                        //If timer got deleted from other device delete timer\n";
  ptr += "                        if ( time == 30 ){\n";
  ptr += "                            const index = timers.findIndex(t => t.timerId === i);\n";
  ptr += "                            if (index !== -1) {\n";
  ptr += "                                timers.splice(index, 1);\n";
  ptr += "                            }\n";
  ptr += "                            timerEntry.remove();\n";
  ptr += "                        \n";
  ptr += "                        //If timer got changed change the displayed on too\n";
  ptr += "                        } else {\n";
  ptr += "                            let newInnerHTML = `\n";
  ptr += "                                Timer set for ${displayTime} to ${action}\n";
  ptr += "                                <button class='delete-timer' onclick='deleteTimer(${i}, this)'>X</button>\n";
  ptr += "                            `;\n";
  ptr += "\n";
  ptr += "                            if ( timerEntry.innerHTML != newInnerHTML ){\n";
  ptr += "                                timerEntry.innerHTML = newInnerHTML;\n";
  ptr += "                            }\n";
  ptr += "                        }\n";
  ptr += "\n";
  ptr += "                    //If not displayed display timer\n";
  ptr += "                    } else if ( time != 30 ){\n";
  ptr += "                        displayTimer(displayTime, action, i);\n";
  ptr += "                    }\n";
  ptr += "                }\n";
  ptr += "            })\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        //Check in a defined frequence the states\n";
  ptr += "        function checkStates(){\n";
  ptr += "            setTimeout(() => {\n";
  ptr += "                updateStates();\n";
  ptr += "                checkStates();\n";
  ptr += "            }, updateTimeOut );\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        updateStates();\n";
  ptr += "        checkStates();\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "        //Turn the light ON/OFF\n";
  ptr += "        function toggleLight() {\n";
  ptr += "            lightOn = !lightOn;\n";
  ptr += "\n";
  ptr += "            if ( lightOn ){ fetch( espIp + 'turnon' ) }\n";
  ptr += "            else { fetch( espIp + 'turnoff' ) }    \n";
  ptr += "            \n";
  ptr += "            updateLightStatus();\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Change the brightness\n";
  ptr += "        brightnessSlider.oninput = function() {\n";
  ptr += "            if (lightOn) {\n";
  ptr += "                console.log( 'set brightness to ', this.value );\n";
  ptr += "                toSetBrightValue = this.value;\n";
  ptr += "                changeBrightness = true;\n";
  ptr += "            }\n";
  ptr += "        };\n";
  ptr += "\n";
  ptr += "        //Add a new timer\n";
  ptr += "        addTimerButton.onclick = function() {\n";
  ptr += "            const time = timerInput.value;\n";
  ptr += "            const action = timerAction.value;\n";
  ptr += "            if (time) {\n";
  ptr += "                setTimer(time, action);\n";
  ptr += "            }\n";
  ptr += "            updateStates();\n";
  ptr += "        };\n";
  ptr += "\n";
  ptr += "        //Set the light to nightmode\n";
  ptr += "        nightmodeButton.onclick = function() {\n";
  ptr += "            fetch( espIp + 'nightmode' );\n";
  ptr += "            toSetBrightValue = 0;\n";
  ptr += "            changeBrightness = false;\n";
  ptr += "            currentBrightValue = 0;\n";
  ptr += "            brightnessSlider.value = 0;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Change the color of the light\n";
  ptr += "        colorChangeButton.onclick = function(){\n";
  ptr += "            fetch( espIp + 'temp' );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //If the light is displayed on although it is off reset the state\n";
  ptr += "        resetButton.onclick = function(){\n";
  ptr += "            fetch( espIp + 'resetOnOffState' );\n";
  ptr += "\n";
  ptr += "            setTimeout(() => {updateStates();}, 100 );\n";
  ptr += "            \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Display the sun if the light is on and the moon if the light is off\n";
  ptr += "        function updateLightStatus() {\n";
  ptr += "            let wrapper = document.getElementById( 'wrapper' );\n";
  ptr += "            \n";
  ptr += "            if ( wrapper.classList.contains( 'day' ) && !lightOn ){\n";
  ptr += "                wrapper.classList.remove( 'day' );\n";
  ptr += "            } else if ( lightOn ) {\n";
  ptr += "                wrapper.classList.add( 'day' );\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "            if (lightOn) {\n";
  ptr += "                document.body.style.backgroundColor = 'lightblue';\n";
  ptr += "                toggleLightButton.classList.add('sun');\n";
  ptr += "                toggleLightButton.classList.remove('moon');\n";
  ptr += "            } else {\n";
  ptr += "                document.body.style.backgroundColor = '#151843';\n";
  ptr += "                toggleLightButton.classList.add('moon');\n";
  ptr += "                toggleLightButton.classList.remove('sun');\n";
  ptr += "            }\n";
  ptr += "            brightnessSlider.disabled = !lightOn;\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Update the brightness step by step\n";
  ptr += "        function updateBrightness() {\n";
  ptr += "            if ( toSetBrightValue > currentBrightValue && changeBrightness == true ){\n";
  ptr += "                fetch( espIp + 'brighten' );\n";
  ptr += "                updateTimeOut = 500;\n";
  ptr += "            } else if ( toSetBrightValue < currentBrightValue && changeBrightness == true ){\n";
  ptr += "                fetch( espIp + 'dim' ); \n";
  ptr += "                updateTimeOut = 500;\n";
  ptr += "            } else {\n";
  ptr += "                updateTimeOut = 5000;\n";
  ptr += "                changeBrightness = false;\n";
  ptr += "            }\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Send a request to the server to set a new timer\n";
  ptr += "        function setTimer(time, action) {\n";
  ptr += "            const [hours, minutes] = time.split(':');\n";
  ptr += "\n";
  ptr += "            var url;\n";
  ptr += "            action = action.toUpperCase();\n";
  ptr += "            if ( action == 'ON' ){\n";
  ptr += "                url = 'timer/hour/' + hours + '/minute/' + minutes + '/state/true';\n";
  ptr += "            } else {\n";
  ptr += "                url = 'timer/hour/' + hours + '/minute/' + minutes + '/state/false';\n";
  ptr += "            }\n";
  ptr += "\n";
  ptr += "            fetch( espIp + url ).then( response => response.text() ).then( body => {\n";
  ptr += "                timerId = parseInt( body );\n";
  ptr += "                displayTimer( hours + ':' + minutes, action, timerId);\n";
  ptr += "            });\n";
  ptr += "            \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Display a timer to the user\n";
  ptr += "        function displayTimer(time, action, timerId) {\n";
  ptr += "            const timerElement = document.createElement('div');\n";
  ptr += "            timerElement.className = 'timer-item';\n";
  ptr += "            timerElement.innerHTML = `\n";
  ptr += "                Timer set for ${time} to ${action.toUpperCase()}\n";
  ptr += "                <button class='delete-timer' onclick='deleteTimer(${timerId}, this)'>X</button>\n";
  ptr += "            `;\n";
  ptr += "            timerElement.id = 'timerId-' + timerId.toString();\n";
  ptr += "            timerElement.dataset.timerId = timerId;\n";
  ptr += "            timerList.appendChild(timerElement);\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        //Delete a timer if it is not usefull\n";
  ptr += "        function deleteTimer(timerId, element) {\n";
  ptr += "            const index = timers.findIndex(t => t.timerId === timerId);\n";
  ptr += "            if (index !== -1) {\n";
  ptr += "                timers.splice(index, 1);\n";
  ptr += "            }\n";
  ptr += "            element.parentElement.remove();\n";
  ptr += "\n";
  ptr += "            fetch( espIp + 'deleteTimer/timerId/' + timerId );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        updateLightStatus(); // Initialize status on page load\n";
  ptr += "\n";
  ptr += "        //Standby Modus\n";
  ptr += "        var timeoutTimer = 0;\n";
  ptr += "\n";
  ptr += "        //Countdown which is updated every second. if no action on the webpage happens in a defined interval the standby mode is activated\n";
  ptr += "        function sec(){\n";
  ptr += "            setTimeout( () => {\n";
  ptr += "                if ( !lightOn ){\n";
  ptr += "                    timeoutTimer = timeoutTimer + 1;\n";
  ptr += "                }\n";
  ptr += "            \n";
  ptr += "                if ( timeoutTimer > MAXTIMEOUT && !lightOn ){\n";
  ptr += "                    const standby = document.getElementById( 'standby' );\n";
  ptr += "                    standby.style.zIndex = '999';\n";
  ptr += "                    standby.style.backgroundColor = 'black';\n";
  ptr += "\n";
  ptr += "                    toggleLightButton.disabled = true; \n";
  ptr += "                    brightnessSlider.disabled = true; \n";
  ptr += "                    timerInput.disabled = true; \n";
  ptr += "                    timerAction.disabled = true; \n";
  ptr += "                    addTimerButton.disabled = true; \n";
  ptr += "                    timerList.disabled = true;\n";
  ptr += "                    nightmodeButton.disabled = true;\n";
  ptr += "                    colorChangeButton.disabled = true; \n";
  ptr += "                    resetButton.disabled = true; \n";
  ptr += "                }\n";
  ptr += "\n";
  ptr += "                sec();\n";
  ptr += "            }, 1000 );\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        function removeStandBy(){\n";
  ptr += "            const standby = document.getElementById( 'standby' );\n";
  ptr += "            standby.style.zIndex = '-999';\n";
  ptr += "            standby.style.backgroundColor = '';\n";
  ptr += "\n";
  ptr += "            toggleLightButton.disabled = false; \n";
  ptr += "            brightnessSlider.disabled = false; \n";
  ptr += "            timerInput.disabled = false; \n";
  ptr += "            timerAction.disabled = false; \n";
  ptr += "            addTimerButton.disabled = false; \n";
  ptr += "            timerList.disabled = false;\n";
  ptr += "            nightmodeButton.disabled = false;\n";
  ptr += "            colorChangeButton.disabled = false; \n";
  ptr += "            resetButton.disabled = false; \n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        \n";
  ptr += "        function checkTimeout(){\n";
  ptr += "            if ( timeoutTimer > MAXTIMEOUT ){\n";
  ptr += "                removeStandBy();\n";
  ptr += "                toggleLight();\n";
  ptr += "            }\n";
  ptr += "            timeoutTimer = 0;\n";
  ptr += "        }\n";
  ptr += "        \n";
  ptr += "        window.onclick = () => {\n";
  ptr += "            checkTimeout();\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        window.ontouchstart = () => {\n";
  ptr += "            checkTimeout();\n";
  ptr += "        }\n";
  ptr += "\n";
  ptr += "        sec();\n";
  ptr += "\n";
  ptr += "    </script>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";

  return ptr;
}