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

// !!! MAKE SURE TO EDIT settings.h.template and rename to settings.h  !!!
#include "Settings.h"   
#include "Powersave.h"
#include "OpenWeatherMapHandler.h"
#include "WeatherDisplay.h"

#include "M5Stack.h"

#include <Arduino.h>

#include <time.h>
#include <sys/time.h>

#include <driver/dac.h>

WeatherData data;
WeatherDisplay display = WeatherDisplay();

uint32_t screen = SCREEN_SUMMARY;
unsigned long wakeupIdleTime;

void handleButtonPress(uint16_t buttonPin, boolean awake) {
    if (buttonPin == BUTTON_A_PIN) {
      (screen > 0) ? screen-- : screen = SCREEN_MAX;
    }
    if (buttonPin == BUTTON_B_PIN) {
      if ( awake ) {
        updateData(&data, &display, true);
      } else {
        // nothing
      }
    }
    if (buttonPin == BUTTON_C_PIN) {
      (screen < SCREEN_MAX) ? screen++ : screen = 0;
    }
    wakeupIdleTime = millis();
}

// TODO evaluate sleep2() from https://community.m5stack.com/topic/1162/getting-longer-battery-run-time/6

void setup() {
  initalPowerSettings();

  M5.begin(false, false, true, false); // LCD false SD false Serial true I2C false
  M5.Power.begin();

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // startup caused by timer
  if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    // restore data from slow_mem
    screen = RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_screen_var_offset];
    memcpy(&data, &RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_data_var_offset], sizeof(data));

    // this function is one of the main reasons to choose LovyanGFX over Bodmer's lib
    // use lcdSleepBrightness in order to silently update
    display.init_without_reset(lcdSleepBrightness);
  
    updateData(&data, &display, true);
  }
  // startup caused by ULP wake event
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_ULP) {
    // restore data from slow_mem
    screen = RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_screen_var_offset];
    memcpy(&data, &RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_data_var_offset], sizeof(data));

    // since ulp_wakeup_button_var_offset is written from within ULP code, it needs to be filtered
    uint16_t buttonPin = RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_wakeup_button_var_offset] & 0xffff;

    handleButtonPress(buttonPin, false);

    // signal ULP to stop, now that we are ready to init the display
    RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_enable_var_offset] = 0;

    // this function is one of the main reasons to choose LovyanGFX over Bodmer's lib
    display.init_without_reset(lcdActiveBrightness);
  }
  // startup for very first time after poweron
  else {
    // turn off noise as much possible on M5Stack gray
    // https://community.m5stack.com/topic/61/noise-on-speaker/16
    // https://community.m5stack.com/post/10908
    dacWrite (SPEAKER_PIN,0);

    // initialize dispaly
    display.init();
    display.showStartupScreen();

    // turn off wifi AP mode
    WiFi.mode(WIFI_MODE_STA);
     
    updateData(&data, &display, true);
  }
}

// update button states
void updateButtons() {
  M5.update(); 
  if (M5.BtnA.wasPressed()) {
    handleButtonPress(BUTTON_A_PIN, true);
  }
  //if (M5.BtnB.pressedFor(2000)) {
  //  M5.Power.powerOFF();
  //}
  if (M5.BtnB.wasPressed()) {
    handleButtonPress(BUTTON_B_PIN, true);
  }
  if (M5.BtnC.wasPressed()) {
    handleButtonPress(BUTTON_C_PIN, true);
  }
}

void updateScreen() {
  switch (screen) {
  case 0:
    display.showMainWeatherScreen(&data);
    break;
  case 1:
    display.showTodayWeatherScreen(&data);
    break;
  case 2:
    display.showForecastScreen(&data);
    break;
  default:
    display.showTestScreen(&data);
    break;
  }
}

void loop() {
  updateButtons();

  updateData(&data, &display, false);

  updateScreen();
 
  unsigned long passedIdleMillis = millis() - wakeupIdleTime;
  
  if (passedIdleMillis > DISPLAY_WAKE_MILLIS) {
    Serial.print("idle for ");
    Serial.println(passedIdleMillis);

    RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_screen_var_offset] = screen;
    memcpy(&RTC_SLOW_MEM[rtc_slow_mem_vars_start + ulp_data_var_offset], &data, sizeof(data));

    time_t now;
    time(&now);
    uint64_t sleepTime = 1000L*1000L*UPDATE_INTERVAL_SECS - (now - data.timeSinceLastWUpdate);
    if ( sleepTime <= 0 ) sleepTime = 1000L*1000L*UPDATE_INTERVAL_SECS;
    Serial.println("going to sleep");

    enterDeepSleep(sleepTime);
  }
}

