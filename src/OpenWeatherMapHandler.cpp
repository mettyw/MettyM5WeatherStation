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

#include "OpenWeatherMapHandler.h"

#include "WeatherDisplay.h"

// time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

void updateData(WeatherData *data, WeatherDisplay *display, bool force) {
  time_t now;
  if (!force) {
    time(&now);
    // check if it is time for an update
    if (now - data->timeSinceLastWUpdate < (1000L*UPDATE_INTERVAL_SECS)) {
      return;
    }
  }

  display->showProgressOverlay(0, "Updating...");
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < WIFI_CONNECTION_ATTEMPTS) {
    delay(500);
    Serial.print(".");
    switch (counter % 4) {
      case 0:
        display->showProgressOverlay(10, "Connecting wifi | ");
        break;
      case 1:
        display->showProgressOverlay(10, "Connecting wifi / ");
        break;
      case 2:
        display->showProgressOverlay(10, "Connecting wifi - ");
        break;
      case 3:
        display->showProgressOverlay(10, "Connecting wifi \\ ");
        break;
      default:
        display->showProgressOverlay(10, "Connecting wifi - ");
        break;
    }
    counter++;
  }
  if ( WiFi.status() != WL_CONNECTED ) {
    String msg = "Error connecting to WIFI";
    Serial.println(msg);

    display->showError(msg);
    delay(30000);
    esp_restart();
  }

  display->showProgressOverlay(30, "Updating time...");
  // Get time from network time service
  configTime(LOCATION_TZ, LOCATION_DST_MINS, "pool.ntp.org");

  display->showProgressOverlay(60, "Updating weather...");

  OpenWeatherMapOneCallClient client;
  client.updateDataById(data, OPEN_WEATHER_MAP_APP_ID, LOCATION_LAT, LOCATION_LON, MAX_FORECASTS);
  if ( client.isStatusError() ) {
    String msg = client.getStatusMessage();
    Serial.println(msg);
    display->showError( msg );
    delay(30000);
    esp_restart();
  }

  display->showProgressOverlay(100, "Update finished.");
  delay(1000);
Serial.println(1);
  char buff[15];
  sprintf_P(buff, PSTR("FREE %02d"), ESP.getFreeHeap());
Serial.println(buff);


  // disconnect from network
// FIXME disabled due to error and reboot  WiFi.disconnect(true);
Serial.println(2);
//  WiFi.mode(WIFI_OFF);
  // disable wifi to save energy
 // WiFi.enableSTA(false);
Serial.println(3);

  time(&now);
Serial.println(4);
  data->timeSinceLastWUpdate = now;
Serial.println(5);
}

OpenWeatherMapOneCallClient::OpenWeatherMapOneCallClient() {

}

void OpenWeatherMapOneCallClient::updateDataById(WeatherData *data, String appId, String lat, String lon, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;

  String units = "metric";
  // chaning makes no much sence, since the font is only 7-bit, which means that no special characters such as umlauts are supported
  String language = "en";
  // https://openweathermap.org/api/one-call-api
  String path = "/data/2.5/onecall?exclude=hourly,minutely&lat=" + lat + "&lon=" + lon  + "&appid=" + appId + "&units=" + units + "&lang=" + language;
  doUpdate(data, path);
}

void OpenWeatherMapOneCallClient::doUpdate(WeatherData *data, String path) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->weatherItemCounter = 0;
  this->currentKey = "";
  this->currentParent = "";
  this->currentParentParent = "";
  this->isDailyArray = false;
  this->dailyItemCounter = 0;
  this->statusCode = 200; // pre-init with success status, since the "cod" field might not be part of the JSON
  this->statusMessage = "";
  this->data = data;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("[HTTP] Requesting resource at http://%s:%u%s\n", host.c_str(), port, path.c_str());

  WiFiClient client;
  if(client.connect(host.c_str(), port)) {
    bool isBody = false;
    char c;
    client.print("GET " + path + " HTTP/1.1\r\n"
                 "Host: " + host + "\r\n"
                 "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
      if (client.available()) {
        if ((millis() - lost_do) > lostTest) {
          this->statusCode = 408; // we made this one up...
          this->statusMessage = "[HTTP] lost in client with a timeout";
          client.stop();
          //esp_restart();
          return;
        }
        c = client.read();
        if (c == '{' || c == '[') {
          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
      }
      // give WiFi and TCP/IP libraries a chance to handle pending events
      yield();
    }
    client.stop();
  } else {
    this->statusCode = 999; // we made this one up...
    this->statusMessage = "[HTTP] failed to connect to host";
  }
  this->data = nullptr;
}

boolean OpenWeatherMapOneCallClient::isStatusError() {
  return this->statusCode != 200;
}

String OpenWeatherMapOneCallClient::getStatusMessage() {
  return this->statusMessage;
}

void OpenWeatherMapOneCallClient::whitespace(char c) {
}

void OpenWeatherMapOneCallClient::key(String key) {
  currentKey = String(key);
}

