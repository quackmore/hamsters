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
#include "app_test.hpp"
#include "espbot_global.hpp"
#include "espbot_debug.hpp"
#include "library.hpp"

// function for testing purpose

extern "C" char *f2str(char *str, float value, int decimals);

static void ICACHE_FLASH_ATTR output_seq_completed(void *param)
{
    struct do_seq *seq = (struct do_seq *)param;
    free_do_seq(seq);
    os_printf("Test completed\n");
}

static void ICACHE_FLASH_ATTR input_seq_completed(void *param)
{
    struct di_seq *seq = (struct di_seq *)param;
    if (seq->ended_by_timeout)
        os_printf("Input sequence reading ended by timeout timer\n");
    else
        os_printf("Input sequence reading completed\n");
    {
        int idx = 0;
        char level;
        uint32 duration;
        os_printf("Sequence acquired:\n");
        for (idx = 0; idx < get_di_seq_length(seq); idx++)
        {
            level = get_di_seq_pulse_level(seq, idx);
            duration = get_di_seq_pulse_duration(seq, idx);
            if (level == ESPBOT_LOW)
                os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
            else
                os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
        }
        os_printf("Sequence end.\n");
    }
    free_di_seq(seq);
}

uint32 start_time;
uint32 end_time;
static di_seq *dht_input;

static void ICACHE_FLASH_ATTR print_sensor(sensor_t *sensor)
{
    Heap_chunk str(20);
    os_printf("         SENSOR\n");
    os_printf("===>       name: %s\n", sensor->name);
    os_printf("===>         id: %d\n", sensor->sensor_id);
    os_printf("===>       type: %d\n", sensor->type);
    os_printf("===>  max value: %s\n", f2str(str.ref, sensor->max_value, 2));
    os_printf("===>  min value: %s\n", f2str(str.ref, sensor->min_value, 2));
    os_printf("===> resolution: %s\n", f2str(str.ref, sensor->resolution, 2));
    os_printf("===>  min delay: %d\n", sensor->min_delay);
}

static void ICACHE_FLASH_ATTR print_event(sensors_event_t *event, int decimals)
{
    Heap_chunk str(20);
    os_printf("          EVENT\n");
    os_printf("==>   sensor id: %d\n", event->sensor_id);
    os_printf("==> sensor type: %d\n", event->type);
    os_printf("==>     invalid: %d\n", event->invalid);
    os_printf("==> temperature: %s\n", f2str(str.ref, event->temperature, decimals));
    os_printf("==>   timestamp: %s\n", esp_sntp.get_timestr(event->timestamp));
}

static void ICACHE_FLASH_ATTR get_and_print_max6675_event(void *param)
{
    sensors_event_t event;
    max6675->getEvent(&event);
    print_event(&event, 2);
}

static void ICACHE_FLASH_ATTR get_and_print_dht22_temperature_event(void *param)
{
    sensors_event_t event;
    dht22->temperature.getEvent(&event);
    print_event(&event, 1);
}

static void ICACHE_FLASH_ATTR get_and_print_dht22_humidity_event(void *param)
{
    sensors_event_t event;
    dht22->humidity.getEvent(&event);
    print_event(&event, 1);
}

static void ICACHE_FLASH_ATTR dht_reading_completed(void *param)
{
    struct di_seq *seq = (struct di_seq *)param;
    os_printf("start DHT -> start reading = %d\n", (end_time - start_time));
    if (seq->ended_by_timeout)
        os_printf("DHT reading ended by timeout timer\n");
    else
        os_printf("DHT reading completed\n");
    {
        int idx = 0;
        char level;
        uint32 duration;
        os_printf("DHT sequence acquired:\n");
        for (idx = 0; idx < get_di_seq_length(seq); idx++)
        {
            level = get_di_seq_pulse_level(seq, idx);
            duration = get_di_seq_pulse_duration(seq, idx);
            if (level == ESPBOT_LOW)
                os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
            else
                os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
        }
        os_printf("DHT sequence end.\n");
    }
    free_di_seq(seq);
}

