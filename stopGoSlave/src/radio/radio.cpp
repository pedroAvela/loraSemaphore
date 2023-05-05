#include "radio.h"
#include "Arduino.h"
#include <LoRa.h>
#include <SPI.h>

#define SCK 18  // GPIO5  -- SX127x's SCK
#define MISO 19 // GPIO19 -- SX127x's MISO
#define MOSI 23 // GPIO27 -- SX127x's MOSI
#define SS 5    // GPIO18 -- SX127x's CS
#define RST 14  // GPIO14 -- SX127x's RESET
#define DI0 13  // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND 915E6 // you can set band here directly,e.g. 868E6,915E6
#define PABOOST true

#define RADIOPORTON 27

#define destination 0x01
#define localAddress 0x01
#define msgCount 0

radio::radio()
{
    //pinMode(RADIOPORTON, OUTPUT);
}

void radio ::start()
{
    //digitalWrite(RADIOPORTON, HIGH);

    LoRa.setPins(SS, RST, DI0);
    LoRa.setSpreadingFactor(13);
    LoRa.setTxPower(18);
    LoRa.setSignalBandwidth(10.4E3);
    if (!LoRa.begin(BAND))
    {
        Serial.println("Starting LoRa failed!");
    }
    Serial.println("LoRa ok!!");
}
void radio ::send(String msgLoRa)
{
    LoRa.beginPacket();
    LoRa.write(destination);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(msgLoRa.length());        // add payload length
    //LoRa.print(outgoing);
    LoRa.print(msgLoRa);
    LoRa.endPacket();
    LoRa.flush();
    LoRa.receive();
}

String radio ::listen()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize == 0)
        return "";

    //char msg[packetSize] = "";
    String msgReceived = "";

    int recipient = LoRa.read();          // recipient address
    byte sender = LoRa.read();            // sender address
    byte incomingMsgId = LoRa.read();     // incoming msg ID
    byte incomingLength = LoRa.read();    // incoming msg length

    if(sender != 0x01){
        //Serial.println("message not for me");
        return "";
    }

    //int i = 0;
    if (packetSize)
    {
        while (LoRa.available())
        {
            msgReceived += (char)LoRa.read();
        }
    }
    return msgReceived;
}

void radio ::resetModule()
{
    digitalWrite(RADIOPORTON, LOW);
    vTaskDelay(pdMS_TO_TICKS(2000));
    digitalWrite(RADIOPORTON, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
}