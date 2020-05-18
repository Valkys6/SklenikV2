/* Cerpadlo (NADRZ2)
 *  
 *  Podruzna ovladaci jednotka systemu, ktera sleduje stav na vstupech a prijima zpravy pres radiovy modul DR3100 z jednotky u skleniku (NADRZ1)
 *  
 *  Vice k projektu na: https://github.com/Valkys6/SklenikV2
 *  
 *  Popis chovani kodu:
 *    1) Sleduje stav hladiny v NADRZ2 ze vstupu PIN_PLOV3; PIN_PLOV4 a pokyny z PIN_SWITCH2. Nasledovne prizpusobuje sve chovani. Cerpadlo (rele) zapne v pripade:
 *          a. ze neni NADRZ2 prazdna: (PIN_BUTTON3 = LOW); !!!DULEZITE I PRO VSECHNY NASLEDUJICI PODMINKY!!!
 *          b. automaticky pokud je NADRZ2 plna a NADRZ1 neni uplne plna: (PIN_PLOV3 = LOW) + (PIN_PLOV4 = LOW) + prijima zpravu "Relay_01"
 *          c. pokud je sepnut prepinac (PIN_SWITCH2 = HIGH)
 *    2) Prijima radiova data z NADRZ1 a podle nich vykonava nasledujici prikazy. Pokud je to:
 *          a. "Relay_00!": cerpadlo vypne (i pokud prijme jinou, nize nedefinovanou zpravu)
 *          b. "Relay_01!": cerpadlo je vypnute, ale je pripraveno sepnout. Ceka, jestli bude prepinac 2 sepnut (PIN_SWITCH2 = HIGH). Pokud se stane, je cerpadlo sepnuto, dokud neobdrzi "Relay_00!", nebo dokud neni NADRZ2 prazdna (PIN_PLOV3 = LOW)
 *          c. "Relay_02!": cerpadlo zapne dokud neobdrzi zpravu "Relay_00!" nebo "Relay_01!" a pokud neni NADRZ2 prazdna (PIN_PLOV3 = HIGH)
 *          d. "Relay_03!": cerpadlo zapne dokud neobdrzi zpravu "Relay_00!" nebo "Relay_01!" a pokud neni NADRZ prazdna (PIN_PLOV3 = HIGH)
 *          e. "Relay_04!": cerpadlo zapne dokud neobdrzi zpravu "Relay_00!" nebo "Relay_01!" a pokud neni NADRZ prazdna (PIN_PLOV3 = HIGH)
 *    3) Blikne integrovanou LED po prijmuti urcite zpravy:
 *          a. 1x = "Relay_00!"
 *          b. 2x = "Relay_01!"
 *          c. 3x = "Relay_02!"
 *          d. 4x = "Relay_03!"
 *          e. 5x = "Relay_04!"
 *          f. 0x = neprichazi zadna zprava = CHYBOVY STAV - v pripade, ze bylo rele pred timto stavem zapnuto, vypne jej po 20 sekundach
 *    4) Rozsiti LED3 (zluta) nebo LED4 (take zluta)
 *          a. PIN_LED3 = HIGH: Prichazi zprava "Relay_04!" (prepinac 1 na NADRZ1 sepnut)
 *          b. PIN_LED4 = HIGH: pokud je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
 *          
 *    !!!Program se opakuje kazde 3 sekundy!!! - Nutne overit casovani v provozu!!
 */

// Použíté knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Neni zde pouzito, ale je potreba kompilovat

// Definice pozic digitalnch vstupu
#define PIN_RELAY  2              // Pozice relatka pro spinani cerpadla
#define PIN_SWITCH2  5            // Pozice pro prepinac 2 v NADRZ 2
#define PIN_PLOV3  3              // Pozice pro dolni plovakovy senzor v NADRZ2
#define PIN_PLOV4  4              // Pozice pro horni plovakovy senzor v NADRZ2

// Definice pozic digitalnich vystupu (LED_BUILDIN je automaticky na pinu 13) 
#define PIN_LED3  12              // Pozice pro LED3 k indikaci, ze prepinac 2 je sepnut

