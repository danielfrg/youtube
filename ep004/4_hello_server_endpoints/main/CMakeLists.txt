idf_component_register(
    SRCS "main.c" "wifi_connect.c" "http_server.c"
    PRIV_REQUIRES esp_driver_gpio esp_wifi nvs_flash esp_http_server
    INCLUDE_DIRS ""
)


