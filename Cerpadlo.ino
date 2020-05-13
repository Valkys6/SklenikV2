/* Cerpadlo (NADRZ2)
 *  
 *  Podruzna ovladaci jednotka systemu, ktera sleduje stav na vstupech a prijima zpravy pres radiovy modul DR3100 z jednotky u skleniku (NADRZ1)
 *  
 *  Vice k projektu na: https://github.com/Valkys6/SklenikV2
 *  
 *  Popis chovani kodu:
 *    1) Sleduje stav hladiny v NADRZ2 ze vstupu PIN_PLOV3; PIN_PLOV4 a pokyny z PIN_SWITCH2. Nasledovne prizpusobuje sve chovani. Cerpadlo (rele) zapne v pripade:
 *          a. sepne jen v pripade, ze neni NADRZ2 prazdna: (PIN_BUTTON3 = LOW); !!!DULEZITE I PRO VSECHNY NASLEDUJICI PODMINKY!!!
 *          b. automaticky pokud je NADRZ2 plna a NADRZ1 neni uplne plna: (PIN_PLOV3 = LOW) + (PIN_PLOV4 = LOW) + prijima zpravu "Relay_01"
 *          c. pokud je sepnut prepinac (PIN_SWITCH2 = HIGH)
 *    2) Prijima radiova data z NADRZ1 a podle nich vykonava nasledujici prikazy. Pokud je to:
 *          a. "Relay_00!": cerpadlo vypne (i pokud prijme jinou, nize nedefinovanou hodnotu)
 *          b. "Relay_01!": cerpadlo muze sepnout (viz bod 1b) a pokud se stane, je sepnuto, dokud neobdrzi "Relay_00!"
 *          c. "Relay_02!": cerpadlo zapne dokud neobdrzi zpravu "Relay_00!" a po 20s bude kod pokracovat - TADY BUDEME POKRACOVAT PRISTE
 * !!!
 *          d. "Relay_03!": cerpadlo zapne (pri stavu na NADRZ1: spodni plovak sepnut (PIN_PLOV1 = HIGH) + (PIN_PLOV2 = LOW) a GMT cas je mezi 5:00 a 19:00  (-2 hodiny letniho casu) dokud neni horni plovak v pozici HIGH)
 *          e. "Relay_04!": cerpadlo zapne (pri stavu na NADRZ1: prepinac sepnut (PIN_SWITCH1 = HIGH) dokud nebude opet v pozici LOW
 *    3) Blikne integrovanou LED po prijmuti urcite zpravy v poctu:
 *          a. 0x = neprichazi zadna zprava = CHYBOVY STAV - v pripade, ze bylo rele pred timto stavem zapnuto, vypne jej po 20 sekundach
 *          b. 1x = "Relay_00!"
 *          c. 2x = "Relay_01!"
 *          d. 3x = "Relay_02!"
 *          e. 4x = "Relay_03!"
 *          f. 5x = "Relay_04!"
 *          
 *    !!!Program se opakuje kazde 2 sekundy!!!
 */

// Použíté knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Neni zde pouzito, ale je potreba kompilovat

#define PIN_RELAY  2              // Pozice relatka pro spinani cerpadla
#define PIN_SWITCH2  5            // Pozice pro prepinac 2 v NADRZ 2
#define PIN_PLOV3  3              // Pozice pro horni plovakovy senzor v NADRZ2
#define PIN_PLOV4  4              // Pozice pro dolni plovakovy senzor v NADRZ2

RH_ASK driver;                    // Objekt ovladace radia
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup() {
  // Pripravime si seriak, abychom tam mohli zvracet moudra
  Serial.begin(9600);             // Nastav rychlost prenosu
  Serial.println(F("Valkys super RF Cerpadlo driver v1.0\n=================================\n")); // Ukaz na po pripojeni na seriovy port

  // Nastav vystupy a vstupy 
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace LED_BUILDIN jako výstup
  pinMode(PIN_RELAY, OUTPUT);     // Inicializace digitálního pinu pro RELAY
  pinMode(PIN_SWITCH2, INPUT);    // Inicializace prepinace jako vstup
  pinMode(PIN_PLOV3, INPUT);      // Inicializace horniho plovakoveho senzoru - PIN_PLOV3
  pinMode(PIN_PLOV4, INPUT);      // Inicializace dolniho plovakoveho senzoru - PIN_PLOV4
  relay(0);                       // Defaultne je rele vypnute
  reltim=0;                       // Necasujeme
  
  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("Radio driver init failed"); // Nastavení komunikace rádiového přijímače - pin 11 (určeno asi knihovnou)
}

void loop() {
  uint8_t buf[20];                // Deklarace bufferu, radeji s rezervickou
  uint8_t buflen;                 // Promenna bufferu

  //obsluha radia
  buflen=sizeof(buf);             // Do promenny naperu velikost bufiku, to asi slouzi radio knihovne, aby nejela nekam za roh
  if (driver.recv(buf, &buflen)) {  // Pokud nam neco dorazilo po radiu, tak se tomu budeme venovat
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, bo prisla zprava
    Serial.print(F("Message: "));  // Ukazeme
    Serial.println((char*)buf);   // Co to vlastne dorazilo        
    if ((strcmp("Relay_01!",buf)==0) && (digitalRead(PIN_PLOV3) == LOW)) {  // Pokud je to prikaz pro zapnuti a pokud je v NADRZ2 dostatek vody
      while (digitalRead(PIN_PLOV3) == LOW) { // Pokud plati ze NADRZ2 neni prazdna
        relay(1);                 // Zapneme relatko
        reltim=20;                // A nastavim casovadlo, aby rele v pripade neprijmuti zadne zpravy a nebo je voda z NADRZe vycerpana vyplo po 20 sekundach
      }
    } else if (strcmp("Relay_00!",buf)==0) { // Pokud je to prikaz pro vypnuti 
        relay(0);                   // Okamzite vypneme relatko
        reltim=0;                   // A casovadlo muzem zarazit, ikdyz by to nevadilo, ale je to tak hezcejsi
    } else {                      // Kdyz prijde nabourana zprava
        Serial.println(F("Unknown command")); // Tak ukaz ze to neznas
    }
  }

  //casovadlo
  delay(1000);                    // Cekame vterinu, takze cela smyce pojede 1x za vterinu   
  digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
  if (reltim!=0) {                // Pokud casovadlo jede, budeme casovat
    if ((--reltim)==0) {          // Cukneme a pokud to prave dojelo, 
      relay(0);                   // Vypnem relatko        
    }
  }
}

void relay(uint8_t mode) {        // Funkce pro odeslani retezce
  if (mode==0) {                  // Prejem si vypnout
    Serial.println(F("Relay OFF")); // Ukaz ze chceme vypnou rele
    digitalWrite(PIN_RELAY, LOW); // Vypinam relatko
  } else {                        // Nebo si prejem zapnout
    Serial.println(F("Relay ON"));  // Ukaz ze chceme zapnout rele
    digitalWrite(PIN_RELAY, HIGH);  // Zapni relatko
  } Serial.println();             // Pridej radek mezi jednotlivymi zpravami
}
