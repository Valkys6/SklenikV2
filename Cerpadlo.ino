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
 *          c. pokud je sepnut prepinac (PIN_SWITCH2 = HIGH). !!!Nefunguje, pokud je i na NADRZ1 sepnut prepinac (R. To slouzi jako ochrana proti sepnuti ze dvou mist (osetreno samotnou strukturou kodu)
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
 *    4) Rozvsiti LED3 (cervena) nebo LED4 (zluta) pokud:
 *          a. PIN_LED3 = HIGH: je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
 *          b. PIN_LED4 = HIGH: je prepinac 1 na NADRZ1 sepnut a prichazi zprava "Relay_04!" na NADRZ1 sepnut)
 *          
 *    !!!Program se opakuje kazde 2 sekundy!!!
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
#define PIN_LED3  6               // Pozice pro LED3 (cervena) k indikaci, ze prepinac 2 je sepnut
#define PIN_LED4  7               // Pozice pro LED4 (zluta) k indikaci, ze prepinac 1 na NADRZ1 je sepnut

// Pouzite ovladace periferii
RH_ASK driver;                    // Objekt ovladace radia
uint8_t reltim;                   // Casovadlo, pozor, umi to max 255 vterin (max 0xFF)

void setup() {
  // Pripravime si seriak, abychom tam mohli ukazovat moudra
  Serial.begin(9600);             // Nastav rychlost prenosu
  Serial.println(F("Valkys super RF Cerpadlo driver v1.0\n=================================\n")); // Ukaz na po pripojeni na seriovy port

  // Nastav vystupy
  pinMode(LED_BUILTIN, OUTPUT);   // Inicializace LED_BUILDIN jako vystup
  pinMode(PIN_LED3, OUTPUT);      // Inicializace PIN_LED3 jako vystup
  pinMode(PIN_LED4, OUTPUT);      // Inicializace PIN_LED4 jako vystup
  pinMode(PIN_RELAY, OUTPUT);     // Inicializace digitálního pinu pro RELAY
  relay(0);                       // Defaultne je rele vypnute
  reltim=0;                       // Necasujeme
  
  // Nastav vstupy
  pinMode(PIN_SWITCH2, INPUT);    // Inicializace prepinace jako vstup
  pinMode(PIN_PLOV3, INPUT);      // Inicializace horniho plovakoveho senzoru - PIN_PLOV3
  pinMode(PIN_PLOV4, INPUT);      // Inicializace dolniho plovakoveho senzoru - PIN_PLOV4
  
  // Ukaz pokud nedostanes zpravu z periferii
  if (!driver.init()) Serial.println("Radio driver init failed"); // Nastavení komunikace rádiového přijímače - pin 11 (určeno asi knihovnou)
}

void loop() {
  uint8_t buf[20];                 // Deklarace bufferu, radeji s rezervickou
  uint8_t buflen;                 // Promenna bufferu

  //obsluha radia
  buflen=sizeof(buf);             // Do promenny naperu velikost bufiku, to asi slouzi radio knihovne, aby nejela nekam za roh
  if (driver.recv(buf, &buflen)) {  // Pokud nam neco dorazilo po radiu, tak se tomu budeme venovat
    Serial.print(F("Message: ")); // Ukazeme
    Serial.println((char*)buf);   // Co to vlastne dorazilo
    // Podminka pro "Relay_00!"
    if (strstr(buf,"Relay_00!") == buf) { // Cerpadlo nemuze automaticky sepnout, pokud je NADRZ1 plna
        relay(0);                 // Skocime na relay case 0
        reltim=0;                 // A casovadlo muzem zarazit
    }
    // Podminka pro "Relay_01!" + prepinac 2 nesepnut
    else if ((strstr(buf,"Relay_01!") == buf) && (digitalRead(PIN_PLOV3) == LOW) && (digitalRead(PIN_SWITCH2) == LOW)) { // Cerpadlo je vypnute, ale je pripraveno sepnout. Ceka, jestli bude prepinac 2 sepnut (PIN_SWITCH2 = HIGH). Pokud se stane, je cerpadlo sepnuto, dokud neobdrzi "Relay_00!", nebo dokud neni NADRZ2 prazdna (PIN_PLOV3 = LOW)
      relay(1);                   // Skocime na relay case 1
      while (digitalRead(PIN_PLOV3) == LOW) {  // Opakuj, dokud neni NARDZ2 prazdna, nebo dokud je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
      }
      reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy (a nebo je NADRZ2 vycerpana) vyplo po 20 sekundachch
    }
    // Podminka pro "Relay_01!" + prepinac 2 sepnut
    else if ((strstr(buf,"Relay_01!") == buf) && (digitalRead(PIN_PLOV3) == LOW) && (digitalRead(PIN_SWITCH2) == HIGH)) {  // Cerpadlo je vypnute, ale je pripraveno sepnout. Ceka, jestli bude prepinac 2 sepnut (PIN_SWITCH2 = HIGH). Pokud se stane, je cerpadlo sepnuto, dokud neobdrzi "Relay_00!", nebo dokud neni NADRZ2 prazdna (PIN_PLOV3 = LOW)
      relay(2);                   // Skocime na relay case 2
      while (digitalRead(PIN_PLOV3) == HIGH) { // Opakuj, dokud neni NARDZ2 prazdna, nebo dokud je prepinac 2 sepnut (PIN_SWITCH2 = HIGH)
      }
      reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy (a nebo je NADRZ2 vycerpana) vyplo po 20 sekundachch
    }
    // Podminka pro "Relay_02!"
    else if ((strstr(buf,"Relay_02!") == buf) && (digitalRead(PIN_PLOV3) == LOW)) { // Pokud je to prikaz pro zapnuti a pokud neni NADRZ2 prazdna
      relay(3);                   // Skocime na relay case 3
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud neprijde jina zprava nebo NADRZ2 neni prazdna
      }
      reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy (a nebo je NADRZ2 vycerpana) vyplo po 20 sekundachch
    }
    // Podminka pro "Relay_03!"
    else if ((strstr(buf,"Relay_03!") == buf) && (digitalRead(PIN_PLOV3) == LOW)) { // Pokud je to prikaz pro zapnuti a pokud neni NADRZ2 prazdna
      relay(4);                 // Skocime na relay case 4
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud neprijde jina zprava nebo NADRZ2 neni prazdna
      }
    reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy (a nebo je NADRZ2 vycerpana) vyplo po 20 sekundachch
    }
    // Podminka pro "Relay_04!"
    else if ((strstr(buf,"Relay_04!") == buf) && (digitalRead(PIN_PLOV3) == LOW)) { // Pokud je to prikaz pro zapnuti a pokud neni NADRZ2 prazdna
      relay(5);                 // Skocime na relay case 5
      while (digitalRead(PIN_PLOV3) == LOW) { // Opakuj, dokud neprijde jina zprava nebo NADRZ2 neni prazdna
      }
      reltim=20;                // Nastavime casovadlo, aby rele v pripade neprijmuti zadne zpravy (a nebo je NADRZ2 vycerpana) vyplo po 20 sekundachch
    }
    else {                        // Kdyz prijde nabourana nebo nedefinovana zprava
        Serial.println(F("Unknown command")); // Ukaz ze zpravu neznas
    }
  }

  //casovadlo
  delay(1000);                    // Cekame 1s, takze cely program pojede 1x za sekundu 
  digitalWrite(LED_BUILTIN, LOW); // Zhasneme ledku
  if (reltim!=0) {                // Pokud casovadlo jede, budeme casovat
    if ((--reltim)==0) {          // Cukneme a pokud to prave dojelo, 
      relay(0);                   // Vypnem relatko        
    }
  }
}

