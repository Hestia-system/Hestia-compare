# Hestia-Compare

Deterministic event comparison engine for embedded systems.

## Overview

Hestia-Compare is a minimal, allocation-free library that provides cycle-synchronous
comparison events for firmware structured around a deterministic `loop()` (or main
cycle). It is designed to complement Hestia-Tempo and finite state logic by making
comparisons explicit, readable and latched per-write. The library supports
booleans, integers, floats and C-strings, with a range of detectors (diff, rise,
fall, thresholds, hysteresis, range violations) that each fire at most once per
write operation.

The API separates **acquisition** (write phase) from **decision** (event
checking phase), preventing hidden side effects and enabling predictable
behavior. Events do _not_ schedule callbacks, queue tasks or implicitly alter
values; they simply provide deterministically latched boolean results.

---

## Features

- Stateful comparisons with two-sample history (`old`, `now`)
- Numeric `diff(delta)`, `increasing(delta)` and `decreasing(delta)` compare
  against the last value that triggered the same detector
- Latched-per-write events: each detector returns true once then resets
- Boolean, integer, float and string support
- Event types:
  - `diff`, `rise`, `fall` on bools
  - `diff(delta)`, `increasing`, `decreasing` for accumulated numeric variations
  - `sampleDiff(delta)`, `sampleIncreasing`, `sampleDecreasing` for sample-to-sample variations
  - `equal(target, tol)`, `greater`, `less` for threshold crossings
  - `outOfRange(min,max)` for absolute range violations
  - `diffStr`, `equal(const char*)` for strings
- Introspection via `Read` view (old, now, delta, absolute delta)
- Fixed slot pool, predictable memory footprint (no dynamic allocation)
- Compatible with Arduino and ESP32-class microcontrollers

## Numeric Variation Semantics

Numeric detectors are latched per `write()`, but they do not all use the same
reference:

- `diff(delta)` returns true when `abs(now - reference) > delta`. The reference
  is initialized by the first `write()` and updated only when `diff(delta)`
  returns true. This detects slow accumulated drift.
- `sampleDiff(delta)` returns true when `abs(now - old) > delta`. This detects
  a large step between two consecutive samples.
- `increasing(delta)` returns true when `now > reference + delta`. Its reference
  is updated only when `increasing(delta)` returns true. This detects slow
  accumulated upward drift.
- `decreasing(delta)` returns true when `now < reference - delta`. Its reference
  is updated only when `decreasing(delta)` returns true. This detects slow
  accumulated downward drift.
- `sampleIncreasing(delta)` returns true when `now > old + delta`. This detects
  a single upward step larger than `delta`.
- `sampleDecreasing(delta)` returns true when `now < old - delta`. This detects
  a single downward step larger than `delta`.

Example with `delta = 2` and samples `20 -> 21 -> 22 -> 23`:

- `diff(2)` triggers at `23`, because `23 - 20 > 2`.
- `sampleDiff(2)` does not trigger, because each step is only `+1`.
- `increasing(2)` triggers at `23`, because `23 > 20 + 2`.
- `sampleIncreasing(2)` does not trigger, because no single upward step is
  greater than `2`.

---

## Example (Arduino sketch)

```cpp
#include "Hestia-compare.h"

const Compare::Id temp_id = 1;
const Compare::Id button_id = 2;

void setup() {
    Serial.begin(9600);
}

void loop() {
    static int temp = 20;
    temp += random(-2, 3);
    Compare::event(temp_id).write(temp);
    static bool btn = false;
    if (millis() % 2000 < 50) btn = !btn;
    Compare::event(button_id).write(btn);

    if (Compare::event(temp_id).diff(2)) {
        Serial.println("Temp changed by >2°C");
    }
    if (Compare::event(button_id).rise()) {
        Serial.println("Button pressed");
    }
    delay(500);
}
```

---

## Documentation & Usage

See `Documentation/Besoin fonctionnal de la librairie.docx` for conceptual
details. The library header (`Hestia-compare.h`) is commented extensively and
serves as the primary API reference.

---

## License

MIT

---

# Hestia-Compare (français)

Moteur d'événements de comparaison déterministe pour systèmes embarqués.

## Présentation

Hestia-Compare est une bibliothèque minimale, sans allocation, qui fournit des
événements de comparaison synchronisés sur le cycle principal pour des firmwares
structurés autour d'une `loop()` déterministe. Elle est destinée à compléter
Hestia-Tempo et la logique à états finis en rendant les comparaisons explicites,
lisibles et verrouillées par écriture. La bibliothèque supporte les booléens,
entiers, flottants et chaînes C, avec une gamme de détecteurs (diff, montée,
descente, seuils, hystérésis, violations de plage) qui se déclenchent au plus
une fois par écriture.

