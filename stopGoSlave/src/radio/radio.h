#ifndef RADIO_H
#define RADIO_H

#include "Arduino.h"

class radio
{
public:
    radio();
    void start();
    void send(String msg);
    String listen();
    void resetModule();
};

#endif
