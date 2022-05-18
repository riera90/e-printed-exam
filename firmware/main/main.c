#include "EPD4in2b_v2.h"
#include "painter.h"

/* Hello World Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define GPIO_OUTPUT_IO_0    14
#define GPIO_OUTPUT_IO_1    16
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     13
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

void app_main()
{
    edp4in2bV2Init();    

    unsigned char data_[1500];
    struct Painting image;
    image.data = data_;
    image.inv = 1;
    image.h = 28;
    image.w = 400;

    edp4in2bV2ClearFrame();
    //edp4in2bV2DisplayFrame();

    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "e-Paper Demo", &Font24, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 100, 40, image.w, image.h);

    PainterClear(&image, COLORED);
    PainterDrawStringAt(&image, 40, 2, "riera90 0x4455434b", &Font24, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, 64, image.w, image.h);

    image.h = 64;
    image.w = 64;

    PainterClear(&image, UNCOLORED);
    PainterDrawRectangle(&image, 0, 0, 40, 50, COLORED);
    PainterDrawLine(&image, 0, 0, 40, 50, COLORED);
    PainterDrawLine(&image, 40, 0, 0, 50, COLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 72, 120, image.w, image.h);

    PainterClear(&image, UNCOLORED);
    PainterDrawCircle(&image, 32, 32, 30, COLORED);
    PainterDrawLine(&image, 11, 11, 55, 54, COLORED);
    PainterDrawLine(&image, 11, 54, 54, 11, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 200, 120, image.w, image.h);

    PainterClear(&image, UNCOLORED);
    PainterDrawFilledRectangle(&image, 0, 0, 40, 50, COLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 72, 200, image.w, image.h);

    PainterClear(&image, UNCOLORED);
    PainterDrawFilledCircle(&image, 32, 32, 30, COLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 200, 200, image.w, image.h);

    edp4in2bV2DisplayFrame();
    while (true)
    {
        //epdIfDigitalWrite(CS_PIN,LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //epdIfDigitalWrite(CS_PIN,HIGH);
    }
    
    

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
            chip_info.cores);

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
