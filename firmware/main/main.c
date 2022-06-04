#include "EPD4in2b_v2.h"
#include "painter.h"
#include "secrets.h"
#include "wireless.h"
#include "web_client.h"
#include "spiffs_files.h"

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
#include "esp_sleep.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define GPIO_OUTPUT_IO_0    14
#define GPIO_OUTPUT_IO_1    16
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     13
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

#define ENABLE_SCREEN

bool connected_wifi = false;
bool connected_server = false;
bool autenticated = false;
char token[256];
unsigned int ttl = 0;

struct State {
    char token[256];
    char vendor[7];
    char uuid[7];
    bool has_document;
    unsigned int current_page;
    unsigned int document_pages;
}state;

struct Painting image;

void wifi_post_ip_phase() {
    connected_wifi = true;
}

void http_data_event_callback(char* data, int length){
    ESP_LOGI("HTTP CB", "%.*s", length, data);
    connected_server = true;
    char* token_key_ptr = strstr(data, "token");
    if (token_key_ptr != NULL) {
        char* token_ptr = token_key_ptr+=8;
        if (token_ptr[0] == '"'){
            token_ptr++;
            char* end_token_ptr = strchr(token_ptr, '"');
            if (end_token_ptr != NULL){
                int token_length = end_token_ptr - token_ptr;
                if (token_length < 256){
                    autenticated = true;
                    memcpy(state.token, token_ptr, token_length);
                    ESP_LOGI("HTTP CB", "token: %s", state.token);
                    return;
                }else{
                    ESP_LOGE("HTTP CB", "too long of a token");
                    esp_restart();
                }
            }
        }
        ESP_LOGI("HTTP CB", "token not in response");
    }
}
void http_error_event_callback(){
    connected_server = false;
}

void print_config() {
    ESP_LOGI("CONFIG", "token:          %s", state.token);
    ESP_LOGI("CONFIG", "vendor:         %s", state.vendor);
    ESP_LOGI("CONFIG", "uuid:           %s", state.uuid);
    ESP_LOGI("CONFIG", "has_document:   %x", state.has_document);
    ESP_LOGI("CONFIG", "current_page:   %u", state.current_page);
    ESP_LOGI("CONFIG", "document_pages: %u", state.document_pages);
}

void load_state_from_spiffs() {
    spiffs_init();
    FILE* conf = fopen("/spiffs/config.bin", "rb");
    if (conf == NULL) {
        // the configuration file was not found, load the default values
        ESP_LOGI("MAIN", "Failed to open configuration file, fallback to default values");
        bzero((void*)&state, sizeof(struct State));
        spiffs_deinit();
        return;
    }
    // load the configration from the spiffs file
    ESP_LOGI("MAIN", "Loading configuration from config file");
    bzero((void*)&state, sizeof(struct State));
    int bytes = fread((void*)&state, sizeof(struct State), 1, conf);
    print_config();
    fclose(conf);
    spiffs_deinit();
}

void dump_state_to_spiffs() {
    spiffs_init();
    FILE* conf = fopen("/spiffs/config.bin", "wb");
    if (conf == NULL) {
        ESP_LOGE("MAIN", "Failed to open configuration file in write mode");
        spiffs_deinit();
        return;
    }
    ESP_LOGI("MAIN", "saving configuration to file");
    print_config();
    int bytes = fwrite((void*)&state, sizeof(struct State), 1, conf);
    fclose(conf);
    spiffs_deinit();
}

void init_gpio() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_2)|(1ULL<<GPIO_NUM_3);
    io_conf.pull_down_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}

