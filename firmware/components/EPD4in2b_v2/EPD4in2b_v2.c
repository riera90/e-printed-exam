#include "EPD4in2b_v2.h"
#include "esp_log.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"



int edp4in2bV2Init(void) {
    epdIfInit();
    /* EPD hardware init start */
    edp4in2bV2Reset();
    edp4in2bV2SendCommand(POWER_ON);
    edp4in2bV2WaitUntilIdle();
    edp4in2bV2SendCommand(PANEL_SETTING);
    edp4in2bV2SendData(0x0F);     // LUT from OTP
	
    /* EPD hardware init end */
    return 0;
}

int edp4in2bV2Deinit(void) {
    epdIfDeinit();
    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void edp4in2bV2SendCommand(uint32_t command) {
    epdIfDigitalWrite(DC_PIN, LOW);
    epdIfSpiTransfer(command, 8);
    epdIfDelayMs(0);
    epdIfDelayMs(0);
}

/**
 *  @brief: basic function for sending data
 */
void edp4in2bV2SendData(uint32_t data) {
    epdIfDigitalWrite(DC_PIN, HIGH);
    epdIfSpiTransfer(data, 8);
    epdIfDelayMs(0);
    epdIfDelayMs(0);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void edp4in2bV2WaitUntilIdle(void) {
    while(!gpio_get_level(BUSY_PIN))      //0: busy, 1: idle
        epdIfDelayMs(100);
}

/**
 *  @brief: module reset. 
 *          often used to awaken the module in deep sleep, 
 *          see Epd::Sleep();
 */
void edp4in2bV2Reset(void) {
    epdIfDigitalWrite(RST_PIN, 0xFFFFFFFF);
    epdIfDelayMs(200);
    epdIfDigitalWrite(RST_PIN, 0x00000000);
    epdIfDelayMs(2);
    epdIfDigitalWrite(RST_PIN, 0xFFFFFFFF);
    epdIfDelayMs(200);   
}

/**
 *  @brief: transmit partial data to the SRAM
 */
void edp4in2bV2SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {
    edp4in2bV2SendCommand(PARTIAL_IN);
    edp4in2bV2SendCommand(PARTIAL_WINDOW);
    edp4in2bV2SendData(x >> 8);
    edp4in2bV2SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) >> 8);
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) | 0x07);
    edp4in2bV2SendData(y >> 8);        
    edp4in2bV2SendData(y & 0xff);
    edp4in2bV2SendData((y + l - 1) >> 8);        
    edp4in2bV2SendData((y + l - 1) & 0xff);
    edp4in2bV2SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            edp4in2bV2SendData(buffer_black[i]);  
        }  
    }
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            edp4in2bV2SendData(buffer_red[i]);  
        }  
    }
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(PARTIAL_OUT);  
}

/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void edp4in2bV2SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l) {
    edp4in2bV2SendCommand(PARTIAL_IN);
    edp4in2bV2SendCommand(PARTIAL_WINDOW);
    edp4in2bV2SendData(x >> 8);
    edp4in2bV2SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) >> 8);
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) | 0x07);
    edp4in2bV2SendData(y >> 8);        
    edp4in2bV2SendData(y & 0xff);
    edp4in2bV2SendData((y + l - 1) >> 8);        
    edp4in2bV2SendData((y + l - 1) & 0xff);
    edp4in2bV2SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            edp4in2bV2SendData(buffer_black[i]);  
        }  
    }
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(PARTIAL_OUT);  
}

/**
 *  @brief: transmit partial data to the red part of SRAM
 */
void edp4in2bV2SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l) {
    edp4in2bV2SendCommand(PARTIAL_IN);
    edp4in2bV2SendCommand(PARTIAL_WINDOW);
    edp4in2bV2SendData(x >> 8);
    edp4in2bV2SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) >> 8);
    edp4in2bV2SendData(((x & 0xf8) + w  - 1) | 0x07);
    edp4in2bV2SendData(y >> 8);        
    edp4in2bV2SendData(y & 0xff);
    edp4in2bV2SendData((y + l - 1) >> 8);        
    edp4in2bV2SendData((y + l - 1) & 0xff);
    edp4in2bV2SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            edp4in2bV2SendData(buffer_red[i]);  
        }  
    }
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(PARTIAL_OUT);  
}

/**
 * @brief: refresh and displays the frame
 */
void edp4in2bV2DisplayFramePart(const unsigned char* frame_black, const unsigned char* frame_red) {
    if (frame_black != NULL) {
        edp4in2bV2SendCommand(DATA_START_TRANSMISSION_1);
        epdIfDelayMs(2);
        for (int i = 0; i < WIDTH / 8 * HEIGHT; i++) {
            edp4in2bV2SendData(frame_black[i]);
        }
        epdIfDelayMs(2);
    }
    if (frame_red != NULL) {
        edp4in2bV2SendCommand(DATA_START_TRANSMISSION_2);
        epdIfDelayMs(2);
        for (int i = 0; i < WIDTH / 8 * HEIGHT; i++) {
            edp4in2bV2SendData(frame_red[i]);
        }
        epdIfDelayMs(2);
    }
    edp4in2bV2SendCommand(DISPLAY_REFRESH);
    edp4in2bV2WaitUntilIdle();
}

/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void edp4in2bV2ClearFrame(void) {
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_1);           
    epdIfDelayMs(2);
    for(int i = 0; i < WIDTH / 8 * HEIGHT; i++) {
        edp4in2bV2SendData(0xFF);  
    }  
    epdIfDelayMs(2);
    edp4in2bV2SendCommand(DATA_START_TRANSMISSION_2);           
    epdIfDelayMs(2);
    for(int i = 0; i < WIDTH / 8 * HEIGHT; i++) {
        edp4in2bV2SendData(0xFF);  
    }  
    epdIfDelayMs(2);
}

/**
 * @brief: This displays the frame data from SRAM
 */
void edp4in2bV2DisplayFrame(void) {
    edp4in2bV2SendCommand(DISPLAY_REFRESH); 
    epdIfDelayMs(100);
    edp4in2bV2WaitUntilIdle();
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void edp4in2bV2Sleep() {
    edp4in2bV2SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    edp4in2bV2SendData(0xF7);     // border floating
    edp4in2bV2SendCommand(POWER_OFF);
    edp4in2bV2WaitUntilIdle();
    edp4in2bV2SendCommand(DEEP_SLEEP);
    edp4in2bV2SendData(0xA5);     // check code
}


/* END OF FILE */