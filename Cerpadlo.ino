/* Cerpadlo:
 *  Po přijmu zprávy "Relay_ON!" sepne relé na čerpadle
 *  Po příjmu zprávy "Relay_OF!" čerpadlo vypne
 *  Po příjmu jakékoli zprávy rozsvítí integrovanou LED
 *  V případě, že nepřichází zpráva, vypni relé po 10 sekundách
*/

// Použíté knihovny:
#include <RH_ASK.h>
#include <SPI.h>                 // Not actually used but needed to compile

#define  PIN_RELAY 2             //kde mame relatko
RH_ASK driver;                   //objekt ovladace radia
uint8_t reltim;                  //casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void relay(uint8_t mode) {       //Funkce pro odeslani retezce
  if (mode==0) {                 //prejem si vypnout
    Serial.println(F("Relay OFF"));
    digitalWrite(PIN_RELAY, LOW); //tak vypinam relatko    
  } else { //prejem si zapnout
    Serial.println(F("Relay ON"));
    digitalWrite(PIN_RELAY, HIGH); //zapni relatko
  }
}

void setup() {
  //pripravime si seriak, abychom tam mohli zvracet moudra
  Serial.begin(9600);            //Debugging only
  Serial.println(F("Valkys super RF Cerpadlo driver v1.0\n=================================\n"));
  pinMode(LED_BUILTIN, OUTPUT);  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PIN_RELAY, OUTPUT);    // inicializace digitálního pinu pro RELAY
  relay(0);                      //vypni to
  reltim=0;                      //necasujeme 
  if (!driver.init()) Serial.println("Radio driver init failed"); // nastavení komunikace rádiového přijímače - pin 11 (určeno asi knihovnou)
}

void loop() {
  
  //ja programuju pekne v C, cize nejprve delklarace, pouziti az dale, a asi pouzivam jiny funkce nez normalni arduinisti, treba strcmp ;-)
  uint8_t buf[20];               //bufik, radeji s rezervickou
  uint8_t buflen;                //promenna

  //obsluha radia
  buflen=sizeof(buf);            //do promenny naperu velikost bufiku, to asi slouzi radio knihovne, aby nejela neka za roh
  if (driver.recv(buf, &buflen)) { //pokud nam neco dorazilo po radiu, tak se tomu budeme venovat
    digitalWrite(LED_BUILTIN, HIGH);   //rozsvitime ledku, bo prisla zprava
    Serial.print(F("Message: ")); //ukazeme
    Serial.println((char*)buf);   //co to vlastne dorazilo
    if (strcmp("Relay_ON!",buf)==0) { //pokud je to prikaz pro zapnuti
      relay(1);                   //zapneme relatko
      reltim=10;                  //a nastavim casovadlo, aby to po case zdechlo
    } else if (strcmp("Relay_OF!",buf)==0) { //pokud je to prikaz pro vypnuti
      relay(0);                   //vypneme relatko
      reltim=0;                   //a casovadlo muzem zarazit, ikdyz by to nevadilo, ale je to tak hezcejsi
    } else {                      //je to nejaka kravina,
      Serial.println(F("Unknown command")); //budeme drzkovat
    }
  }
  //casovadlo
  delay(1000);                    //a cekame vterinu, takze cela smyce (fukci loop vola "system" furt dokola jak debil) pojede 1x za vterinu, takze i casovadlo bude ve vterinach      
  digitalWrite(LED_BUILTIN, LOW); //zhasneme ledku
  if (reltim!=0) {                //pokud casovadlo jede, budeme casovat
    if ((--reltim)==0) {          //cukneme a pokud to prave dojelo, 
      relay(0);                   //a vypnem relatko        
    }
  }    
}
