/* Sklenik (NADRZ1):
 *  
 *  Hlavni ovladaci jednotka systemu, ktera sleduje stav na vstupech a odesila zpravy pres radiovy modul DR3100 k jednotce u cerpadla (NADRZ2)
 *  
 *  Vice k projektu na: https://github.com/Valkys6/SklenikV2
 *  
 *  Popis chovani kodu :
 *    1) Arduino pomoci RTC modulu DS3231 ukaze datum a cas (GMT) na seriovem portu
 *    2) Potom ukaze teplotu na ridici jednotce (teplomer soucasti RTC modulu)
 *    3) Ukaze aktualni hodinu pro podminku neposilani prikazu k zapnuti cerpadla v case nocniho klidu
 *    4) Vysle a ukaze jednu z nasledujicich zprav zpravu kazde 2 sekundy za techto podminek:
 *          a) "Relay_00!" (Relay OFF) = vychozi zprava (cerpadlo vypnuto)
 *          b) "Relay_01!" (Relay ON) = tlacitko stiknuto (PIN_BUTTON1 = HIGH) + horni plovak nesepnut (PIN_PLOV2 = LOW) dokud neni horni plovak sepnut (PIN_PLOV2 = HIGH)
 *          c) "Relay_02!" (Relay ON) = spodni plovak sepnut (PIN_PLOV1 = HIGH) + (PIN_PLOV2 = LOW) a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu) dokud neni horni plovak v pozici HIGH
 *          d) "Relay_03!" (Relay ON) = prepinac sepnut (PIN_SWITCH1 = HIGH) dokud nebude opet v pozici LOW
 *    5) Blikne integrovanou LED po vyslani urcite zpravy pri podmince:
 *          a) "Relay_00!" blikne integrovanou LED 1x
 *          b) "Relay_01!" blikne integrovanou LED 2x
 *          c) "Relay_02!" blikne integrovanou LED 3x
 *          c) "Relay_03!" blikne integrovanou LED 4x
 *  
 *  !Funkce se opakuje kazde 2 sekundy!
 */
 
// Pouzite knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Knihovna pro praci se sbernici I2C
#include <RTClib.h>               // Knihovna ovladace RTC

// Definice pozic vstupu
#define PIN_BUTTON1  2            // Pozice pro tlacitko
#define PIN_SWITCH1  5            // Pozice pro prepinac 
#define PIN_PLOV1  3              // Pozice pro horni plovakova senzor
#define PIN_PLOV2  4              // Pozice pro dolni plovakova senzor

// Pouzite ovladace periferii
RH_ASK driver;                    // Objekt ovladace radia
RTC_DS3231 rtc;                   // Objekt ovladace RTC
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup()  {
  // Pripravime si seriak, abychom tam mohli zvracet moudra
  Serial.begin(9600);             // Nastav rychlost prenosu
  Serial.println(F("Valkys super RF Sklenik driver v1.0\n=================================\n"));  // Ukaz pri inicializici

  // Nastav vystupy a vstupy
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace integrovane LED jako vystup
  pinMode(PIN_BUTTON1, INPUT);    // Inicilazace tlacitka jako vstup
  pinMode(PIN_SWITCH1, INPUT);    // Inicilazace prepinace jako vstup
  pinMode(PIN_PLOV1, INPUT);      // Inicializace horniho plovakoveho senzoru - PIN_PLOV1
  pinMode(PIN_PLOV2, INPUT);      // Inicializace spodniho plovakoveho senzoru - PIN_PLOV2
  RadioMessage(0);                // Defaultne posilame pokyn k vypnuti rele

  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("Radio driver init failed");  // Nastavení komunikace radioveho vysilace - pin 12 (urceno knihovnou)
  if (!rtc.begin()) {             // Nastaveni komunikace RTC modulu
    Serial.println("Couldn't find RTC");  // Pokud selze RTC modul, vysli error (baterie v modulu by mela vydrzet 8 let) 
    while (1);                    // Kdyz je RTC modul ziv a zdrav, posila data
  }
}

