/**The MIT License (MIT)
Copyright (c) 2017 by Daniel Eichhorn
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
See more at https://blog.squix.org
*/


/*****************************
 * Important: see settings.h to configure your settings!!!
 * ***************************/
#include "settings.h"

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include "WeatherStationFonts.h"




/***
 * Install the following libraries through Arduino Library Manager
 * - Mini Grafx by Daniel Eichhorn
 * - ESP8266 WeatherStation by Daniel Eichhorn
 * - Json Streaming Parser by Daniel Eichhorn
 * - simpleDSTadjust by neptune2
 ***/

#include <JsonListener.h>
#include <WundergroundConditions.h>
//#include <WundergroundForecast.h>
#include <WundergroundAstronomy.h>
#include <WundergroundHourly.h>
#include <MiniGrafx.h>
#include <EPD_WaveShare.h>


#include "ArialRounded.h"
#include <MiniGrafxFonts.h>
#include "moonphases.h"
#include "weathericons.h"




#define MINI_BLACK 0
#define MINI_WHITE 1


#define MAX_FORECASTS 12

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      };

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 296
#define BITS_PER_PIXEL 1


EPD_WaveShare epd(EPD2_9, CS, RST, DC, BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

WGConditions conditions;
//WGForecast forecasts[MAX_FORECASTS];
WGAstronomy astronomy;
WGHourly hourlies[24];

// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawButtons();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawCurrentWeatherDetail();
void drawLabelValue(uint8_t line, String label, String value);
void drawBattery();
String getMeteoconIcon(String iconText);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1();
void drawForecast2();

long lastDownloadUpdate = millis();

String moonAgeImage = "";
uint16_t screen = 0;
long timerPress;
bool canBtnPress;

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  //Manual Wifi
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (i>80) i=0;
    drawProgress(i,"Connecting to WiFi");
    i+=10;
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);


  gfx.init();
  gfx.setRotation(1);
  connectWifi();

  updateData();
  gfx.fillBuffer(MINI_WHITE);

  drawTime();
  drawBattery();
  drawCurrentWeather();
  drawForecast1();
  drawAstronomy();
  drawButtons();

  gfx.commit();
  ESP.deepSleep(20 * 60 * 1000000);
}


void loop() {

  
}

// Update the internet based information and update screen
void updateData() {


  //gfx.fillBuffer(MINI_WHITE);
  gfx.setColor(MINI_BLACK);
  gfx.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
  gfx.setColor(MINI_WHITE);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 12, "Refreshing data...");
  gfx.commit();
  
  //gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);

  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);

  WundergroundConditions *conditionsClient = new WundergroundConditions(IS_METRIC);
  conditionsClient->updateConditions(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete conditionsClient;
  conditionsClient = nullptr;

  WundergroundHourly *hourlyClient = new WundergroundHourly(IS_METRIC, !IS_STYLE_12HR);
  hourlyClient->updateHourly(hourlies, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete hourlyClient;
  hourlyClient = nullptr;
  /*WundergroundForecast *forecastClient = new WundergroundForecast(IS_METRIC);
  forecastClient->updateForecast(forecasts, MAX_FORECASTS, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete forecastClient;
  forecastClient = nullptr;*/

  WundergroundAstronomy *astronomyClient = new WundergroundAstronomy(IS_STYLE_12HR);
  astronomyClient->updateAstronomy(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete astronomyClient;
  astronomyClient = nullptr;
  moonAgeImage = String((char) (65 + 26 * (((15 + astronomy.moonAge.toInt()) % 30) / 30.0)));

}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  /*gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(23, 30, SquixLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 80, "https://blog.squix.org");
  gfx.setColor(MINI_WHITE);

  gfx.drawString(120, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 168, 240 - 20, 15);
  gfx.setColor(MINI_WHITE);
  gfx.fillRect(12, 170, 216 * percentage / 100, 11);

  gfx.commit();*/
}

// draws the clock
void drawTime() {

  char *dstAbbrev;
  char time_str[30];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime (&now);

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setColor(MINI_BLACK);
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900 + timeinfo->tm_year);
  //gfx.drawString(2, 108, date);


  if (IS_STYLE_12HR) {
    int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d", hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(2, -2, "Updated: " + String(time_str));
  } else {
    sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(2, -2, "Updated: " + String(time_str));
  }
  gfx.drawLine(0, 11, SCREEN_WIDTH, 11);

}

