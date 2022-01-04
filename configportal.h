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

#include <ESP8266WebServer.h>
#include <MiniGrafx.h>

const char HTML_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTML_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input, select{padding:5px;font-size:1em;} input, select{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTML_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTML_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTML_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTML_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTML_FORM_START[] PROGMEM      = "<form method='post' action='save'><br/>";
const char HTML_FORM_PARAM[] PROGMEM      = "<label for='{i}'>{p}</label><br/><input id='{i}' name='{n}' maxlength={l}  value='{v}' {c}><br/><br/>";
const char HTML_FORM_END[] PROGMEM        = "<br/><button type='submit'>Save</button></form><br/><form action=\"/reset\" method=\"get\"><button>Restart ESPaper Display</button></form>";
const char HTML_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTML_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTML_END[] PROGMEM             = "</div></body></html>";
const char HTML_OPTION_ITEM[] PROGMEM     = "<option value=\"{v}\" {s}>{n}</option>";
const char HTML_WG_LANGUAGES[] PROGMEM    = "See <a href='https://www.wunderground.com/weather/api/d/docs?d=language-support' target=_blank>Language Codes</a> for explanation.";

const char language_0[] PROGMEM = "Arabic|ar";
const char language_1[] PROGMEM = "Bulgarian|bg";
const char language_2[] PROGMEM = "Catalan|ca";
const char language_3[] PROGMEM = "Czech|cz";
const char language_4[] PROGMEM = "German|de";
const char language_5[] PROGMEM = "Greek|el";
const char language_6[] PROGMEM = "English|en";
const char language_7[] PROGMEM = "Persian (Farsi)|fa";
const char language_8[] PROGMEM = "Finnish|fi";
const char language_9[] PROGMEM = "French|fr";
const char language_10[] PROGMEM = "Galician|gl";
const char language_11[] PROGMEM = "Croatian|hr";
const char language_12[] PROGMEM = "Hungarian|hu";
const char language_13[] PROGMEM = "Italian|it";
const char language_14[] PROGMEM = "Japanese|ja";
const char language_15[] PROGMEM = "Korean|kr";
const char language_16[] PROGMEM = "Latvian|la";
const char language_17[] PROGMEM = "Lithuanian|lt";
const char language_18[] PROGMEM = "Macedonian|mk";
const char language_19[] PROGMEM = "Dutch|nl";
const char language_20[] PROGMEM = "Polish|pl";
const char language_21[] PROGMEM = "Portuguese|pt";
const char language_22[] PROGMEM = "Romanian|ro";
const char language_23[] PROGMEM = "Russian|ru";
const char language_24[] PROGMEM = "Swedish|se";
const char language_25[] PROGMEM = "Slovak|sk";
const char language_26[] PROGMEM = "Slovenian|sl";
const char language_27[] PROGMEM = "Spanish|es";
const char language_28[] PROGMEM = "Turkish|tr";
const char language_29[] PROGMEM = "Ukrainian|ua";
const char language_30[] PROGMEM = "Vietnamese|vi";
const char language_31[] PROGMEM = "Chinese Simplified|zh_cn";
const char language_32[] PROGMEM = "Chinese Traditional|zh_tw";

const char* const language_table[] PROGMEM = {language_0, language_1, language_2, language_3, language_4, language_5, language_6,
  language_7, language_8, language_9, language_10, language_11, language_12, language_13, language_14, language_16, language_17,
  language_18, language_19, language_20, language_21, language_22, language_23, language_24, language_25, language_26, language_27,
  language_28, language_29, language_30, language_31, language_32};

ESP8266WebServer server (80);

String getFormField(String id, String placeholder, String length, String value, String customHTML) {
    String pitem = FPSTR(HTML_FORM_PARAM);

    pitem.replace("{i}", id);
    pitem.replace("{n}", id);
    pitem.replace("{p}", placeholder);
    pitem.replace("{l}", length);
    pitem.replace("{v}", value);
    pitem.replace("{c}", customHTML);
  return pitem;
}

boolean saveConfig() {
  File f = SPIFFS.open("/espaper.txt", "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  f.print("WIFI_SSID=");
  f.println(WIFI_SSID);
  f.print("WIFI_PASS=");
  f.println(WIFI_PASS);
  f.print("DISPLAYED_CITY_NAME=");
  f.println(DISPLAYED_CITY_NAME);
  f.print("OPEN_WEATHER_MAP_APP_ID=");
  f.println(OPEN_WEATHER_MAP_APP_ID);
  f.print("OPEN_WEATHER_MAP_LOCATION_ID=");
  f.println(OPEN_WEATHER_MAP_LOCATION_ID);
  f.print("OPEN_WEATHER_MAP_LANGUAGE=");
  f.println(OPEN_WEATHER_MAP_LANGUAGE);
  f.close();
  Serial.println("Saved values");
  return true;
}

boolean loadConfig() {
  Serial.println("Loading config");
  File f = SPIFFS.open("/espaper.txt", "r");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  while(f.available()) {
      //Lets read line by line from the file
      String key = f.readStringUntil('=');
      String value = f.readStringUntil('\r');
      f.read();
      Serial.println(key + " = [" + value + "]");
      Serial.println(key.length());
      if (key == "WIFI_SSID") {
        WIFI_SSID = value;
      }
      if (key == "WIFI_PASS") {
        WIFI_PASS = value;
      }
      if (key == "DISPLAYED_CITY_NAME") {
        DISPLAYED_CITY_NAME = value;
      }
      if (key == "OPEN_WEATHER_MAP_APP_ID") {
        OPEN_WEATHER_MAP_APP_ID = value;
      }
      if (key == "OPEN_WEATHER_MAP_LOCATION_ID") {
        OPEN_WEATHER_MAP_LOCATION_ID = value;
      }
      if (key == "OPEN_WEATHER_MAP_LANGUAGE") {
        OPEN_WEATHER_MAP_LANGUAGE = value;
      }
  }

  f.close();
  Serial.println("Loaded config");
  return true;
}

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  String header = FPSTR(HTML_HEAD);
  header.replace("{v}", "Options");
  server.sendContent(header);
  server.sendContent(FPSTR(HTML_SCRIPT));
  server.sendContent(FPSTR(HTML_STYLE));
  server.sendContent(FPSTR(HTML_HEAD_END));
  server.sendContent("<h1>ESPaper Configuration</h1>");

  //page += FPSTR(HTML_PORTAL_OPTIONS;
  server.sendContent(FPSTR(HTML_FORM_START));
  server.sendContent(getFormField("ssid", "WiFi SSID", "20", WIFI_SSID, ""));
  server.sendContent(getFormField("password", "WiFi Password", "20", WIFI_PASS, ""));
  server.sendContent(getFormField("displayedcityname", "Displayed City Name", "40", DISPLAYED_CITY_NAME, ""));
  server.sendContent(getFormField("openweathermapkey", "OpenWeatherMap App ID", "40", OPEN_WEATHER_MAP_APP_ID, ""));
  server.sendContent(getFormField("openweathermaplocationid", "OpenWeatherMap Location ID (e.g. 2657896)", "40", OPEN_WEATHER_MAP_LOCATION_ID, ""));
  server.sendContent("<label for=\"openweathermaplanguage\">OpenWeatherMap Language</label>");
  server.sendContent("<select id=\"openweathermaplanguage\" name=\"openweathermaplanguage\">");

  for (int i = 0; i < 32; i++) {
    String option = FPSTR(HTML_OPTION_ITEM);
    String language = FPSTR(language_table[i]);
    int separatorPos = language.indexOf("|");

    String label = language.substring(0, separatorPos);
    String key = language.substring(separatorPos + 1);
    option.replace("{v}", key);
    option.replace("{n}", label);
    if (key == OPEN_WEATHER_MAP_LANGUAGE) {
      option.replace("{s}", "selected");
    } else {
      option.replace("{s}", "");
    }
    server.sendContent(option);
    yield();
  }
  server.sendContent("</select>");
  server.sendContent("<br/><br/>");
  server.sendContent(FPSTR(HTML_FORM_END));
  server.sendContent(FPSTR(HTML_END));
  server.sendContent("");
  server.client().stop();

}

void handleSave() {
  WIFI_SSID = server.arg("ssid");
  WIFI_PASS = server.arg("password");
  DISPLAYED_CITY_NAME = server.arg("displayedcityname");
  OPEN_WEATHER_MAP_APP_ID = server.arg("openweathermapkey");
  OPEN_WEATHER_MAP_LOCATION_ID = server.arg("openweathermaplocationid");
  OPEN_WEATHER_MAP_LANGUAGE = server.arg("openweathermaplanguage");
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASS);
  Serial.println(DISPLAYED_CITY_NAME);
  Serial.println(OPEN_WEATHER_MAP_APP_ID);
  Serial.println(OPEN_WEATHER_MAP_LOCATION_ID);
  Serial.println(OPEN_WEATHER_MAP_LANGUAGE);
  saveConfig();
  handleRoot();
}

void handleNotFound() {
  //digitalWrite ( led, 1 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  //digitalWrite ( led, 0 );
}

void startConfigPortal(MiniGrafx *gfx) {

  server.on ( "/", handleRoot );
  server.on ( "/save", handleSave);
  server.on ( "/reset", []() {
     saveConfig();
     ESP.restart();
  } );
  server.onNotFound ( handleNotFound );
  server.begin();

  boolean connected = WiFi.status() == WL_CONNECTED;

  gfx->fillBuffer(1);
  gfx->setColor(0);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialMT_Plain_16);

  if (connected) {
      Serial.println ( "Open browser at http://" + WiFi.localIP().toString() );

      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnected to: " + WiFi.SSID() + "\nOpen browser at\nhttp://" + WiFi.localIP().toString());
  } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(CONFIG_SSID.c_str());
      IPAddress myIP = WiFi.softAPIP();
      Serial.println(myIP);

      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnect WiFi to:\n" + CONFIG_SSID + "\nOpen browser at\nhttp://" + myIP.toString());
  }

  gfx->commit();

  Serial.println ( "HTTP server started" );

  while(true) {
    server.handleClient();
    yield();
  }
}
