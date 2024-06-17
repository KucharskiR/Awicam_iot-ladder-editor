/*
Esp sends logs via usb-jtag and uart0, this can be change by recompiling arduino-esp libraries but then new libraries need to be copied and configured to ladder diagram generator. This file can avoid this problems by redefining sending function.
After restart ROM image is send via usb-jtag and uart0, this probaly have not impact on functionality.
*/
#include "defines.h"
#ifdef USE_LOGGER_USB
#pragma once
#include <Arduino.h>
#include "esp_log.h"
#include "esp32-hal-log.h"
#include "hal/usb_serial_jtag_ll.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag.h"

// Diffrent formating
// #ifdef ARDUHAL_LOG_FORMAT
// #undef ARDUHAL_LOG_FORMAT
// #endif
// #define ARDUHAL_LOG_FORMAT(letter, format)  ARDUHAL_LOG_COLOR_ ## letter "[%6u][" #letter "]" format ARDUHAL_LOG_RESET_COLOR "\r\n", (unsigned long) (esp_timer_get_time() / 1000ULL)


#ifdef log_v
#undef log_v
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#ifndef USE_ESP_IDF_LOG
#define log_v(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define log_v(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_VERBOSE, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_v(format, ...)  do {} while(0)
#endif

#ifdef log_d
#undef log_d
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#ifndef USE_ESP_IDF_LOG
#define log_d(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT (D, format), ##__VA_ARGS__)
#else
#define log_d(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_DEBUG, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_d(format, ...)  do {} while(0)
#endif

#ifdef log_i
#undef log_i
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#ifndef USE_ESP_IDF_LOG
#define log_i(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT (I, format), ##__VA_ARGS__)
#else
#define log_i(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_i(format, ...) do {} while(0)
#endif

#ifdef log_w
#undef log_w
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#ifndef USE_ESP_IDF_LOG
#define log_w(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT (W, format), ##__VA_ARGS__)
#else
#define log_w(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_w(format, ...) do {} while(0)
#endif

#ifdef log_e
#undef log_e
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#ifndef USE_ESP_IDF_LOG
#define log_e(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT (E, format), ##__VA_ARGS__)
#else
#define log_e(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_e(format, ...) do {} while(0)
#endif

#ifdef log_n
#undef log_n
#endif
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_NONE
#ifndef USE_ESP_IDF_LOG
#define log_n(format, ...) log_printf_USB(ARDUHAL_LOG_FORMAT (E, format), ##__VA_ARGS__)
#else
#define log_n(format, ...) do {ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR, TAG, format, ##__VA_ARGS__);}while(0)
#endif
#else
#define log_n(format, ...) do {} while(0)
#endif

int log_printfv_USB(const char *format, va_list arg)
{
    static char loc_buf[64];
    char * temp = loc_buf;
    uint32_t len;
    va_list copy;
    va_copy(copy, arg);
    len = vsnprintf(NULL, 0, format, copy);
    
    va_end(copy);
    if(len >= sizeof(loc_buf)){
        temp = (char*)malloc(len+1);
        if(temp == NULL) {
            return 0;
        }
    }

    // Timeout to not freeze when usb is not connected
    int32_t prev = static_cast<int32_t>(esp_timer_get_time());
    vsnprintf(temp, len+1, format, arg);
    for (uint32_t i = 0; i < len; i++)
    {
        while(USB_SERIAL_JTAG.ep1_conf.serial_in_ep_data_free == 0 && static_cast<int32_t>(esp_timer_get_time()) - prev < 500)
        {}

        USB_SERIAL_JTAG.ep1.rdwr_byte = temp[i];
    }
    USB_SERIAL_JTAG.ep1_conf.wr_done = 1;        
    
    if(len >= sizeof(loc_buf)){
        free(temp);
    }
    return len;
}

int log_printf_USB(const char *format, ...)
{
    int len;
    va_list arg;
    va_start(arg, format);
    len = log_printfv_USB(format, arg);
    va_end(arg);
    return len;
}
#endif