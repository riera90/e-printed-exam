#ifndef EPDIF_H
#define EPDIF_H

#include "driver/gpio.h"

// SPI pin description

#define RST_PIN         8
#define DC_PIN          9
#define CS_PIN          10
#define BUSY_PIN        7

#define LOW false
#define HIGH true

#define IN_PIN_SEL   (1ULL<<BUSY_PIN)
//#define OUT_PIN_SEL  (1ULL<<RST_PIN || 1ULL<<DC_PIN || 1ULL<<CS_PIN)
#define OUT_PIN_SEL  (1ULL<<CS_PIN)

class EpdIf {
public:
    EpdIf(void);
    ~EpdIf(void);

    static int  IfInit(void);
    static void DigitalWrite(gpio_num_t pin, uint32_t value); 
    static void DigitalWrite(int pin, uint32_t value); 
    static uint32_t  DigitalRead(gpio_num_t pin);
    static uint32_t  DigitalRead(int pin);
    static void DelayMs(unsigned int delaytime);
    static void SpiTransfer(unsigned char data);
};

#endif