L'API sépare **acquisition** (phase d'écriture) et **décision** (phase de
vérification d'événements), ce qui évite les effets de bord cachés et garantit
un comportement prévisible. Les événements ne planifient pas de rappels, ne
mettent pas en file d'attente de tâches et n'altèrent pas implicitement les
valeurs ; ils fournissent simplement des résultats booléens déterministes.

## Fonctionnalités

- Comparaisons à état avec deux échantillons (`old`, `now`)
- `diff(delta)`, `increasing(delta)` et `decreasing(delta)` numériques comparent
  avec la dernière valeur qui a déclenché le même détecteur
- Événements verrouillés par écriture : chaque détecteur retourne true une fois
  puis se réinitialise
- Prise en charge des booléens, entiers, flottants et chaînes
- Types d'événements :
  - `diff`, `rise`, `fall` pour booléens
  - `diff(delta)`, `increasing`, `decreasing` pour variations numériques cumulées
  - `sampleDiff(delta)`, `sampleIncreasing`, `sampleDecreasing` pour variations entre deux échantillons
  - `equal(target, tol)`, `greater`, `less` pour franchissements de seuil
  - `outOfRange(min,max)` pour violations de plage absolue
  - `diffStr`, `equal(const char*)` pour chaînes
- Introspection via la vue `Read` (old, now, delta, delta absolu)
- Pool de cases fixe, empreinte mémoire prédictible (pas d'allocation dynamique)
- Compatible avec Arduino et microcontrôleurs de la classe ESP32

## Sémantique des variations numériques

Les détecteurs numériques sont verrouillés par `write()`, mais ils n'utilisent
pas tous la même référence :

- `diff(delta)` retourne true quand `abs(now - reference) > delta`. La référence
  est initialisée par le premier `write()` et mise à jour seulement quand
  `diff(delta)` retourne true. Ce détecteur voit donc une dérive lente cumulée.
- `sampleDiff(delta)` retourne true quand `abs(now - old) > delta`. Ce détecteur
  voit seulement un grand pas entre deux échantillons consécutifs.
- `increasing(delta)` retourne true quand `now > reference + delta`. Sa
  référence est mise à jour seulement quand `increasing(delta)` retourne true.
  Ce détecteur voit donc une hausse lente cumulée.
- `decreasing(delta)` retourne true quand `now < reference - delta`. Sa
  référence est mise à jour seulement quand `decreasing(delta)` retourne true.
  Ce détecteur voit donc une baisse lente cumulée.
- `sampleIncreasing(delta)` retourne true quand `now > old + delta`. Ce
  détecteur voit seulement une hausse d'un seul pas plus grande que `delta`.
- `sampleDecreasing(delta)` retourne true quand `now < old - delta`. Ce
  détecteur voit seulement une baisse d'un seul pas plus grande que `delta`.

Exemple avec `delta = 2` et les échantillons `20 -> 21 -> 22 -> 23` :

- `diff(2)` déclenche à `23`, parce que `23 - 20 > 2`.
- `sampleDiff(2)` ne déclenche pas, parce que chaque pas vaut seulement `+1`.
- `increasing(2)` déclenche à `23`, parce que `23 > 20 + 2`.
- `sampleIncreasing(2)` ne déclenche pas, parce qu'aucun pas de hausse n'est
  plus grand que `2`.

## Exemple (sketch Arduino)

```cpp
#include "Hestia-compare.h"

const Compare::Id temp_id = 1;
const Compare::Id button_id = 2;

void setup() {
    Serial.begin(9600);
}

void loop() {
    static int temp = 20;
    temp += random(-2, 3);
    Compare::event(temp_id).write(temp);
    static bool btn = false;
    if (millis() % 2000 < 50) btn = !btn;
    Compare::event(button_id).write(btn);

    if (Compare::event(temp_id).diff(2)) {
        Serial.println("Température changée de > 2°C");
    }
    if (Compare::event(button_id).rise()) {
        Serial.println("Bouton pressé");
    }
    delay(500);
}
```

## Documentation & utilisation

Consultez `Documentation/Besoin fonctionnel de la librairie.docx` pour les
explications conceptuelles. L'en-tête (`Hestia-compare.h`) est abondamment
commenté et constitue la référence API principale.

## Licence

MIT
