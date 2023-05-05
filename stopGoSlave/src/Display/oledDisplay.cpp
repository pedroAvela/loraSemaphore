#include "oledDisplay.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

//qontec logo 64x62

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

const unsigned char* imgArray[2] = {
    imgiconqontec_1_,
    radio_walkietalkie
};

void OledDisplay :: startDisplay(){
    reset();

    Wire.begin(OLED_SDA, OLED_SCL);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
        for(;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    //printMessage("Display okay!", 1);
    draw();

}

void OledDisplay :: printMessage(String request, int fontSize){
    display.clearDisplay();
    vTaskDelay(pdMS_TO_TICKS(250));
    display.setTextSize(fontSize);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.cp437(true);
    //vTaskDelay(pdMS_TO_TICKS(500));
    for (int i = 0; i < request.length(); i++){
        display.write(request[i]);
    }
    //display.print(request);
    vTaskDelay(pdMS_TO_TICKS(1000));
    display.display();
}

void OledDisplay :: reset(){
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
}

void OledDisplay :: draw(int drawing){
    
    display.clearDisplay();
    display.drawBitmap(
        (SCREEN_WIDTH - 64)/2,
        (SCREEN_HEIGHT - 62)/2,
        imgArray[drawing],
        64,
        62,
        1 
    );
    display.display();
    vTaskDelay(pdMS_TO_TICKS(1000));
    display.clearDisplay();
}
