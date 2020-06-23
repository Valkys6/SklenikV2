/* Sklenik (NADRZ1)
 *  
 *  Hlavni ovladaci jednotka systemu, ktera sleduje stav na vstupech a odesila zpravy pres radiovy modul DR3100 k jednotce u cerpadla (NADRZ2)
 *  
 *  Vice k projektu na: https://github.com/Valkys6/SklenikV2
 *  
 *  Popis chovani kodu:
 *    1) Pomoci RTC modulu DS3231 ukaze datum a cas (GMT) na seriovem portu
 *    2) Potom ukaze teplotu na ridici jednotce (teplomer soucasti RTC modulu)
 *    3) Ukaze aktualni hodinu pro podminku neposilani prikazu k zapnuti cerpadla v case nocniho klidu
 *    4) Vysle a ukaze jednu z nasledujicich zprav za techto podminek:
 *         a. "Relay_00!": Cerpadlo nemuze automaticky sepnout, pokud je NADRZ1 plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = HIGH)); nebo pokud posila nize nespecifikovany stav
 *         b. "Relay_01!": Cerpadlo muze sepnout, pokud neni NADRZ1 plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW))
 *         c. "Relay_02!": Tlacitko stiknuto (PIN_BUTTON1 = HIGH) + NADRZ1 neni plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW)), dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
 *         d. "Relay_03!": NADRZ1 je prazdna ((PIN_PLOV1 = HIGH) && (PIN_PLOV2 = LOW)) a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu), dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
 *         e. "Relay_04!": Prepinac 1 sepnut (PIN_SWITCH1 = HIGH) dokud nebude opet v pozici LOW
 *    5) Blikne integrovanou LED (LED_BUILDIN) po vyslani urcite zpravy:
 *         a. 1x = "Relay_00!"
 *         b. 2x = "Relay_01!"
 *         c. 3x = "Relay_02!"
 *         d. 4x = "Relay_03!"
 *         e. 5x = "Relay_04!"
 *         f. 0x = !!!CHYBOVY STAV!!! (neodchazi zadna zprava)
 *    6) Rozvsiti LED1 nebo LED2 v pripade 
 *         a. PIN_LED1 = HIGH: pokud je tlacitko 1 stisknuto (PIN_SWITCH2 = HIGH) + NADRZ1 neni plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW)), dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
 *         b. PIN_LED2 = HIGH: pokud je prepinac 1 sepnut (PIN_SWITCH2 = HIGH)
 *  
 *    !!!Program se opakuje kazde 4 sekundy!!!
*/
 
// Pouzite knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Knihovna pro praci se sbernici I2C
#include <RTClib.h>               // Knihovna ovladace RTC

// Definice pozic digitalnich vstupu
#define PIN_BUTTON1  2            // Pozice pro tlacitko
#define PIN_SWITCH1  5            // Pozice pro prepinac v NADRZ1
#define PIN_PLOV1  3              // Pozice pro dolni plovakovy senzor v NADRZ1
#define PIN_PLOV2  4              // Pozice pro horni plovakovy senzor v NADRZ1

// Definice pozic digitalnich vystupu (LED_BUILDIN je automaticky na pinu 13) 
#define PIN_LED1  8              // Pozice pro LED2 k indikaci, ze tlacitko 1 bylo stisknuto a sviti, dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
#define PIN_LED2  9              // Pozice pro LED2 k indikaci, ze prepinac 1 je sepnut

// Pouzite ovladace periferii
RH_ASK driver;                    // Objekt ovladace radia
RTC_DS3231 rtc;                   // Objekt ovladace RTC
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup() {
  // Pripravime si seriak, abychom tam mohli ukazovat moudra
  Serial.begin(9600);             // Nastav rychlost prenosu
  Serial.println(F("Valkys super RF Sklenik driver v1.0\n=================================\n"));  // Ukaz pri inicializici

  // Nastav vystupy a vstupy
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace integrovane LED jako vystup
  pinMode(PIN_BUTTON1, INPUT);    // Inicilazace tlacitka jako vstup
  pinMode(PIN_LED1, OUTPUT);      // Inicilazace LED1 jako vystup
  pinMode(PIN_SWITCH1, INPUT);    // Inicilazace prepinace jako vstup
  pinMode(PIN_LED2, OUTPUT);      // Inicilazace LED2 jako vystup
  pinMode(PIN_PLOV1, INPUT);      // Inicializace dolniho plovakoveho senzoru - PIN_PLOV1
  pinMode(PIN_PLOV2, INPUT);      // Inicializace horniho plovakoveho senzoru - PIN_PLOV2
  RadioMessage(0);                // Defaultne posilame pokyn k vypnuti rele "RELAY_00!"

  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("Radio driver init failed");  // Nastavení komunikace radioveho vysilace - pin 12 (urceno knihovnou)
  if (!rtc.begin()) {             // Nastaveni komunikace RTC modulu
    Serial.println("Couldn't find RTC");  // Pokud selze RTC modul, vysli error (baterie v modulu by mela vydrzet 8 let) 
    while (1);                    // Kdyz je RTC modul ziv a zdrav, posila data
  }
}

