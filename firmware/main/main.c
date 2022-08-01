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

struct State {
    int ttl;
    char token[256];
    char vendor[7];
    char uuid[7];
    bool has_document;
    unsigned int current_page;
    unsigned int document_pages;
    bool is_autenticated;
    bool is_conected_wifi;
    bool is_conected_server;
    bool prove_complete;
}state;

struct Painting image;

void wifi_post_ip_phase() {
    state.is_conected_wifi = true;
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


void print_char_line_init(){
    image.h = 16;
    image.w = 400;
    image.data = (unsigned char*) malloc((400*16)/8);
}

void print_char_line_deinit(){
    free(image.data);
} 

void print_char_line(char* line, int line_num) {
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
    /*edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
    ESP_LOGE("HTTP CB", "writing to screen!");
    ESP_LOGW("HTTP CB", "current state x=%i, y=%i", target_x, target_y);*/
    free(image.data);
}



void http_data_event_callback(char* data, int length) {
    if (is_image_response){
        char* end_token_ptr = strchr(data, '"');
        if (end_token_ptr)
            length -= 2;
        /*ESP_LOGI("HTTP CB", "start: %.*s", 5, data-1);
        ESP_LOGI("HTTP CB", "end: %.*s", 5, data+length-4);
        ESP_LOGW("HTTP CB", "chunked!");
        ESP_LOGW("HTTP CB", "current entry state x=%i, y=%i", target_x, target_y);*/
        for (size_t input_index = 0; input_index < length && part < parts; input_index++){
            if (data[input_index] == '1'){
                PainterDrawPixel(&image, target_x, target_y, UNCOLORED);
            }else{
                PainterDrawPixel(&image, target_x, target_y, COLORED);
            }
            target_x++;
            if (target_x == 400){
                target_x = 0;
                target_y ++;
                //ESP_LOGW("HTTP CB", "increasing line counter x=%i, y=%i", target_x, target_y);
            }
            if (target_y == y_step){
                edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
                PainterClear(&image, UNCOLORED);
                edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
                /*ESP_LOGE("HTTP CB", "writing to screen!");
                ESP_LOGW("HTTP CB", "current state x=%i, y=%i to x=%i, y=%i", target_x, target_y, 0, image.h*part);*/
                target_y = 0;
                part++;
            }
        }
        //ESP_LOGW("HTTP CB", "current leaving state x=%i, y=%i", target_x, target_y);
        return;
    }

    ESP_LOGI("HTTP CB", "%.*s", length, data);
    state.is_conected_server = true;

    // check if it is ack/proving response
    char* ttl_ptr = strstr(data, "ttl");
    char* token_ptr = strstr(data, "token");
    char* document_pages_ptr = strstr(data, "document_pages");
    if (ttl_ptr && token_ptr && document_pages_ptr) {
        ttl_ptr = strchr(strchr(ttl_ptr+1, '"')+1, '"')+1;
        token_ptr = strchr(strchr(token_ptr+1, '"')+1, '"')+1;
        document_pages_ptr = strchr(strchr(document_pages_ptr+1, '"')+1, '"')+1;


        int token_length = strchr(token_ptr, '"') - token_ptr;
        if (token_length > 0){
            memcpy(state.token, token_ptr, token_length);
            state.is_autenticated = true;
        }
        char aux[32];
        int ttl_length = strchr(ttl_ptr, '"') - ttl_ptr;
        if (ttl_length > 0){
            bzero(aux, 32);
            memcpy(aux, ttl_ptr, ttl_length);
            state.ttl = atoi(aux);
        }

        int document_pages_length = strchr(document_pages_ptr, '"') - document_pages_ptr;
        if (ttl_length > 0){
            bzero(aux, 32);
            memcpy(aux, document_pages_ptr, document_pages_length);
            state.document_pages = atoi(aux);
            if (state.document_pages > 0)
                state.has_document = true;
            else
                state.has_document = false;
        }

        state.prove_complete = true;
    }


    // check if it is display data response
    char* type_ptr = strstr(data, "type");
    if (type_ptr != NULL) {
        type_ptr+=8;
        // check if the data is an image
        if (type_ptr[0] == 'I'){
            // the data is image
            y_step = 300/parts;
            ESP_LOGI("HTTP CB", "Image");
            is_image_response = true;
            edp4in2bV2ClearFrame();
            print_part_of_image_init(parts);

            part = 0;
            target_x = 0;
            target_y = 0;
            input_image_ptr = strstr(data, "\"data\"");
            input_image_ptr += 9;
            int data_length = length - (input_image_ptr-data);

            /*ESP_LOGI("HTTP CB", "start: %.*s", 5, input_image_ptr-1);
            ESP_LOGI("HTTP CB", "end: %.*s", 5, input_image_ptr+data_length-4);
            ESP_LOGW("HTTP CB", "current entry state x=%i, y=%i", target_x, target_y);*/
            for (size_t input_index = 0; input_index < data_length && part < parts; input_index++){
                if (input_image_ptr[input_index] == '1'){
                    PainterDrawPixel(&image, target_x, target_y, UNCOLORED);
                }else{
                    PainterDrawPixel(&image, target_x, target_y, COLORED);
                }
                target_x++;
                if (target_x == 400){
                    target_x = 0;
                    target_y ++;
                    //ESP_LOGW("HTTP CB", "increasing line counter x=%i, y=%i", target_x, target_y);
                }
                if (target_y == y_step){
                    edp4in2bV2SetPartialWindowBlack(image.data, 0, image.h*part, image.w, image.h);
                    PainterClear(&image, UNCOLORED);
                    edp4in2bV2SetPartialWindowRed(image.data, 0, image.h*part, image.w, image.h);
                    /*ESP_LOGE("HTTP CB", "writing to screen!");
                    ESP_LOGW("HTTP CB", "current state x=%i, y=%i to x=%i, y=%i", target_x, target_y, 0, image.h*part);*/
                    target_y = 0;
                    part++;
                }
            }
            //ESP_LOGW("HTTP CB", "current leaving state x=%i, y=%i", target_x, target_y);
        }else{
            // the data is text
            char* title_ptr = strstr(data, "\"title\"");
            if (title_ptr){
                int line_num = 0;
                ESP_LOGI("HTTP CB", "text");
                is_text_response = true;
                edp4in2bV2ClearFrame();
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
                print_char_line_init();
                while(line_end_ptr){
                    bzero(line,64);
                    memcpy(line, line_ptr, line_end_ptr-line_ptr);
                    ESP_LOGI("HTTP CB", "line: %s", line);
                    print_char_line(line, line_num++);
                    line_ptr = line_end_ptr+4;
                    line_end_ptr = strstr(line_ptr-1, "\\r\\n");
                }
            }
        }
        return;
    }

    // check if it is display data error response, normally indicating end of document
    char* error_ptr = strstr(data, "page_error");
    if (error_ptr){
        ESP_LOGI("HTTP CB", "end of document:");
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
        state.current_page = 1;
        sprintf(page, "%u", state.current_page);
        strcat(payload, page);
        char* entrypoint = (char*)malloc(256);
        if (entrypoint == NULL){
            ESP_LOGE("httpcb", "COULD NOT ALLOCATE MEMORY");
            esp_restart();
        }
        bzero(entrypoint, 256);
        strcat(entrypoint, SERVER_ADDR);
        strcat(entrypoint, "/device/get_page/");
        http_post(entrypoint, payload);
        free(payload);
        free(entrypoint);
        free(image.data);
        return;
    }
    ESP_LOGE("HTTP CB", "ERROR RESPONSE NOT HANLDED");
}


void http_error_event_callback(){
    state.is_conected_server = false;
}

void http_end_event_callback(){
    if (is_text_response){
        print_char_line_deinit();
    }
    if(is_image_response){
        print_part_of_image_deinit();
        ESP_LOGW("HTTP CB", "printing image");
    }
    is_text_response = false;
    is_image_response = false;
}

void print_config() {
    ESP_LOGI("CONFIG", "token:              %s", state.token);
    ESP_LOGI("CONFIG", "vendor:             %s", state.vendor);
    ESP_LOGI("CONFIG", "uuid:               %s", state.uuid);
    ESP_LOGI("CONFIG", "has_document:       %x", state.has_document);
    ESP_LOGI("CONFIG", "current_page:       %u", state.current_page);
    ESP_LOGI("CONFIG", "document_pages:     %u", state.document_pages);
    ESP_LOGI("CONFIG", "is_autenticated:    %x", state.is_autenticated);
    ESP_LOGI("CONFIG", "is_conected_wifi:   %x", state.is_conected_wifi);
    ESP_LOGI("CONFIG", "is_conected_server: %x", state.is_conected_server);
    ESP_LOGI("CONFIG", "prove_complete:     %x", state.prove_complete);
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
    //print_config();
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
    //print_config();
    int bytes = fwrite((void*)&state, sizeof(struct State), 1, conf);
    fclose(conf);
    spiffs_deinit();
}

#define NONE_PRESS 0
#define NEXT_PRESS 1
#define PREV_PRESS 2
#define BOTH_PRESS 3

unsigned char get_button_press() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_4)|(1ULL<<GPIO_NUM_12);
    io_conf.pull_down_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    int prv = !gpio_get_level(GPIO_NUM_12);
    int nxt = !gpio_get_level(GPIO_NUM_4);

    if (prv && nxt) ESP_LOGE("LOOP", "BOTH buttons are active");
    else if (prv) ESP_LOGE("LOOP", "prv is active");
    else if (nxt) ESP_LOGE("LOOP", "nxt is active");
    else ESP_LOGE("LOOP", "NO button is active");

    if (prv && nxt) return BOTH_PRESS;
    if (prv) return PREV_PRESS;
    if (nxt) return NEXT_PRESS;
    return NONE_PRESS;
}


