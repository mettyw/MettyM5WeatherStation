# MettyM5WeatherStation
This program is a weather station for the M5Stack (tested on M5Stack grey). It queries an online service for the weather at a given location and displays it on the M5Stack LCD using several screens. 

## Features
Compared to other weather station code on the web, this programm attempts to be different in several ways and appempts to work around some of the quirks or limitations of the M5Stack. Most notable features are:
* Displays additional weather data such as day/night temperate and UVI (by using the [OpenWeatherMap one call API](https://openweathermap.org/api/one-call-api))
* flicker free update (by using [LovyanGFX](https://github.com/lovyan03/LovyanGFX) instead of [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI))
* display sleep mode with dimmed screen (using ULP)
* wake from sleep with any button (using ULP)
* can run on battery for several hours (depending on capacity)
* color display including icons
* low speaker noise on M5Stack
* degree "symbol"

## Compatibilty
I tested the program on an M5Stack grey. Testers for other M5 devices are welcome!

## Screenshots
<img src="screenshots/current.jpg"
     alt="Current Screen"
     style="float: left; margin-right: 10px;" width="30%" />
<img src="screenshots/today.jpg"
     alt="Today Screen"
     style="float: left; margin-right: 10px;" width="30%" />
<img src="screenshots/forecast.jpg"
     alt="Forecast Screen"
     style="float: left; margin-right: 10px;" width="30%" />

## Usage
Left and right button cycle the different screens, center button
wakes device from sleep or forces a weather refresh, in case of
already awake. Device goes to sleep after some seconds, when not
used for DISPLAY_WAKE_MILLIS. Weather updated is triggered
automatically every UPDATE_INTERVAL_SECS seconds.

## Configuration
Open the file Settings.h.template, change as needed and store as
Settings.h.

## Contributions
I basically also used this program to learn C. Pull requests to 
improve the code are highly apreciated!

## License
MIT License