void loop() {
  DateTime time = rtc.now();      // Optej se RTC modulu jaky mame cas
  // Podminka pro "Relay_01!"
  if ((digitalRead(PIN_BUTTON1) == LOW) && (digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == LOW) && (digitalRead(PIN_SWITCH1) == LOW)) {  // Cerpadlo muze sepnout, pokud neni NADRZ1 plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW) && (PIN_SWITCH1 = LOW))
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(1);              // Ukaz zpravu na seriovym portu a blikni LED podle zpravy
    send_msg("Relay_01!");        // Odesli zpravu pres radio
  }
  // Podminka pro "Relay_02!"
  else if ((digitalRead(PIN_BUTTON1) == HIGH) && (digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == LOW) && (digitalRead(PIN_SWITCH1) == LOW)) {  // Tlacitko stiknuto (PIN_BUTTON1 = HIGH) + NADRZ1 neni plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW) && (PIN_SWITCH1 = LOW))
    while (digitalRead(PIN_PLOV2) == LOW) {     // Opakuj, dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(2);            // Ukaz zpravu na seriovym portu a blikni LED podle zpravy
      send_msg("Relay_02!");      // Odesli zpravu pres radio
    }
  }
  // Podminka pro "Relay_03!"
  else if ((digitalRead(PIN_BUTTON1) == LOW) && (digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == LOW) && (time.hour() > 5) && (time.hour() < 19) && (digitalRead(PIN_SWITCH1) == LOW)) {  // NADRZ1 je prazdna ((PIN_PLOV1 = HIGH) && (PIN_PLOV2 = LOW) && (PIN_SWITCH1 = LOW)) a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu)
    while (digitalRead(PIN_PLOV2) == LOW) { // Opakuj, dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(3);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_03!");      // Odesli zpravu pres radio
    }
  }
  // Podminka pro "Relay_04!"
  else if (digitalRead(PIN_SWITCH1) == HIGH) { // Pokud chceme cerpat bez ohledu na plovaky v NADRZ1 (slouzi pro zalevani zahrady atp. po prepojeni na hadici) 
    while (digitalRead(PIN_SWITCH1) == HIGH) { // Opakuj dokud je PIN_SWITCH1 sepnut
      RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(4);              // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_04!");        // Odesli zpravu pres radio
    } 
  }
  // Podminka pro "Relay_00!" a vsem ostatnim (nedefinovanym) zpravam
  else {                          // V kazdem jinem pripade, vysli "Relay_00!"
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(0);              // Ukaz zpravu na seriovym portu, ze chceme vypnout relatko (cerpadlo)
    send_msg("Relay_00!");        // Volam funkci odeslani retezce
  }
}

void RTC(uint8_t mode) {          // Funkce RTC modulu
  if (mode==1) {                  // Pokud je RTC modul dostupny
    DateTime time = rtc.now();    // Optej se jaky mame cas
    Serial.println(String("GMT:")+time.timestamp(DateTime::TIMESTAMP_FULL));  // Ukaz GMT cas
    Serial.print("Hour: ");       // Ukaz retezec s popisem, ze chceme ukazat hodinu
    Serial.println(time.hour(), DEC); // Jakou mame hodinu
    Serial.print("RTCTemp: ");    // Ukaz retezec s popisem, ze chceme ukazat teplotu
    Serial.print(rtc.getTemperature());  // Jakou mame teplotu
    Serial.println(" C");         // At ma pan Celsius radost a ukonci radek
  }
}

void send_msg(const char* msg) {  // Funkce pro odeslani retezce  
  driver.send((uint8_t *)msg, strlen(msg)); // Posli to
  driver.waitPacketSent();        // Cekej, az to bude cely venku                            
  delay(4000);                    // Opakuj odeslani zpravy kazde 4s
}

void RadioMessage(uint8_t mode) { // Funkce pro odeslani retezce na seriovy port a ovladani LED vystupu (LED_BUILDIN, LED1, LED2)
  switch (mode) {                 // Prejdi na pripad, ktereho se to tyka
    case 0:                       // Zprava "Relay_00"
      Serial.println(F("Relay OFF")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED1, LOW);  // Zhasni LED1
      digitalWrite(PIN_LED2, LOW);  // Zhasni LED2
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN, ze odesla zprava
      delay(100);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
      delay(100);                  // Pockej 50ms
      break;
    case 1:                       // Zprava "Relay_01"
      Serial.println(F("Relay possible to ON")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED1, LOW);  // Zhasni LED1
      digitalWrite(PIN_LED2, LOW);  // Zhasni LED2
      for (int x = 0; x < 2; x++) { // Blikni LED_BUILDIN 2x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 50ms
      }
      break;
    case 2:                       // Zprava "Relay_02"
      Serial.println(F("Relay ON by BUTTON1")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED1, HIGH); // Rozsvit LED1 dokud neprijde jina zprava
      digitalWrite(PIN_LED2, LOW);  // Zhasni LED2
      for (int x = 0; x < 3; x++) { // Blikni LED_BUILDIN 3x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 50ms
      }
      break;
    case 3:                       // Zprava "Relay_03"
      Serial.println(F("Relay ON by PLOV1"));  // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED1, LOW);  // Zhasni LED1
      digitalWrite(PIN_LED2, LOW);  // Zhasni LED2
      for (int x = 0; x < 4; x++) { // Blikni LED_BUILDIN 4x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 50ms
      }
      break;
    case 4:                       // Zprava "Relay_04"
      Serial.println(F("Relay ON by SWITCH1"));  // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED1, LOW);  // Zhasni LED1
      digitalWrite(PIN_LED2, HIGH); // Rozsvit LED2
      for (int x = 0; x < 5; x++) { // Blikni LED_BUILDIN 5x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 50ms
      }
      break;
    default:                      // Kdyz stav neznam (chybovy stav)
      return;                     // Tak ukonci smycy a zacni znovu     
  }
}
