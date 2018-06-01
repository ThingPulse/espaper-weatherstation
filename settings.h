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
See more at http://blog.squix.ch
*/

#include <simpleDSTadjust.h>

// see text.h for available text resources. Also change WUNDERGRROUND_LANGUAGE below to fetch weather data in different language 
#define LANG 'EN'
#include "text.h"

// Config mode SSID
const String CONFIG_SSID = "ESPaperConfig";

// Setup
String WIFI_SSID = "yourssid";
String WIFI_PASS = "yourpassw0rd";

const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 20 minutes

/*
 * BUSY---->gpio4 
 * RST---->gpio2 
 * DC---->gpio5 
 * CS---->gpio15 
 * CLK---->gpio14 
 * DIN---->gpio13
 * Buttons : Reset ( RST pins on esp ) , 
 * Flash ( GPIO-0 10K pull up ) 
 * User button ( GPIO-12 10K pull up )
 */
 /*
 Connect the following pins:
 Display  NodeMCU
 BUSY     D1
 RST      D2
 DC       D8
 CS       D3
 CLK      D5
 DIN      D7
 GND      GND
 3.3V     3V3
*/
/*
 * BUSY>gpio4 RST>gpio2 DC>gpio5 CS>gpio15 CLK>gpio14 DIN>gpio13
 */
#define CS 15  // D8
#define RST 2  // D4
#define DC 5   // D1
#define BUSY 4 // D2
#define USR_BTN 12 // D6


   
// Wunderground Settings
// To check your settings first try them out in your browser:
// http://api.wunderground.com/api/WUNDERGROUND_API_KEY/conditions/q/WUNDERGROUND_COUNTTRY/WUNDERGROUND_CITY.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CH/Zurich.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CA/SAN_FRANCISCO.json <- note that in the US you use the state instead of country code
// If you want to have control over the exact weather station ("observation location") that WU uses you need to replace
// the city with "pws:STATION_ID". So, for the Aquatic Park in San Francisco you'd use "pws:KCASANFR359".
// Alternative query options are documented at https://www.wunderground.com/weather/api/d/docs?d=data/index&MR=1#standard_request_url_format

String DISPLAYED_CITY_NAME = "Zurich";
String OPEN_WEATHER_MAP_APP_ID = "";
String OPEN_WEATHER_MAP_LOCATION = "Zurich,CH";
/*
Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
*/
String OPEN_WEATHER_MAP_LANGUAGE = "en";

const String MOON_PHASES[] = {"New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
                              "Full Moon", "Waning Gibbous", "Third quarter", "Waning Crescent"};

#define UTC_OFFSET + 1
struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0};       // Central European Time = UTC/GMT +1 hour

// Settings for Boston
// #define UTC_OFFSET -5
// struct dstRule StartRule = {"EDT", Second, Sun, Mar, 2, 3600}; // Eastern Daylight time = UTC/GMT -4 hours
// struct dstRule EndRule = {"EST", First, Sun, Nov, 1, 0};       // Eastern Standard time = UTC/GMT -5 hour

// values in metric or imperial system?
bool IS_METRIC = true;

// Change for 12 Hour/ 24 hour style clock
bool IS_STYLE_12HR = false;

// change for different ntp (time servers)
#define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"
// #define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"




/***************************
 * End Settings
 **************************/

