/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "esp_http_client.h"

#include "web_client.h"


static const char *TAG = "HTTP_CLIENT";

char http_last_event = HTTP_EVENT_HEADER_SENT;

extern void http_data_event_callback(char* response, int length);
extern void http_error_event_callback();

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            http_last_event = HTTP_EVENT_ERROR;
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            http_last_event = HTTP_EVENT_ON_CONNECTED;
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            http_last_event = HTTP_EVENT_HEADER_SENT;
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            http_last_event = HTTP_EVENT_ON_HEADER;
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            http_last_event = HTTP_EVENT_ON_DATA;
            if (!esp_http_client_is_chunked_response(evt->client)) {
                //printf("%.*s", evt->data_len, (char*)evt->data);
                http_data_event_callback((char*)evt->data, evt->data_len);
            }else{
                ESP_LOGW(TAG, "CHUNKED RESPONSE!!");
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            http_last_event = HTTP_EVENT_ON_FINISH;
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            if (http_last_event != HTTP_EVENT_ON_FINISH)
                http_error_event_callback();
            http_last_event = HTTP_EVENT_DISCONNECTED;
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void http_post(const char* url, const char* payload)
{
    esp_http_client_config_t config = {
    .url = url,
    .event_handler = _http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, payload, strlen(payload));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);
}



void http_get(const char* url)
{
    esp_http_client_config_t config = {
    .url = url,
    .event_handler = _http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);
}
