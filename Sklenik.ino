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
 *         e. "Relay_04!": Prepinac sepnut (PIN_SWITCH1 = HIGH) dokud nebude opet v pozici LOW
 *    5) Blikne integrovanou LED po vyslani urcite zpravy:
 *         a. 1x = "Relay_00!"
 *         b. 2x = "Relay_01!"
 *         c. 3x = "Relay_02!"
 *         d. 4x = "Relay_03!"
 *         e. 5x = "Relay_04!"
 *         f. 0x = !!!CHYBOVY STAV!!! (neodchazi zadna zprava)
 *  
 *    !!!Program se opakuje kazde 2 sekundy!!!
*/
 
// Pouzite knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Knihovna pro praci se sbernici I2C
#include <RTClib.h>               // Knihovna ovladace RTC

// Definice pozic vstupu
#define PIN_BUTTON1  2            // Pozice pro tlacitko
#define PIN_SWITCH1  5            // Pozice pro prepinac v NADRZ1
#define PIN_PLOV1  3              // Pozice pro dolni plovakovy senzor v NADRZ1
#define PIN_PLOV2  4              // Pozice pro horni plovakovy senzor v NADRZ1

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
  // Podminka pro "Relay_00!"
  if ((digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == HIGH)) {  // Cerpadlo nemuze automaticky sepnout, pokud je NADRZ1 plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = HIGH))
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(0);              // Ukaz zpravu na seriovym portu a blikni LED podle zpravy
    send_msg("Relay_00!");        // Odesli zpravu pres radio
  }
  // Podminka pro "Relay_01!"
  else if ((digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == LOW)) {  // Cerpadlo muze sepnout, pokud neni NADRZ1 plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW))
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(1);              // Ukaz zpravu na seriovym portu a blikni LED podle zpravy
    send_msg("Relay_01!");        // Odesli zpravu pres radio
  }
  // Podminka pro "Relay_02!"
  else if ((digitalRead(PIN_BUTTON1) == HIGH) && (digitalRead(PIN_PLOV1) == LOW) && (digitalRead(PIN_PLOV2) == LOW)) {  // Tlacitko stiknuto (PIN_BUTTON1 = HIGH) + NADRZ1 neni plna ((PIN_PLOV1 = LOW) && (PIN_PLOV2 = LOW))
    while (digitalRead(PIN_PLOV2) == LOW) {     // Opakuj, dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(2);            // Ukaz zpravu na seriovym portu a blikni LED podle zpravy
      send_msg("Relay_02!");      // Odesli zpravu pres radio
    }
  }
  // Podminka pro "Relay_03!"
  else if ((digitalRead(PIN_PLOV1) == HIGH) && (digitalRead(PIN_PLOV2) == LOW) && (time.hour() > 5) && (time.hour() < 19)) {  // NADRZ1 je prazdna ((PIN_PLOV1 = HIGH) && (PIN_PLOV2 = LOW)) a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu)
    while (digitalRead(PIN_PLOV2) == LOW) { // Opakuj, dokud neni NADRZ1 plna (PIN_PLOV2 = HIGH)
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(2);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_01!");      // Odesli zpravu pres radio
    }
  }
  // Podminka pro "Relay_04!"
  else if (digitalRead(PIN_SWITCH1) == HIGH) { // Pokud chceme cerpat bez ohledu na plovaky v NADRZ1 (slouzi pro zalevani zahrady atp. po prepojeni na hadici) 
    RTC(1);                       // Ukaz na seriovym portu stav na RTC modulu
    RadioMessage(3);              // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
    send_msg("Relay_01!");        // Odesli zpravu pres radio
  } 
  // Ochrana proti nabouranym zpravam
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
  delay(2000);                    // opakuj odeslani zpravy kazde 2s
}

void RadioMessage(uint8_t mode) { // Funkce pro odeslani retezce na seriovy port a ovladani integrovane LED
  switch (mode) {                 // Prejdi na pripad, ktereho se to tyka
    case 0:                       // Zprava "Relay_00"
      Serial.println(F("Relay OFF")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej mezeru mezi zpravami
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, ze odesla zprava
      delay(50);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
      delay(50);                  // Pockej 50ms
      break;
    case 1:                       // Zprava "Relay_01"
      Serial.println(F("Relay possible to ON")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej mezeru mezi zpravami
      for (int x = 0; x < 2; x++) { // Blikni ledkou 2x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
        delay(50);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
        delay(50);                // Pockej 50ms
      }
      break;
    case 2:                       // Zprava "Relay_02"
      Serial.println(F("Relay ON by BUTTON1")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej mezeru mezi zpravami
      for (int x = 0; x < 3; x++) { // Blikni ledkou 3x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
        delay(50);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
        delay(50);                // Pockej 50ms
      }
      break;
    case 3:                       // Zprava "Relay_03"
      Serial.println(F("Relay ON by PLOV1"));  // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej mezeru mezi zpravami
      for (int x = 0; x < 4; x++) { // Blikni ledkou 4x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
        delay(50);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
        delay(50);                // Pockej 50ms
      }
      break;
    case 4:                       // Zprava "Relay_04"
      Serial.println(F("Relay ON prepinacem"));  // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej mezeru mezi zpravami
      for (int x = 0; x < 5; x++) { // Blikni ledkou 5x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku
        delay(50);                // Pockej 50ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
        delay(50);                // Pockej 50ms
      }
      break;
    default:                      // Kdyz stav neznam (chybovy stav)
      return;                     // Tak ukonci smycy a zacni znovu     
  }
}
