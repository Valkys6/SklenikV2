/* Sklenik:
 *  Vysli zpravu kazde 2 sekundy
 *  "Relay_ON!" pri podmince tlacitko v pouici HIGH (updatovat v kodu po osazeni do krabice) + horni plovak v pozici LOW a drz to, dokud neni horni plovak v pozici HIGH
 *  "Relay_ON!" pri podmince spodni i horni plovak v pozici LOW a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu) a drz to, dokud neni horni plovak v pozici HIGH
 *  "Relay_OF!" pokud neni neni zadna z predchozich podminek splnena
 *  Serial port ukaze co to odesilame a pomoci RTC modulu DS3231 ukaze cas a stav teploty v krabici ridici jednotky skleniku na seriovem portu
 *  Po vyslani zpravy "Relay_OF!" blikne integrovanou LED 1x
 *  Po vyslani zpravy "Relay_ON!" blikne integrovanou LED 2x
 */
 
// Pouzite knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Neni zde pouzito, ale je potreba kompilovat// Nativni knihovna pro praci se sbernici I2C
#include <RTClib.h>               // Knihovna ovladace RTC

// Definice pozic vstupu:
#define PIN_BUTTON1  2            // Pozice pro tlacitko 1 - zapni relé
#define PIN_PLOV1  3              // Pozice pro horni plovakova senzor
#define PIN_PLOV2  4              // Pozice pro dolni plovakova senzor

// Pouzite ovladace periferii:
RH_ASK driver;                    // Objekt ovladace radia
RTC_DS3231 rtc;                   // Objekt ovladace RTC
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

// Jednoducha synchornizace data a casu RTC modulu rucnim zadanim. Pozor, v kodu pocitame s GMT (-2 hodiny naseho letniho casu)
// Pri prvnim spusteni je treba nastavit cas. Pro vyssi presnost bychom jej mohli nastavit treba rucnim zadanim pres seriovou linku az za behu. Ja naopak natvrdo v kodu vytvorim objekt s casem a ten se pak pri spusteni nahraje do pameti RTC cipu.
// Postupne: Rok, mesic, den, hodina, minuta, sekunda, den v tydnu (nedele = 0)
// Abych si hodiny nerozhodil po kazdem zapnuti, je funkce zaremovana: 
// DateTime time(2020, 04, 18, 15, 0, 0, 6);
// Neovereno, ale az bude potreba, tak snad bude fungovat

void setup() {
  // Pripravime si seriak, abychom tam mohli zvracet moudra, které zvracíme po rádiu
  Serial.begin(9600);             // Debugging only
  Serial.println(F("Valkys super RF Sklenik driver v1.0\n=================================\n"));  // Ukaz pri inicializici

  // Nastav vystupy a vstupy
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace LED_BUILDIN jako výstup
  pinMode(PIN_BUTTON1, INPUT);    // Inicilazace tlacitka jako vstup
  pinMode(PIN_PLOV1, INPUT);      // Inicializace horniho plovakoveho senzoru - PIN_PLOV1
  pinMode(PIN_PLOV2, INPUT);      // Inicializace spodniho plovakoveho senzoru - PIN_PLOV2
  RadioMessage(0);                // Defaultne posilame pokyn k vypnuti rele

  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("init failed");  // Nastavení komunikace radioveho vysilace - pin 12 (urceno knihovnou)
  if (!rtc.begin()) {            // Nastaveni komunikace RTC modulu
    Serial.println("Couldn't find RTC");  // Bude drzkovat, kdyz nenajde RTC modul (baterie v modulu by mela vydrzet 8 let) 
    while (1);                    // Kdyz je RTC modul ziv a zdrav, posila data (absenci slozenych zavorek a tu jednicku nechapu)
  }
}

void send_msg(const char* msg) {  // Funkce pro odeslani retezce  
  driver.send((uint8_t *)msg, strlen(msg)); // Posli to
  driver.waitPacketSent();        // Cekej, az to bude cely venku                             
  delay(2000);                    // Flakej se 2s
}

void loop() {
  
  DateTime time = rtc.now();      // Optej se jaky mame cas
  
  if ((digitalRead(PIN_BUTTON1) == LOW) && (digitalRead(PIN_PLOV1) == LOW)) {  // Pokud je sepnuto tlacitko a sud neni zcela naplnen tak zacni cerpat
    while (digitalRead(PIN_PLOV1) == LOW) {  // Pokud plati ze sud neni naplnen opakuj nasledujici
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(1);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_ON!");      // Volam funkci odeslani retezce
    }
  }

  else if ((digitalRead(PIN_PLOV2) == HIGH) && (digitalRead(PIN_PLOV1) == LOW) && (time.hour() > 5) && (time.hour() < 19)) {  // Pokud je hladina pod spodnim plovakem, horni plovak neni sepnut (ochrana, proti selhani Plov2) a je mezi 5:00 a 19:00 chceme cerpat
    while (digitalRead(PIN_PLOV1) == LOW) { // Pokud plati ze sud neni naplnen opakuj nasledujici
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(1);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_ON!");      // Volam funkci odeslani retezce
    }
  }

  else {
    RTC(1);
    RadioMessage(0);              // Ukaz zpravu na seriovym portu, ze chceme vypnout relatko (cerpadlo)
    send_msg("Relay_OF!");        // Volam funkci odeslani retezce
  }
}

void RTC(uint8_t mode) {
  if (mode==1) {
    DateTime time = rtc.now();    // Optej se jaky mame cas
    Serial.print(String("GMT:")+time.timestamp(DateTime::TIMESTAMP_FULL));  // Ukaz GMT cas
    Serial.println();             // Ukonci radek
    Serial.print("Hour: ");       // Ukaz retezec s popisem, ze chceme ukazat hodinu
    Serial.print(time.hour(), DEC); // Jakou mame hodinu
    Serial.println();             //Ukonci radek
    Serial.print("RTCTemp: ");    // Ukaz retezec s popisem, ze chceme ukazat teplotu
    Serial.print(rtc.getTemperature());  // Jakou mame teplotu
    Serial.println(" C");         // At ma pan Celsius radost a ukonci radek
  }
}

void RadioMessage(uint8_t mode) { // Funkce pro odeslani retezce na seriovy port - kontrola co to vlastne odesilame
  if (mode==0) {                  //Prejem si vypnout 
    Serial.println(F("Relay OFF")); // Ukaz na seriovem portu "Relay OFF"
    Serial.println();             // Pridej mezeru za touto zpravou
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, ze odesla zprava
    delay(50);                    // Pockej 50ms
    digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
    delay(50);                    // Pockej 50ms
  } else {                        // Prejem si zapnout 
    Serial.println(F("Relay ON"));  // Tak ukaz na seriovem portu "Relay ON"
    Serial.println();             // Pridej mezeru mezi zpravami
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, ze odesla zprava "Relay ON"
    delay(50);                    // Pockej 50ms
    digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
    delay(50);                    // Pockej 50ms
    digitalWrite(LED_BUILTIN, HIGH);  // Jeste jednou rozsvitime, ze odesla zprava "Relay ON"
    delay(50);                    // Pockej 50ms
    digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
    delay(50);                    // Pockej 50ms
  }
}
