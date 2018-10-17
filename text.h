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

#if LANG == 'EN'
  const char TEXT_REFRESH_BUTTON[] PROGMEM     = "REFRESH";
  const char TEXT_CONFIG_BUTTON[] PROGMEM      = "CONFIG + RST";
  const char TEXT_UPDATED[] PROGMEM            = "Updated:";
  const char TEXT_SUN[] PROGMEM                = "Sun:";
  const char TEXT_MOON[] PROGMEM               = "Moon:";
  const String MOON_PHASES[]                   = {"New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
                                                  "Full Moon", "Waning Gibbous", "Third quarter", "Waning Crescent"};
#elif LANG == 'DE'
  const char TEXT_REFRESH_BUTTON[] PROGMEM     = "AKTUALISIEREN";
  const char TEXT_CONFIG_BUTTON[] PROGMEM      = "CONFIG + RST";
  const char TEXT_UPDATED[] PROGMEM            = "Aktualisiert:";
  const char TEXT_SUN[] PROGMEM                = "Sonne:";
  const char TEXT_MOON[] PROGMEM               = "Mond:";
  const String MOON_PHASES[]                   = {"Neumond", "erstes Viertel", "zunehmender Mond", "zweites Viertel",
                                                  "Vollmond", "drittes Viertel", "abnehmender Mond", "letztes Viertel"};
#elif LANG == 'FR'
  const char TEXT_REFRESH_BUTTON[] PROGMEM     = "ACTUALISER";
  const char TEXT_CONFIG_BUTTON[] PROGMEM      = "CONFIG + RST";
  const char TEXT_UPDATED[] PROGMEM            = "Mise à jour ";
  const char TEXT_SUN[] PROGMEM                = "Soleil:";
  const char TEXT_MOON[] PROGMEM               = "Lune:";
  const String MOON_PHASES[]                   = {"nouvelle lune", "premier croissant", "premier quartier", "lune croissante",
                                                  "pleine lune", "lune décroissante", "dernier quartier", "dernier croissant"};  
#endif
