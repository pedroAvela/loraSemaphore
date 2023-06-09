#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
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
#define buttonSem2 36

OledDisplay oled;

void send(String sem);
void listen();
void startLoRa();
void onReceive(int packetSize);
void sendDone();
void getCommand();
void redLight();
void greenLight();
void blinkRed();

String message = "";
bool isButtonPressed = false;
bool firstStart = true;
bool blink = true;
unsigned long blinkTime = 0;
unsigned long blinkInterval = 1500;
int rssi;

void setup() {
  Serial.begin(9600);
  Serial.println("LoRa Receiver");
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buttonSem2, INPUT);
  startLoRa();
  oled.startDisplay();
  LoRa.onTxDone(sendDone);
  LoRa.onReceive(onReceive);
  LoRa.receive();
  oled.printMessage("Waiting main...", 1);
}

void loop() {
  getCommand();
  if(blink){
    blinkRed();
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
  

  if(sender != 0x00){
    Serial.println("message not for me");
    return;
  }

  while (LoRa.available()){
    msg += (char)LoRa.read();
  }

  if(msg != ""){
    rssi = LoRa.rssi();
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
  LoRa.write(0x00);
  LoRa.write(0x01);
  LoRa.write(0);
  LoRa.write(messageToSend.length());
  LoRa.print(messageToSend);
  LoRa.endPacket(true);
}

void getCommand(){
  if(message != ""){
    Serial.println(message);
    Serial.println(message == "Hello");
    if(message == "Hello"){
      oled.printMessage("Main found it", 1);
      send("Hi");
    }

    if(message == "red"){
      oled.printMessage(message + rssi, 1);
      redLight();
      blink = false;
      send("rOk");
    }

    if(message == "tran"){
      oled.printMessage(message + rssi, 1);
      redLight();
      send("tOk");
    }

    if(message == "green"){
      oled.printMessage(message + rssi, 1);
      greenLight();
      send("gOk");
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
