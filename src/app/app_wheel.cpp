/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */

// SDK includes
extern "C"
{
#include "mem.h"
#include "library_dio_task.h"
#include "esp8266_io.h"
}

#include "app_wheel.hpp"
#include "espbot_global.hpp"
#include "espbot_gpio.hpp"


os_timer_t wheel_timer;
os_timer_t wheel_relay;
int wheel_count;
#define WHEEL_TIMER 600000
#define WHEEL_RELAY 150000
#define WHEEL_COUNT_MAX (6 * 3) // 6 volte all'ora per 3 ore
#define WHEEL_COUNT_RESET (6 * 6) // 6 volte all'ora per 6 ore
#define WHEEL_DO ESPBOT_D4

void app_stop_wheel(void)
{
    esp_gpio.set(WHEEL_DO, ESPBOT_HIGH);
}

void app_start_wheel(void)
{
    wheel_count++;
    os_timer_arm(&wheel_timer, WHEEL_TIMER, 0);
    if(wheel_count > WHEEL_COUNT_RESET)
        wheel_count = 0;
    if (wheel_count > WHEEL_COUNT_MAX)
        return;
    esp_gpio.set(WHEEL_DO, ESPBOT_LOW);
    os_timer_arm(&wheel_relay, WHEEL_RELAY, 0);
}

void app_init_wheel(void)
{
    esp_gpio.config(WHEEL_DO, ESPBOT_GPIO_OUTPUT);
    esp_gpio.set(WHEEL_DO, ESPBOT_HIGH);
    wheel_count = 0;
    os_timer_disarm(&wheel_timer);
    os_timer_disarm(&wheel_relay);

    os_timer_setfn(&wheel_timer, (os_timer_func_t *)app_start_wheel, (void *)NULL);
    os_timer_setfn(&wheel_relay, (os_timer_func_t *)app_stop_wheel, (void *)NULL);

    os_timer_arm(&wheel_timer, WHEEL_TIMER, 0);
}