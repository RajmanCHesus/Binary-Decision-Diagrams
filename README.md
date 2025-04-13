# Hlavný cieľ zadania

Cieľom je vytvoriť program, ktorý umožňuje zostaviť dátovú štruktúru reprezentujúcu BDD. Táto dátová štruktúra by mala slúžiť ako efektívny prostriedok na reprezentáciu a vyhodnocovanie Booleovských funkcií, pričom musí byť zároveň redukovaná (teda zbytočné/duplicitné uzly musia byť odstránené).

## Požadované funkcie

### BDD_create

#### Úlohy:

- Vytvorenie BDD na základe zadaného výrazu Booleovskej funkcie (napr. v DNF forme).
- Zohľadnenie špecifikovaného poradia premenných (môže ísť o pole, zoznam, alebo iný dátový typ), v akom sa budú prechádzať premenné funkcie počas zostavovania diagramu.
- Výstupom je ukazovateľ na štruktúru BDD, ktorá musí obsahovať aspoň počet premenných, veľkosť BDD (počet uzlov) a ukazovateľ na koreň stromu.

**Poznámka:**

Výraz (bfunkcia) musí byť spracovaný priamo, nie prostredníctvom vektorov – v opačnom prípade hrozí odpočítanie 5 bodov.

Redukcia diagramu môže prebiehať priebežne (ideálna možnosť) alebo až po kompletnej výstavbe diagramu. Ak sa redukcia vykonáva len po kompletnej výstavbe, riešenie je penalizované (miesto efektivity – 5 bodov).

### BDD_create_with_best_order

#### Úlohy:

- Nájsť optimálne poradie premenných, ktoré minimalizuje počet uzlov výsledného BDD.
- K tomu sa opakovane volá funkcia BDD_create s rôznymi poradiami. Riešenie môže skúšať napríklad cyklické permutácie alebo dokonca všetky možné permutácie, pričom musí byť vyskúšaných minimálne N rôznych poradí (kde N je počet premenných).
- Výsledný BDD, ktorý má najnižší počet uzlov, sa potom použije ako optimálna reprezentácia funkcie.

**Bodové ohodnotenie:**

2 body.

### BDD_use

#### Úlohy:

- Vyhodnotenie vytvoreného BDD na základe dodanej kombinácie hodnôt vstupných premenných.
- Funkcia prechádza diagramom od koreňa k listu podľa reťazca vstupov (napr. "1010", kde každý index reprezentuje konkrétnu premennú) a vráti výsledok ako znak '1' alebo '0'.
- V prípade chyby (napríklad chybné vstupy) vráti zápornú hodnotu (napr. -1).

**Bodové ohodnotenie:**

1 bod.

## Testovanie a hodnotenie efektívnosti

### Automatizované testovanie:

- Náhodné generovanie Booleovských funkcií pre testovanie (minimálne 100 rôznych funkcií pre rovnaký počet premenných).
- Pre každý generovaný BDD overenie správnosti pomocou postupného dosadzovania všetkých možných kombinácií vstupov a porovnávanie výsledkov.
- Kľúčové je otestovať riešenie aspoň pre 13 premenných, pričom efektívnosť redukcie (percentuálna miera odstránených uzlov) a extra redukcia dosiahnutá pomocou BDD_create_with_best_order by mali byť vyhodnotené a zdokumentované.

### Meriace nástroje a analýza:

- Testovací program (funkcia main) musí obsahovať meranie času vykonania a hodnotenie veľkosti BDD v závislosti od počtu premenných.
- Výsledky testovania by mali obsahovať priemerné percento zredukovaných uzlov a (priemerný) čas vykonania pre obe hlavné funkcie (BDD_create a BDD_create_with_best_order).

## Dokumentácia

Okrem implementácie samotných funkcií je potrebné vytvoriť aj technickú dokumentáciu, ktorá obsahuje:

- Úvodnú hlavičku s údajmi o študentovi a zadaní.
- Popis použitých algoritmov s nákresmi/obrázkami.
- Ukáž
