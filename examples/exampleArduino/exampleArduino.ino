#include "Hestia-compare.h"
#include <Arduino.h>

// Exemple d'utilisation de la bibliothèque Compare
// Simule un capteur de température et détecte des événements

const Compare::Id temp_id = 1;
const Compare::Id button_id = 2;

void setup()
{
  Serial.begin(9600);
  Serial.println("Démarrage du test Compare");
}

void loop()
{
  // Simulation d'une température qui varie
  static int temp = 20;
  temp += random(-2, 3); // Variation aléatoire
  if (temp < 15)
    temp = 15;
  if (temp > 30)
    temp = 30;

  // Écriture de la température (phase acquisition)
  Compare::event(temp_id).write(temp);

  // Simulation d'un bouton (true/false)
  static bool button_state = false;
  static unsigned long last_toggle = 0;
  if (millis() - last_toggle > 2000)
  { // Toggle toutes les 2 secondes
    button_state = !button_state;
    last_toggle = millis();
  }
  Compare::event(button_id).write(button_state);

  // Phase décision : vérifier les événements
  if (Compare::event(temp_id).diff(2))
  {
    Serial.print("Température changée de plus de 2°C: ");
    Serial.print(Compare::event(temp_id).read().oldInt());
    Serial.print(" -> ");
    Serial.println(Compare::event(temp_id).read().nowInt());
  }

  if (Compare::event(temp_id).greater(25, 0))
  {
    Serial.println("Température dépasse 25°C");
  }

  if (Compare::event(temp_id).less(18, 0))
  {
    Serial.println("Température descend en dessous de 18°C");
  }

  if (Compare::event(button_id).rise())
  {
    Serial.println("Bouton pressé (rise)");
  }

  if (Compare::event(button_id).fall())
  {
    Serial.println("Bouton relâché (fall)");
  }

  // Affichage périodique
  static unsigned long last_print = 0;
  if (millis() - last_print > 1000)
  {
    Serial.print("Temp actuelle: ");
    Serial.print(Compare::event(temp_id).read().nowInt());
    Serial.print("°C, Bouton: ");
    Serial.println(Compare::event(button_id).read().nowBool() ? "ON" : "OFF");
    last_print = millis();
  }

  delay(500); // Boucle toutes les 500ms
}