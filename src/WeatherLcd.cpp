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

#include "WeatherLcd.h"


static LGFX lcddriver;

WeatherLcd::WeatherLcd() : LGFX_Sprite(&lcddriver) {

}

void WeatherLcd::init() {
  lcddriver.init();
  _init();
}

void WeatherLcd::init_without_reset(std::uint8_t brightness) {
  // need to manually set the brightness to avoid flicker
  int retry = 3;
  while (!lcddriver.autodetect(false) && --retry);
  lcddriver.getPanel()->brightness = brightness;
  //taken from private function lcddriver.lgfx::LGFX_SPI<lgfx::LGFX_Config>::init_impl(false);
  lcddriver.lgfx::LGFX_SPI<lgfx::LGFX_Config>::initBus(); 
  lcddriver.lgfx::LGFX_SPI<lgfx::LGFX_Config>::initPanel(false); 
  lcddriver.lgfx::LGFX_SPI<lgfx::LGFX_Config>::initTouch(); 

//  lcddriver.init_without_reset();
  _init();
}

void WeatherLcd::_init() {
  setColorDepth(lgfx::rgb332_1Byte);
//  setColorDepth(lgfx::rgb565_2Byte);
  if ( ! createSprite(320, 240) ) {
      Serial.println("FAIL WeatherDisplay::init()");
  }
}

void WeatherLcd::clear() {
  fillSprite(COLOR_BG);
}

void WeatherLcd::show() {
  pushSprite(0, 0);
}

void WeatherLcd::setBrightness(std::uint8_t brightness) {
  lcddriver.setBrightness(brightness);
}


int16_t WeatherLcd::drawText(const String& string, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {
  setTextDatum(datum);
  setFont(font);
  setTextColor(color);

  return drawString(string, posX, posY);  
}

void WeatherLcd::drawWrappedText(const char *string, int32_t posX, int32_t posY, int32_t width, uint16_t color, const lgfx::GFXfont *font) {
  setTextDatum(TL_DATUM);
  setFont(font);
  setTextColor(color);
  int32_t x = posX;
  int32_t y = posY;

  if (string && string[0]) {
    do {
      int32_t tw = textWidth(String(string[0]));
      if ( x + tw > posX + width ) {
        x = posX;
        y += font->yAdvance;
      }
      x += drawChar(*string, x, y);
    } while (*(++string));
  }
//  return drawString(string, posX, posY);  
}

int16_t WeatherLcd::drawTemperature(float temperature, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {
  // https://community.m5stack.com/topic/1698/degrees-c-symbol/11#
  int16_t symbolShiftX;
  int16_t symbolShiftY;
  const lgfx::GFXfont *symbolFont;
  int16_t width;

  // my C++ understanding is really, really limited; originally I wanted to define the fonts with #defines
  // but if I do that, these equality checks do not work at all (at least if this function here is in a class
  // and not in main.cpp; I'd be happy if someone could enlighten me :-)
  if (font == FONT_REGULAR_HUGE) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSansBold9pt7b;
  } else if (font == FONT_REGULAR_LARGE) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else if (font == FONT_REGULAR_NORMAL) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else if (font == FONT_BOLD_NORMAL) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else {
    symbolShiftX = 0;
    symbolShiftY = 0;
    symbolFont = &fonts::FreeSans9pt7b;
    char buff[60];
    sprintf(buff, "ptr: %p %p %p", font, this->FONT_REGULAR_NORMAL, &fonts::FreeSans9pt7b);
    Serial.println(buff);
  }
  
  String temp = String(temperature, 1);
  // TODO investigate alternative https://forum.arduino.cc/index.php?topic=170670.0
  String degree_sign = "o"; // lowercase "o"; what a hack...
  String unit = "C";

  int16_t alignX = 0;
  setFont(font);
  alignX += textWidth(temp);
  alignX += textWidth(unit);
  setFont(symbolFont);
  alignX += textWidth(degree_sign);
  // TODO implement for TR_DATUM
  if ( datum == TC_DATUM) {
    alignX = -1 * alignX / 2;
  } else  if ( datum == TL_DATUM) {
    alignX = 0;
  } else {
    alignX *=  -1;
  }

  // since we did the alignment computation on our own, we can hard-code it to TL_DATUM
  setTextDatum(TL_DATUM);
  posX += alignX;

  setTextColor(color);

  setFont(font);
  width = drawString(temp, posX, posY);
  width += symbolShiftX;
  setFont(symbolFont);
  width += drawString(degree_sign, posX + width, posY + symbolShiftY);
  setFont(font);
  width += drawString(unit, posX + width, posY);
  return width;
}

void WeatherLcd::drawProgressImmediate(int percentage, String label) {
  LGFX_Sprite overlay = LGFX_Sprite(&lcddriver);

  overlay.setColorDepth(8);
  overlay.createSprite(320, 25);
  overlay.fillSprite(COLOR_BG);

  overlay.setTextDatum(TL_DATUM);
  overlay.setFont(FONT_REGULAR_NORMAL);
  overlay.setTextColor(COLOR_FG);

  char buff[8];
  sprintf(buff, " (%02d%%)", percentage);
  overlay.drawString(label+String(buff), 5, 2);

  overlay.drawFastHLine(0, (FONT_REGULAR_NORMAL)->yAdvance + 2, 320, COLOR_FG);

  overlay.pushSprite(0,0);
  overlay.deleteSprite();
}

void WeatherLcd::pushTransImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transColor) {
     bool swap = getSwapBytes();
     setSwapBytes(true);
     pushImage(x, y, w, h, data, transColor);
     setSwapBytes(swap);
}

