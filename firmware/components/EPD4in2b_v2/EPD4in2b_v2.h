#ifndef EPD4IN2_V2_H
#define EPD4IN2_V2_H

#include "EPDif.h"

// Display resolution
#define EPD_WIDTH  400
#define EPD_HEIGHT 300

// EPD4IN2 commands
#define PANEL_SETTING                   0x00
#define POWER_SETTING                   0x01
#define POWER_OFF                       0x02
#define POWER_OFF_SEQUENCE_SETTING      0x03
#define POWER_ON                        0x04
#define POWER_ON_MEASURE                0x05
#define BOOSTER_SOFT_START              0x06
#define DEEP_SLEEP                      0x07
#define DATA_START_TRANSMISSION_1       0x10
#define DATA_STOP                       0x11
#define DISPLAY_REFRESH                 0x12
#define DATA_START_TRANSMISSION_2       0x13
#define LUT_FOR_VCOM                    0x20 
#define LUT_WHITE_TO_WHITE              0x21
#define LUT_BLACK_TO_WHITE              0x22
#define LUT_WHITE_TO_BLACK              0x23
#define LUT_BLACK_TO_BLACK              0x24
#define PLL_CONTROL                     0x30
#define TEMPERATURE_SENSOR_COMMAND      0x40
#define TEMPERATURE_SENSOR_SELECTION    0x41
#define TEMPERATURE_SENSOR_WRITE        0x42
#define TEMPERATURE_SENSOR_READ         0x43
#define VCOM_AND_DATA_INTERVAL_SETTING  0x50
#define LOW_POWER_DETECTION             0x51
#define TCON_SETTING                    0x60
#define RESOLUTION_SETTING              0x61
#define GSST_SETTING                    0x65
#define GET_STATUS                      0x71
#define AUTO_MEASUREMENT_VCOM           0x80
#define READ_VCOM_VALUE                 0x81
#define VCM_DC_SETTING                  0x82
#define PARTIAL_WINDOW                  0x90
#define PARTIAL_IN                      0x91
#define PARTIAL_OUT                     0x92
#define PROGRAM_MODE                    0xA0
#define ACTIVE_PROGRAMMING              0xA1
#define READ_OTP                        0xA2
#define POWER_SAVING                    0xE3



#define WIDTH 400
#define HEIGHT 300

int  edp4in2bV2Init(void);
int  edp4in2bV2Deinit(void);
void edp4in2bV2SendCommand(uint32_t command);
void edp4in2bV2SendData(uint32_t data);
void edp4in2bV2WaitUntilIdle(void);
void edp4in2bV2Reset(void);
void edp4in2bV2SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l);
void edp4in2bV2SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l);
void edp4in2bV2SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l);
void edp4in2bV2DisplayFramePart(const unsigned char* frame_black, const unsigned char* frame_red);
void edp4in2bV2DisplayFrame(void);
void edp4in2bV2ClearFrame(void);
void edp4in2bV2Sleep(void);


#endif