// draws current weather information
void drawCurrentWeather() {
  gfx.setColor(MINI_BLACK);
  //gfx.drawXbm(1, 16, 50, 50, RoundBox_bits);
  //gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setFont(Meteocons_Plain_42);
  String weatherIcon = getMeteoconIcon(conditions.weatherIcon);
  gfx.drawString(5, 20, weatherIcon);
  // Weather Text

  gfx.setColor(MINI_BLACK);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(55, 15, DISPLAYED_CITY_NAME);

  gfx.setFont(ArialMT_Plain_24);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }

  String temp = conditions.currentTemp + degreeSign;
      
  gfx.drawString(55, 25, temp);

  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(55, 50, conditions.weatherText);
  gfx.drawLine(0, 65, SCREEN_WIDTH, 65);
  

}

void drawForecast1() {
  drawForecastDetail(SCREEN_WIDTH / 2 - 20, 15, 3);
  drawForecastDetail(SCREEN_WIDTH / 2 + 22, 15, 6);
  drawForecastDetail(SCREEN_WIDTH / 2 + 64, 15, 9);
  drawForecastDetail(SCREEN_WIDTH / 2 + 106, 15, 12);
}

void drawForecast2() {
  /*drawForecastDetail(x + 10, y + 165, 6);
  drawForecastDetail(x + 95, y + 165, 8);
  drawForecastDetail(x + 180, y + 165, 10);*/
}


// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t index) {


  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  String hour = hourlies[index].hour;
  hour.toUpperCase();
  gfx.drawString(x + 25, y - 2, hour);

  gfx.setColor(MINI_BLACK);
  gfx.drawString(x + 25, y + 12, hourlies[index].temp + "° " + hourlies[index].PoP + "%");
  //gfx.drawString(x + 25, y + 24, hourlies[index].PoP + "%");

  
  gfx.setFont(Meteocons_Plain_21);
  String weatherIcon = getMeteoconIcon(hourlies[index].icon);
  gfx.drawString(x + 25, y + 24, weatherIcon);
  gfx.drawLine(x + 2, 12, x + 2, 65);
  gfx.drawLine(x + 2, 25, x + 43, 25);


}

// draw moonphase and sunrise/set and moonrise/set
void drawAstronomy() {

  gfx.setFont(MoonPhases_Regular_36);
  gfx.setColor(MINI_BLACK);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(5, 72, moonAgeImage);

  gfx.setFont(ArialMT_Plain_10);
  gfx.drawString(55, 72, "Sun:");
  gfx.drawString(85, 72,  astronomy.sunriseTime + " - " + astronomy.sunsetTime);
  gfx.drawString(55, 84, "Moon:");
  gfx.drawString(85, 84, astronomy.moonriseTime + " - " + astronomy.moonsetTime);
  gfx.drawString(55, 96, astronomy.moonPhase);
}

void drawCurrentWeatherDetail() {
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 2, "Current Conditions");

  //gfx.setTransparentColor(MINI_BLACK);
  //gfx.drawPalettedBitmapFromPgm(0, 20, getMeteoconIconFromProgmem(conditions.weatherIcon));

  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }
  // String weatherIcon;
  // String weatherText;
  drawLabelValue(0, "Temperature:", conditions.currentTemp + degreeSign);
  drawLabelValue(1, "Feels Like:", conditions.feelslike + degreeSign);
  drawLabelValue(2, "Dew Point:", conditions.dewPoint + degreeSign);
  drawLabelValue(3, "Wind Speed:", conditions.windSpeed);
  drawLabelValue(4, "Wind Dir:", conditions.windDir);
  drawLabelValue(5, "Humidity:", conditions.humidity);
  drawLabelValue(6, "Pressure:", conditions.pressure);
  drawLabelValue(7, "Precipitation:", conditions.precipitationToday);
  drawLabelValue(8, "UV:", conditions.UV);

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(15, 185, "Description: ");
  gfx.setColor(MINI_WHITE);
  //gfx.drawStringMaxWidth(15, 200, 240 - 2 * 15, forecasts[0].forecastText);
}

