#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "clock.h"

static struct {
    volatile int8_t hour, minute, second;
} data;

ISR(RTC_PIT_vect) {
    RTC.PITINTFLAGS = RTC_PI_bm;
    data.second++;
    if (data.second >= 60) {
        data.second = 0;
        data.minute++;
        if (data.minute >= 60) {
            data.minute = 0;
            data.hour++;
            if (data.hour >= 24) {
                data.hour = 0;
            }
        }
    }
}

void clock_init(void) {
    CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = CLKCTRL_ENABLE_bm; // external crystal
    // CLKCTRL.XOSC32KCTRLA = CLKCTRL_SEL_bm | CLKCTRL_ENABLE_bm // external clock
    RTC.CLKSEL           = RTC_CLKSEL_TOSC32K_gc;
    RTC.CTRLA            = RTC_PRESCALER_DIV32_gc | RTC_RTCEN_bm; // 1024Hz ticks
    RTC.PITINTCTRL       = RTC_PI_bm;
    RTC.PITCTRLA         = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm; // 1s interrupt
    while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm));
}

uint16_t clock_ticks(void) {
    return RTC.CNT;
}

void clock_now(int8_t *hour, int8_t *minute, int8_t *second) {
    cli();
    *hour   = data.hour;
    *minute = data.minute;
    *second = data.second;
    sei();
}

void clock_set(int8_t hour, int8_t minute, int8_t second) {
    while (second >= 60) { second -= 60; minute++; }
    while (second < 0)   { second += 60; minute--; }
    while (minute >= 60) { minute -= 60; hour++;   }
    while (minute < 0)   { minute += 60; hour--;   }
    while (hour   >= 24) { hour   -= 24; }
    while (hour   < 0)   { hour   += 24; }

    cli();
    data.hour   = hour;
    data.minute = minute;
    data.second = second;
    sei();
}
