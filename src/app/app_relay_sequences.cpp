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
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "ip_addr.h"
#include "library_di_sequence.h"
#include "library_do_sequence.h"
#include "esp8266_io.h"
#include "gpio.h"
}

#include "app.hpp"
#include "app_relay_sequences.hpp"
#include "espbot_global.hpp"
#include "espbot_debug.hpp"
#include "library.hpp"

// sequence definition
//  1   2   3   4   5
//     ___     ___     relay open   (output HIGH)
// ___|   |___|   |___ relay closed (output LOW)
//
//                               1          2          3          4          5          6
static int pulse_sequence_1[] = {100, 1000, 100, 1000, 100, 1000, 100, 1000, 100, 1000,
                                 100, 1000, 100, 1000, 100, 1000, 100, 1000, 100, 1000,
                                 100, 1000, 100, 1000, 100, 1000, 100, 1000, 100, 1000,
                                 100, 1000, 100, 1000, 100, 1000, 100, 1000, 100};

static int pulse_sequence_2[] = {100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100, 1000,
                                 100, 100, 100, 1000, 100, 100, 100};

static int pulse_sequence_3[] = {100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100};

static int pulse_sequence_4[] = {100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100, 1000,
                                 100, 100, 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 100, 100, 100};

#define RELAY_SEQUENCES_COUNT 4

static int relay_do[RELAY_SEQUENCES_COUNT] = {ESPBOT_D1,
                                              ESPBOT_D2,
                                              ESPBOT_D3,
                                              ESPBOT_D4};

static int *pulse_sequence[RELAY_SEQUENCES_COUNT] = {pulse_sequence_1,
                                                     pulse_sequence_2,
                                                     pulse_sequence_3,
                                                     pulse_sequence_4};

static int pulse_sequence_size[RELAY_SEQUENCES_COUNT] = {sizeof(pulse_sequence_1) / sizeof(pulse_sequence_1[0]),
                                                         sizeof(pulse_sequence_2) / sizeof(pulse_sequence_2[0]),
                                                         sizeof(pulse_sequence_3) / sizeof(pulse_sequence_3[0]),
                                                         sizeof(pulse_sequence_4) / sizeof(pulse_sequence_4[0])};

static bool relay_seq_enabled[RELAY_SEQUENCES_COUNT];
static int32 relay_seq_period[RELAY_SEQUENCES_COUNT];
static os_timer_t relay_seq_timer[RELAY_SEQUENCES_COUNT];
static struct do_seq *relay_seq[RELAY_SEQUENCES_COUNT];

static void ICACHE_FLASH_ATTR relay_sequence_timer_func(void *param)
{
    int idx = (int)param;
    exe_do_seq_ms(relay_seq[idx]);
}

void ICACHE_FLASH_ATTR init_relay_seq_default(void)
{
    int ii;
    for (ii = 0; ii < RELAY_SEQUENCES_COUNT; ii++)
    {
        relay_seq_enabled[ii] = false;
        relay_seq_period[ii] = 60000;
        os_timer_disarm(&relay_seq_timer[ii]);
        os_timer_setfn(&relay_seq_timer[ii], (os_timer_func_t *)relay_sequence_timer_func, (void *)ii);
        init_relay_sequences(ii, ii);
    }
}

static void ICACHE_FLASH_ATTR relay_sequence_completed(void *param)
{
    int idx = (int)param;
    esplog.trace("relay sequence [%d] completed\n", idx);
}

void ICACHE_FLASH_ATTR init_relay_sequences(int idx, int pulse_idx)
{
    PIN_FUNC_SELECT(gpio_MUX(relay_do[idx]), gpio_FUNC(relay_do[idx]));
    GPIO_OUTPUT_SET(gpio_NUM(relay_do[idx]), ESPBOT_HIGH);
    int pulse_sequence_length = pulse_sequence_size[pulse_idx];
    relay_seq[idx] = new_do_seq(gpio_NUM(relay_do[idx]), pulse_sequence_length);
    set_do_seq_cb(relay_seq[idx], relay_sequence_completed, (void *)idx, task);
    out_seq_clear(relay_seq[idx]);
    int ii;
    int level = ESPBOT_LOW;
    for (ii = 0; ii < pulse_sequence_length; ii++)
    {
        out_seq_add(relay_seq[idx], level, pulse_sequence[pulse_idx][ii]);
        if (level == ESPBOT_LOW)
            level = ESPBOT_HIGH;
        else
            level = ESPBOT_LOW;
    }
    // {
    //     int ii = 0;
    //     char level;
    //     uint32 duration;
    //     os_printf("Sequence defined as:\n");
    //     for (ii = 0; ii < get_do_seq_length(relay_seq[idx]); ii++)
    //     {
    //         level = get_do_seq_pulse_level(relay_seq[idx], ii);
    //         duration = get_do_seq_pulse_duration(relay_seq[idx], ii);
    //         if (level == ESPBOT_LOW)
    //             os_printf("pulse %d: level  'LOW' - duration %d\n", ii, duration);
    //         else
    //             os_printf("pulse %d: level 'HIGH' - duration %d\n", ii, duration);
    //     }
    //     os_printf("Sequence end.\n");
    // }
}

void ICACHE_FLASH_ATTR exe_relay_sequences(int idx)
{
    exe_do_seq_ms(relay_seq[idx]);
}

void ICACHE_FLASH_ATTR enable_relay_seq(int idx, int period)
{
    esplog.trace("enabling relay sequence [%d] period: %d\n", idx, period);
    relay_seq_enabled[idx] = true;
    relay_seq_period[idx] = period;
    os_timer_arm(&relay_seq_timer[idx], relay_seq_period[idx], 1);
    // exe_relay_sequences(idx);
}

void ICACHE_FLASH_ATTR disable_relay_seq(int idx)
{
    esplog.trace("disabling relay sequence [%d]\n", idx);
    relay_seq_enabled[idx] = false;
    os_timer_disarm(&relay_seq_timer[idx]);
}