void OpenWeatherMapOneCallClient::value(String value) {
  if (currentKey == "cod") {
    this->statusCode = value.toInt();
  }
  if (currentKey == "message") {
    this->statusMessage = value;
  }
  if (currentKey == "timezone_offset") {
    data->timezone_offset = value.toInt();
  }
  if ( currentParent == "current") {
    Serial.println("[current] " +currentParentParent + ", " + currentParent + ", " + currentKey + ", " + value);
    if (currentKey == "dt") {
      data->current.observationTime = value.toInt();
    }
    if (currentKey == "sunrise") {
      data->current.sunrise = value.toInt();
    }
    if (currentKey == "sunset") {
      data->current.sunset = value.toInt();
    }
    if (currentKey == "temp") {
      data->current.temp = value.toFloat();
    }
    if (currentKey == "feels_like") {
      data->current.feels_like = value.toFloat();
    }
    if (currentKey == "pressure") {
      data->current.pressure = value.toInt();
    }
    if (currentKey == "humidity") {
      data->current.humidity = value.toInt();
    }
    if (currentKey == "dew_point") {
      data->current.dew_point = value.toFloat();
    }
    if (currentKey == "uvi") {
      data->current.uvi = value.toFloat();
    }
    if (currentKey == "clouds") {
      data->current.clouds = value.toInt();
    }
    if (currentKey == "visibility") {
      data->current.visibility = value.toInt();
    }
    if (currentKey == "wind_speed") {
      data->current.windSpeed = value.toFloat();
    }
    if (currentKey == "wind_deg") {
      data->current.windDeg = value.toFloat();
    }
  }
  // weatherItemCounter: only get the first item if more than one is available
  if ( currentParentParent == "current" && weatherItemCounter == 0) {
      if (currentKey == "description") {
        // partially inspired by https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
        strlcpy(data->current.description, value.c_str(), sizeof(data->current.description));
      }
      if (currentKey == "icon") {
        strlcpy(data->current.icon, value.c_str(), sizeof(data->current.icon));
      }
  }
  if ( currentParent == "daily") {
    if (dailyItemCounter >= maxForecasts) {
      return;
    }    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d %s %s", dailyItemCounter, currentKey.c_str(), value.c_str());
    Serial.println(buffer);
    if (currentKey == "dt") {
      data->forecasts[dailyItemCounter].observationTime = value.toInt();
    }
    if (currentKey == "sunrise") {
      // not implemented
    }
    if (currentKey == "sunset") {
      // not implemented
    }
    if (currentKey == "pressure") {
      data->forecasts[dailyItemCounter].pressure = value.toFloat();
    }
    if (currentKey == "humidity") {
      data->forecasts[dailyItemCounter].humidity = value.toInt();
    }
    if (currentKey == "dew_point") {
      data->forecasts[dailyItemCounter].dew_point = value.toFloat();
    }
    if (currentKey == "wind_speed") {
      data->forecasts[dailyItemCounter].windSpeed = value.toFloat();
    }
    if (currentKey == "wind_deg") {
      data->forecasts[dailyItemCounter].windDeg = value.toFloat();
    }
    if (currentKey == "clouds") {
      data->forecasts[dailyItemCounter].clouds = value.toInt();
    }
    if (currentKey == "pop") {
      // not implemented
    }
    if (currentKey == "uvi") {
      data->forecasts[dailyItemCounter].uvi = value.toFloat();
    }
  }
  if ( currentParentParent == "daily" ) {
    if (currentParent == "temp") {
      if (currentKey == "day") {
        data->forecasts[dailyItemCounter].tempDay = value.toFloat();
      }
      if (currentKey == "night") {
        data->forecasts[dailyItemCounter].tempNight = value.toFloat();
      }
    }
    if ( currentParent == "weather" && weatherItemCounter == 0 ) {
      if (currentKey == "description") {
        strlcpy(data->forecasts[dailyItemCounter].description, value.c_str(), sizeof(data->forecasts[dailyItemCounter].description));
      }
      if (currentKey == "icon") {
        strlcpy(data->forecasts[dailyItemCounter].icon, value.c_str(), sizeof(data->forecasts[dailyItemCounter].icon));
        //strlcpy(data->forecasts[dailyItemCounter].iconMeteoCon, getMeteoconIcon(value).c_str(), sizeof(data->forecasts[dailyItemCounter].iconMeteoCon));
      }
    }
  }
}

void OpenWeatherMapOneCallClient::startObject() {
  //Serial.println("startObject " +currentParentParent + ", " + currentParent + ", " + currentKey);
  // if currentKey is empty we are in an array
  if ( currentKey == "" && isDailyArray ) {
    //Serial.println("next daily array ");
    dailyItemCounter++;
  }
  else {
    currentParentParent = currentParent;
    currentParent = currentKey;
  }
}

void OpenWeatherMapOneCallClient::endObject() {
  //Serial.println("endObject " +currentParentParent + ", " + currentParent + ", " + currentKey);
  currentKey = "";
  if ( currentParent == "daily" && isDailyArray ) {
    //Serial.println("finish daily array ");
  } else {
    currentParent = currentParentParent;
    currentParentParent = "";
  }
  if (currentParent == "weather") {
    weatherItemCounter++;
  } else {
    weatherItemCounter = 0;
  }
}

void OpenWeatherMapOneCallClient::startArray() {
  //Serial.println("startArray " +currentParentParent + ", " + currentParent + ", " + currentKey);
  if ( currentKey == "daily") {
    //Serial.println("startArray daily");
    isDailyArray = true;
    dailyItemCounter = 0;
  }
}

void OpenWeatherMapOneCallClient::endArray() {
  Serial.println("endArray " +currentParentParent + ", " + currentParent + ", " + currentKey);
  if ( currentKey == "daily") {
    isDailyArray = false;
    dailyItemCounter = 0;
    //Serial.println("endArray daily");
  }

}

void OpenWeatherMapOneCallClient::startDocument() {
}


void OpenWeatherMapOneCallClient::endDocument() {

}

    
