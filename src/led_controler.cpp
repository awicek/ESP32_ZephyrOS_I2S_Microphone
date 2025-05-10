#include "led_controler.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(leds, LOG_LEVEL_DBG);


static led_mode_t s_mode = LED_WAITING_FOR_CONNECTION;
static bool s_new_mode = true;
static const struct gpio_dt_spec s_green_led = 
    GPIO_DT_SPEC_GET(DT_NODELABEL(green_led), gpios);
static const struct gpio_dt_spec s_red_led = 
    GPIO_DT_SPEC_GET(DT_NODELABEL(red_led), gpios);

void led_control(void*, void*, void*);

K_THREAD_DEFINE(leds, 1024, led_control, NULL, NULL, NULL, 10, 0, 0);


void led_control(void *, void *, void *)
{
    if (!device_is_ready(s_green_led.port))
    {
        LOG_ERR("led device not ready");
        return;
    }
    gpio_pin_configure_dt(&s_green_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&s_red_led, GPIO_OUTPUT_ACTIVE);

    while (1)
    {
        if (s_new_mode)
        {
            s_new_mode = false;
            switch (s_mode)
            {
            case LED_WAITING_FOR_CONNECTION:
                gpio_pin_set_dt(&s_green_led, 0);
                gpio_pin_set_dt(&s_red_led, 1);
                break;
            case LED_WAITING_FOR_SERVER:
                gpio_pin_set_dt(&s_green_led, 1);
                gpio_pin_set_dt(&s_red_led, 1);
                break;
            case LED_WAITING_FOR_RESPONSE:
                gpio_pin_set_dt(&s_green_led, 1);
                gpio_pin_set_dt(&s_red_led, 0);
                break;
            case LED_READY:
                gpio_pin_set_dt(&s_green_led, 1);   
                gpio_pin_set_dt(&s_red_led, 0);
                break;
            case LED_RECORDING:
                gpio_pin_set_dt(&s_green_led, 1);   
                gpio_pin_set_dt(&s_red_led, 0);
                break;
            default:
                break;
            }
        }
        else
        {
            switch (s_mode)
            {
            case LED_WAITING_FOR_CONNECTION:
                gpio_pin_toggle_dt(&s_red_led);
                break;
            case LED_WAITING_FOR_SERVER:
            case LED_WAITING_FOR_RESPONSE:
                gpio_pin_toggle_dt(&s_red_led);
                gpio_pin_toggle_dt(&s_green_led);
                break;
            case LED_READY:
                break;
            case LED_RECORDING:
                gpio_pin_toggle_dt(&s_green_led);
                break;
            default:
                break;
            }
        }
        k_msleep(500);
    }

}


void led_set_mode(led_mode_t mode)
{
    s_mode = mode;
    s_new_mode = true;
}