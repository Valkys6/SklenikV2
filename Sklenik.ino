/* Sklenik:
 *  Vysli zpravu kazde 2 sekundy
 *  "Relay_ON!" pri podmince tlacitko + horni plovak v pozici HIGH a drž, to dokud není Horní plovák v pozici LOW
 *  "Relay_ON!" pri podmince spodni plovak v pozici LOW + GMT cas je mezi 20:00 a 5:00 (-2 hodiny letniho casu) 
 *  "Relay_OF!" pokud neni neni zadna z predchozich podminek splnena
 *  Serial port ukaze co to odesilame a pomoci RTC modulu DS3231 ukaze cas a stav teploty v krabici ridici jednotky skleniku
 *  Po vyslani jakekoli zpravy rozsviti integrovanou LEDku
 */
 
// Pouzite knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Not actually used but needed to compile
#include <RTClib.h>               // Knihovna ovladace RTC (V originalu je teda "RTClib.h", ale snad to nebude vadit)

// Definice pozic vstupu ()
#define PIN_BUTTON1  2            // Pozice pro tlacitko 1 - zapni relé
#define PIN_PLOV1  3              // Pozice pro horni plovakova senzor
#define PIN_PLOV2  4              // Pozice pro dolni plovakova senzor

// Pouzite ovladace periferii
RH_ASK driver;                    // Objekt ovladace radia
RTC_DS3231 rtc;                   // Objekt ovladace RTC

void setup() {
  // Pripravime si seriak, abychom tam mohli zvracet moudra, které zvracíme po rádiu
  Serial.begin(9600);           // Debugging only
  Serial.println(F("Valkys super RF Sklenik driver v1.0\n=================================\n"));  // Ukaz pri inicializici

  // Nastav vystupy a vstupy
  pinMode(LED_BUILTIN, OUTPUT); // Inicializace LED_BUILDIN jako výstup
  pinMode(PIN_BUTTON1, INPUT);  // Inicilazace tlacitka jako vstup
  pinMode(PIN_PLOV1, INPUT);    // Inicializace horniho plovakoveho senzoru - PIN_PLOV1
  pinMode(PIN_PLOV2, INPUT);    // Inicializace spodniho plovakoveho senzoru - PIN_PLOV2
  RadioMessage(0);              // Defaultne posilame pokyn k vypnuti rele

  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("init failed");  // Nastavení komunikace radioveho vysilace - pin 12 (urceno knihovnou)
  if (! rtc.begin()) {          // Nastaveni komunikace RTC modulu
    Serial.println("Couldn't find RTC");  // Bude drzkovat, kdyz nenajde RTC modul (baterie v modulu by mela vydrzet 8 let) 
    while (1);                  // Kdyz je RTC modul ziv a zdrav, posila data (absenci slozenych zavorek a tu jednicku nechapu)
  }
}

void send_msg(const char* msg) {  // Funkce pro odeslani retezce  
  driver.send((uint8_t *)msg, strlen(msg)); // Posli to
  driver.waitPacketSent();      // Cekej, az to bude cely venku                             
  delay(2000);                  // Flakej se 2s
}

void loop() {
  
  if ((digitalRead(PIN_BUTTON1) == LOW) && (digitalRead(PIN_PLOV1) == HIGH)) {  // Pokud je sepnuto tlacitko pod podminko
    while (digitalRead(PIN_PLOV1) == HIGH) {  // Nez se naplní sud opakuj tohle
      RTC(1);                     // Ukaz na seriovym portu stav na RTC modulu
      RadioMessage(1);            // Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
      send_msg("Relay_ON!");      // Volam funkci odeslani retezce
    }
  }

  else if ((digitalRead(PIN_PLOV2) == LOW) && (digitalRead(PIN_PLOV1) == HIGH)) {  // Pokud je sepnuto tlacitko pod podminko
    while (digitalRead(PIN_PLOV1) == HIGH) {
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
    DateTime time = rtc.now();    //Optej se jaky mame cas
    Serial.print(String("GMT:")+time.timestamp(DateTime::TIMESTAMP_FULL));  //Ukaz GMT cas
    Serial.println();             //Ukonci radek
    Serial.print("Hour: ");       //Ukaz retezec s popisem, ze chceme ukazat hodinu
    Serial.print(time.hour(), DEC);  //Jakou mame hodinu
    Serial.println();             //Ukonci radek
    Serial.print("RTCTemp: ");    //Ukaz retezec s popisem, ze chceme ukazat teplotu
    Serial.print(rtc.getTemperature());  //Jakou mame teplotu
    Serial.println(" C");         //At ma pan Celsius radost a ukonci radek
  }
}

void RadioMessage(uint8_t mode) {        //Funkce pro odeslani retezce na seriovy port - kontrola co to vlastne odesilame
  if (mode==0) {                  //Prejem si vypnout 
    Serial.println(F("Relay OFF"));  // Ukaz na seriovem portu "Relay OFF"
    Serial.println();             // přidej mezeru mezi zpravami
    digitalWrite(LED_BUILTIN, HIGH);   // Rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
    digitalWrite(LED_BUILTIN, LOW);   //rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
  } else {                        //Prejem si zapnout 
    Serial.println(F("Relay ON"));  //tak ukaz na seriovem portu "Relay ON"
    Serial.println();             // přidej mezeru mezi zpravami
    digitalWrite(LED_BUILTIN, HIGH);   //rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
    digitalWrite(LED_BUILTIN, LOW);   //rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
    digitalWrite(LED_BUILTIN, LOW);   //rozsvitime ledku, bo odesla zprava
    delay(50);                    //Počkej 50ms
  }
}
