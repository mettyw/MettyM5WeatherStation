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

#include "WeatherDisplay.h"

#include "OpenWeatherMapHandler.h"

#include "Icons.h"
#include "WeatherImages.h"

WeatherDisplay::WeatherDisplay() {

}

void WeatherDisplay::init() {
  mylcd.init(); 
}

void WeatherDisplay::init_without_reset(std::uint8_t brightness) {
  mylcd.init_without_reset(brightness);
}

void WeatherDisplay::showStartupScreen() {
  mylcd.setBrightness(lcdActiveBrightness);
  mylcd.clear();
  mylcd.show();

  uint8_t x = 0;
  uint8_t y = 0;

  for(uint8_t i = 0; i < 28; i++) {
    mylcd.clear();
    mylcd.setColor(COLOR_AC);
  
    x = 175 + 40 * sinf((6+i)/100.0f * PI);
    y = 120 - 35 * cosf((6+i)/100.0f * PI);
    mylcd.fillCircle(x, y, 28);
  
    mylcd.setColor(COLOR_FG);
    mylcd.fillCircle(126, 115, 32);
    mylcd.fillCircle(166, 100, 40);
    mylcd.fillCircle(206, 127, 20);
    mylcd.fillRect(126, 127, 80, 22);
    mylcd.drawText("M5WeatherStation", 160, 165, TC_DATUM, COLOR_FG, mylcd.FONT_REGULAR_LARGE);
    mylcd.show();
  }

  mylcd.drawText("(c) 2021 mettyw", 160, 200, TC_DATUM, COLOR_AC, mylcd.FONT_BOLD_NORMAL);
  mylcd.show();

}

void WeatherDisplay::showMainWeatherScreen(WeatherData *data) {
  mylcd.clear();
  drawHeader(data, 0, 0, "Current");
  drawCurrentWeather(data, 0,30);
  drawForecast(data, 0,150);
  mylcd.show();
}

void WeatherDisplay::showTodayWeatherScreen(WeatherData *data) {
  mylcd.clear();
  drawHeader(data, 0, 0, "Today");
  drawTodayWeatherDetails(data, 0,30);
  mylcd.show();
}

void WeatherDisplay::showForecastScreen(WeatherData *data) {
  mylcd.clear();
  drawHeader(data, 0, 0, "Forecast");
  drawForecastList(data, 0,30);
  mylcd.show();
}

void WeatherDisplay::showTestScreen(WeatherData *data) {
  mylcd.clear();
/*  char buff[15];
  sprintf_P(buff, PSTR("FREE %02d"), ESP.getFreeHeap());
  mylcd.setFont(mylcd.FONT_REGULAR_NORMAL);
  mylcd.drawString(buff, 0,0);
  mylcd.pushTransImage(0*72, 0*72 + 50, weather_icon_width, weather_icon_height, weather_01d, 0x0000);
  mylcd.pushTransImage(1*72, 0*72 + 50, weather_icon_width, weather_icon_height, weather_11d, 0x0000);
  mylcd.pushTransImage(2*72, 0*72 + 50, weather_icon_width, weather_icon_height, weather_09d, 0x0000);
  mylcd.pushTransImage(3*72, 0*72 + 50, weather_icon_width, weather_icon_height, weather_50d, 0x0000);

  mylcd.pushTransImage(0*72, 1*72 + 50, weather_icon_width, weather_icon_height, weather_01n, 0x0000);
  mylcd.pushTransImage(1*72, 1*72 + 50, weather_icon_width, weather_icon_height, weather_11n, 0x0000);
  mylcd.pushTransImage(2*72, 1*72 + 50, weather_icon_width, weather_icon_height, weather_09n, 0x0000);
  mylcd.pushTransImage(3*72, 1*72 + 50, weather_icon_width, weather_icon_height, weather_50n, 0x0000);

  mylcd.pushTransImage(0*72, 2*72 + 50, weather_icon_width, weather_icon_height, weather_11d, 0x0000);
  mylcd.pushTransImage(1*72, 2*72 + 50, weather_icon_width, weather_icon_height, weather_11n, 0x0000);
  mylcd.pushTransImage(2*72, 2*72 + 50, weather_icon_width, weather_icon_height, weather_04, 0x0000);
  mylcd.pushTransImage(3*72, 2*72 + 50, weather_icon_width, weather_icon_height, weather_13d, 0x0000);*/
  mylcd.show();
}

