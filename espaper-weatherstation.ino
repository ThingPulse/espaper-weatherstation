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
  Copyright (c) 2017 by Daniel Eichhorn
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
*/


/*****************************
   Important: see settings.h to configure your settings!!!
 * ***************************/
#include "settings.h"

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include "WeatherStationFonts.h"




/***
   Install the following libraries through Arduino Library Manager
   - Mini Grafx by Daniel Eichhorn
   - ESP8266 WeatherStation by Daniel Eichhorn
   - Json Streaming Parser by Daniel Eichhorn
   - simpleDSTadjust by neptune2
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
#include "configportal.h"



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
void drawForecast();


long lastDownloadUpdate = millis();

String moonAgeImage = "";
uint16_t screen = 0;
long timerPress;
bool canBtnPress;

boolean connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  //Manual Wifi
  Serial.print("[");
  Serial.print(WIFI_SSID.c_str());
  Serial.print("]");
  Serial.print("[");
  Serial.print(WIFI_PASS.c_str());
  Serial.print("]");
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not connect to WiFi");
      return false;
    }
    Serial.print(".");
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(USR_BTN, INPUT_PULLUP);
  int btnState = digitalRead(USR_BTN);

  gfx.init();
  gfx.setRotation(1);

  // load config if it exists. Otherwise use defaults.
  boolean mounted = SPIFFS.begin();
  if (!mounted) {
    SPIFFS.format();
    SPIFFS.begin();
  }
  loadConfig();

  Serial.println("State: " + String(btnState));
  if (btnState == LOW) {
    boolean connected = connectWifi();
    startConfigPortal(&gfx);
  } else {
    boolean connected = connectWifi();
    if (connected) {
      updateData();
      gfx.fillBuffer(MINI_WHITE);

      drawTime();
      drawBattery();
      drawCurrentWeather();
      drawForecast();
      drawAstronomy();
      drawButtons();
      gfx.commit();
    } else {
      gfx.fillBuffer(MINI_WHITE);
      gfx.setColor(MINI_BLACK);
      gfx.setTextAlignment(TEXT_ALIGN_CENTER);
      gfx.drawString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, "Could not connect to WiFi\nPress LEFT + RIGHT button\nto enter config mode");
      gfx.commit();
    }

    ESP.deepSleep(UPDATE_INTERVAL_SECS * 1000000);
  }
}


void loop() {


}

// Update the internet based information and update screen
void updateData() {
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);

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



  bool cityIsPws = strncmp(WUNDERGROUND_CITY.c_str(), "pws:", 4) == 0;
  String cityWithoutPws;
  if(cityIsPws) {
    cityWithoutPws = String(&WUNDERGROUND_CITY.c_str()[4]);
  }
  
  WundergroundConditions *conditionsClient = new WundergroundConditions(IS_METRIC);
  if(cityIsPws) {
    conditionsClient->updateConditionsPWS(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, cityWithoutPws);
  } else {
    conditionsClient->updateConditions(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  }
  delete conditionsClient;
  conditionsClient = nullptr;

  WundergroundHourly *hourlyClient = new WundergroundHourly(IS_METRIC, !IS_STYLE_12HR);
  if(cityIsPws) {
    hourlyClient->updateHourlyPWS(hourlies, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, cityWithoutPws);
  } else {
    hourlyClient->updateHourly(hourlies, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  }
  
  delete hourlyClient;
  hourlyClient = nullptr;

  WundergroundAstronomy *astronomyClient = new WundergroundAstronomy(IS_STYLE_12HR);
  if(cityIsPws) {
    astronomyClient->updateAstronomyPWS(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, cityWithoutPws);
  } else {
    astronomyClient->updateAstronomy(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  }
  delete astronomyClient;
  astronomyClient = nullptr;
  moonAgeImage = String((char) (65 + 26 * (((astronomy.moonAge.toInt()) % 30) / 30.0)));

  // Wait max. 3 seconds to make sure the time has been sync'ed
  Serial.println("\nWaiting for time");
  unsigned timeout = 3000;
  unsigned start = millis();
  while (millis() - start < timeout) {
    time_t now = time(nullptr);
    if (now > (2016 - 1970) * 365 * 24 * 3600) {
      return;
    }
    Serial.println(".");
    delay(100);
  }

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
  date = date.substring(0, 11) + String(1900 + timeinfo->tm_year);

  if (IS_STYLE_12HR) {
    int hour = (timeinfo->tm_hour + 11) % 12 + 1; // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d", hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(2, -2, String(FPSTR(TEXT_UPDATED)) + String(time_str));
  } else {
    sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    gfx.drawString(2, -2, String(FPSTR(TEXT_UPDATED)) + String(time_str));
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

unsigned int hourAddWrap(unsigned int hour, unsigned int add) {
  hour += add;
  if(hour > 23) hour -= 24;

  return hour;
}

void drawForecast() {
  time_t now = dstAdjusted.time(nullptr);
  struct tm * timeinfo = localtime (&now);
  
  unsigned int curHour = timeinfo->tm_hour;
  if(timeinfo->tm_min > 29) curHour = hourAddWrap(curHour, 1);

  drawForecastDetail(SCREEN_WIDTH / 2 - 20, 15, hourAddWrap(curHour, 3));
  drawForecastDetail(SCREEN_WIDTH / 2 + 22, 15, hourAddWrap(curHour, 6));
  drawForecastDetail(SCREEN_WIDTH / 2 + 64, 15, hourAddWrap(curHour, 9));
  drawForecastDetail(SCREEN_WIDTH / 2 + 106, 15, hourAddWrap(curHour, 12));
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
  gfx.drawString(55, 72, FPSTR(TEXT_SUN));
  gfx.drawString(95, 72,  astronomy.sunriseTime + " - " + astronomy.sunsetTime);
  gfx.drawString(55, 84, FPSTR(TEXT_MOON));
  gfx.drawString(95, 84, astronomy.moonriseTime + " - " + astronomy.moonsetTime);
  gfx.drawString(55, 96, FPSTR(TEXT_PHASE));
  gfx.drawString(95, 96, astronomy.moonPhase);
}

void drawCurrentWeatherDetail() {
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 2, "Current Conditions");

  String degreeSign = "°F";
  if (IS_METRIC) {
    degreeSign = "°C";
  }

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



void drawBattery() {
  uint8_t percentage = 100;
  float adcVoltage = analogRead(A0) / 1024.0;
  // This values where empirically collected
  float batteryVoltage = adcVoltage * 4.945945946 -0.3957657658;
  if (batteryVoltage > 4.2) percentage = 100;
  else if (batteryVoltage < 3.3) percentage = 0;
  else percentage = (batteryVoltage - 3.3) * 100 / (4.2 - 3.3);

  gfx.setColor(MINI_BLACK);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(SCREEN_WIDTH - 22, -1, String(batteryVoltage, 2) + "V " + String(percentage) + "%");
  gfx.drawRect(SCREEN_WIDTH - 22, 0, 19, 10);
  gfx.fillRect(SCREEN_WIDTH - 2, 2, 2, 6);
  gfx.fillRect(SCREEN_WIDTH - 20, 2, 16 * percentage / 100, 6);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
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
  gfx.drawString(0.5 * third, SCREEN_HEIGHT - 12, FPSTR(TEXT_CONFIG_BUTTON));
  gfx.drawString(2.5 * third, SCREEN_HEIGHT - 12, FPSTR(TEXT_REFRESH_BUTTON));
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


