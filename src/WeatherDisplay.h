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

#ifndef WEATHER_DISPLAY_H
#define WEATHER_DISPLAY_H

#include "Settings.h"  
#include "WeatherLcd.h"

const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

const uint8_t SCREEN_SUMMARY = 0;
const uint8_t SCREEN_DETAILS = 1;
const uint8_t SCREEN_FORECAST = 2;
const uint8_t SCREEN_MAX = SCREEN_FORECAST;

class WeatherData;

class WeatherDisplay  {
      private:
        WeatherLcd mylcd = WeatherLcd();
        void drawProgress(int percentage, String label);
        void drawCurrentWeather(WeatherData *data, int16_t x, int16_t y);
        void drawTodayWeatherDetails(WeatherData *data, int16_t x, int16_t y);
        void drawForecast(WeatherData *data, int16_t x, int16_t y);
        void drawForecastDetails(WeatherData *data, int x, int y, int dayIndex);
        void drawForecastList(WeatherData *data, int16_t x, int16_t y);
        void drawForecastListItem(WeatherData *data, int x, int y, int dayIndex);
        void drawHeader(WeatherData *data, int16_t x, int16_t y, String title);

      public:
        WeatherDisplay();
        void init();
        void init_without_reset(std::uint8_t brightness);

        void showStartupScreen();
        void showMainWeatherScreen(WeatherData *data);
        void showTodayWeatherScreen(WeatherData *data);
        void showForecastScreen(WeatherData *data);
        void showTestScreen(WeatherData *data);

        void showError(String msg);
        void showProgressOverlay(int percentage, String label);
};


const uint16_t* getWeatherImage(String icon);
String getBearing(float heading);
String getMeteoconIcon(String icon);



#endif