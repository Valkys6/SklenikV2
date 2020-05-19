# Projekt SklenikV2
Zavlažování skleníku pomocí dvou modulů Arduino Nano

Popis projektu:

Návrh zavlažovacího systému skleníku verze 2.

U skleníku je 120 l sud (dále NADRZ1) ze kterého je přiváděna voda samospádem 4mm trubičkami (vnitří průměr) do kapkových vývodů rozmístěných ve skleníku.
Pokud dochází voda v nádrži u skleniku, radiovým signálem se vyšle povel k sepnutí čerpadla umístěného v kubíkové nádrži (dále NADRZ2) u domu. NADRZ2 je zhruba ve vzdálenosti 40m a o cca o 5m níže, než je vtok do nádrže 1. Mezi nádržemi je natažena 1" hadice (zakopána v zemi).

Projekt je rozdělen na dvě části - dvě ovládací jednotky - Skleník/Čerpadlo:

## 1. Skleník (NADRZ1) je jednotka pro sledování stavu hladiny v sudu u sklenáku a vysílač. Obsahuje:
    - Arduino Nano - řídící jednotka
    - RTC modul DS3231
    - Zdroj 5V - Solární panel 12V, stabilizátor napětí + nabíječka baterií + baterie (na pin 5V) + StepUp convertor 5V (tak daleko od baráku totiž elektriku nemam (a je to eko))
    - Vysílač DR3100 - 433Mhz (Pin 12)
    - Horní plovákový senzor PLOV1 (Pin 3) - detekce naplnění NADRZ1 (ochrana proti přetečení)
    - Spodní plovákový senzor PLOV2 (Pin 4) - detekce prázdné NADRZ1
    - Tlačítko 1 (Pin 2) - pro okamžité spuštění čerpadla (například v případě dolévání z konve) - čerpadlo nemůže sepnout, pokud horní   plovákový senzor detekuje naplnění NADRZ1
    - Přepínač 1 (Pin 5) - pro okamžité spuštění čerpadla (pro případ zalevání okolních záhonků z hadice) - čerpadlo může sepnout bez ohledu na naplnění NADRZ1. Čerpadlo nesepne, pokud je NADRZ2 prázdná.
    
 #####  Později (plány do budoucna):
        - Průtokoměr ke sledování tlaku za čerpadlem. Slouží k ochraně čerpadla, aby mohlo vypnout, pokud jsou všechny výstupní ventily uzavřeny
        - Display LCD 1602 - zobrazování aktuálních hodnot senzorů níže
        - I2C modul pro řízení LCD přes 4 dráty
        - Teplotní senzor - venkovní (sleduje teplotu mimo skledník)
        - Teplotní senzor - vnitřní (sleduje teplotu uvnitř skleníku)
        - Vlhkoměr vzduchu - vnitřní (sleduje vlhkost uvnitř skleníku)
        - Půdní vlhkoměr
        - Ovládání oken - odvětrávání pomocí pneu/hydro zvedáků
    
## 2. Čerpadlo (NADRZ2) je jednotka pro sledování hladiny nádrže u domu, přijímač a ovladač čerpadla. Obsahuje:
    - Arduino Nano - zpracování signálu ze Skleníku a ovládání relé
    - Trafo zdroj 230V / 5V
    - Přijímač DR3100 - 433Mhz (Pin 11) pro příjem signálu ze skleníku
    - Spodní plovákový senzor PLOV3 (Pin 3) - detekce prázdné nádrže 2
    - Horní plovákový senzor PLOV4 (Pin 4) - detekce naplnění nádrže 2 (pokud je sepnuto, hrozí přeplnění - pokud není nádrž 1 zcela plná, sepne čerpadlo k jejímu doplnění)
    - Spínač 2 - pro sepnutí čerpadla pokud není NADRZ2 prázdná, i jeho vypnutí. Slouží pro zalévání okolních záhonků NADRZ2
    - Relé (Pin D2) - spíná čerpadlo
    - Přívodní kabel 230V na relé 
    - Čerpadlo (bude specifikováno)
