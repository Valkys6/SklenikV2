# Projekt Sklenik
Zavlažování skleníku pomocí dvou modulů Arduino Nano

Popis projektu:
```
Návrh zavlažovacího systému skleníku.

U skleníku je sud ze kterého je přiváděna voda samospádem 8mm trubičkami do kapkových vývodů rozmístěných ve skleníku.
Pokud dochází voda v sudu, radiovým signálem se vyšle povel k sepnutí čerpadla umístěného v 1000 l nádrži. Nádrž je zhruba 40m od sudu a o zhruba 5m níže, než je vtok do sudu.

Projekt je rozdělen na dvě části - dvě ovládací jednotky - Skleník/Čerpadlo:

## 1. Skleník - řídící jednotka (sběrnice dat a vysílač) obsahuje:
    - Arduino Nano - řídící jednotka
    - Zdroj 5V - Solární panel, stabilizátor napětí + baterie (na pin Uin) - tak daleko od baráku totiž elektriku nemam
    - Vysílač 433Mhz (Pin 12) se spirálovou anténou pro vysílání stavu sudu a pokynů z tlačítka
    - Horní plovákový senzor (Pin 3) - detekce naplnění sudu (ochrana proti přetečení)
    - Spodní plovákový senzor (Pin 4) - detekce prázdného sudu
    - Tlačítko (Pin 2)- pro okamžité spuštění čerpadla (například v případě dolévání z konve) - čerpadlo nemůže sepnout, pokud horní   plovákový senzor detekuje naplněný sud
    
 #####  Později - (rozuměj "Nice to have" ale asi je vypustim neb času je málo):
        - Display LCD 1602 - zobrazování aktuálních hodnot senzorů níže
        - I2C modul pro řízení LCD přes 4 dráty
        - Teplotní senzor - venkovní (sleduje teplotu mimo skledník)
        - Teplotní senzor - vnitřní (sleduje teplotu uvnitř skleníku)
        - Vlhkoměr vzduchu - vnitřní (sleduje vlhkost uvnitř skleníku)
        - 3x půdní vlhkoměr
        - Teplotní senzor - Sleduje teplotu v krabici ovládací jednotky - stříbrná páaska po obvodu asi postačí
        - Větráček - odvětrávání v případě vyšších teplot uvnitř krabice ovládací jednotky
    
## 2) Čerpadlo - přijímač (přijímač, spínání napájení čerpadla)
    - Arduino Nano - čtení siggnálu ze Skleníku a ovládání relé
    - Trafo zdroj 230V / 5V
    - Přijímač 433Mhz (Pin 11) se spirálovou anténou pro příjem signálu ze skleníku
    - Relé (Pin D2) - spíná čerpadlo
    - Přívodní kabel 230V na relé 
    - Čerpadlo se zabudovaným plovákovým senzorem (zatím nespecifikováno

