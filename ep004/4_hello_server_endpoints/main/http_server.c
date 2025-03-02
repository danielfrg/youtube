#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2

static const char *TAG = "http_server";

// Global variable for LED state
bool led_state = false;

// Handler: GET /
static esp_err_t root_get_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Serving /index.html from LittleFS");

  // Open the file from /littlefs
  FILE *f = fopen("/littlefs/index.html", "r");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open /littlefs/index.html");
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  // Allocate memory to store file contents
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *html_data = malloc(fsize + 1);
  if (html_data == NULL) {
    fclose(f);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  size_t read_len = fread(html_data, 1, fsize, f);
  fclose(f);
  html_data[read_len] = '\0';

  // replace a placeholder in the HTML with the current LED state
  char *placeholder = strstr(html_data, "{{LED_STATE}}");
  if (placeholder) {
    const char *state_str = led_state ? "ON" : "OFF";
    size_t prefix_len = placeholder - html_data;
    size_t state_len = strlen(state_str);
    size_t placeholder_len = strlen("{{LED_STATE}}");
    size_t new_size = fsize - placeholder_len + state_len;
    char *new_html = malloc(new_size + 1);
    if (new_html) {
      memcpy(new_html, html_data, prefix_len);
      memcpy(new_html + prefix_len, state_str, state_len);
      strcpy(new_html + prefix_len + state_len, placeholder + placeholder_len);
      free(html_data);
      html_data = new_html;
      fsize = new_size;
    }
  }

  // Set content type and send the response
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html_data, fsize);
  free(html_data);
  return ESP_OK;
}

// Handler: POST /api/on
static esp_err_t api_on_post_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Received POST /api/on");
  led_state = true;
  gpio_set_level(LED_PIN, 1);
  const char *resp_str = "{\"status\":\"on\"}";
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, resp_str, strlen(resp_str));
  return ESP_OK;
}

// Handler: POST /api/off
static esp_err_t api_off_post_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Received POST /api/off");
  led_state = false;
  gpio_set_level(LED_PIN, 0);
  const char *resp_str = "{\"status\":\"off\"}";
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, resp_str, strlen(resp_str));
  return ESP_OK;
}

httpd_handle_t start_webserver(void) {
  // Set LED pin as output
  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);
  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
  }

  // Handler GET /
  httpd_uri_t root_uri = {.uri = "/",
                          .method = HTTP_GET,
                          .handler = root_get_handler,
                          .user_ctx = NULL};
  httpd_register_uri_handler(server, &root_uri);

  // Handler POST /api/on
  httpd_uri_t api_on_uri = {.uri = "/api/on",
                            .method = HTTP_POST,
                            .handler = api_on_post_handler,
                            .user_ctx = NULL};
  httpd_register_uri_handler(server, &api_on_uri);

  // Handler: POST /api/off
  httpd_uri_t api_off_uri = {.uri = "/api/off",
                             .method = HTTP_POST,
                             .handler = api_off_post_handler,
                             .user_ctx = NULL};
  httpd_register_uri_handler(server, &api_off_uri);

  return server;
}

void stop_webserver(httpd_handle_t server) { httpd_stop(server); }