//
void relay(uint8_t mode) {        // Funkce pro odeslani retezce na seriovy port, ovladani rele a LED vystupu (LED_BUILDIN, LED1, LED2)
  switch (mode) {                 // Prejdi na pripad, ktereho se to tyka
    case 0:                       // Zprava "Relay_00"
      Serial.println(F("Relay OFF")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, LOW); // Vypni cerpadlo
      digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN, ze prisla zprava
      delay(100);                  // Pockej 50ms
      digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
      delay(100);                  // Pockej 50ms
      break;
    case 1:                       // Zprava "Relay_01" + prepinac 2 nesepnut
      Serial.println(F("Relay possible to ON")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, LOW); // Vypni cerpadlo
      for (int x = 0; x < 2; x++) { // Blikni LED_BUILDIN 2x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 100ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 100ms
      }
      break;
    case 2:                       // Zprava "Relay_01" + prepinac 2 sepnut
      Serial.println(F("Relay ON by SWITCH2")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED3, HIGH); // Rozsvit LED3 dokud neprijde jina zprava
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, HIGH);  // Zapni cerpadlo
      for (int x = 0; x < 2; x++) { // Blikni LED_BUILDIN 2x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 100ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 100ms
      }
      break;
    case 3:                       // Zprava "Relay_02"
      Serial.println(F("Relay ON by BUTTON1")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, HIGH);  // Zapni cerpadlo
      for (int x = 0; x < 3; x++) { // Blikni LED_BUILDIN 3x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 100ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 100ms
      }
      break;
    case 4:                       // Zprava "Relay_03"
      Serial.println(F("Relay ON by BUTTON1")); // Ukaz zpravu na seriovem portu
      Serial.println();           // Pridej radek mezi jednotlivymi zpravami
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, LOW);  // Zhasni LED4
      digitalWrite(PIN_RELAY, HIGH);  // Zapni cerpadlo
      for (int x = 0; x < 4; x++) { // Blikni LED_BUILDIN 4x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 100ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 100ms
      }
      break;
    case 5:                       // Zprava "Relay_04"
      Serial.println(F("Relay ON by SWITCH1")); // Ukaz zpravu na seriovem portu
      Serial.println(); 
      digitalWrite(PIN_LED3, LOW);  // Zhasni LED3
      digitalWrite(PIN_LED4, HIGH); // Rozsvit LED4 dokud neprijde jina zprava
      digitalWrite(PIN_RELAY, HIGH);  // Zapni cerpadlo
      for (int x = 0; x < 5; x++) { // Blikni LED_BUILDIN 4x
        digitalWrite(LED_BUILTIN, HIGH);  // Rozsvitime LED_BUILDIN
        delay(100);                // Pockej 100ms
        digitalWrite(LED_BUILTIN, LOW); // Zhasneme LED_BUILDIN
        delay(100);                // Pockej 100ms
      }
      break;
    default:                      // Kdyz stav neznam (chybovy stav)
      return;                     // Tak ukonci smycy a zacni znovu
  }
}
