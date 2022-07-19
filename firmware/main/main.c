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
//#define DISPLAY_FRAME

bool connected_wifi = false;
bool connected_server = false;
bool autenticated = false;
char token[256];

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


bool is_text_response = false;
bool is_image_response = false;


int parts = 6; // NEEDS to be divisible by 300
int part;
int y_step;
int target_x;
int target_y;
int input_index;
char* input_image_ptr;


void print_line_init(){
    image.h = 16;
    image.w = 400;
    image.data = (unsigned char*) malloc((400*16)/8);
}

void print_line_deinit(){
    free(image.data);
} 

void print_line(char* line, int line_num) {
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, 42-15 + (line_num * 15), image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, line, &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 42-15 + (line_num * 15), image.w, image.h);
}

void print_part_of_image_init(int step){
    image.h = 300/step;
    image.w = 400;
    image.data = (unsigned char*) malloc((400*(300/step))/8);
}
void print_part_of_image_deinit(){
    edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
    ESP_LOGE("HTTP CB", "writing to screen!");
    ESP_LOGW("HTTP CB", "current state x=%i, y=%i", target_x, target_y);
    free(image.data);
}



void http_data_event_callback(char* data, int length) {
    if (is_image_response){
        ESP_LOGI("HTTP CB", "start: %.*s", 5, data-1);
        ESP_LOGI("HTTP CB", "end: %.*s", 5, data+length-4);
        ESP_LOGW("HTTP CB", "chunked!");
        ESP_LOGW("HTTP CB", "current state x=%i, y=%i", target_x, target_y);
        char* end_token_ptr = strchr(data, '"');
        if (end_token_ptr)
            length -= 2;
        for (size_t input_index = 0; input_index < length && part < parts; input_index++){
            if (data[input_index] == '1'){
                PainterDrawPixel(&image, target_x, target_y, UNCOLORED);
            }else{
                PainterDrawPixel(&image, target_x, target_y, COLORED);
            }
            target_x++;
            if (target_x >= 400){
                target_x = 0;
                target_y ++;
            }
            if (target_y >= y_step){
                edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
                PainterClear(&image, UNCOLORED);
                edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
                ESP_LOGE("HTTP CB", "writing to screen!");
                ESP_LOGW("HTTP CB", "current state x=%i, y=%i", target_x, target_y);
                target_y = 0;
                part++;
            }
        }
        return;
    }

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
                if (token_length < 256 && token_length < 32){
                    autenticated = true;
                    memcpy(state.token, token_ptr, token_length);
                    return;
                }else{
                    ESP_LOGE("HTTP CB", "too long of a token");
                    esp_restart();
                }
            }
        }
        //ESP_LOGI("HTTP CB", "token not in response");
    }
    char* type_ptr = strstr(data, "type");
    if (type_ptr != NULL) {
        type_ptr+=8;
        if (type_ptr[0] == 'I'){
            y_step = 300/parts;
            ESP_LOGI("HTTP CB", "Image");
            is_image_response = true;
            print_part_of_image_init(parts);

            part = 0;
            target_x = -44;
            target_y = 0;
            input_image_ptr = strstr(data, "\"data\"");
            input_image_ptr += 9;
            ESP_LOGI("HTTP CB", "start: %.*s", 5, input_image_ptr-1);
            int data_length = length - (input_image_ptr-data);
            ESP_LOGI("HTTP CB", "end: %.*s", 5, input_image_ptr+data_length-4);
            for (size_t input_index = input_image_ptr; input_index < data_length && part < parts; input_index++){
                if (data[input_index] == '1'){
                    PainterDrawPixel(&image, target_x, target_y, UNCOLORED);
                }else{
                    PainterDrawPixel(&image, target_x, target_y, COLORED);
                }
                target_x++;
                if (target_x >= 400){
                    target_x = 0;
                    target_y ++;
                }
                if (target_y >= y_step){
                    edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
                    PainterClear(&image, UNCOLORED);
                    edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
                    ESP_LOGE("HTTP CB", "writing to screen!");
                    ESP_LOGW("HTTP CB", "current state x=%i, y=%i", target_x, target_y);
                    target_y = 0;
                    part++;
                }
            }
        }else{
            char* title_ptr = strstr(data, "\"title\"");
            if (title_ptr){
                int line_num = 0;
                ESP_LOGI("HTTP CB", "text");
                is_text_response = true;
                
                title_ptr+=10;
                char title[124];
                bzero(title, 124);
                char* title_end_ptr = strchr(title_ptr, '"');
                memcpy(title, title_ptr, title_end_ptr-title_ptr);
                ESP_LOGI("HTTP CB", "title: %s", title);

                image.h = 24;
                image.w = 400;
                image.data = (unsigned char*) malloc((400*24)/8);
                PainterClear(&image, UNCOLORED);
                edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
                edp4in2bV2SetPartialWindowBlack(image.data, 0, 24, image.w, image.h);
                edp4in2bV2SetPartialWindowBlack(image.data, 0, 300-24, image.w, image.h);
                PainterClear(&image, COLORED);
                PainterDrawStringAt(&image, 0, 0, title, &Font24, UNCOLORED);
                edp4in2bV2SetPartialWindowRed(image.data, 0, 0, image.w, image.h);
                free(image.data);

                char* body_ptr = strstr(data, "\"body\"");
                body_ptr+=9;
                ESP_LOGI("HTTP CB", "body: %.*s", 10, body_ptr);
                char line[64];
                char* line_ptr = body_ptr;
                char* line_end_ptr = strstr(line_ptr+1, "\\r\\n");
                print_line_init();
                while(line_end_ptr){
                    bzero(line,64);
                    memcpy(line, line_ptr, line_end_ptr-line_ptr);
                    ESP_LOGI("HTTP CB", "line: %s", line);
                    print_line(line, line_num++);
                    line_ptr = line_end_ptr+4;
                    line_end_ptr = strstr(line_ptr-1, "\\r\\n");
                }
            }
        }
        return;
    }
    char* error_ptr = strstr(data, "page_error");
    if (error_ptr){
        ESP_LOGI("HTTP CB", "end of document:");
        state.current_page = 1;
        image.h = 16;
        image.w = 400;
        image.data = (unsigned char*) malloc(16*400);
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 0, 0, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "fin del documento", &Font16, COLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 0, 16, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "regresando al inicio", &Font16, COLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 0, 16, image.w, image.h);
        edp4in2bV2DisplayFrame();
        epdIfDelayMs(1000);
        char* payload = (char*)malloc(256);
        if (payload == NULL){
            ESP_LOGE("httpcb", "COULD NOT ALLOCATE MEMORY");
            esp_restart();
        }
        bzero(payload, 256);
        strcpy(payload, "secret=");
        strcat(payload, SERVER_SECRET);
        strcat(payload, "&uuid=");
        strcat(payload, state.uuid);
        strcat(payload, "&token=");
        strcat(payload, state.token);
        strcat(payload, "&page=");
        char page[5];
        bzero(page, 5);
        sprintf(page, "%u", state.current_page);
        strcat(payload, page);
        http_post("http://192.168.1.105:8000/device/get_page/", payload);
        free(payload);
        free(image.data);
        return;
    }
    //ESP_LOGI("HTTP CB", "page type not in response");
}


