#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Preferences.h>
#include "Display/oledDisplay.h"

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define PABOOST true

#define ledRed 12
#define ledGreen 13
#define buttonMan 0

OledDisplay oled;
Preferences preferences;

void send(String sem);
void listen();
void startLoRa();
void onReceive(int packetSize);
void sendDone();
void getCommand();
void sync();
void cycle();
void redLight();
void greenLight();
void blinkRed();
void manLoop();
void load();
void save(bool flag, int red = 10, int tran = 10, int green = 10);

String message = "";
bool isButtonPressed = false;
bool firstStart = true;
bool goToMenu = true;
bool manMode = false;
bool synch = false;
bool greenCycle = false, redCycle = false, tCycle = false, cgGreen = false;
bool rOk = false, gOk = false, tOk = false;

unsigned long timeToSend = 0;
unsigned long sendInterval = 1000;
unsigned long redTimeInterval = 10*1000;
unsigned long transitionTimeInterval = 10*1000;
unsigned long greenTimeInterval = 10*1000;
unsigned long redTime = 0, transitionTime = 0, greenTime = 0;
unsigned long blinkInterval = 1000;
unsigned long blinkTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("LoRa Main");
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buttonMan, INPUT);
  load();
  startLoRa();
  oled.startDisplay();
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(sendDone);
  LoRa.receive();
}

void loop() {
  getCommand();

  if(!digitalRead(buttonMan)){
    manMode = true;
  }

  if(!manMode){
    if(!synch){
      sync();
      blinkRed();
    }else{
      cycle();
    }
  }else{
    manLoop();
  }

  vTaskDelay(pdMS_TO_TICKS(10));
}

void onReceive(int packetSize){
  if(packetSize == 0 )return;
  String msg = "";

  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();
  

  if(sender != 0x01){
    Serial.println("message not for me");
    return;
  }

  while (LoRa.available()){
    msg += (char)LoRa.read();
  }

  if(msg != ""){
    int rssi = LoRa.rssi();
    Serial.print("Rssi: ");
    Serial.println(rssi);
    message = msg;
  }
}


void sendDone(){
  LoRa.receive();
}

void startLoRa()
{
  LoRa.setPins(SS,RST,DI0);
  LoRa.setSpreadingFactor(13);
  LoRa.setTxPower(18);
  //LoRa.setSignalBandwidth(10.4E3);
  LoRa.setSignalBandwidth(500E3);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
  }
  Serial.println("radio ok");
}

void send(String sem)
{
  String messageToSend = sem;
  LoRa.beginPacket();
  LoRa.write(0x01);
  LoRa.write(0x00);
  LoRa.write(0);
  LoRa.write(messageToSend.length());
  LoRa.print(messageToSend);
  LoRa.endPacket(true);
}

void sync(){
  oled.printMessage("Sync...", 1);
  if(millis() - timeToSend >= sendInterval){
    Serial.println("sending");
    timeToSend = millis();
    send("Hello");
  }
}

void cycle(){
  if(greenCycle){
    if(rOk){
      if(millis() - greenTime >= greenTimeInterval){
        oled.printMessage("Changing to red...", 1);
        tCycle = true;
        greenCycle = false;
        rOk = false;
      }
      greenLight();
    }else{
      if(millis() - timeToSend >= sendInterval){
        timeToSend = millis();
        send("red");
      }
    }
  }

  if(tCycle){
    if(tOk){
      if(millis() - transitionTime >= transitionTimeInterval){
        oled.printMessage("Transistion", 1);
        tCycle = false;
        if(cgGreen){
          greenCycle = true;
          cgGreen = false;
        }else{
          redCycle = true;
        }
        tOk = false;
      }
      redLight();
    }else{
      if(millis() - timeToSend >= sendInterval){
        timeToSend = millis();
        send("tran");
      }
      blinkRed();
    }
  }

  if(redCycle){
    if(gOk){
      if(millis() - redTime >= redTimeInterval){
        oled.printMessage("Changing to green...", 1);
        tCycle = true;
        cgGreen = true;
        redCycle = false;
        gOk = false;
      }
      redLight();
    }else{
      if(millis() - timeToSend >= sendInterval){
        timeToSend = millis();
        send("green");
      }
    }
  }

}

void getCommand(){
  if(message != ""){
    if(message == "Hi"){
      oled.printMessage("Sync...okay", 1);
      synch = true;
      greenCycle = true;
      message = "";
    }

    if(message == "rOk"){
      oled.printMessage("Green", 1);
      greenTime = millis();
      rOk = true;
    }

    if(message == "tOk"){
      oled.printMessage("Tran.", 1);
      transitionTime = millis();
      tOk = true;
    }

    if(message == "gOk"){
      oled.printMessage("Red", 1);
      redTime = millis();
      gOk = true;
    }
    message = "";
  }
}

void redLight(){
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
}

void greenLight(){
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
}

void blinkRed(){
  if(millis() - blinkTime >= blinkInterval){
    blinkTime = millis();
    digitalWrite(ledRed, !digitalRead(ledRed));
  }
  digitalWrite(ledGreen, LOW);
}

void manLoop(){
  Serial.println("Maintenance Mode");
  Serial.println("Escreva o tempo do ciclo vermelho:");
  while (Serial.available() == 0){}
  String redTimeS = Serial.readString();
  Serial.print(redTimeS);
  Serial.println("Escreva o tempo do ciclo verde: ");
  while (Serial.available() == 0){}
  String greenTimeS = Serial.readString();
  Serial.print(greenTimeS);
  Serial.println("Escreva o tempo do ciclo transição: ");
  while (Serial.available() == 0){}
  String tranTimeS = Serial.readString();
  Serial.print(tranTimeS);

  int redTimeI = redTimeS.toInt();
  int greenTimeI = greenTimeS.toInt();
  int tranTimeI = tranTimeS.toInt();

  save(0, redTimeI, tranTimeI, greenTimeI);

  redTimeInterval = redTimeI*1000;
  greenTimeInterval = greenTimeI*1000;
  transitionTimeInterval = tranTimeI*1000;
  Serial.println("Exiting maintenace mode...");
  manMode = false;
}

void load(){
  preferences.begin("interval");
  int redTimeI = preferences.getInt("red", 0);
  int tranTimeI = preferences.getInt("tran", 0);
  int greenTimeI = preferences.getInt("green", 0);

  if(redTime != 0 && tranTimeI != 0 && greenTimeI != 0){
    redTimeInterval = redTimeI*1000;
    greenTimeInterval = greenTimeI*1000;
    transitionTimeInterval = tranTimeI*1000;
    preferences.end();
    return;
  }else{
    redTimeInterval = 10*1000;
    greenTimeInterval = 10*1000;
    transitionTimeInterval = 10*1000;

    save(1);
    return;
  }
}

void save(bool flag, int red, int tran, int green){
  if(!flag){
    preferences.begin("interval");
  }

  preferences.putInt("red", red);
  preferences.putInt("tran", tran);
  preferences.putInt("green", green);

  preferences.end();
}