void drawLabelValue(uint8_t line, String label, String value) {
  const uint8_t labelX = 15;
  const uint8_t valueX = 150;
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(labelX, 30 + line * 15, label);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(valueX, 30 + line * 15, value);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

void drawBattery() {
   uint8_t percentage = 100;
   float power = analogRead(A0) * 49 / 10240.0;
   if (power > 4.15) percentage = 100;
   else if (power < 3.7) percentage = 0;
   else percentage = (power - 3.7) * 100 / (4.15-3.7);
   
   gfx.setColor(MINI_BLACK);
   gfx.setFont(ArialMT_Plain_10);
   gfx.setTextAlignment(TEXT_ALIGN_RIGHT);  
   gfx.drawString(SCREEN_WIDTH - 22, -1, String(power, 2) + "V " + String(percentage) + "%");
   gfx.drawRect(SCREEN_WIDTH - 22, 0, 19, 10);
   gfx.fillRect(SCREEN_WIDTH - 2, 2, 2, 6);   
   gfx.fillRect(SCREEN_WIDTH - 20, 2, 16 * percentage / 100, 6);
}

void drawWifiQuality() {
  int8_t quality = getWifiQuality();
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);  
  gfx.drawString(228, 9, String(quality) + "%");
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        gfx.setPixel(230 + 2 * i, 18 - j);
      }
    }
  }
}


void drawButtons() {
  gfx.setColor(MINI_BLACK);

  uint16_t third = SCREEN_WIDTH / 3;
  gfx.setColor(MINI_BLACK);
  //gfx.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
  gfx.drawLine(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12);
  gfx.drawLine(2 * third, SCREEN_HEIGHT - 12, 2 * third, SCREEN_HEIGHT);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER); 
  gfx.setFont(ArialMT_Plain_10);
  gfx.drawString(2.5 *third, SCREEN_HEIGHT - 12, "REFRESH");
}

String getMeteoconIcon(String iconText) {
  if (iconText == "chanceflurries") return "F";
  if (iconText == "chancerain") return "Q";
  if (iconText == "chancesleet") return "W";
  if (iconText == "chancesnow") return "V";
  if (iconText == "chancetstorms") return "S";
  if (iconText == "clear") return "B";
  if (iconText == "cloudy") return "Y";
  if (iconText == "flurries") return "F";
  if (iconText == "fog") return "M";
  if (iconText == "hazy") return "E";
  if (iconText == "mostlycloudy") return "Y";
  if (iconText == "mostlysunny") return "H";
  if (iconText == "partlycloudy") return "H";
  if (iconText == "partlysunny") return "J";
  if (iconText == "sleet") return "W";
  if (iconText == "rain") return "R";
  if (iconText == "snow") return "W";
  if (iconText == "sunny") return "B";
  if (iconText == "tstorms") return "0";

  if (iconText == "nt_chanceflurries") return "F";
  if (iconText == "nt_chancerain") return "7";
  if (iconText == "nt_chancesleet") return "#";
  if (iconText == "nt_chancesnow") return "#";
  if (iconText == "nt_chancetstorms") return "&";
  if (iconText == "nt_clear") return "2";
  if (iconText == "nt_cloudy") return "Y";
  if (iconText == "nt_flurries") return "9";
  if (iconText == "nt_fog") return "M";
  if (iconText == "nt_hazy") return "E";
  if (iconText == "nt_mostlycloudy") return "5";
  if (iconText == "nt_mostlysunny") return "3";
  if (iconText == "nt_partlycloudy") return "4";
  if (iconText == "nt_partlysunny") return "4";
  if (iconText == "nt_sleet") return "9";
  if (iconText == "nt_rain") return "7";
  if (iconText == "nt_snow") return "#";
  if (iconText == "nt_sunny") return "4";
  if (iconText == "nt_tstorms") return "&";

  return ")";
}