void get_mac_addr(){
    while (!state.is_conected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);
    // get the mac address
    uint8_t* base_raw_mac = (unsigned char*) malloc(6*sizeof(uint8_t));
    if (base_raw_mac == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
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
}

void print_device_state() {
    image.data = (unsigned char*) malloc(912);
    if (image.data == NULL){
        ESP_LOGE("MAIN", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    image.h = 24;
    image.w = 304;
    // write to display mem but dont refresh
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, 40, image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, "e-Printed Exam", &Font24, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 40, image.w, image.h);
    free(image.data);

    get_mac_addr();

    // write the uuid and system state into the display
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
    PainterDrawStringAt(&image, 0, 0, state.is_conected_wifi?"ok":"FAILED", &Font16, COLORED);
    if (state.is_conected_wifi)
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 70, image.w, image.h);
    else
        edp4in2bV2SetPartialWindowRed(image.data, 130, 70, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 130, 90, image.w, image.h);
    PainterClear(&image, UNCOLORED);
    PainterDrawStringAt(&image, 0, 0, state.uuid[0]!='\0'?"ok":"FAILED", &Font16, COLORED);
    if (state.uuid[0]!='\0')
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 130, image.w, image.h);
    else
        edp4in2bV2SetPartialWindowRed(image.data, 130, 130, image.w, image.h);
    if (state.is_conected_server){
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
    if (!state.is_autenticated){
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 150, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "FAILED", &Font16, COLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 130, 150, image.w, image.h);
    }else {
        PainterClear(&image, UNCOLORED);
        edp4in2bV2SetPartialWindowRed(image.data, 130, 150, image.w, image.h);
        PainterDrawStringAt(&image, 0, 0, "ok", &Font16, COLORED);
        edp4in2bV2SetPartialWindowBlack(image.data, 130, 150, image.w, image.h);
    }
    free(image.data);
}