void WeatherDisplay::showError(String msg) {
  mylcd.clear();
  mylcd.drawText(msg, 160, 120, MC_DATUM, COLOR_ERROR, mylcd.FONT_REGULAR_LARGE);
  mylcd.show();
}

void WeatherDisplay::showProgressOverlay(int percentage, String label) {
  mylcd.drawProgressImmediate(percentage, label);
}

void WeatherDisplay::drawCurrentWeather(WeatherData *data, int16_t x, int16_t y) {
  char buffer[14];

  const uint16_t* icon = getWeatherImage(data->current.icon);
  mylcd.pushTransImage(40 + x, 4 + y, weather_icon_width, weather_icon_height, icon, 0x0000);
  mylcd.setColor(COLOR_AC);

  //mylcd.drawTextLeftOrCenteredIfSmall(data->current.description, 15 + x, 4*24 - 5 + y, 160 - 15*2, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  uint32_t max_width = 160 - 15 - 5;
  mylcd.setFont(mylcd.FONT_REGULAR_NORMAL);
  if ( mylcd.textWidth(data->current.description) <= max_width) {
    mylcd.drawText(data->current.description, 76 + x, 4*24 - 5 + y, TC_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  } else {
    mylcd.drawWrappedText(data->current.description, 15 + x, 4*24 - 5 + y, max_width, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  }

  mylcd.drawTemperature(data->current.temp, 165, 4 + y, TL_DATUM, COLOR_AC, mylcd.FONT_REGULAR_HUGE);
  snprintf(buffer, sizeof(buffer), "%01d %%", data->current.humidity);
  mylcd.drawXBitmapAndText(icon_water_remove_outline_customized, buffer, 165, 2*24 - 5 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%0.1f uvi", data->current.uvi);
  mylcd.drawXBitmapAndText(icon_weather_sunny_alert, buffer, 165, 3*24 - 5 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%0.1f km/h (%s)", data->current.windSpeed * 3.6f, getBearing(data->current.windDeg).c_str());
  mylcd.drawXBitmapAndText(icon_weather_windy, buffer, 165, 4*24 - 5 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
}

void WeatherDisplay::drawForecast(WeatherData *data, int16_t x, int16_t y) {
  mylcd.drawFastHLine(0, y, 320, COLOR_FG);

  drawForecastDetails(data, x + 20 + 80*0, y + 7, 0);
  drawForecastDetails(data, x + 20 + 80*1, y + 7, 1);
  drawForecastDetails(data, x + 20 + 80*2, y + 7, 2);
  drawForecastDetails(data, x + 20 + 80*3, y + 7, 3);
}

void WeatherDisplay::drawForecastDetails(WeatherData *data, int x, int y, int dayIndex) {
  time_t observationTimestamp = data->forecasts[dayIndex].observationTime;
  observationTimestamp += data->timezone_offset;
  struct tm* timeInfo;
  timeInfo = gmtime(&observationTimestamp);
  mylcd.drawText(WDAY_NAMES[timeInfo->tm_wday], x + 20, y, TC_DATUM, COLOR_AC, mylcd.FONT_BOLD_NORMAL);
  mylcd.drawText(getMeteoconIcon(data->forecasts[dayIndex].icon), x + 20, y + 18, TC_DATUM, COLOR_FG, mylcd.FONT_METEO_NORMAL);
 
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%0.1f/%0.1f", data->forecasts[dayIndex].tempDay, data->forecasts[dayIndex].tempNight);
  mylcd.drawText(buffer, x + 20, y + 62, TC_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
}

void WeatherDisplay::drawTodayWeatherDetails(WeatherData *data, int16_t x, int16_t y) {
  char buffer[24];

  mylcd.drawTemperature(data->current.temp, 10, 6 + y, TL_DATUM, COLOR_AC, mylcd.FONT_REGULAR_HUGE);
  uint32_t max_width = 130;
  mylcd.setFont(mylcd.FONT_REGULAR_NORMAL);
  if ( mylcd.textWidth(data->forecasts[0].description) <= max_width) {
    mylcd.drawText(data->forecasts[0].description, 10, 2*24 + 6 -5 + y, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  } else {
    mylcd.drawWrappedText(data->forecasts[0].description, 10, 2*24 + 6 - 5 + y, max_width, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  }

  const uint16_t* icon = getWeatherImage(data->forecasts[0].icon);
  // small y - 5 displacement -- should be okay
  mylcd.pushTransImage(160-weather_icon_width/2, y-5, weather_icon_width, weather_icon_height, icon, 0x0000);

  // special handling for temperature to get degree symbol
  mylcd.drawXBitmap(160+weather_icon_width/2 + 20, 0*24 + 6  - 5 + y, icon_thermometer_chevron_up, icon_width, icon_height, COLOR_FG);
  mylcd.drawTemperature(data->forecasts[0].tempDay, 160+weather_icon_width/2 + 20 + 6 + icon_width, 0*24 + 6 + y, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  mylcd.drawXBitmap(160+weather_icon_width/2 + 20, 1*24 + 6 - 5 + y, icon_thermometer_chevron_down, icon_width, icon_height, COLOR_FG);
  mylcd.drawTemperature(data->forecasts[0].tempNight, 160+weather_icon_width/2 + 20 + 6 + icon_width, 1*24 + 6 + y, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  // column 1

  // special handling for temperature to get degree symbol
  mylcd.drawXBitmap(10+x, 3*24 + 16 - 5 + y, icon_cursor_pointer, icon_width, icon_height, COLOR_FG);
  mylcd.drawTemperature(data->current.feels_like, 10 + 6 + icon_width + x, 3*24 + 16 + y, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%0.1f km", data->current.visibility/1000.0f);
  mylcd.drawXBitmapAndText(icon_eye_outline, buffer, 10+x, 4*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%01d %%", data->forecasts[0].clouds);
  mylcd.drawXBitmapAndText(icon_cloud_outline, buffer, 10+x, 5*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%0.1f uvi", data->forecasts[0].uvi);
  mylcd.drawXBitmapAndText(icon_weather_sunny_alert, buffer, 10+x, 6*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%0.1f km/h (%s)", data->forecasts[0].windSpeed * 3.6f, getBearing(data->forecasts[0].windDeg).c_str());
  mylcd.drawXBitmapAndText(icon_weather_windy, buffer, 10+x, 7*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  // column 2

  // special handling for temperature to get degree symbol
  mylcd.drawXBitmap(190+x, 3*24 + 16 - 5 + y, icon_humidity, icon_width, icon_height, COLOR_FG);
  mylcd.drawTemperature(data->forecasts[0].dew_point, 190 + 6 + icon_width + x, 3*24 + 16 + y, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  snprintf(buffer, sizeof(buffer), "%01d %%", data->forecasts[0].humidity);
  mylcd.drawXBitmapAndText(icon_water_remove_outline_customized, buffer, 190+x, 4*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  snprintf(buffer, sizeof(buffer), "%01d hPa", data->forecasts[0].pressure);
  mylcd.drawXBitmapAndText(icon_pressure, buffer, 190+x, 5*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  time_t time;
  struct tm* timeInfo;
  char timeVal[12];

  time = data->current.sunrise;
  time += data->timezone_offset;
  timeInfo = gmtime(&time);
  snprintf(timeVal, sizeof(timeVal), "%02d:%02d:%02d h", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  mylcd.drawXBitmapAndText(icon_sunrise, timeVal, 190+x, 6*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  
  time = data->current.sunset;
  time += data->timezone_offset;
  timeInfo = gmtime(&time);
  snprintf(timeVal, sizeof(timeVal), "%02d:%02d:%02d h", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  mylcd.drawXBitmapAndText(icon_sunset, timeVal, 190+x, 7*24 + 16 + y, icon_width, icon_height, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
}

void WeatherDisplay::drawForecastList(WeatherData *data, int16_t x, int16_t y) {
  drawForecastListItem(data, x, y + 0, 0);
  drawForecastListItem(data, x, y + 50, 1);
  drawForecastListItem(data, x, y + 100, 2);
  drawForecastListItem(data, x, y + 150, 3);
}

void WeatherDisplay::drawForecastListItem(WeatherData *data, int x, int y, int dayIndex) {

  time_t observationTimestamp = data->forecasts[dayIndex].observationTime;
  observationTimestamp += data->timezone_offset;
  struct tm* timeInfo;
  timeInfo = gmtime(&observationTimestamp);

  int16_t iconW = 0;
  iconW += mylcd.drawText(getMeteoconIcon(data->forecasts[dayIndex].icon), x + 5, y + 3, TL_DATUM, COLOR_FG, mylcd.FONT_METEO_NORMAL);

  int16_t width = 0;

  char buffer[18];

  // first line

  width = iconW;
  width += 14;
  width += mylcd.drawText(WDAY_NAMES[timeInfo->tm_wday], x + width, y + 3, TL_DATUM, COLOR_AC, mylcd.FONT_BOLD_NORMAL);
  width += 16;
  snprintf(buffer, sizeof(buffer), "%0.1f/%0.1f", data->forecasts[dayIndex].tempDay, data->forecasts[dayIndex].tempNight);
  width += mylcd.drawText(buffer, x + width, y + 3, TL_DATUM, COLOR_AC, mylcd.FONT_BOLD_NORMAL);

  snprintf(buffer, sizeof(buffer), "%0.1f km/h (%s)", data->forecasts[dayIndex].windSpeed * 3.6f, getBearing(data->forecasts[dayIndex].windDeg).c_str());
  width += mylcd.drawText(buffer, 320, y + 3, TR_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  // second line

  width = iconW;
  width += 14;
  width += mylcd.drawText(data->forecasts[dayIndex].description, x + width, y + 24 + 3, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);

  snprintf(buffer, sizeof(buffer), "%0.1f uvi", data->forecasts[0].uvi);
  width += mylcd.drawText(buffer, 320, y + 24 + 3, TR_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);


  mylcd.drawFastHLine(0, y+49, 320, COLOR_FG);
}

void WeatherDisplay::drawHeader(WeatherData *data, int16_t x, int16_t y, String title) {
  int16_t posX = x;
  posX += mylcd.drawText(title, posX, y+2, TL_DATUM, COLOR_AC, mylcd.FONT_BOLD_NORMAL);
  posX += 12;

  char buffer[28];
  struct tm* timeInfo;
  time_t time = data->current.observationTime;
  time += data->timezone_offset;
  timeInfo = gmtime(&time);
  snprintf(buffer, sizeof(buffer), "(updated %02d:%02d:%02d)",  timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  mylcd.drawText(buffer, posX, y+2, TL_DATUM, COLOR_FG, mylcd.FONT_REGULAR_NORMAL);
  
  uint8_t *bitmap;
  if ( M5.Power.isCharging() ) {
    bitmap = icon_battery_charging;
  } else {
    uint8_t bat_Level = M5.Power.getBatteryLevel();
    if (bat_Level > 75) {
      bitmap = icon_battery_full;
    } else if (bat_Level > 50) {
      bitmap = icon_battery_70;
    } else if (bat_Level > 25) {
      bitmap = icon_battery_50;
    } else {
      bitmap = icon_battery_20;
    }
  }
  mylcd.drawXBitmap(320 - icon_width - 2, y - 1, bitmap, icon_width, icon_height, COLOR_FG);

  mylcd.drawFastHLine(0, y + (mylcd.FONT_REGULAR_NORMAL)->yAdvance + 2, 320, COLOR_FG);
}

const uint16_t* getWeatherImage(String icon) {
 	// clear sky
  // 01d
  if (icon == "01d") 	{
    return weather_01d;
  }
  // 01n
  if (icon == "01n") 	{
    return weather_01n;
  }
  // few clouds
  // 02d
  if (icon == "02d") 	{
    return weather_02d;
  }
  // 02n
  if (icon == "02n") 	{
    return weather_02n;
  }
  // scattered clouds
  // 03d
  if (icon == "03d") 	{
    return weather_03d;
  }
  // 03n
  if (icon == "03n") 	{
    return weather_03n;
  }
  // broken clouds
  // 04d
  if (icon == "04d") 	{
    return weather_04;
  }
  // 04n
  if (icon == "04n") 	{
    return weather_04;
  }
  // shower rain
  // 09d
  if (icon == "09d") 	{
    return weather_09d;
  }
  // 09n
  if (icon == "09n") 	{
    return weather_09n;
  }
  // rain
  // 10d
  if (icon == "10d") 	{
    return weather_10d;
  }
  // 10n
  if (icon == "10n") 	{
    return weather_10n;
  }
  // thunderstorm
  // 11d
  if (icon == "11d") 	{
    return weather_11d;
  }
  // 11n
  if (icon == "11n") 	{
    return weather_11n;
  }
  // snow
  // 13d
  if (icon == "13d") 	{
    return weather_13d;
  }
  // 13n
  if (icon == "13n") 	{
    return weather_13n;
  }
  // mist
  // 50d
  if (icon == "50d") 	{
    return weather_50d;
  }
  // 50n
  if (icon == "50n") 	{
    return weather_50n;
  }
  // Nothing matched: N/A
  return weather_01d;
}

String getBearing(float heading) {
  const char *directions[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"};
  uint8_t index = (heading + 23.0) / 45.0;
  return directions[index];
}

String getMeteoconIcon(String icon) {
 	// clear sky
  // 01d
  if (icon == "01d") 	{
    return "B";
  }
  // 01n
  if (icon == "01n") 	{
    return "C";
  }
  // few clouds
  // 02d
  if (icon == "02d") 	{
    return "H";
  }
  // 02n
  if (icon == "02n") 	{
    return "4";
  }
  // scattered clouds
  // 03d
  if (icon == "03d") 	{
    return "N";
  }
  // 03n
  if (icon == "03n") 	{
    return "5";
  }
  // broken clouds
  // 04d
  if (icon == "04d") 	{
    return "Y";
  }
  // 04n
  if (icon == "04n") 	{
    return "%";
  }
  // shower rain
  // 09d
  if (icon == "09d") 	{
    return "R";
  }
  // 09n
  if (icon == "09n") 	{
    return "8";
  }
  // rain
  // 10d
  if (icon == "10d") 	{
    return "Q";
  }
  // 10n
  if (icon == "10n") 	{
    return "7";
  }
  // thunderstorm
  // 11d
  if (icon == "11d") 	{
    return "P";
  }
  // 11n
  if (icon == "11n") 	{
    return "6";
  }
  // snow
  // 13d
  if (icon == "13d") 	{
    return "W";
  }
  // 13n
  if (icon == "13n") 	{
    return "#";
  }
  // mist
  // 50d
  if (icon == "50d") 	{
    return "M";
  }
  // 50n
  if (icon == "50n") 	{
    return "M";
  }
  // Nothing matched: N/A
  return ")";

}
