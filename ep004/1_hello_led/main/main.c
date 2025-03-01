#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN GPIO_NUM_2
#define TOOGLE_DELAY_MS 1000

void app_main(void) {
  // Set LED pin as output
  gpio_reset_pin(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  bool led_state = false;

  while (1) {
    led_state = !led_state;

    gpio_set_level(LED_PIN, led_state);

    vTaskDelay(TOOGLE_DELAY_MS / portTICK_PERIOD_MS);
  }
}
