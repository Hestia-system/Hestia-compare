# Hestia-Compare

Deterministic event comparison engine for embedded systems.

## Overview

Hestia-Compare is a minimal, allocation-free library that provides cycle-synchronous
old/new comparison events for firmware structured around a deterministic `loop()`
(or main cycle). It is designed to complement Hestia-Tempo and finite state logic
by making comparisons explicit, readable and latched per-write. The library
supports booleans, integers, floats and C-strings, with a range of detectors
(diff, rise, fall, thresholds, hysteresis, range violations) that each fire at
most once per write operation.

The API separates **acquisition** (write phase) from **decision** (event
checking phase), preventing hidden side effects and enabling predictable
behavior. Events do _not_ schedule callbacks, queue tasks or implicitly alter
values; they simply provide deterministically latched boolean results.

---

## Features

- Stateful comparisons with two-sample history (`old`, `now`)
- Latched-per-write events: each detector returns true once then resets
- Boolean, integer, float and string support
- Event types:
  - `diff`, `rise`, `fall` on bools
  - `diff(delta)`, `increasing`, `decreasing` for numeric variations
  - `equal(target, tol)`, `greater`, `less` for threshold crossings
  - `outOfRange(min,max)` for absolute range violations
  - `diffStr`, `equal(const char*)` for strings
- Introspection via `Read` view (old, now, delta, absolute delta)
- Fixed slot pool, predictable memory footprint (no dynamic allocation)
- Compatible with Arduino and ESP32-class microcontrollers

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
événements de comparaison ancien/nouveau synchronisés sur le cycle principal
pour des firmwares structurés autour d'une `loop()` déterministe. Elle est
destinée à compléter Hestia-Tempo et la logique à états finis en rendant les
comparaisons explicites, lisibles et verrouillées par écriture. La bibliothèque
supporte les booléens, entiers, flottants et chaînes C, avec une gamme de
détecteurs (diff, montée, descente, seuils, hystérésis, violations de plage)
qui se déclenchent au plus une fois par écriture.

L'API sépare **acquisition** (phase d'écriture) et **décision** (phase de
vérification d'événements), ce qui évite les effets de bord cachés et garantit
un comportement prévisible. Les événements ne planifient pas de rappels, ne
mettent pas en file d'attente de tâches et n'altèrent pas implicitement les
valeurs ; ils fournissent simplement des résultats booléens déterministes.

## Fonctionnalités

- Comparaisons à état avec deux échantillons (`old`, `now`)
- Événements verrouillés par écriture : chaque détecteur retourne true une fois
  puis se réinitialise
- Prise en charge des booléens, entiers, flottants et chaînes
- Types d'événements :
  - `diff`, `rise`, `fall` pour booléens
  - `diff(delta)`, `increasing`, `decreasing` pour variations numériques
  - `equal(target, tol)`, `greater`, `less` pour franchissements de seuil
  - `outOfRange(min,max)` pour violations de plage absolue
  - `diffStr`, `equal(const char*)` pour chaînes
- Introspection via la vue `Read` (old, now, delta, delta absolu)
- Pool de cases fixe, empreinte mémoire prédictible (pas d'allocation dynamique)
- Compatible avec Arduino et microcontrôleurs de la classe ESP32

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
