# Projekt SklenikV2
Zavlažování skleníku pomocí dvou modulů Arduino Nano

Popis projektu:

Návrh zavlažovacího systému skleníku verze 2.

U skleníku je 80 l sud (dále Nádrž 1) ze kterého je přiváděna voda samospádem 7mm trubičkami do kapkových vývodů rozmístěných ve skleníku.
Pokud dochází voda v nádrži u skleniku, radiovým signálem se vyšle povel k sepnutí čerpadla umístěného v 1000 l nádrži (dále Nádrž 2) u domu. Nádrž 2 je zhruba ve vzdálenosti 40m a o cca o 5m níže, než je vtok do nádrže 1. Mezi nádržemi je natažena 1" hadice (zakopána v zemi).

Projekt je rozdělen na dvě části - dvě ovládací jednotky - Skleník/Čerpadlo:

## 1. Skleník (neboli Nádrž 1) je jednotka pro sledování stavu hladiny nádrže 1 a vysílač. Obsahuje:
    - Arduino Nano - řídící jednotka
    - RTC modul DS3231
    - Zdroj 5V - Solární panel, stabilizátor napětí + baterie (na pin 5V) - tak daleko od baráku totiž elektriku nemam (a je to eko)
    - Vysílač DR3100 - 433Mhz (Pin 12) pro vysílání stavu Nádrže 2 a pokynů z tlačítka
    - Horní plovákový senzor PLOV1 (Pin 3) - detekce naplnění nádrže 1 (ochrana proti přetečení)
    - Spodní plovákový senzor PLOV2 (Pin 4) - detekce prázdné nádrže 1
    - Tlačítko 1 (Pin 2) - pro okamžité spuštění čerpadla (například v případě dolévání z konve) - čerpadlo nemůže sepnout, pokud horní   plovákový senzor detekuje naplnění Nádrže 1
    
 #####  Později (rozuměj "Nice to have" v plánu do budoucna):
        - Tlačíko 2 - pro sepnutí i vypnutí čerpadla bez ohledu na stav nádrže pro zalévání okolních záhonků
        - Display LCD 1602 - zobrazování aktuálních hodnot senzorů níže
        - I2C modul pro řízení LCD přes 4 dráty
        - Teplotní senzor - venkovní (sleduje teplotu mimo skledník)
        - Teplotní senzor - vnitřní (sleduje teplotu uvnitř skleníku)
        - Vlhkoměr vzduchu - vnitřní (sleduje vlhkost uvnitř skleníku)
        - 3x půdní vlhkoměr
        - Ovládání oken - odvětrávání pomocí pneu/hydro zvedáků
        - Teplotní senzor - Sleduje teplotu v krabici ovládací jednotky - stříbrná páaska po obvodu asi postačí
        - Větráček - odvětrávání v případě vyšších teplot uvnitř krabice ovládací jednotky - zatím umístíme jednotku na stinné místo
    
## 2. Čerpadlo (neboli Nádrž 2) je jednotka pro sledování hladiny nádrže 2, přijímač a ovladač čerpadla. Obsahuje:
    - Arduino Nano - zpracování signálu ze Skleníku a ovládání relé
    - Trafo zdroj 230V / 5V
    - Přijímač DR3100 - 433Mhz (Pin 11) se spirálovou anténou pro příjem signálu ze skleníku
    - Horní plovákový senzor PLOV3 (Pin 3) - detekce naplnění nádrže 2 (pokud je sepnuto, hrozí přeplnění - pokud není nádrž 1 zcela plná, sepne čerpadlo k jejímu doplnění)
    - Spodní plovákový senzor PLOV4 (Pin 4) - detekce prázdné nádrže 2
    - Relé (Pin D2) - spíná čerpadlo
    - Přívodní kabel 230V na relé 
    - Čerpadlo se zabudovaným plovákovým senzorem (zatím nespecifikováno)
