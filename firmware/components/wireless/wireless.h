#ifndef WIRELESS_H
#define WIRELESS_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"


void wifi_post_ip_phase(); // to be defined by the user

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

void wifi_init_softap(const char* ssid, const char* psk, int conns);

void wifi_init_sta(const char* ssid, const char* psk);
void wifi_stop();
void wifi_start();
void wifi_restore();
void wifi_deinit();



#endif