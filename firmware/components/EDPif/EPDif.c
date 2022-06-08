#include "EPDif.h"

#include "driver/gpio.h"
#include "driver/spi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <string.h>


void epdIfDigitalWrite(int pin, uint32_t value) {
    gpio_set_level((gpio_num_t)pin, value);
}

uint32_t epdIfDigitalRead(int pin) {
    return gpio_get_level((gpio_num_t)pin);
}


int epdIfDelayMs(unsigned int delaytime) {
    int aux = 0;
    if (delaytime < 20){
        int target = delaytime*28000;
        for (size_t i = 0; i < target; i++)
            aux += i;
        return aux;
    }
    vTaskDelay(delaytime / portTICK_PERIOD_MS);
    return aux;
}

/* SPI transmit data, format: 8bit command (read value: 3, write value: 4) + 8bit address(value: 0x0) + 64byte data */
static void IRAM_ATTR epdIfSpiMasterTransmit(bool send, uint32_t* data, uint32_t len)
{
    spi_trans_t trans;
    uint16_t cmd;
    uint32_t addr = 0x0;

    if (len > 16) {
        ESP_LOGE("epdIfInit", "ESP8266 only support transmit 64bytes(16 * sizeof(uint32_t)) one time");
        return;
    }

    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0;            // clear all bit

    if (send) {
        cmd = SPI_MASTER_WRITE_DATA_TO_SLAVE_CMD;  
        trans.bits.mosi = len * 32;             // One time transmit only support 64bytes
        trans.mosi = data;
    } else {
        cmd = SPI_MASTER_READ_DATA_FROM_SLAVE_CMD;
        trans.bits.miso = len * 32;
        trans.miso = data;
    }

    trans.bits.cmd = 8 * 1;
    trans.bits.addr = 8 * 1;     // transmit data will use 8bit address
    trans.cmd = &cmd;
    trans.addr = &addr;
    
    spi_trans(HSPI_HOST, &trans); 
}

void epdIfSpiTransfer(uint32_t data, uint32_t bits) {
    spi_trans_t trans;
    memset(&trans, 0x0, sizeof(trans));
    uint16_t cmd = 0;
    uint32_t addr = 0;
    uint32_t MOSI = data;
    uint32_t MISO;
    trans.cmd = &cmd;
    trans.addr = &addr;
    trans.mosi = &MOSI;
    trans.miso = &MISO;
    trans.bits.cmd = 0;
    trans.bits.addr = 0;
    trans.bits.mosi = bits;
    trans.bits.miso = 0;
    spi_trans(HSPI_HOST, &trans);
}

void epdIfSpiTransferBatch(uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i < (size + 63)/64; i++) {
        // transmit data, ESP8266 only transmit 64bytes one time
        epdIfSpiMasterTransmit(SPI_SEND, (uint32_t*) data + (i * 16), 64);
    }
}

int epdIfInit(void) {
    ESP_LOGI("epdIfInit", "init gpio");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = OUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    ESP_LOGI("epdIfInit", "configured output gpio");
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = IN_PIN_SEL;
    gpio_config(&io_conf);
    ESP_LOGI("epdIfInit", "configured input gpio");
    ESP_LOGI("epdIfInit", "init gpio succesfully");

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
    spi_config.clk_div = SPI_10MHz_DIV;
    spi_config.event_cb = NULL;
    spi_config.intr_enable.val = HSPI_HOST;
    spi_config.event_cb = NULL;
    spi_config.mode = SPI_MASTER_MODE;
    spi_config.clk_div = SPI_2MHz_DIV;
    spi_config.interface.bit_tx_order = SPI_BIT_ORDER_LSB_FIRST;
    spi_config.interface.bit_rx_order = SPI_BIT_ORDER_LSB_FIRST;
    spi_config.interface.byte_tx_order = SPI_BYTE_ORDER_LSB_FIRST;
    spi_config.interface.byte_rx_order = SPI_BYTE_ORDER_LSB_FIRST;
    spi_init(HSPI_HOST, &spi_config);
    ESP_LOGI("epdIfInit", "init spi succesfully");

    /*pinMode(CS_PIN, OUTPUT);
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));*/
    return 0;
}


int epdIfDeinit(void) {
    ESP_LOGI("epdIfInit", "deinit gpio");
    spi_deinit(HSPI_HOST);
    return 0;
}