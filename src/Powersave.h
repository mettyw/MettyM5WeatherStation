/**The MIT License (MIT)

Copyright (c) 2021 by Matthias Wetzka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef POWERSAVE_H
#define POWERSAVE_H

#include "esp32-hal-dac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "esp_attr.h"
#include "esp_intr.h"
#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

#include <esp_bt_main.h>
#include <esp_bt.h>
#include <esp32/ulp.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>

// https://community.m5stack.com/topic/596/display-on-during-deep-sleep/23#
// RTC slow memory map (vars are in the last 1 kB of total 8 kB)
// Note: all ULP commands / vars are 4 bytes in size
const size_t rtc_slow_mem_total_size = 2048; // 8 kB = 4 * 2048
const size_t rtc_slow_mem_vars_space = 256;  // 1 kB = 4 * 256
const size_t rtc_slow_mem_vars_start = rtc_slow_mem_total_size - rtc_slow_mem_vars_space;
const size_t rtc_slow_mem_prog_start = 0;
const size_t rtc_slow_mem_prog_space = rtc_slow_mem_total_size - rtc_slow_mem_vars_space; // 7 kB

const size_t ulp_enable_var_offset = 0;
const size_t ulp_wakeup_button_var_offset = 4;
const size_t ulp_screen_var_offset = 8;
const size_t ulp_data_var_offset = 12;

#define RTC_GET_SCREEN() \
    RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_screen_var_offset]


// https://esp32.com/viewtopic.php?t=10116
#define I_READ_PIN(pin) \
    I_RD_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + (uint32_t)rtc_gpio_desc[pin].rtc_num, RTC_GPIO_IN_NEXT_S + (uint32_t)rtc_gpio_desc[pin].rtc_num)

 // https://community.m5stack.com/topic/596/display-on-during-deep-sleep/23
void ulp_start(void) {
  // RTC slow mem init (except space allocated for vars)
  memset(RTC_SLOW_MEM, 0, rtc_slow_mem_prog_space * sizeof(uint32_t));
  // M5Stack LCD backlight is connected to GPIO32 (specify by +14)
  const gpio_num_t lcdPWMPin = GPIO_NUM_32;
  const int lcdPWMBit = RTCIO_GPIO32_CHANNEL + 14;
  // GPIO32 initialization (set to output and initial value is 0)
  rtc_gpio_init(lcdPWMPin);
  rtc_gpio_set_direction(lcdPWMPin, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_set_level(lcdPWMPin, 0);

  // signals the ULP to keep going until value is 0
  RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_enable_var_offset] = 1;

  enum {
    LBL_START,
    LBL_LED,
    LBL_WAKE,
    LBL_END,
  };
  // Define ULP program
  const ulp_insn_t ulp_prog[] = {
    M_LABEL(LBL_START),
    I_MOVI(R2, rtc_slow_mem_vars_start),
 //   // FIXME reenable as-is (beeping should be gone)
 //   I_LD(R1, R2, ulp_enable_var_offset),
     // we want the PWM to end only if the main program signals to stop
 //   M_BL(LBL_END, 1),
    I_READ_PIN(BUTTON_A_PIN),
    // store button in R1 to write this later to memory for retrieval by the main program
    I_MOVI(R1, BUTTON_A_PIN),
    // wake if button read value is 0
    M_BL(LBL_WAKE,1),
    I_READ_PIN(BUTTON_B_PIN),
    // store button in R1 to write this later to memory for retrieval by the main program
    I_MOVI(R1, BUTTON_B_PIN),
    // wake if button read value is 0
    M_BL(LBL_WAKE,1),
    I_READ_PIN(BUTTON_C_PIN),
    // store button in R1 to write this later to memory for retrieval by the main program
    I_MOVI(R1, BUTTON_C_PIN),
    // wake if button read value is 0
    M_BL(LBL_WAKE,1),

    M_LABEL(LBL_LED),
    // if no button pressed, continue with LED PWM    
    I_WR_REG(RTC_GPIO_OUT_REG, lcdPWMBit, lcdPWMBit, 1), // on
    I_DELAY(lcdSleepBrightness * 100),
    I_WR_REG(RTC_GPIO_OUT_REG, lcdPWMBit, lcdPWMBit, 0), // off
    I_DELAY(25500),
    M_BX(LBL_START),
    M_LABEL(LBL_WAKE),
    // R1 contains the value of the button that has been pressed, write to rtc slow mem
    I_ST(R1, R2, ulp_wakeup_button_var_offset),
    I_WAKE(),
    M_BX(LBL_LED),
    M_LABEL(LBL_END),
    I_END(),
    I_HALT(),
  };
  // Run ULP program
  size_t size = sizeof(ulp_prog) / sizeof(ulp_insn_t);

  ulp_process_macros_and_load(rtc_slow_mem_prog_start, ulp_prog, &size);
  ulp_run(rtc_slow_mem_prog_start);
}

void enterDeepSleep(uint64_t sleepTime) {
    //https://community.m5stack.com/topic/596/display-on-during-deep-sleep/29#
    M5.Power.setPowerBoostKeepOn(true);
  
    ulp_start();

    esp_sleep_enable_timer_wakeup(sleepTime);
    esp_sleep_enable_ulp_wakeup();

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    rtc_gpio_init((gpio_num_t)BUTTON_A_PIN);
    rtc_gpio_set_direction((gpio_num_t)BUTTON_A_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_hold_en((gpio_num_t) BUTTON_A_PIN);

    rtc_gpio_init((gpio_num_t)BUTTON_B_PIN);
    rtc_gpio_set_direction((gpio_num_t)BUTTON_B_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_hold_en((gpio_num_t) BUTTON_B_PIN);

    rtc_gpio_init((gpio_num_t)BUTTON_C_PIN);
    rtc_gpio_set_direction((gpio_num_t)BUTTON_C_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_hold_en((gpio_num_t) BUTTON_C_PIN);

    esp_deep_sleep_start();
}

void initalPowerSettings(void) {
  // reduce power consunption by reducing CPU freq from 160 to 80 -- https://www.savjee.be/2019/12/esp32-tips-to-increase-battery-life/
 //TODO setCpuFrequencyMhz(80);
  // turn off bluetooth since we do not need it
  btStop();
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_mem_release(ESP_BT_MODE_BTDM);
  // turn off ADC since we do not need it
  adc_power_off();
}


#endif