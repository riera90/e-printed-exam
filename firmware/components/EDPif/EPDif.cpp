#include "EPDif.hpp"

#include "driver/gpio.h"
#include "driver/spi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <cstring>

EpdIf::EpdIf() {
};

EpdIf::~EpdIf() {
};

void EpdIf::DigitalWrite(gpio_num_t pin, uint32_t value) {
    gpio_set_level(pin, value);
}

uint32_t EpdIf::DigitalRead(gpio_num_t pin) {
    return gpio_get_level(pin);
}

void EpdIf::DigitalWrite(int pin, uint32_t value) {
    gpio_set_level((gpio_num_t)pin, value);
}

uint32_t EpdIf::DigitalRead(int pin) {
    return gpio_get_level((gpio_num_t)pin);
}


void EpdIf::DelayMs(unsigned int delaytime) {
    vTaskDelay(delaytime / portTICK_PERIOD_MS);
}

void EpdIf::SpiTransfer(unsigned char data) {
    uint32_t data_uint32 = (uint32_t)data;
    int len = 8;
    DigitalWrite(CS_PIN, LOW);
    spi_trans_t trans;
    uint16_t cmd;
    uint32_t addr = 0x0;

    if (len > 16) {
        ESP_LOGE("EpdIf", "ESP8266 only support transmit 64bytes(16 * sizeof(uint32_t)) one time");
        return;
    }

    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0;            // clear all bit

    cmd = SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD;  
    trans.bits.mosi = len * 32;             // One time transmit only support 64bytes
    trans.mosi = &data_uint32;
    trans.bits.cmd = 8 * 1;
    trans.bits.addr = 8 * 1;     // transmit data will use 8bit address
    trans.cmd = &cmd;
    trans.addr = &addr;
    
    spi_trans(HSPI_HOST, &trans);
    DigitalWrite(CS_PIN, HIGH);
}

int EpdIf::IfInit(void) {
    ESP_LOGI("EPDif::IfInit", "init gpio");
    fflush(stdout);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = IN_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    ESP_LOGI("EPDif::IfInit", "configured output gpio");
    fflush(stdout);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = OUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    ESP_LOGI("EPDif::IfInit", "configured input gpio");
    ESP_LOGI("EPDif::IfInit", "init gpio succesfully");
    fflush(stdout);


    ESP_LOGI("EPDif::IfInit", "init spi");
    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;

    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Set SPI to master mode
    // ESP8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_20MHz_DIV;
    spi_config.event_cb = NULL;
    spi_init(CSPI_HOST, &spi_config);
    ESP_LOGI("EPDif::IfInit", "init spi succesfully");

    /*pinMode(CS_PIN, OUTPUT);
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));*/
    return 0;
}