void http_error_event_callback(){
    connected_server = false;
}

void http_end_event_callback(){
    if (is_text_response){
        print_line_deinit();
        edp4in2bV2DisplayFrame();
    }
    if(is_image_response){
        print_part_of_image_deinit();
        edp4in2bV2DisplayFrame();
        ESP_LOGW("HTTP CB", "printing image");
    }
    is_text_response = false;
    is_image_response = false;
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
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_4)|(1ULL<<GPIO_NUM_12);
    io_conf.pull_down_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    //gpio_install_isr_service(0);
    //gpio_isr_handler_add(GPIO_NUM_2, &gpio_isr_handler_nxt, (void *) GPIO_NUM_2);
    //gpio_isr_handler_add(GPIO_NUM_3, &gpio_isr_handler_prv, (void *) GPIO_NUM_3);
}

void config_sleep() {
    gpio_wakeup_enable(GPIO_NUM_2, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    //esp_sleep_enable_timer_wakeup(24*60*60*1000000); // 4294967295 ttl*1000000
    esp_sleep_enable_timer_wakeup(3600*1000000); // 4294967295 ttl*1000000
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
    edp4in2bV2SetPartialWindowRed(image.data, 0, 40, image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, "e-Printed Exam", &Font24, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 40, image.w, image.h);
#endif
#ifdef DISPLAY_FRAME
    edp4in2bV2DisplayFrame();
#endif
#ifdef ENABLE_SCREEN
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
    #endif
    #ifdef DISPLAY_FRAME
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
#endif
#ifdef DISPLAY_FRAME
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
#endif
#ifdef DISPLAY_FRAME
    edp4in2bV2DisplayFrame();
#endif
    free(image.data);
    free(payload);
}


void app_main() {
    connected_wifi = false;
    //load_state_from_spiffs();
    image.rot = ROTATE_0;
    image.inv = 1;
    init_gpio();
    init_secuence();
    config_sleep();
    if (image.data == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    int nxt = 0, prv = 0;
    state.current_page = 1;
    while (true){
        image.data = (unsigned char*) malloc(323);
        image.h = 16;
        image.w = 120;
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 0, 0, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "wait", &Font16, COLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
        //edp4in2bV2DisplayFrame();
        free(image.data);
        while (!connected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);    // configure the sleep mode
        char* payload = (char*)malloc(256);
        if (payload == NULL){
            ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
            esp_restart();
        }
        if (nxt)
            state.current_page++;
        if (prv)
            state.current_page--;
        bzero(payload, 256);
        strcpy(payload, "secret=");
        strcat(payload, SERVER_SECRET);
        strcat(payload, "&uuid=");
        strcat(payload, state.uuid);
        strcat(payload, "&token=");
        strcat(payload, state.token);
        strcat(payload, "&page=");
        char page[5];
        bzero(page, 5);
        sprintf(page, "%u", state.current_page);
        strcat(payload, page);
        http_post("http://192.168.1.105:8000/device/get_page/", payload);
        free(payload);
        wifi_stop();
        edp4in2bV2Deinit();
        connected_wifi = false;
        //esp_deep_sleep(30*1000000);
        ESP_LOGE("LOOP", "sleep!");
        esp_light_sleep_start();
        ESP_LOGE("LOOP", "wake up!");
        init_gpio();
        prv = !gpio_get_level(GPIO_NUM_4);
        nxt = !gpio_get_level(GPIO_NUM_12);
        if (prv) ESP_LOGE("LOOP", "prv is active");
        if (nxt) ESP_LOGE("LOOP", "nxt is active");;
        wifi_start();
        ESP_LOGE("LOOP", "wifi started");;
        config_sleep();
        ESP_LOGE("LOOP", "config sleep started");;
        edp4in2bV2Init();
        ESP_LOGE("LOOP", "display started");
    }

    free(image.data);
}
