/* Skleník:
 *  Vyšli zprávu "Relay_ON!" (tlačítko) a sepni čerpadlo po dobu 10 sekund
 *  Čerpaldlo čte zprávu každé 3 sekundy
 *  Pokud nenbylo tlašítko sepnuto během 5s intervalu, vypni relé = pošli ("Relay_OF!")
 */

// Použíté knihovny:
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

//pozice pro tlačítko
const int buttonPin = 2;     // the number of the pushbutton pin
//pozice pro horní plovákový senzor
const int plov1Pin = 3;
//pozice pro dolní plovákový senzor
const int plov2Pin = 4;
// pozice pro integrovanou LED:
const int ledPin =  13;      // the number of the LED pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int plov1State = 0;            // proměnná pro výchozí stav horního plovákového senzoru
int plov2State = 0;            // proměnná pro výchozí stav spodního plovákového senzoru

RH_ASK driver;
void setup()
{
  // initialize the LED pin as an output:
    pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
    pinMode(buttonPin, INPUT);
  // inicializace horního plovákového senzoru - plov1Pin
    pinMode(plov1Pin, INPUT);
  // inicializace spodního plovákového senzoru - plov2Pin
    pinMode(plov2Pin, INPUT);
  // nastavení komunikace rádiového vysílače - pin 12 (určeno asi knihovnou)
    Serial.begin(9600);    // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

//Funkce pro odeslani retezce
void send_msg(const char* msg) {
  driver.send((uint8_t *)msg, strlen(msg)); //posli to
  driver.waitPacketSent();  //cekej, az to bude cely venku                             
  delay(1000); //flakej se 1 s
}

//Hlavni smyce arduina
void loop() {
    // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // čti status horního plováku
  plov1State = digitalRead(plov1Pin);
 // čti status spodního plováku
  plov2State = digitalRead(plov2Pin);

  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState == LOW) {
  // pokud je splněna podmínka přeskoč na:
  if (plov1State == LOW) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    goto Relay_BT;
  }
 } 
  else {
  // pokud není splněna podmínka přeskoč na:
  goto Relay_OFF;
}

// GoTo funkce pro odeslání "Relay_BT!" při stisknutí tlačítka - odešle 5 x zprávu "Relay_BT!" po 1s  
Relay_BT:
    digitalWrite(ledPin, HIGH);
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    delay(1000); // čekej 1000ms
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    delay(1000); // čekej 1000ms
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    delay(1000); // čekej 1000ms
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    delay(1000); // čekej 1000ms
    send_msg("Relay_BT!");  //volam funkci odeslani retezce
    delay(1000); // čekej 1000ms

// GoTo funkce pro odeslání "Relay_ON!" při stavu plováku 2 na pozici HIGH (Nízká hladina)
Relay_ON:
    digitalWrite(ledPin, HIGH);
    send_msg("Relay_ON!");  //volam funkci odeslani retezce
    delay(10000);   // čekej 10 s do další akce
// GoTo funkce pro odeslání "Relay_OF!"
Relay_OFF:
    digitalWrite(ledPin, LOW);
    send_msg("Relay_OF!");  //volam funkci odeslani retezce
}
