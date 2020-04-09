#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

/* Cerpadlo:
 *  Po přijmu zprávy "Relay_ON!" sepne relé na čerpadle po dobu 20 sekund
*/

#define  PIN_RELAY 2     //kde mame relatko
RH_ASK driver; //objekt ovladace radia
uint8_t reltim; //casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void relay(uint8_t mode) {
  if (mode==0) { //prejem si vypnout
    Serial.println(F("Relay OFF"));
    digitalWrite(PIN_RELAY, LOW); //tak vypinam relatko    
  } else { //prejem si zapnout
    Serial.println(F("Relay ON"));
    digitalWrite(PIN_RELAY, HIGH); //zapni relatko
  }
}

void setup() {
  //pripravime si seriak, abychom tam mohli zvracet moudra
  Serial.begin(9600);  // Debugging only
  Serial.println(F("Valkys super RF relay driver v1.0\n=================================\n"));
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // inicializace digitálního pinu pro RELAY
  pinMode(PIN_RELAY, OUTPUT);   
  relay(0); //vypni to
  reltim=0; //necasujeme
  //iniclaizace bezdratu
  if (!driver.init()) Serial.println("Radio driver init failed");
}

void loop() {
  //ja programuju pekne v C, cize nejprve delklarace, pouziti az dale, a asi pouzivam jiny funkce nez normalni arduinisti, treba strcmp ;-)
  uint8_t buf[20];  //bufik, radeji s rezervickou
  uint8_t buflen; //promenna

  //obsluha radia
  buflen=sizeof(buf); //do promenny naperu velikost bufiku, to asi slouzi radio knihovne, aby nejela neka za roh
  if (driver.recv(buf, &buflen)) { //pokud nam neco dorazilo po radiu, tak se tomu budeme venovat
    digitalWrite(LED_BUILTIN, HIGH);   //rozsvitime ledku, bo prisla zprava
    Serial.print(F("Message: ")); //ukazeme
    Serial.println((char*)buf); //co to vlastne dorazilo
    if (strcmp("Relay_BT!",buf)==0) { //pokud je to prikaz pro zapnuti
      relay(1); //zapneme relatko
      reltim=10; //a nastavim casovadlo, aby to po case zdechlo
    } else if (strcmp("Relay_ON!",buf)==0) { //pokud je to prikaz pro vypnuti
      relay(0); //vypneme relatko
      reltim=0; //a casovadlo muzem zarazit, ikdyz by to nevadilo, ale je to tak hezcejsi
    } else if (strcmp("Relay_OF!",buf)==0) { //pokud je to prikaz pro vypnuti
      relay(0); //vypneme relatko
      reltim=0; //a casovadlo muzem zarazit, ikdyz by to nevadilo, ale je to tak hezcejsi
    } else { //je to nejaka kravina,
      Serial.println(F("Unknown command")); //budeme drzkovat
      relay(0); //vypneme relatko
    }
  }
  //casovadlo
  delay(3000); //a cekame 3 vteriny, takze cela smyce (fukci loop vola "system" furt dokola jak debil) pojede 1x za vterinu, takze i casovadlo bude ve vterinach      
  digitalWrite(LED_BUILTIN, LOW); //zhasneme ledku
  if (reltim!=0) {  //pokud casovadlo jede, budeme casovat
    if ((--reltim)==0) { //cukneme a pokud to prave dojelo, 
      relay(0); //a vypnem relatko        
    }
  }    
}