// Pouzite ovladace periferii
RH_ASK driver;                    // Objekt ovladace radia
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup() {
  // Pripravime si seriak, abychom tam mohli ukazovat moudra
  Serial.begin(9600);             // Nastav rychlost prenosu
  Serial.println(F("Valkys super RF Cerpadlo driver v1.0\n=================================\n")); // Ukaz na po pripojeni na seriovy port

  // Nastav vystupy a vstupy 
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace LED_BUILDIN jako vystup
  pinMode(PIN_LED3, OUTPUT);      // Inicializace PIN_LED4 jako vystup
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
    Serial.print(F("Message: ")); // Ukazeme
    Serial.println((char*)buf);   // Co to vlastne dorazilo  
    // Podminka pro "Relay_00!"
    if (strcmp("Relay_00!",buf)==0) { // Pokud je to prikaz pro vypnuti 
        relay(0);                 // Okamzite vypneme relatko
        reltim=0;                 // A casovadlo muzem zarazit
    }
    // Podminka pro "Relay_01!"; prepinac 1 nesepnut
    else if ((strcmp("Relay_01",buf) == 0) && (digitalRead(PIN_PLOV3) == LOW) && (digitalRead(PIN_SWITCH2) == LOW)) {  // Cerpadlo je vypnute, ale je pripraveno sepnout. Ceka, jestli bude prepinac 2 sepnut (PIN_SWITCH2 = HIGH). Pokud se stane, je cerpadlo sepnuto, dokud neobdrzi "Relay_00!", nebo dokud neni NADRZ2 prazdna (PIN_PLOV3 = LOW)
      while ((digitalRead(PIN_PLOV3) == LOW) || (digitalRead(PIN_SWITCH2) == HIGH)) {  // Opakuj, dokud neni NARDZ2 prazdna, nebo dokud je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
        relay(1);                 // Je mozne zapnout cerpadlo
        reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy a nebo je voda z NADRZ2 vycerpana vyplo po 20 sekundach
      }
    }
    // Podminka pro "Relay_01!"; ceka na stav prepinace 2
    else if ((strcmp("Relay_01",buf) == 0) && (digitalRead(PIN_PLOV3) == LOW) && (digitalRead(PIN_SWITCH2) == HIGH)) {  // Cerpadlo je vypnute, ale je pripraveno sepnout. Ceka, jestli bude prepinac 2 sepnut (PIN_SWITCH2 = HIGH). Pokud se stane, je cerpadlo sepnuto, dokud neobdrzi "Relay_00!", nebo dokud neni NADRZ2 prazdna (PIN_PLOV3 = LOW)
      while ((digitalRead(PIN_PLOV3) == LOW) || (digitalRead(PIN_SWITCH2) == HIGH)) {  // Opakuj, dokud neni NARDZ2 prazdna, nebo dokud je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
        relay(1);                 // Je mozne zapnout cerpadlo
        reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy a nebo je voda z NADRZ2 vycerpana vyplo po 20 sekundach
      }
    }
    // Podminka pro "Relay_02!"
    else if ((strcmp("Relay_02!",buf) == 0) && (digitalRead(PIN_PLOV3) == LOW)) {  // Pokud je to prikaz pro zapnuti a pokud neni NADRZ2 prazdna
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud NADRZ2 neni prazdna
        relay(2);                 // Zapneme cerpadlo
        reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy a dokud NADRZ2 neni prazdna vyplo po 20 sekundach
      }
    }  
    // Podminka pro "Relay_02!"
    else if ((strcmp("Relay_03!",buf) == 0) && (digitalRead(PIN_PLOV3) == LOW)) {  // Pokud je to prikaz pro zapnuti a pokud neni NADRZ2 prazdna
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud NADRZ2 neni prazdna
        relay(3);                 // Zapneme cerpadlo
        reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy a nebo je NADRZ2 prazdna, vyplo po 20 sekundach
      }
    }
    else if (strcmp("Relay_03!",buf) == 0) && (digitalRead(PIN_PLOV3) == LOW)) {  // Pokud je to prikaz pro vypnuti 
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud NADRZ2 neni prazdna
        relay(3);                 // Zapneme cerpadlo
        reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy a nebo je NADRZ2 prazdna, vyplo po 20 sekundach
      }
    }
    else {                        // Kdyz prijde nabourana zprava
        Serial.println(F("Unknown command")); // Tak ukaz ze to neznas
    }
  }

  //casovadlo
  delay(2000);                    // Cekame 2s, takze cela smyce pojede 1x za 2 sekundy 
  digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
  if (reltim!=0) {                // Pokud casovadlo jede, budeme casovat
    if ((--reltim)==0) {          // Cukneme a pokud to prave dojelo, 
      relay(0);                   // Vypnem relatko        
    }
  }
}

void relay(uint8_t mode) {        // Funkce pro odeslani retezce
  switch (mode) {                 // Prejem si vypnout
    case 0:                       // Prejdi na pripad, ktereho se nam to tyka
      Serial.println(F("Relay OFF")); // Ukaz ze chceme vypnou rele
      Serial.println();           // Pridej mezeru mezi zpravami na seriovem portu
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, LOW); // Vypni cerpadlo
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, ze prisla zprava
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
  else {                        // Nebo si prejem zapnout
    Serial.println(F("Relay ON"));  // Ukaz ze chceme zapnout rele
    digitalWrite(PIN_RELAY, HIGH);  // Zapni relatko
  } Serial.println();             // Pridej radek mezi jednotlivymi zpravami
}
