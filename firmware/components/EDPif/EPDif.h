#ifndef EPDIF_H
#define EPDIF_H

#include "driver/gpio.h"

// SPI pin description
#define RST_PIN         GPIO_NUM_5
#define DC_PIN          GPIO_NUM_4
#define BUSY_PIN        GPIO_NUM_0

#define LOW false
#define HIGH true

#define IN_PIN_SEL   (1ULL<<BUSY_PIN)
#define OUT_PIN_SEL  ((1ULL<<RST_PIN) | (1ULL<<DC_PIN))

typedef enum {
    SPI_SEND = 0,
    SPI_RECV
} spi_master_mode_t;

int epdIfInit(void);
int epdIfDeinit(void);
void epdIfDigitalWrite(int pin, uint32_t value); 
uint32_t epdIfDigitalRead(int pin);
int epdIfDelayMs(unsigned int delaytime);
void epdIfSpiTransfer(uint32_t data, uint32_t bits);

#endif