void get_and_display_current_page(){
    ESP_LOGI("get_and_display_current_page", "Displaying page");
    /*image.h = 16;
    image.w = 120;
    // draw wait in display
    image.data = (unsigned char*) malloc(323);
    if (image.data == NULL){
        ESP_LOGE("get_and_display_current_page", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    PainterClear(&image, UNCOLORED);
    edp4in2bV2SetPartialWindowRed(image.data, 0, 0, image.w, image.h);
    PainterDrawStringAt(&image, 0, 0, "wait", &Font16, COLORED);
    edp4in2bV2SetPartialWindowBlack(image.data, 0, 0, image.w, image.h);
    edp4in2bV2DisplayFrame();
    free(image.data);*/

    // build the package
    char* payload = (char*)malloc(256);
    if (payload == NULL){
        ESP_LOGE("get_and_display_current_page", "COULD NOT ALLOCATE MEMORY");
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
    // wait for connection to ap
    // the http cb will automatically render the text or image into the display
    char* entrypoint = (char*)malloc(256);
    if (entrypoint == NULL){
        ESP_LOGE("get_and_display_current_page", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    bzero(entrypoint, 256);
    strcat(entrypoint, SERVER_ADDR);
    while (!state.is_conected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);
    strcat(entrypoint, "/device/get_page/");
    http_post(entrypoint, payload);
    free(entrypoint);
    free(payload);
}

void factory_reset(){
    ESP_LOGE("factory_reset", "resetting state");
    bzero(&state, sizeof(struct State));
    state.current_page = 1;
}

void prove_server(){
    char* payload = (char*)malloc(256);
    if (payload == NULL){
        ESP_LOGE("prove_server", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    ESP_LOGI("prove_server", "proving server");

    bzero(payload, 256);
    strcpy(payload, "secret=");
    strcat(payload, SERVER_SECRET);
    strcat(payload, "&vendor=");
    strcat(payload, state.vendor);
    strcat(payload, "&uuid=");
    strcat(payload, state.uuid);

    char* entrypoint = (char*)malloc(256);
    if (entrypoint == NULL){
        ESP_LOGE("prove_server", "COULD NOT ALLOCATE MEMORY");
        esp_restart();
    }
    bzero(entrypoint, 256);
    strcat(entrypoint, SERVER_ADDR);
    strcat(entrypoint, "/device/ack/");
    while (!state.is_conected_wifi) vTaskDelay(1000 / portTICK_PERIOD_MS);
    http_post(entrypoint, payload);
    free(entrypoint);
    ESP_LOGI("prove_server", "waiting for response");
    while (!state.prove_complete) vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI("prove_server", "proving complete");
}

void app_main() {
    // get the button press first thing in the program
    unsigned char btn_state = get_button_press();

    // try to load current state from rom
    load_state_from_spiffs();

    state.is_conected_wifi = false;
    state.is_autenticated = false;
    state.is_conected_server = false;
    state.prove_complete = false;


    // initialize the wifi connection
    wifi_init_sta(SSID, PSK);
    // initialize the display
    edp4in2bV2Init();
    image.rot = ROTATE_0;
    image.inv = 1;
    // clear the display
    edp4in2bV2ClearFrame();

    // get the mac addr and prove the server
    if (state.uuid[0] == '\0')
        get_mac_addr();

    prove_server();

    print_config();

    // increase or decrease the document current page if a document is loaded
    switch (btn_state) {
        case NEXT_PRESS:{
            state.current_page++;
            if (state.current_page > state.document_pages)
                state.current_page = 1;
            break;
        }
        case PREV_PRESS:{
            state.current_page--;
            if (state.current_page == 0)
                state.current_page = state.document_pages == 0 ? 1 : state.document_pages;
            break;
        }
        case NONE_PRESS:{
            break;
        }
        case BOTH_PRESS:{
            factory_reset();
            break;
        }
    }

    print_config();


    dump_state_to_spiffs();


    if (state.is_autenticated && state.has_document)
        get_and_display_current_page();
    else
        print_device_state();

    wifi_stop();
    edp4in2bV2SendCommand(DISPLAY_REFRESH);
    edp4in2bV2DisplayFrame();
    edp4in2bV2Sleep();
    edp4in2bV2Deinit();
    esp_deep_sleep(30*1000000);
}
