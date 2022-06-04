#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h> 
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "spiffs_files.h"


void spiffs_init(void) {
    size_t total = 0, used = 0;   
    esp_vfs_spiffs_conf_t spiffs_conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE("SPIFFS", "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI("SPIFFS", "Partition size: total: %d, used: %d", total, used);
    }
}

void spiffs_deinit(void) {
    esp_vfs_spiffs_unregister(NULL);
}


bool spiffs_delete_file(const char* path) {
    struct stat st;

    // initializes the spiffs virtual file system
    spiffs_init();

    if (stat(path, &st) == 0) {
        unlink(path);
        ESP_LOGI("SPIFFS", "file %s has been deleted", path);
        spiffs_deinit();
        return true;
    }

    ESP_LOGW("SPIFFS", "spiffs: file %s was not found", path);
    spiffs_deinit();
    return false;
}