// TODO does not work with all Datum
int16_t WeatherLcd::drawXBitmapAndText(const uint8_t *bitmap, const String& string, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {
  int16_t  width = 0;
  int16_t  offY = -5;
  drawXBitmap(x, y+offY, bitmap, w, h, COLOR_FG);
  width += w;
  width += 6; // padding
  width += drawText(string, x+width, y, datum, color, font);

  return width;
}
























WeatherLcd2::WeatherLcd2() : LGFX()  {

}

void WeatherLcd2::init() {
  LGFX::init();
  startWrite();
}

void WeatherLcd2::clear() {
  fillScreen(COLOR_BG);
}

void WeatherLcd2::show() {
  //pushSprite(0, 0);
}

int16_t WeatherLcd2::drawText(const String& string, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {

  setTextDatum(datum);
  setFont(font);
  setTextColor(color);

  setColor(COLOR_BG);
  fillRect(posX, posY, 100, font->yAdvance);
  return drawString(string, posX, posY);  
}

int16_t WeatherLcd2::drawTextLeftOrCenteredIfSmall(const String& string, int32_t posX, int32_t posY, int32_t availableWidth, uint16_t color, const lgfx::GFXfont *font) {
  setTextDatum(TL_DATUM);
  setFont(font);
  setTextColor(color);

  int16_t width;
  width = textWidth(string);
  // shift left, if text is small enough to display centered
  if ( width < availableWidth ) {
    posX += (availableWidth - width)/2;
  }

  return drawString(string, posX, posY);  
}

// https://community.m5stack.com/topic/1698/degrees-c-symbol/11#
int16_t WeatherLcd2::drawTemperature(float temperature, int32_t posX, int32_t posY, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {

  int16_t symbolShiftX;
  int16_t symbolShiftY;
  const lgfx::GFXfont *symbolFont;
  int16_t width;

  // my C++ understanding is really, really limited; originally I wanted to define the fonts with #defines
  // but if I do that, these equality checks do not work at all (at least if this function here is in a class
  // and not in main.cpp; I'd be happy if someone could enlighten me :-)
  if (font == FONT_REGULAR_HUGE) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSansBold9pt7b;
  } else if (font == FONT_REGULAR_LARGE) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else if (font == FONT_REGULAR_NORMAL) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else if (font == FONT_BOLD_NORMAL) {
    symbolShiftX = 2;
    symbolShiftY = -3;
    symbolFont = &fonts::FreeSans9pt7b;
  } else {
    symbolShiftX = 0;
    symbolShiftY = 0;
    symbolFont = &fonts::FreeSans9pt7b;
    char buff[60];
    sprintf(buff, "ptr: %p %p %p", font, this->FONT_REGULAR_NORMAL, &fonts::FreeSans9pt7b);
    Serial.println(buff);
  }
  
  String temp = String(temperature, 1);
  // TODO investigate alternative https://forum.arduino.cc/index.php?topic=170670.0
  String degree_sign = "o"; // lowercase "o"; what a hack...
  String unit = "C";

  int16_t alignX = 0;
  setFont(font);
  alignX += textWidth(temp);
  alignX += textWidth(unit);
  setFont(symbolFont);
  alignX += textWidth(degree_sign);
  // TODO implement for TR_DATUM
  if ( datum == TC_DATUM) {
    alignX = -1 * alignX / 2;
  } else  if ( datum == TL_DATUM) {
    alignX = 0;
  } else {
    alignX *=  -1;
  }

  // since we did the alignment computation on our own, we can hard-code it to TL_DATUM
  setTextDatum(TL_DATUM);
  posX += alignX;

  setTextColor(color);

  setColor(COLOR_BG);
  fillRect(posX, posY, 100, font->yAdvance);

  //displayText(temp, posX, posY, TL_DATUM, COLOR_FG, font);
  setFont(font);
  width = drawString(temp, posX, posY);
  width += symbolShiftX;
  //width = display.textWidth(temp) + shift_x;
  //displayText("o", posX + width, posY + shift_y, TL_DATUM, COLOR_FG, &symbolFont);
  setFont(symbolFont);
  width += drawString(degree_sign, posX + width, posY + symbolShiftY);
  //width += display.textWidth("o");
  //displayText("C", posX + width, posY, TL_DATUM, COLOR_FG, font);
  setFont(font);
  width += drawString(unit, posX + width, posY);
  return width;
}

void WeatherLcd2::drawProgressImmediate(LGFX *parent, int percentage, String label) {
  LGFX_Sprite overlay = LGFX_Sprite(this);

  overlay.setColorDepth(8);
  overlay.createSprite(320, 25);
  overlay.fillSprite(COLOR_BG);

  overlay.setTextDatum(TL_DATUM);
  overlay.setFont(FONT_REGULAR_NORMAL);
  overlay.setTextColor(COLOR_FG);

  char buff[8];
  sprintf(buff, " (%02d%%)", percentage);
  overlay.drawString(label+String(buff), 5, 2);

  overlay.drawFastHLine(0, (FONT_REGULAR_NORMAL)->yAdvance + 2, 320, COLOR_FG);

  overlay.pushSprite(0,0);
  overlay.deleteSprite();
}

void WeatherLcd2::pushTransImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, uint16_t transColor) {

    setColor(COLOR_BG);
    fillRect(x, y, w, h);

     bool swap = getSwapBytes();
     setSwapBytes(true);
     pushImage(x, y, w, h, data, transColor);
     setSwapBytes(swap);

}

// TODO does not work with all Datum
int16_t WeatherLcd2::drawXBitmapAndText(const uint8_t *bitmap, const String& string, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t datum, uint16_t color, const lgfx::GFXfont *font) {
  setColor(COLOR_BG);
  fillRect(x, y, w+6, h);

  int16_t  width = 0;
  int16_t  offY = -5;
  drawXBitmap(x, y+offY, bitmap, w, h, COLOR_FG);
  width += w;
  width += 6; // padding
  width += drawText(string, x+width, y, datum, color, font);

  return width;
}


