#ifndef FONTS_H
#define FONTS_H

#include "esp_log.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

/* Max size of bitmap will based on a font24 (17x24) */
#define MAX_HEIGHT_FONT         24
#define MAX_WIDTH_FONT          17
#define OFFSET_BITMAP           54

struct sFONT {
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
};

extern struct sFONT Font24;
extern struct sFONT Font20;
extern struct sFONT Font16;
extern struct sFONT Font12;
extern struct sFONT Font8;

#endif