/* Cerpadlo:
 *  Po prijmu zpravy "Relay_ON!" sepne rele na cerpadle
 *  Po prijmu zpravy "Relay_OF!" cerpadlo vypne
 *  Po prijmu jakekoli zpravy rozsviti integrovanou LED
 *  V pripade, ze neprichazi zprava, vypni rele po 20 sekundach
*/

// Použíté knihovny:
#include <RH_ASK.h>               // Knihovna ovladace radia
#include <SPI.h>                  // Neni zde pouzito, ale je potreba kompilovat

#define  PIN_RELAY 2              // Kde mame relatko
RH_ASK driver;                    // Objekt ovladace radia
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup() {
  //pripravime si seriak, abychom tam mohli zvracet moudra
  Serial.begin(9600);             // Debugging only
  Serial.println(F("Valkys super RF Cerpadlo driver v1.0\n=================================\n")); // Ukaz na po pripojeni na seriovy port
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace LED_BUILDIN jako výstup
  pinMode(PIN_RELAY, OUTPUT);     // Inicializace digitálního pinu pro RELAY
  relay(0);                       // Vypni to
  reltim=0;                       // Necasujeme 
  if (!driver.init()) Serial.println("Radio driver init failed"); // Nastavení komunikace rádiového přijímače - pin 11 (určeno asi knihovnou)
}

void loop() {
  
  //ja programuju pekne v C, cize nejprve delklarace, pouziti az dale, a asi pouzivam jiny funkce nez normalni arduinisti, treba strcmp ;-)
  uint8_t buf[20];                // Buffer, radeji s rezervickou
  uint8_t buflen;                 // Promenna

  //obsluha radia
  buflen=sizeof(buf);             // Do promenny naperu velikost bufiku, to asi slouzi radio knihovne, aby nejela neka za roh
  if (driver.recv(buf, &buflen)) {  // Pokud nam neco dorazilo po radiu, tak se tomu budeme venovat
    digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime ledku, bo prisla zprava
    Serial.print(F("Message: ")); // Ukazeme
    Serial.println((char*)buf);   // Co to vlastne dorazilo
    if (strcmp("Relay_ON!",buf)==0) { // Pokud je to prikaz pro zapnuti
      relay(1);                   // Zapneme relatko
      reltim=20;                  // A nastavim casovadlo, aby rele v pripade neprijmuti zadne zpravy vyplo po 20 sekundach
    } else if (strcmp("Relay_OF!",buf)==0) { // Pokud je to prikaz pro vypnuti
      relay(0);                   // Okamzite vypneme relatko
      reltim=0;                   // A casovadlo muzem zarazit, ikdyz by to nevadilo, ale je to tak hezcejsi
    } else {                      // Kdyz prijde nabourana zprava
      Serial.println(F("Unknown command")); // Tak ukaz chybu
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

void relay(uint8_t mode) {       // Funkce pro odeslani retezce
  if (mode==0) {                 // Prejem si vypnout
    Serial.println(F("Relay OFF")); // Ukaz ze chceme vypnou rele
    Serial.println();            // Pridej radek mezi jednotlivymi zpravami
    digitalWrite(PIN_RELAY, LOW); // Tak vypinam relatko
    
  } else {                       // Nebo si prejem zapnout
    Serial.println(F("Relay ON"));  // Ukaz ze chceme zapnout rele
    Serial.println();            // Pridej radek mezi jednotlivymi zpravami
    digitalWrite(PIN_RELAY, HIGH); // Zapni relatko
  }
}