void loop() {
  DateTime time = rtc.now();      // Optej se RTC modulu jaky mame cas
 
  if ((digitalRead(PIN_BUTTON1) == LOW) && (digitalRead(PIN_PLOV1) == LOW)) {  // Pokud je sepnuto tlacitko a sud neni zcela naplnen tak zacni cerpat
    while (digitalRead(PIN_PLOV1) == LOW) {  // Pokud plati ze sud neni naplnen opakuj nasledujici
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(1);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_01!");      // Volam funkci odeslani retezce
    }
  } 
  else if ((digitalRead(PIN_PLOV2) == HIGH) && (digitalRead(PIN_PLOV1) == LOW) && (time.hour() > 5) && (time.hour() < 19)) {  // Pokud je hladina pod spodnim plovakem, horni plovak neni sepnut (ochrana, proti selhani Plov2) a je mezi 5:00 a 19:00 chceme cerpat
    while (digitalRead(PIN_PLOV1) == LOW) { // Pokud plati ze sud neni naplnen opakuj nasledujici
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(2);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_01!");      // Volam funkci odeslani retezce
    }
  } 
  if (digitalRead(PIN_SWITCH1) == HIGH) { // Pokud chceme cerpat bez ohledu na plovaky v NADRZ1 (souzi pro zalevani zahrady atp. po prepojeni na hadici) 
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(3);              // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
    send_msg("Relay_01!");        // Volam funkci odeslani retezce
  } 
  else {                          
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(0);            // Ukaz zpravu na seriovym portu, ze chceme vypnout relatko (cerpadlo)
      send_msg("Relay_00!");      // Volam funkci odeslani retezce
    }
  }

void send_msg(const char* msg) {  // Funkce pro odeslani retezce  
  driver.send((uint8_t *)msg, strlen(msg)); // Posli to
  driver.waitPacketSent();        // Cekej, az to bude cely venku                             
  delay(2000);                    // opakuj odeslani zpravy kazde 2s
}

void RTC(uint8_t mode) {
  if (mode==1) {
    DateTime time = rtc.now();    // Optej se jaky mame cas
    Serial.println(String("GMT:")+time.timestamp(DateTime::TIMESTAMP_FULL));  // Ukaz GMT cas
    Serial.print("Hour: ");       // Ukaz retezec s popisem, ze chceme ukazat hodinu
    Serial.println(time.hour(), DEC); // Jakou mame hodinu
    Serial.print("RTCTemp: ");    // Ukaz retezec s popisem, ze chceme ukazat teplotu
    Serial.print(rtc.getTemperature());  // Jakou mame teplotu
    Serial.println(" C");         // At ma pan Celsius radost a ukonci radek
  }
}

void RadioMessage(uint8_t mode) { // Funkce pro odeslani retezce na seriovy port - kontrola co to vlastne odesilame
  if (mode==1) {                  // Prejem si zapnout stiskem tlacitka
    Serial.println(F("Relay ON tlacitkem")); // Ukaz na seriovem portu "Relay ON tlacitkem"
    Serial.println();             // Pridej mezeru mezi zpravami
    for (int x = 0; x < 2; x++) { // Blikni ledkou 2x
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
      delay(50);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
      delay(50);                  // Pockej 50ms
    }
  }
  if (mode==2) { 
    Serial.println(F("Relay ON plovakem"));  // ukaz na seriovem portu "Relay ON plovakem"
    Serial.println();             // Pridej mezeru mezi zpravami
    for (int x = 0; x < 3; x++) { // Blikni ledkou 3x
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
      delay(50);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
      delay(50);                  // Pockej 50ms
    }
  }
  if (mode==3) { 
    Serial.println(F("Relay ON prepinacem"));  // ukaz na seriovem portu "Relay ON plovakem"
    Serial.println();             // Pridej mezeru mezi zpravami
    for (int x = 0; x < 4; x++) { // Blikni ledkou 4x
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
      delay(50);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
      delay(50);                  // Pockej 50ms
    }
  }
  else {                  //Prejem si vypnout 
    Serial.println(F("Relay OFF")); // Ukaz na seriovem portu "Relay OFF"
    Serial.println();             // Pridej mezeru mezi zpravami
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, ze odesla zprava
    delay(50);                    // Pockej 50ms
    digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
    delay(50);                    // Pockej 50ms
  }
}