void config_sleep() {
    //gpio_install_isr_service();
    //gpio_isr_handler_add(GPIO_NUM_16, &gpio_isr_handler, NULL);
    gpio_wakeup_enable(GPIO_NUM_2, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(24*60*60*1000000); // 4294967295 ttl*1000000
}

void init_secuence() {
#ifdef ENABLE_SCREEN
    edp4in2bV2Init();
    edp4in2bV2ClearFrame();

    image.rot = ROTATE_0;
    image.inv = 1;

    image.data = (unsigned char*) malloc(912);
    if (image.data == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    image.h = 24;
    image.w = 304;
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "e-Printed Exam", &Font24, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 40, image.w, image.h);
    edp4in2bV2DisplayFrame();
    free(image.data);
#endif

    wifi_init_sta(SSID, PSK);
        
    while (!connected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    uint8_t* base_raw_mac = (unsigned char*) malloc(6*sizeof(uint8_t));
    if (base_raw_mac == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    };

    if (esp_efuse_mac_get_default(base_raw_mac) != ESP_OK) {
        ESP_LOGE("MAC", "ERROR COULD NOT GET BASE FUSE MAC");
        esp_restart();
    }


    uint8_t* interface_raw_mac = (unsigned char*) malloc(6*sizeof(uint8_t));
    if (interface_raw_mac == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    };
    if (esp_read_mac(interface_raw_mac, 0) != ESP_OK) {
        ESP_LOGE("MAC", "ERROR COULD NOT GET INTERFACE MAC");
        esp_restart();
    }
    

    for (size_t i = 0; i < 6; i++) {
        if (base_raw_mac[i] != interface_raw_mac[i]){
            ESP_LOGE("MAC", "ERROR MACS ADDRS DOES NOT MATCH");
            esp_restart();
        }
    }

    ///  48:3f:da:xx:xx:xx
    ///   vendor |  uuid
    /// checkval | device

    char mac_addr[13];
    for (size_t i = 0; i < 6; i++)
        sprintf(&mac_addr[i<<1],"%x",base_raw_mac[i]);
    mac_addr[12]= '\0';
    memcpy(state.vendor, mac_addr, 6);
    memcpy(state.uuid, &mac_addr[6], 7);
    state.vendor[6] = '\0';
    free(base_raw_mac);
    free(interface_raw_mac);

    ESP_LOGI("MAC", "vendor: %s", state.vendor);
    ESP_LOGI("MAC", "uuid: %s", state.uuid);


#ifdef ENABLE_SCREEN
    image.data = (unsigned char*) malloc(323);
    if (image.data == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    image.h = 16;
    image.w = 120;
    PainterClear(&image, UNCOLORED);
    
    char* text = (char*)malloc(32);
    if (text == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    strcpy(text,"UUID ");
    strcat(text, state.uuid);
    PainterDrawStringAt(&image, 0, 0, text, &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
    free(text);
    
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "red", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 70, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "pantalla", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 90, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "servidor", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 110, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "uuid", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 130, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "credenc.", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 150, image.w, image.h);

    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, connected_wifi?"ok":"FAILED", &Font16, COLORED);
    if (connected_wifi)
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 70, image.w, image.h);
    else
        edp4in2bV2SetPartialWindowRed(image.data, 130, 70, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 90, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "waiting", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 110, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, state.uuid[0]!='\0'?"ok":"FAILED", &Font16, COLORED);
    if (state.uuid[0]!='\0')
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 130, image.w, image.h);
    else
        edp4in2bV2SetPartialWindowRed(image.data, 130, 130, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "waiting", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 150, image.w, image.h);
    edp4in2bV2DisplayFrame();
#endif
    //while(true)vTaskDelay(5000 / portTICK_PERIOD_MS);

    char* payload = (char*)malloc(256);
    if (payload == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    bzero(payload, 256);
    strcpy(payload, "secret=");
    strcat(payload, SERVER_SECRET);
    strcat(payload, "&vendor=");
    strcat(payload, state.vendor);
    strcat(payload, "&uuid=");
    strcat(payload, state.uuid);
    http_post("http://192.168.1.105:8000/device/ack/", payload);
    if (!autenticated){
#ifdef ENABLE_SCREEN
        if (connected_server){
            PainterClear(&image, UNCOLORED);
            edp4in2bV2SetPartialWindowRed(image.data, 130, 110, image.w, image.h);
            PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
            edp4in2bV2SetPartialWindowBlack(image.data, 130, 110, image.w, image.h);
        }else{
            PainterClear(&image, UNCOLORED);
            edp4in2bV2SetPartialWindowBlack(image.data, 130, 110, image.w, image.h);
            PainterDrawStringAt(&image, 0, 0, "FAILED", &Font16, COLORED);
            edp4in2bV2SetPartialWindowRed(image.data, 130, 110, image.w, image.h);
        }
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 150, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "FAILED", &Font16, COLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 130, 150, image.w, image.h);
        edp4in2bV2DisplayFrame();
#endif
        while (!autenticated) {
            http_post("http://192.168.1.105:8000/device/ack/", payload);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
#ifdef ENABLE_SCREEN
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 130, 110, image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 110, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 130, 150, image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 150, image.w, image.h);
    edp4in2bV2DisplayFrame();
#endif
    free(image.data);
    free(payload);
}

void gpio_isr_handler(){
    ESP_LOGE("ISR", "ISR!");
}

void app_main() {
    connected_wifi = false;
    load_state_from_spiffs();
    image.rot = ROTATE_0;
    image.inv = 1;
    if (state.token[0] == '\0'){
        init_secuence();
        dump_state_to_spiffs();
    }else{
        wifi_init_sta(SSID, PSK);
        init_gpio();
        if (gpio_get_level(GPIO_NUM_3) == 0) {
            ESP_LOGW("MAIN", "DELETING CONFIG FILE AND RESTARTING SYSTEM");
            spiffs_delete_file("/spiffs/config.bin");
            esp_restart();
        }
        //config_sleep();
        edp4in2bV2Init();
        edp4in2bV2ClearFrame();
    }  
    image.data = (unsigned char*) malloc(323);
    if (image.data == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    image.h = 16;
    image.w = 120;
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "wait", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
    edp4in2bV2DisplayFrame();
    while (!connected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);    // configure the sleep mode
    http_get("http://192.168.1.105:8000/device/ack/");
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "OK!", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
    edp4in2bV2SendCommand(DISPLAY_REFRESH); // send display refresh and do not wait for end of bussy
    epdIfDelayMs(100);
    wifi_stop();
    free(image.data);
    esp_deep_sleep(30*1000000);
    //esp_light_sleep_start();
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
}
