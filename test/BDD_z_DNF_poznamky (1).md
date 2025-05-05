
# 📘 Poznámky k projektu – BDD z DNF

## 🔧 Dátové štruktúry

### `struct BDDNode`
```cpp
struct BDDNode {
    int var;         // index premennej podľa poradia
    BDDNode *low;    // vetva, keď premenná = 0
    BDDNode *high;   // vetva, keď premenná = 1
    int id;          // jedinečný identifikátor (voliteľné)
};
```

### `struct BDD`
```cpp
struct BDD {
    int numVars;    // počet premenných
    int numNodes;   // aktuálny počet uzlov
    BDDNode *root;  // koreň BDD
    map<tuple<int,int,int>, BDDNode*> uniqueTable;  // unique table
    BDDNode *BDD_TRUE;    // terminálny uzol (1)
    BDDNode *BDD_FALSE;   // terminálny uzol (0)
};
```

## 🔨 Hlavné funkcie

### `BDD_create(dnf, order)`
- **Vstup**: Booleovský výraz v DNF (napr. `A*B+¬C`) a poradie premenných (`ABC`)
- **Výstup**: Koreň výsledného zredukovaného BDD

#### Postup:
1. Rozparsuj DNF na klauzuly
2. Inicializuj BDD a terminálne uzly
3. Pre každú klauzulu:
   - vytvor jej vlastný BDD pomocou `buildClauseBDD`
   - pripoj ho cez `BDD_or`
4. Vráť koreň výsledného BDD

```cpp
function BDD_create(string dnf, string order):
    clauses = parseDNF(dnf)
    init bdd, TRUE/FALSE terminály
    root = BDD_FALSE
    for clause in clauses:
        tempBDD = buildClauseBDD(clause, order, bdd)
        root = BDD_or(root, tempBDD, bdd)
    return bdd
```

### `BDD_create_with_best_order(dnf)`
- Skúsi viacero náhodných poradí a vyberie to najlepšie (najmenší počet uzlov)

```cpp
function BDD_create_with_best_order(dnf):
    vars = extractVariables(dnf)
    bestBDD = null
    bestSize = ∞
    repeat N krát:
        random order
        bdd = BDD_create(dnf, order)
        if bdd.numNodes < bestSize:
            bestBDD = bdd
            bestSize = bdd.numNodes
    return bestBDD
```

### `BDD_use(bdd, inputs)`
- Vyhodnotí BDD pre zadaný vstupný vektor (`"010"`)

```cpp
function BDD_use(bdd, inputs):
    node = bdd.root
    while node is not terminal:
        i = node.var
        if inputs[i] == '0':
            node = node.low
        else:
            node = node.high
    return (node == BDD_TRUE) ? '1' : '0'
```

## 🔧 Pomocné funkcie

### `buildClauseBDD(clause, order, bdd)`
- Rekurzívne vytvorenie BDD pre jednu klauzulu (napr. `A*¬B`)

```cpp
function buildClauseBDD(literals, order, bdd):
    if order je prázdny:
        return BDD_TRUE
    var = order[0]
    rest = order[1:]
    if var in literals:
        low = BDD_FALSE
        high = buildClauseBDD(literals - var, rest)
    else if ¬var in literals:
        low = buildClauseBDD(literals - ¬var, rest)
        high = BDD_FALSE
    else:
        low = buildClauseBDD(literals, rest)
        high = buildClauseBDD(literals, rest)
    if low == high: return low
    return makeNode(var, low, high)
```

### `makeNode(var, low, high, bdd)`
- Zaručuje zdieľanie a redukciu uzlov

```cpp
function makeNode(var, low, high, bdd):
    if low == high: return low
    key = (var, low.id, high.id)
    if key in bdd.uniqueTable:
        return bdd.uniqueTable[key]
    node = new BDDNode(var, low, high)
    node.id = ++bdd.numNodes
    bdd.uniqueTable[key] = node
    return node
```

### `BDD_or(node1, node2, bdd)`
- Kombinácia dvoch BDD OR operáciou (Apply algoritmus)

```cpp
function BDD_or(n1, n2, bdd):
    if n1 == TRUE or n2 == TRUE: return TRUE
    if n1 == FALSE: return n2
    if n2 == FALSE: return n1
    if (n1,n2) in cache: return cache[n1,n2]
    i = min(n1.var, n2.var)
    (low1, high1) = (n1.low/high if n1.var == i else n1)
    (low2, high2) = (n2.low/high if n2.var == i else n2)
    low = BDD_or(low1, low2, bdd)
    high = BDD_or(high1, high2, bdd)
    result = makeNode(i, low, high, bdd)
    cache[n1,n2] = result
    return result
```

## 🧠 Upozornenia
- Zredukované BDD musia byť **bez redundantných uzlov** (`low == high`)
- Zdieľané uzly pomocou `uniqueTable`
- Zníženie veľkosti diagramu pomocou `BDD_create_with_best_order`
- Hodnota v terminálnom uzle sa zisťuje porovnaním na `BDD_TRUE` / `BDD_FALSE`


---

## Teoretické Pozadie BDD

### Čo je BDD (Binárny Rozhodovací Diagram)?
BDD je dátová štruktúra, ktorá reprezentuje Booleovské funkcie v kompaktnom tvare. Je to orientovaný acyklický graf (DAG), kde každý vnútorný uzol reprezentuje logickú premennú a každý list reprezentuje výstup (`0` alebo `1`).

#### Typy BDD:
- **OBDD (Ordered BDD)** – uzly sú zoradené podľa pevného poradia premenných.
- **ROBDD (Reduced OBDD)** – odstránené sú redundantné uzly a zduplikované podgrafy.

### Redukčné Pravidlá
1. **Zlúčenie identických podgrafov:** Ak dva uzly majú rovnakú premennú a ich true/false potomkovia sú identickí, zlúč ich.
2. **Odstránenie zbytočných uzlov:** Ak `low == high`, uzol sa stáva nadbytočný.

### Výhody:
- Menšia pamäťová náročnosť.
- Rýchle vyhodnocovanie funkcie pre zadané vstupy.
- Umožňuje efektívne porovnávanie funkcií.

---

## Zložitosť

- **BDD_create:**
  - Časová: `O(2^n)` v najhoršom prípade bez redukcie.
  - Priestorová: podobne, ale silne závisí na poradi premenných.

- **BDD_create_with_best_order:**
  - Pre `n` premenných: `O(k * T(n))`, kde `k` je počet vyskúšaných permutácií, `T(n)` čas potrebný na zostavenie jedného BDD.
  - Najhorší prípad je `O(n! * T(n))`.

- **BDD_use:**
  - Časová: `O(n)` – prechod z koreňa po list.

---

## Testovanie

- Generovanie náhodných DNF funkcií.
- Pre každú kombináciu vstupov:
  - Vyhodnotiť DNF manuálne (alebo parserom).
  - Porovnať s výsledkom `BDD_use`.

### Príklad testu:

```c
if (BDD_use(bdd, "101") != evaluate_expr("A&!B&C", "101")) {
    printf("Chyba: zle vyhodnotenie
");
}
```

