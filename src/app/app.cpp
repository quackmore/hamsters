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

#include "app.hpp"
#include "app_wheel.hpp"
#include "app_doorbell.hpp"
#include "app_relay_sequences.hpp"
#include "espbot_global.hpp"
#include "espbot_gpio.hpp"
#include "library_dht.hpp"
#include "library_max6675.hpp"

/*
 *  APP_RELEASE is coming from git
 *  'git --no-pager describe --tags --always --dirty'
 *  and is generated by the Makefile
 */

#ifndef APP_RELEASE
#define APP_RELEASE "Unavailable"
#endif

char *app_release = APP_RELEASE;

char *app_name = "HAMSTERS";

void app_init_before_wifi(void)
{
    init_dio_task();
    init_relay_seq_default();
    app_init_wheel();
    app_init_doorbell();
}

void app_init_after_wifi(void)
{
    esp_mDns.start(espbot.get_name());
}

void app_deinit_on_wifi_disconnect()
{
    esp_mDns.stop();
}