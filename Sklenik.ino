/* Skleník:
 *  Vysli zpravu kazde 2 sekundy
 *  "Relay_ON!" pri podmince tlacitko + horni plovak v pozici HIGH a drž, to dokud není Horní plovák v pozici LOW
 *  "Relay_ON!" pri podmince spodni plovak v pozici LOW + GMT cas je mezi 20:00 a 5:00 (-2 hodiny letniho casu) 
 *  "Relay_OF!" pokud neni neni zadna z predchozich podminek splnena
 *  Serial port ukaze co to odesilame a pomoci RTC modulu DS3231 ukaze cas a stav teploty v krabici ridici jednotky skleniku
 *  Po vyslani jakekoli zprávy rozsviti integrovanou LED
 *  
 */

// Použíté knihovny:
#include <RH_ASK.h>               //Knihovna ovladace radia
#include <SPI.h>                  //Not actually used but needed to compile
#include <RTClib.h>               //Knihovna ovladace RTC (V originalu je teda "RTClib.h", ale snad to nebude vadit)

#define PIN_BUTTON1  2            //Pozice pro tlacitko 1 - zapni relé
#define PIN_PLOV1  3              //Pozice pro horni plovakova senzor
#define PIN_PLOV2  4              //Pozice pro dolni plovakova senzor

// variables will change:
int buttonState = 0;              // variable for reading the pushbutton status
int plov1State = 0;               // proměnná pro výchozí stav horního plovákového senzoru
int plov2State = 0;               // proměnná pro výchozí stav spodního plovákového senzoru
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

RH_ASK driver;                    //Objekt ovladace radia
RTC_DS3231 rtc;                   //Objekt ovladace RTC
uint8_t reltim;                   //Casovadlo, pozor, umi to max 255 vterin (max 0xFF) - Bude zde použito?

void relay(uint8_t mode) {        //Funkce pro odeslani retezce na seriovy port - kontrola co to vlastne odesilame
  if (mode==0) {                  //Prejem si vypnout 
    Serial.println(F("Relay OFF"));  // tak ukaz na seriovem portu "Relay OFF"
  } else {                        //Prejem si zapnout 
    Serial.println(F("Relay ON"));  //tak ukaz na seriovem portu "Relay ON"
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
  relay(0);                       //ve vychozim stavu vysilej "Relay_OF!"
  reltim=0;                       //Necasujeme - bude pouzito? Asi ne 
  if (!driver.init()) Serial.println("init failed"); // nastavení komunikace radioveho vysilace - pin 12 (urceno asi knihovnou)
  if (! rtc.begin()) {            //Nastaveni komunikace RTC modulu 
    Serial.println("Couldn't find RTC");  //Bude drzkovat, kdyz nenajde RTC modul (baterie v modulu by mela vydrzet 8 let) 
    while (1);                    //Kdyz je RTC modul ziv a zdrav, posila data (absenci slozenych zavorek a tu jednicku nechapu)
  }
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
  DateTime time = rtc.now();      //Optej se jaky mame cas
  Serial.print(String("GMT:")+time.timestamp(DateTime::TIMESTAMP_FULL));  //Ukaz GMT cas
  Serial.println();               //Ukonci radek
  Serial.print("Hour: ");         //Ukaz retezec s popisem, ze chceme ukazat hodinu
  Serial.print(time.hour(), DEC);  //Jakou mame hodinu
  Serial.println();               //Ukonci radek
  Serial.print("Temperature: ");  //Ukaz retezec s popisem, ze chceme ukazat teplotu
  Serial.print(rtc.getTemperature());  //Jakou mame teplotu
  Serial.println(" C");           //At ma pan Celsius radost a ukonci radek

  if (buttonState == LOW) {       //Pokud je sepnuto tlacitko pod podminkou
      while (plov1State == HIGH){                        //tak dělej tohle
        digitalWrite(LED_BUILTIN, HIGH);   //Rozsvit integrovanou LED ze odesilam zpravu
        send_msg("Relay_ON!");    //volam funkci odeslani retezce
        Serial.println();         //ukonci radek
        relay(1);                 //ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
        delay(1000);              //Počkej 1000ms
        return 0;
      } 
  }
  else if (plov2State == LOW) {   //Nebo když je spodní plovák sepnut(v obrácené pozici se závažím), a to znamena ze hladina je pod plovakem, vysli "Relay_ON!"
      while (plov1State == HIGH){                        //tak dělej tohle
        if (Serial.print(time.hour(), DEC)<=20);  //pokud mame mene nez 20 hodinu GMT, spust to. Pokud je vic, tak opust funkci (nechceme rusit cerpadlem sousedy)
        if (Serial.print(time.hour(), DEC)>=5);  //pokud mame vice nez 5 hodinu GMT, spust to. Pokud je vic, tak opust funkci (nechceme rusit cerpadlem sousedy) 
        digitalWrite(LED_BUILTIN, HIGH);   //Rozsvit integrovanou LED ze odesilam zpravu
        send_msg("Relay_ON!");    //volam funkci odeslani retezce
        Serial.println();         //Ukonci radek
        relay(1);                 //Ukaz zpravu na seriovym portu, ze chceme zapnout relatko (cerpadlo)
        delay(1000);                //Počkej 50ms
        return 0;
      } 
  } else {                        //Nebo ve vsech ostatnich pripadech vysli "Relay_OF!"
    digitalWrite(LED_BUILTIN, HIGH);  //Rozsvit integrovanou LED ze odesilam zpravu
    send_msg("Relay_OF!");        //Vysli "Relay_OF!"
    Serial.println();             //Ukonci radek
    relay(0);                     //Ukaz zpravu na seriovym portu, ze chceme vypnout relatko (cerpadlo)
    }
    
  delay(5000);                    //Cekej 5s
 }
