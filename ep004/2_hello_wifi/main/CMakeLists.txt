idf_component_register(
    SRCS "main.c" "wifi_connect.c"
    PRIV_REQUIRES esp_driver_gpio esp_wifi nvs_flash
    INCLUDE_DIRS ""
)


