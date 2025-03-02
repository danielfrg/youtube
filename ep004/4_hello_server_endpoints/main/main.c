#include "driver/gpio.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>

#include "http_server.h"
#include "wifi_connect.h"

static const char *TAG = "main";

// Mount LittleFS at "/littlefs" using the partition labeled "storage"
static esp_err_t mount_littlefs(void) {
  ESP_LOGI(TAG, "Mounting LittleFS");
  esp_vfs_littlefs_conf_t conf = {
      .base_path = "/littlefs",
      .partition_label = "storage",
      .format_if_mount_failed = true,
  };

  esp_err_t ret = esp_vfs_littlefs_register(&conf);
  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format LittleFS");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find LittleFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
    }
    return ret;
  }
  size_t total = 0, used = 0;
  ret = esp_littlefs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get LittleFS partition info (%s)",
             esp_err_to_name(ret));
    return ret;
  }
  ESP_LOGI(TAG, "LittleFS partition size: total: %d, used: %d", total, used);
  return ESP_OK;
}

void app_main(void) {
  // Initialize NVS (required by WiFi)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize WiFi
  wifi_init_sta();

  // Mount LittleFS (the image is generated at build time from the 'data'
  // directory)
  ret = mount_littlefs();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to mount LittleFS");
    return;
  }

  // Start the HTTP server (implementation in http_server.c)
  httpd_handle_t server = start_webserver();
  if (server == NULL) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
  }
}
