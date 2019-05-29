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
#include <OpenWeatherMapCurrent.h>
#include <OpenWeatherMapForecast.h>
#include <Astronomy.h>
#include <MiniGrafx.h>
#include <EPD_WaveShare.h>



#include "ArialRounded.h"
#include <MiniGrafxFonts.h>
#include "moonphases.h"
#include "weathericons.h"
#include "configportal.h"



#define MINI_BLACK 0
#define MINI_WHITE 1


#define MAX_FORECASTS 20

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                     };

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 296
#define BITS_PER_PIXEL 1


EPD_WaveShare epd(EPD2_9, CS, RST, DC, BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

OpenWeatherMapCurrentData conditions;
Astronomy::MoonData moonData;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];

// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);
uint32_t dstOffset = 0;
uint8_t foundForecasts = 0;

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawButtons();
void drawCurrentWeather();
void drawForecast();
void drawTempChart();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();


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

typedef struct BatteryData {
  uint8_t percentage;
  float voltage;
} BatteryData;

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
  Serial.println("Woke up");
  pinMode(USR_BTN, INPUT_PULLUP);
  int btnState = digitalRead(USR_BTN);

  gfx.init();
  gfx.setRotation(1);
  gfx.setFastRefresh(false);
  
  // load config if it exists. Otherwise use defaults.
  boolean mounted = SPIFFS.begin();
  if (!mounted) {
    Serial.println("FS not formatted. Doing that now");
    SPIFFS.format();
    Serial.println("FS formatted...");
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
      drawWifiQuality();
      drawBattery();
      drawCurrentWeather();
      drawForecast();
      drawTempChart();
      drawAstronomy();
      drawButtons();
      gfx.commit();
    } else {
      gfx.fillBuffer(MINI_WHITE);
      gfx.setColor(MINI_BLACK);
      gfx.setTextAlignment(TEXT_ALIGN_CENTER);
      gfx.drawString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 40, "Could not connect to WiFi.\nPress and hold LEFT button +\npress and release RIGHT button\nto enter config mode");
      gfx.commit();
    }
    Serial.println("Going to sleep");
    ESP.deepSleep(UPDATE_INTERVAL_SECS * 1000000);
  }
}


void loop() {


}

// Update the internet based information and update screen
void updateData() {
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);
  OpenWeatherMapCurrent *conditionsClient = new OpenWeatherMapCurrent();
  conditionsClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  conditionsClient->setMetric(IS_METRIC);
  Serial.println("\nAbout to call OpenWeatherMap to fetch station's current data...");
  conditionsClient->updateCurrentById(&conditions, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);
  delete conditionsClient;
  conditionsClient = nullptr;

  OpenWeatherMapForecast *forecastsClient = new OpenWeatherMapForecast();
  forecastsClient->setMetric(IS_METRIC);
  forecastsClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  foundForecasts = forecastsClient->updateForecastsById(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);
  delete forecastsClient;
  forecastsClient = nullptr;

  // Wait max. 3 seconds to make sure the time has been sync'ed
  Serial.println("\nWaiting for time");
  unsigned timeout = 3000;
  unsigned start = millis();
  while (millis() - start < timeout) {
    time_t now = time(nullptr);
    if (now) {
      break;
    }
    Serial.println(".");
    delay(100);
  }

  dstOffset = UTC_OFFSET * 3600 + dstAdjusted.time(nullptr) - time(nullptr);

  Astronomy *astronomy = new Astronomy();
  time_t now = time(nullptr) + dstOffset;
  moonData = astronomy->calculateMoonData(now);
  // illumination is not uniquely clear to identify moon age. e.g. 70% illumination happens twice in a full cycle
  float lunarMonth = 29.53;
  uint8_t moonAge = moonData.phase <= 4 ? lunarMonth * moonData.illumination / 2 : lunarMonth - moonData.illumination * lunarMonth / 2;
  moonAgeImage = String((char) (65 + ((uint8_t) ((26 * moonAge / 30) % 26))));
  delete astronomy;
  astronomy = nullptr;



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
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(Meteocons_Plain_42);
  gfx.drawString(25, 20, conditions.iconMeteoCon);

  gfx.setColor(MINI_BLACK);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(55, 15, DISPLAYED_CITY_NAME);

  gfx.setFont(ArialMT_Plain_24);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(55, 25, String(conditions.temp,0) + (IS_METRIC ? "°C" : "°F") );

  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.drawString(55, 51, conditions.description);
  gfx.drawLine(0, 65, SCREEN_WIDTH, 65);
  // Remove any condition description that's too long by "covering" it with a white rectangle.
  // It'll later be drawn over by the forecast boxes.
  // Just a workaround until https://github.com/ThingPulse/minigrafx/issues/27 is fixed.
  gfx.setColor(MINI_WHITE);
  gfx.fillRect(SCREEN_WIDTH / 2 - 35, 52, SCREEN_WIDTH, 12);
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

  drawForecastDetail(SCREEN_WIDTH / 2 - 35, 15, 0);
  drawForecastDetail(SCREEN_WIDTH / 2 - 1, 15, 1);
  drawForecastDetail(SCREEN_WIDTH / 2 + 37, 15, 2);
  drawForecastDetail(SCREEN_WIDTH / 2 + 73, 15, 3);
  drawForecastDetail(SCREEN_WIDTH / 2 + 109, 15, 4);
}

// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t index) {
  time_t observation = forecasts[index].observationTime + dstOffset;
  struct tm* observationTm = localtime(&observation);

  gfx.setColor(MINI_BLACK);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);

  gfx.drawString(x + 19, y - 2, String(observationTm->tm_hour) + ":00");
  gfx.drawString(x + 19, y + 9, String(forecasts[index].temp,0) + "°");
  gfx.drawString(x + 19, y + 36, String(forecasts[index].rain,0) + (IS_METRIC ? "mm" : "in"));

  gfx.setFont(Meteocons_Plain_21);
  gfx.drawString(x + 19, y + 20, forecasts[index].iconMeteoCon);
  gfx.drawLine(x + 2, 12, x + 2, 65);

}

void drawTempChart() {
  if (foundForecasts == 0) {
    return;
  }
  float minTemp = 999;
  float maxTemp = -999;
  for (int i = 0; i < foundForecasts; i++) {
    float temp = forecasts[i].temp;
    if (temp > maxTemp) {
      maxTemp = temp;
    }
    if (temp < minTemp) {
      minTemp = temp;
    }
  }
  Serial.printf("Min temp: %f, max temp: %f\n", minTemp, maxTemp);
  float range = maxTemp - minTemp;
  uint16_t maxHeight = 35;
  uint16_t maxWidth = 120;
  uint16_t chartX = 168;
  uint16_t chartY = 70;
  float barWidth = maxWidth / foundForecasts;
  gfx.setColor(MINI_BLACK);
  gfx.drawLine(chartX, chartY + maxHeight, chartX + maxWidth, chartY + maxHeight);
  gfx.drawLine(chartX, chartY, chartX, chartY + maxHeight);
  uint16_t lastX = 0;
  uint16_t lastY = 0;
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(chartX - 5, chartY - 5, String(maxTemp, 0) + "°");
  gfx.drawString(chartX - 5, chartY + maxHeight - 5, String(minTemp, 0) + "°");
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  for (uint8_t i = 0; i < foundForecasts; i++) {
    float temp = forecasts[i].temp;
    float height = (temp - minTemp) * maxHeight / range;
    uint16_t x = chartX + i* barWidth;
    uint16_t y = chartY + maxHeight - height;
    if (i == 0) {
      lastX = x;
      lastY = y;
    }
    gfx.drawLine(x, y, lastX, lastY);
    
    if ((i - 3) % 8 == 0) {
      gfx.drawLine(x, chartY + maxHeight, x, chartY + maxHeight - 3);
      gfx.drawString(x, chartY + maxHeight, getTime(forecasts[i].observationTime));
    }
    lastX = x; 
    lastY = y;
  }
}

String getTime(time_t timestamp) {
  time_t time = timestamp + dstOffset;
  struct tm *timeInfo = localtime(&time);
  
  char buf[6];
  sprintf(buf, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
  return String(buf);
}

// draw moonphase and sunrise/set and moonrise/set
void drawAstronomy() {
  gfx.setFont(MoonPhases_Regular_36);
  gfx.setColor(MINI_BLACK);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(25, 72, moonAgeImage);

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setFont(ArialMT_Plain_10);
  gfx.drawString(55, 69, FPSTR(TEXT_SUN));
  gfx.drawString(55, 79,  getTime(conditions.sunrise) + " - " + getTime(conditions.sunset));
  gfx.drawString(55, 89, FPSTR(TEXT_MOON));
  gfx.drawString(55, 99, MOON_PHASES[moonData.phase]);
  // Moon time not yet supported
  //gfx.drawString(55, 104, "8:30 - 11:20");
}

BatteryData calculateBatteryData() {
  BatteryData data = BatteryData();

  uint8_t percentage = 100;
  float adcVoltage = analogRead(A0) / 1024.0;
  // This values where empirically collected
  float voltage = adcVoltage * 4.945945946 -0.3957657658;
  if (voltage > 4.2) percentage = 100;
  else if (voltage < 3.3) percentage = 0;
  else percentage = (voltage - 3.3) * 100 / (4.2 - 3.3);

  data.percentage = percentage;
  data.voltage = voltage;
  
  return data;
}

void drawBattery() {
  BatteryData data = calculateBatteryData();

  gfx.setColor(MINI_BLACK);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(SCREEN_WIDTH - 22, -1, String(data.voltage, 2) + "V " + String(data.percentage) + "%");
  gfx.drawRect(SCREEN_WIDTH - 22, 0, 19, 10);
  gfx.fillRect(SCREEN_WIDTH - 2, 2, 2, 6);
  gfx.fillRect(SCREEN_WIDTH - 20, 2, 16 * data.percentage / 100, 6);
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

uint8_t getDigits(uint8_t number) {
  int digits = 0; 
  do { 
    number /= 10; 
    digits++;
  } while (number != 0);
  return digits;
}

void drawWifiQuality() {
  int8_t quality = getWifiQuality();
  BatteryData batteryData = calculateBatteryData();

  // 6 pixels per digit
  uint8_t offset = 69 - (6 * (getDigits(quality) + getDigits(batteryData.percentage)));
  
  gfx.setColor(MINI_BLACK);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        gfx.setPixel(SCREEN_WIDTH / 2 + (offset - 10) + 2 * i, 8 - j);
      }
    }
  }

  Serial.println("WiFi: " + String(quality) + "%");
  gfx.drawString(SCREEN_WIDTH / 2 + offset, -1, String(quality) + "%");
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
