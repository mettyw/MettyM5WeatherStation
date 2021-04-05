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

#ifndef WEATHER_LCD_H
#define WEATHER_LCD_H

#include "Settings.h"  
#include "WeatherFonts.h"

#include <HTTPClient.h>

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>


// workaround to acheive some kind of framebuffer - see https://forum.m5stack.com/topic/1480/double-buffer-in-m5stickc/2
class WeatherLcd: public LGFX_Sprite {
      private:
        void _init();
      public:
        // font definitions - changing those is not recommended, as some code depends in the pixel dimensions of deach font here
        const lgfx::GFXfont *FONT_REGULAR_NORMAL =   &fonts::FreeSans9pt7b;
        const lgfx::GFXfont *FONT_BOLD_NORMAL =      &fonts::FreeSansBold9pt7b;
        const lgfx::GFXfont *FONT_REGULAR_LARGE =    &fonts::FreeSans12pt7b;
        const lgfx::GFXfont *FONT_REGULAR_HUGE =     &fonts::FreeSans18pt7b;
        const lgfx::GFXfont *FONT_METEO_NORMAL =     &Meteocons_Regular_36;
        const lgfx::GFXfont *FONT_METEO_HUGE  =      &Meteocons_Regular_72;

        WeatherLcd();
        void init();
        void init_without_reset(std::uint8_t brightness);
        void clear();
        void show();
        void setBrightness(std::uint8_t brightness);
        int16_t drawText(const String& string, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
        void drawWrappedText(const char *string, int32_t posX, int32_t posY, int32_t width, uint16_t color, const lgfx::GFXfont *font);

        int16_t drawTemperature(float temperature, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
        void drawProgressImmediate(int percentage, String label);
        void pushTransImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transColor);

        int16_t drawXBitmapAndText(const uint8_t *bitmap, const String& string, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
};

class WeatherLcd2: public LGFX  {
      private:

      public:
        const lgfx::GFXfont *FONT_REGULAR_NORMAL =   &fonts::FreeSans9pt7b;
        const lgfx::GFXfont *FONT_BOLD_NORMAL =      &fonts::FreeSansBold9pt7b;
        const lgfx::GFXfont *FONT_REGULAR_LARGE =    &fonts::FreeSans12pt7b;
        const lgfx::GFXfont *FONT_REGULAR_HUGE =     &fonts::FreeSans18pt7b;
        const lgfx::GFXfont *FONT_METEO_NORMAL =     &Meteocons_Regular_36;
        const lgfx::GFXfont *FONT_METEO_HUGE  =      &Meteocons_Regular_72;

        WeatherLcd2();
        void init();
        void clear();
        void show();
        int16_t drawText(const String& string, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
        int16_t drawTextLeftOrCenteredIfSmall(const String& string, int32_t posX, int32_t posY, int32_t availableWidth, uint16_t color, const lgfx::GFXfont *font);
        int16_t drawTemperature(float temperature, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
//        void drawPng(int32_t posX, int32_t posY);
        void drawProgressImmediate(LGFX *parent, int percentage, String label);
        void pushTransImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transColor);

        int16_t drawXBitmapAndText(const uint8_t *bitmap, const String& string, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t datum, uint16_t color, const lgfx::GFXfont *font);
  
};
#endif