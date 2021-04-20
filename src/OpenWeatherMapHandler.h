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

#ifndef OPEN_WEATHER_MAP_HANDLER_H
#define OPEN_WEATHER_MAP_HANDLER_H

#include "Settings.h"  

#include <JsonListener.h>
#include <JsonStreamingParser.h>

class WeatherDisplay; // forward declaration to avoid https://stackoverflow.com/questions/14084826/c-has-not-been-declared

const uint8_t MAX_FORECASTS = 4;

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

// https://diyprojects.io/esp32-store-temporary-data-rtc-memory-deep-sleep/
typedef struct WeatherDataCurrent {
  char description[27];
  // "icon": "09d"
  char icon[4];
  float temp;
  float feels_like;
  uint16_t pressure;
  uint8_t humidity;
  float dew_point;
  float uvi;
  uint16_t visibility;
  float windSpeed;
  float windDeg;
  uint8_t clouds;
  uint32_t sunrise;
  uint32_t sunset;
  uint32_t observationTime;
} WeatherDataCurrent;

typedef struct WeatherDataForecast {
  char description[27];
  char icon[4];
  float tempNight;
  float tempDay;
  uint16_t pressure;
  float dew_point;
  float uvi;
  uint8_t humidity;
  float windSpeed;
  float windDeg;
  uint8_t clouds;
  uint32_t observationTime;
} WeatherDataForecast;

typedef struct WeatherData {
  time_t timeSinceLastWUpdate;
  int timezone_offset;
  WeatherDataCurrent current;
  WeatherDataForecast forecasts[MAX_FORECASTS];
} WeatherData;

void updateData(WeatherData *data, WeatherDisplay *display, bool force);


class OpenWeatherMapOneCallClient: public JsonListener {
  private:
    const String host = "api.openweathermap.org";
    const uint16_t port = 80;
    String currentKey;
    String currentParent;
    String currentParentParent;
    boolean isDailyArray;
    WeatherData *data;
    uint8_t weatherItemCounter = 0;
    uint8_t dailyItemCounter = 0;
    uint8_t maxForecasts;
    uint8_t statusCode; // REST call status
    String statusMessage; // REST call message

    void doUpdate(WeatherData *data, String path);

  public:
    OpenWeatherMapOneCallClient();
    void updateDataById(WeatherData *data, String appId, String lat, String lon, uint8_t maxForecasts);
    boolean isStatusError();
    String getStatusMessage();

    virtual void whitespace(char c);
    virtual void startDocument();
    virtual void key(String key);
    virtual void value(String value);
    virtual void endArray();
    virtual void endObject();
    virtual void endDocument();
    virtual void startArray();
    virtual void startObject();
};

#endif