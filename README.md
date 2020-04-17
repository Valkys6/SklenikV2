# Sklenik
Zavlažování skleníku pomocí dvou modulů Arduino Nano

Popis projektu:
Návrh zavlažovacího systému skleníku.
U skleníku je sud (cca 30m od domu) a ten je doplňován z 1000l nádrže s dešťovou vodou.


Projekt je rozdělen na dvě části - dvě ovládací jednotky - Skleník/Čerpadlo:
1) Skleník - řídící jednotka (sběrnice dat a vysílač)
Obsahuje:
a) Arduino Nano - řídící jednotka
b) Zdroj 5V - Solární panel, stabilizátor napětí + baterie (na pin Uin) - tak daleko od baráku totiž elektriku nemam
c) Vysílač 433Mhz (Pin 12)- pro vysílání stavu sudu a pokynů z tlačítka
d) Horní plovákový senzor (Pin 3) - detekce naplnění sudu (ochrana proti přetečení)
e) Spodní plovákový senzor (Pin 4) - detekce prázdného sudu
f) Tlačítko (Pin 2)- pro okamžité spuštění čerpadla (například v případě dolévání z konve) - čerpadlo nemůže sepnout, pokud horní plovákový senzor detekuje naplněný sud
    
   Později - (rozuměj "Nice to have" ale asi je vypustim bo neni času p***) Nutný přechod na Arduino Mega??:
    f) Display - zobrazování aktuálních hodnot senzorů níže
    g) Teplotní senzor - venkovní (sleduje teplotu mimo skledník)
    h) Teplotní senzor - vnitřní (sleduje teplotu uvnitř skleníku)
    i) Vlhkoměr vzduchu - vnitřní (sleduje vlhkost uvnitř skleníku)
    j) 3x půdní vlhkoměr
    k) Teplotní senzor - Sleduje teplotu v krabici ovládací jednotky - stříbrná páaska po obvodu asi postačí
    l) Větráček - odvětrávání v případě vyšších teplot uvnitř krabice ovládací jednotky
    
2) Čerpadlo - přijímač (přijímač, spínání napájení čerpadla)
  a) Arduino Nano - čtení siggnálu ze Skleníku a ovládání relé 
  b) Přijímač 433Mhz (Pin 11)- pro příjem signálu ze skleníku
  c) Relé (Pin D2) - spíná čerpadlo
  

