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

#include "app_doorbell.hpp"
#include "espbot_global.hpp"
#include "espbot_gpio.hpp"
#include "app_relay_sequences.hpp"

os_timer_t doorbell_timer;
os_timer_t doorbell_relay;
int doorbell_count;
bool doorbell_prev_timer_short;

//#define DOORBELL_TIMER_LONG 1020000
#define DOORBELL_TIMER_LONG 600000
#define DOORBELL_TIMER_SHORT 300
#define DOORBELL_RELAY 150
#define DOORBELL_COUNT_MAX (6 * 3)   // 6 volte all'ora per 3 ore
#define DOORBELL_COUNT_RESET (6 * 6) // 6 volte all'ora per 6 ore
#define DOORELL_DO ESPBOT_D3

void app_stop_doorbell(void)
{
    esp_gpio.set(DOORELL_DO, ESPBOT_HIGH);
}
 
void app_start_doorbell(void)
{
    // doorbell_count++;
    // if (doorbell_prev_timer_short)
    // {
    //     os_timer_arm(&doorbell_timer, DOORBELL_TIMER_LONG, 0);
    //     doorbell_prev_timer_short = false;
    // }
    // else
    // {
    //     unsigned long value = os_random();
    //     if (value > (__LONG_MAX__ / 2))
    //     {
    //         os_timer_arm(&doorbell_timer, DOORBELL_TIMER_SHORT, 0);
    //         doorbell_prev_timer_short = true;
    //     }
    //     else
    //         os_timer_arm(&doorbell_timer, DOORBELL_TIMER_LONG, 0);
    // }
    // // if(doorbell_count > DOORBELL_COUNT_RESET)
    // //     doorbell_count = 0;
    // // if (doorbell_count > DOORBELL_COUNT_MAX)
    // //     return;
    // esp_gpio.set(DOORELL_DO, ESPBOT_LOW);
    // os_timer_arm(&doorbell_relay, DOORBELL_RELAY, 0);
    static int sequence_idx = 0;
    exe_relay_sequences(sequence_idx);
    sequence_idx++;
    if (sequence_idx > 3)
        sequence_idx = 0;
    os_timer_arm(&doorbell_timer, DOORBELL_TIMER_LONG, 0);
}

void app_init_doorbell(void)
{
    esp_gpio.config(DOORELL_DO, ESPBOT_GPIO_OUTPUT);
    esp_gpio.set(DOORELL_DO, ESPBOT_HIGH);
    doorbell_count = 0;
    doorbell_prev_timer_short = false;
    os_timer_disarm(&doorbell_timer);
    os_timer_disarm(&doorbell_relay);

    os_timer_setfn(&doorbell_timer, (os_timer_func_t *)app_start_doorbell, (void *)NULL);
    os_timer_setfn(&doorbell_relay, (os_timer_func_t *)app_stop_doorbell, (void *)NULL);

    os_timer_arm(&doorbell_timer, DOORBELL_TIMER_LONG, 0);
}