#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "http_server";

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

  // Set content type and send the response
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html_data, fsize);
  free(html_data);
  return ESP_OK;
}

httpd_handle_t start_webserver(void) {
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

  return server;
}

void stop_webserver(httpd_handle_t server) { httpd_stop(server); }
