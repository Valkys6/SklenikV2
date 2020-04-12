/* Skleník:
 *  Vyšli zprávu každé 2 sekundy
 *  "Relay_ON!" při podmínce tlačítko + horní plovák v pozici HIGH
 *  "Relay_ON!" při podmínce spodní plovák v pozici LOW
 *  "Relay_OF!" pokud není některá z podmínek splněna
 *  Po příjmu jakékoli zprávy rozsvítí integrovanou LED
 */

// Použíté knihovny:
#include <RH_ASK.h>               //Knihovna ovladace radia
#include <SPI.h>                  //Not actually used but needed to compile
#include <RTClib.h>               //Knihovna ovladace RTC (V originalu je teda "RTClib.h", ale snad to nebude vadit)

#define PIN_BUTTON1  2            //Pozice pro tlacitko 1 - zapni relé
#define PIN_PLOV1  3              //Pozice pro horni plovakova senzor
#define PIN_PLOV2  4              //Pozice pro dolni plovakova senzor

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int plov1State = 0;            // proměnná pro výchozí stav horního plovákového senzoru
int plov2State = 0;            // proměnná pro výchozí stav spodního plovákového senzoru
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

RH_ASK driver;                    //Objekt ovladace radia
RTC_DS3231 rtc;                   //Objekt ovladace RTC
uint8_t reltim;                   //Casovadlo, pozor, umi to max 255 vterin (max 0xFF) - Bude zde použito?

void relay(uint8_t mode) {        //Funkce pro odeslani retezce na seriovy port - kontrola co to vlastne odesilame
  if (mode==0) {                  //Prejem si vypnout tak ukaž na seriovém portu "Relay OFF"
    Serial.println(F("Relay OFF"));
  } else {                        //Prejem si zapnout tak ukaž na seriovém portu "Relay ON"
    Serial.println(F("Relay ON"));
  }
}

void setup()  {
  //pripravime si seriak, abychom tam mohli zvracet moudra, které zvracíme po rádiu
  Serial.begin(9600);             //Debugging only
  Serial.println(F("Valkys super RF Sklenik driver v1.0\n=================================\n"));
  pinMode(LED_BUILTIN, OUTPUT);   //Inicializace LED_BUILDIN jako výstup
  pinMode(PIN_BUTTON1, INPUT);    //Inicilazace tlacitka jako vstup
  pinMode(PIN_PLOV1, INPUT);      //Inicializace horniho plovakoveho senzoru - PIN_PLOV1
  pinMode(PIN_PLOV2, INPUT);      //Inicializace spodniho plovakoveho senzoru - PIN_PLOV2
  relay(0);                       //Vypni to
  reltim=0;                       //Necasujeme 
  if (!driver.init()) Serial.println("init failed"); // nastavení komunikace radioveho vysilace - pin 12 (urceno asi knihovnou)
}

void send_msg(const char* msg) {  //Funkce pro odeslani retezce
  driver.send((uint8_t *)msg, strlen(msg)); //posli to
  driver.waitPacketSent();        //Cekej, az to bude cely venku                             
  delay(2000);                    //Flakej se 2s
}

void loop() {                     //Hlavni smyce arduina

  buttonState = digitalRead(PIN_BUTTON1);  //Cti status tlacitka 1
  plov1State = digitalRead(PIN_PLOV1);     //Cti status horniho plovaku
  plov2State = digitalRead(PIN_PLOV2);     //Cti status spodniho plovaku

  if (buttonState == LOW) {       //Pokud je sepnuto tlacitko pod podminkou
      do {                        //tak dělej tohle
        //if (plov1State == HIGH)   //zkontroluj jestli sud není úplně plnej a pokud ne
        digitalWrite(LED_BUILTIN, HIGH);   //Rozsvit integrovanou LED ze odesilam zpravu
        send_msg("Relay_ON!");    //volam funkci odeslani retezce
        relay(1);                 //ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
        delay(50);                //Počkej 50ms
      } while (plov1State == LOW); { //dokud není sud plnej
        }
  }
  else if (plov2State == LOW) {   //Nebo když je spodní plovák sepnut(v obrácené pozici se závažím), a to znamena ze hladina je pod plovakem, vysli "Relay_ON!"
      do {                        //tak dělej tohle
        //if (plov1State == HIGH)   //zkontroluj jestli sud není úplně plnej a pokud ne
        digitalWrite(LED_BUILTIN, HIGH);   //Rozsvit integrovanou LED ze odesilam zpravu
        send_msg("Relay_ON!");    //volam funkci odeslani retezce
        relay(1);                 //ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
        delay(50);                //Počkej 50ms
      } while (plov1State == LOW); { //dokud není sud plnej
        }
  } else {                        //Nebo ve vsech ostatnich pripadech vysli "Relay_OF!"
    digitalWrite(LED_BUILTIN, HIGH);  //Rozsvit integrovanou LED ze odesilam zpravu
    send_msg("Relay_OF!");        //Vysli "Relay_OF!"
    relay(0);                     //ukaz zpravu na seriovym portu, ze chceme vypnout relatko (cerpadlo)
    }
 }