static void dht_start_completed(void *param)
{
    struct do_seq *seq = (struct do_seq *)param;
    // start reading from DHT
    PIN_FUNC_SELECT(ESPBOT_D2_MUX, ESPBOT_D2_FUNC);
    PIN_PULLUP_EN(ESPBOT_D2_MUX);
    GPIO_DIS_OUTPUT(ESPBOT_D2_NUM);

    read_di_sequence(dht_input);

    // free output sequence
    free_do_seq(seq);
}

void ICACHE_FLASH_ATTR run_test(int idx)
{
    struct do_seq *seq;
    switch (idx)
    {
    case 1:
    {
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 1);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_ms(seq);
    }
    break;
    case 2:
    {
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1500);
        out_seq_add(seq, ESPBOT_LOW, 2000);
        out_seq_add(seq, ESPBOT_HIGH, 2500);
        out_seq_add(seq, ESPBOT_LOW, 3000);
        out_seq_add(seq, ESPBOT_HIGH, 3500);
        out_seq_add(seq, ESPBOT_LOW, 4000);
        out_seq_add(seq, ESPBOT_HIGH, 4500);
        out_seq_add(seq, ESPBOT_LOW, 5000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_ms(seq);
    }
    break;
    case 3:
    {
        // sequence 1
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1500);
        out_seq_add(seq, ESPBOT_LOW, 2000);
        out_seq_add(seq, ESPBOT_HIGH, 2500);
        out_seq_add(seq, ESPBOT_LOW, 3000);
        out_seq_add(seq, ESPBOT_HIGH, 3500);
        out_seq_add(seq, ESPBOT_LOW, 4000);
        out_seq_add(seq, ESPBOT_HIGH, 4500);
        out_seq_add(seq, ESPBOT_LOW, 5000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        // sequence 2
        struct do_seq *seq_2;
        PIN_FUNC_SELECT(ESPBOT_D5_MUX, ESPBOT_D5_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D5_NUM, ESPBOT_HIGH);
        seq_2 = new_do_seq(ESPBOT_D5_NUM, 8);
        set_do_seq_cb(seq_2, output_seq_completed, (void *)seq_2, task);

        out_seq_clear(seq_2);
        out_seq_add(seq_2, ESPBOT_LOW, 2000);
        out_seq_add(seq_2, ESPBOT_HIGH, 3000);
        out_seq_add(seq_2, ESPBOT_LOW, 2000);
        out_seq_add(seq_2, ESPBOT_HIGH, 3000);
        out_seq_add(seq_2, ESPBOT_LOW, 2000);
        out_seq_add(seq_2, ESPBOT_HIGH, 3000);
        out_seq_add(seq_2, ESPBOT_LOW, 2000);
        out_seq_add(seq_2, ESPBOT_HIGH, 3000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq_2); idx++)
            {
                level = get_do_seq_pulse_level(seq_2, idx);
                duration = get_do_seq_pulse_duration(seq_2, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_ms(seq);
        exe_do_seq_ms(seq_2);
    }
    break;
    case 4:
    {
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 150);
        out_seq_add(seq, ESPBOT_HIGH, 150);
        out_seq_add(seq, ESPBOT_LOW, 150);
        out_seq_add(seq, ESPBOT_HIGH, 150);
        out_seq_add(seq, ESPBOT_LOW, 150);
        out_seq_add(seq, ESPBOT_HIGH, 150);
        out_seq_add(seq, ESPBOT_LOW, 150);
        out_seq_add(seq, ESPBOT_HIGH, 150);
        out_seq_add(seq, ESPBOT_LOW, 150);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_ms(seq);
    }
    break;
    case 5:
    {
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 150000);
        out_seq_add(seq, ESPBOT_HIGH, 150000);
        out_seq_add(seq, ESPBOT_LOW, 150000);
        out_seq_add(seq, ESPBOT_HIGH, 150000);
        out_seq_add(seq, ESPBOT_LOW, 150000);
        out_seq_add(seq, ESPBOT_HIGH, 150000);
        out_seq_add(seq, ESPBOT_LOW, 150000);
        out_seq_add(seq, ESPBOT_HIGH, 150000);
        out_seq_add(seq, ESPBOT_LOW, 150000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_us(seq);
    }
    break;
    case 6:
    {
        // checking input sequence reading (ms)
        // defining input sequence
        PIN_FUNC_SELECT(ESPBOT_D5_MUX, ESPBOT_D5_FUNC);
        PIN_PULLUP_EN(ESPBOT_D5_MUX);
        GPIO_DIS_OUTPUT(ESPBOT_D5_NUM);

        struct di_seq *input_seq = new_di_seq(ESPBOT_D5_NUM, 9, 20, TIMEOUT_MS);
        set_di_seq_cb(input_seq, input_seq_completed, (void *)input_seq, task);
        read_di_sequence(input_seq);

        // now the output
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_us(seq);
    }
    break;
    case 7:
    {
        // checking input sequence reading (us)
        // defining input sequence
        PIN_FUNC_SELECT(ESPBOT_D5_MUX, ESPBOT_D5_FUNC);
        PIN_PULLUP_EN(ESPBOT_D5_MUX);
        GPIO_DIS_OUTPUT(ESPBOT_D5_NUM);

        struct di_seq *input_seq = new_di_seq(ESPBOT_D5_NUM, 9, 750000, TIMEOUT_US);
        set_di_seq_cb(input_seq, input_seq_completed, (void *)input_seq, task);

        // now the output
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 9);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        read_di_sequence(input_seq);
        exe_do_seq_ms(seq);
    }
    break;
    case 8:
    {
        // checking input sequence reading end by timeout (ms)
        // defining input sequence
        PIN_FUNC_SELECT(ESPBOT_D5_MUX, ESPBOT_D5_FUNC);
        PIN_PULLUP_EN(ESPBOT_D5_MUX);
        GPIO_DIS_OUTPUT(ESPBOT_D5_NUM);

        struct di_seq *input_seq = new_di_seq(ESPBOT_D5_NUM, 9, 20, TIMEOUT_MS);
        set_di_seq_cb(input_seq, input_seq_completed, (void *)input_seq, task);
        read_di_sequence(input_seq);

        // now the output
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 7);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        out_seq_add(seq, ESPBOT_HIGH, 1000);
        out_seq_add(seq, ESPBOT_LOW, 1000);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        exe_do_seq_us(seq);
    }
    break;
    case 9:
    {
        // checking input sequence reading end by timeout (us)
        // defining input sequence
        PIN_FUNC_SELECT(ESPBOT_D5_MUX, ESPBOT_D5_FUNC);
        PIN_PULLUP_EN(ESPBOT_D5_MUX);
        GPIO_DIS_OUTPUT(ESPBOT_D5_NUM);

        struct di_seq *input_seq = new_di_seq(ESPBOT_D5_NUM, 9, 50000, TIMEOUT_US);
        set_di_seq_cb(input_seq, input_seq_completed, (void *)input_seq, task);

        // now the output
        PIN_FUNC_SELECT(ESPBOT_D4_MUX, ESPBOT_D4_FUNC);
        GPIO_OUTPUT_SET(ESPBOT_D4_NUM, ESPBOT_HIGH);
        seq = new_do_seq(ESPBOT_D4_NUM, 7);
        set_do_seq_cb(seq, output_seq_completed, (void *)seq, task);

        out_seq_clear(seq);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        out_seq_add(seq, ESPBOT_HIGH, 5);
        out_seq_add(seq, ESPBOT_LOW, 5);
        {
            int idx = 0;
            char level;
            uint32 duration;
            os_printf("Sequence defined as:\n");
            for (idx = 0; idx < get_do_seq_length(seq); idx++)
            {
                level = get_do_seq_pulse_level(seq, idx);
                duration = get_do_seq_pulse_duration(seq, idx);
                if (level == ESPBOT_LOW)
                    os_printf("pulse %d: level  'LOW' - duration %d\n", idx, duration);
                else
                    os_printf("pulse %d: level 'HIGH' - duration %d\n", idx, duration);
            }
            os_printf("Sequence end.\n");
        }
        read_di_sequence(input_seq);
        exe_do_seq_ms(seq);
    }
    break;
        //    case 10:
        //    {
        //        // DHT reading
        //        // Send start sequence
        //        // High ____                    ____
        //        // Low      |_____ 1,5 ms _____|
        //        PIN_FUNC_SELECT(ESPBOT_D2_MUX, ESPBOT_D2_FUNC);
        //        GPIO_OUTPUT_SET(ESPBOT_D2_NUM, ESPBOT_HIGH);
        //        seq = new_do_seq(ESPBOT_D2_NUM, 1);
        //        set_do_seq_cb(seq, dht_start_completed, (void *)seq, task);
        //        out_seq_add(seq, ESPBOT_LOW, 1500);
        //        // out_seq_add(seq, ESPBOT_HIGH, 20);
        //        // prepare input sequence
        //        dht_input = new_di_seq(ESPBOT_D2_NUM, 82, 1000, TIMEOUT_MS);
        //        set_di_seq_cb(dht_input, dht_reading_completed, (void *)dht_input, task);
        //
        //        exe_do_seq_us(seq);
        //    }
        //    break;
        //    case 11:
        //    {
        //        // Dht class test
        //        int idx;
        //        os_printf("DHT22 samples\n");
        //        for (idx = 0; idx < 30; idx++)
        //        {
        //            if (dht22.get_invalid(idx))
        //                os_printf("--- %d (invalid) %d %d %s",
        //                          idx,
        //                          (int)(dht22.get_temperature(Celsius, idx) * 10),
        //                          (int)(dht22.get_humidity(idx) * 10),
        //                          esp_sntp.get_timestr(dht22.get_timestamp(idx)));
        //            else
        //                os_printf("--- %d           %d %d %s",
        //                          idx,
        //                          (int)(dht22.get_temperature(Celsius, idx) * 10),
        //                          (int)(dht22.get_humidity(idx) * 10),
        //                          esp_sntp.get_timestr(dht22.get_timestamp(idx)));
        //        }
        //        os_printf("DHT22 end\n");
        //    }
        //    break;
        //    case 12:
        //    {
        //        // Dht class test
        //        os_printf("Latest DHT22 reading\n");
        //        if (dht22.get_invalid())
        //            os_printf("--- (invalid) %d %d %s",
        //                      (int)(dht22.get_temperature(Celsius) * 10),
        //                      (int)(dht22.get_humidity() * 10),
        //                      esp_sntp.get_timestr(dht22.get_timestamp()));
        //        else
        //            os_printf("---           %d %d %s",
        //                      (int)(dht22.get_temperature(Celsius) * 10),
        //                      (int)(dht22.get_humidity() * 10),
        //                      esp_sntp.get_timestr(dht22.get_timestamp()));
        //    }
        //    break;
        //    case 13:
        //    {
        //        // Dht class test
        //        os_printf("Latest DHT22 reading\n");
        //        if (dht22.get_invalid())
        //            os_printf("--- (invalid) %d %d %s",
        //                      (int)(dht22.get_temperature(Fahrenheit) * 10),
        //                      (int)(dht22.get_humidity() * 10),
        //                      esp_sntp.get_timestr(dht22.get_timestamp()));
        //        else
        //            os_printf("---           %d %d %s",
        //                      (int)(dht22.get_temperature(Fahrenheit) * 10),
        //                      (int)(dht22.get_humidity() * 10),
        //                      esp_sntp.get_timestr(dht22.get_timestamp()));
        //    }
        //    break;
        //    case 14:
        //    {
        //        // Max6675 class test
        //        int idx;
        //        os_printf("Max6675 samples\n");
        //        for (idx = 0; idx < 30; idx++)
        //        {
        //            if (max6675.get_invalid(idx))
        //                os_printf("--- %d (invalid) %d %s",
        //                          idx,
        //                          (int)(max6675.get_temperature(Celsius, idx) * 100),
        //                          esp_sntp.get_timestr(max6675.get_timestamp(idx)));
        //            else
        //                os_printf("--- %d           %d %s",
        //                          idx,
        //                          (int)(max6675.get_temperature(Celsius, idx) * 100),
        //                          esp_sntp.get_timestr(max6675.get_timestamp(idx)));
        //        }
        //        os_printf("Max6675 end\n");
        //    }
        //    break;
        //    case 15:
        //    {
        //        // Dht class test
        //        os_printf("Latest Max6675 reading\n");
        //        if (max6675.get_invalid())
        //            os_printf("--- %d (invalid) %d %s",
        //                      idx,
        //                      (int)(max6675.get_temperature(Celsius, idx) * 100),
        //                      esp_sntp.get_timestr(max6675.get_timestamp(idx)));
        //        else
        //            os_printf("---           %d %s",
        //                      (int)(max6675.get_temperature(Celsius) * 100),
        //                      esp_sntp.get_timestr(max6675.get_timestamp()));
        //    }
        //    break;
        //    case 16:
        //    {
        //        // Dht class test
        //        os_printf("Latest Max6675 reading\n");
        //        if (max6675.get_invalid())
        //            os_printf("--- %d (invalid) %d %s",
        //                      idx,
        //                      (int)(max6675.get_temperature(Celsius, idx) * 100),
        //                      esp_sntp.get_timestr(max6675.get_timestamp(idx)));
        //        else
        //            os_printf("---           %d %s",
        //                      (int)(max6675.get_temperature(Fahrenheit) * 100),
        //                      esp_sntp.get_timestr(max6675.get_timestamp()));
        //    }
        //    break;
    case 17:
    {
        // MAX6675 single reading and force reading
        sensor_t sensor;
        max6675->getSensor(&sensor);
        print_sensor(&sensor);
        sensors_event_t event;
        max6675->getEvent(&event);
        print_event(&event, 2);

        max6675->force_reading(get_and_print_max6675_event, NULL);
    }
    break;
    case 18:
    {
        // MAX6675 all events reading
        sensor_t sensor;
        max6675->getSensor(&sensor);
        print_sensor(&sensor);
        int idx;
        sensors_event_t event;
        for (idx = 0; idx < max6675->get_max_events_count(); idx++)
        {
            max6675->getEvent(&event, idx);
            print_event(&event, 2);
        }
    }
    break;
    case 19:
    {
        // DHT temperature single reading and force reading
        sensor_t sensor;
        dht22->temperature.getSensor(&sensor);
        print_sensor(&sensor);
        sensors_event_t event;
        dht22->temperature.getEvent(&event);
        print_event(&event, 1);

        dht22->temperature.force_reading(get_and_print_dht22_temperature_event, NULL);
    }
    break;
    case 20:
    {
        // DHT temperature all events reading
        sensor_t sensor;
        dht22->temperature.getSensor(&sensor);
        print_sensor(&sensor);
        int idx;
        sensors_event_t event;
        for (idx = 0; idx < dht22->temperature.get_max_events_count(); idx++)
        {
            dht22->temperature.getEvent(&event, idx);
            print_event(&event, 1);
        }
    }
    break;
    case 21:
    {
        // DHT humidity single reading and force reading
        sensor_t sensor;
        dht22->humidity.getSensor(&sensor);
        print_sensor(&sensor);
        sensors_event_t event;
        dht22->humidity.getEvent(&event);
        print_event(&event, 1);

        dht22->humidity.force_reading(get_and_print_dht22_humidity_event, NULL);
    }
    break;
    case 22:
    {
        // DHT humidity all events reading
        sensor_t sensor;
        dht22->humidity.getSensor(&sensor);
        print_sensor(&sensor);
        int idx;
        sensors_event_t event;
        for (idx = 0; idx < dht22->humidity.get_max_events_count(); idx++)
        {
            dht22->humidity.getEvent(&event, idx);
            print_event(&event, 1);
        }
    }
    break;
    default:
        break;
    }
}