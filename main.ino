#include <FastLED.h>

#define NUM_LEDS 42
#define NUM_MODES 3
#define NUM_COLORS 8
#define NUM_ANIMATIONS 5
#define LED_PIN 13           // GPIO14
#define BUT_MODE_PIN 4     // GPIO4
#define BUT_CHOICE_PIN 12   // GPIO12

CRGB leds[NUM_LEDS];

// Variables de mode
int currentMode = 0;
int currentColorIndex = 0;
int currentAnimationIndex = 0;

// Variables pour les boutons
bool lastModeButtonState = HIGH;
bool lastChoiceButtonState = HIGH;
unsigned long lastModeDebounceTime = 0;
unsigned long lastChoiceDebounceTime = 0;
unsigned long debounceDelay = 50;

// Variables pour les animations
unsigned long lastAnimationUpdate = 0;
int animationStep = 0;
uint8_t hueOffset = 0;

// Tableau des couleurs
CRGB colors[NUM_COLORS] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Magenta,
  CRGB::White,
  CRGB::Orange
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Système de LEDs RGB ===");
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(200);
  
  pinMode(BUT_MODE_PIN, INPUT_PULLUP);
  pinMode(BUT_CHOICE_PIN, INPUT_PULLUP);
  
  // 🎆 ANIMATION DE DÉMARRAGE - WOW EFFECT !
  startupAnimation();
  
  clearLeds();
  printStatus();
}

void loop() {
  // Debug en temps réel
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 2000) {
    Serial.print("Mode actuel: ");
    Serial.print(currentMode);
    Serial.print(" | Bouton MODE: ");
    Serial.print(digitalRead(BUT_MODE_PIN));
    Serial.print(" | Bouton CHOIX: ");
    Serial.println(digitalRead(BUT_CHOICE_PIN));
    lastDebugPrint = millis();
  }
  
  checkModeButton();
  checkChoiceButton();
  
  // Exécuter le mode actuel
  switch(currentMode) {
    case 0:
      // Mode 0 : Éteint
      clearLeds();
      break;
      
    case 1:
      // Mode 1 : Couleur fixe
      setAllLeds(colors[currentColorIndex]);
      break;
      
    case 2:
      // Mode 2 : Animations
      runAnimation(currentAnimationIndex);
      break;
  }
  
  FastLED.show();
}

// ========== GESTION DES BOUTONS ==========

void checkModeButton() {
  static bool wasPressed = false;
  int buttonState = digitalRead(BUT_MODE_PIN);
  
  // Bouton vient d'être pressé
  if (buttonState == LOW && !wasPressed) {
    wasPressed = true;
    delay(50); // Anti-rebond simple
    
    Serial.println(">>> BOUTON MODE PRESSÉ <<<");
    Serial.print("Mode AVANT: ");
    Serial.println(currentMode);
    
    currentMode++;
    if (currentMode >= NUM_MODES) {
      currentMode = 0;
    }
    
    Serial.print("Mode APRÈS: ");
    Serial.println(currentMode);
    
    animationStep = 0;
    printStatus();
  }
  
  // Bouton relâché
  if (buttonState == HIGH) {
    wasPressed = false;
  }
}

void checkChoiceButton() {
  static bool wasPressed = false;
  int buttonState = digitalRead(BUT_CHOICE_PIN);
  
  // Bouton vient d'être pressé
  if (buttonState == LOW && !wasPressed) {
    wasPressed = true;
    delay(50); // Anti-rebond simple
    
    Serial.println(">>> BOUTON CHOIX PRESSÉ <<<");
    
    if (currentMode == 1) {
      // Mode couleur : changer de couleur
      currentColorIndex++;
      if (currentColorIndex >= NUM_COLORS) {
        currentColorIndex = 0;
      }
      Serial.print("Couleur : ");
      printColorName(currentColorIndex);
      
    } else if (currentMode == 2) {
      // Mode animation : changer d'animation
      currentAnimationIndex++;
      if (currentAnimationIndex >= NUM_ANIMATIONS) {
        currentAnimationIndex = 0;
      }
      animationStep = 0; // Reset animation
      Serial.print("Animation : ");
      printAnimationName(currentAnimationIndex);
    }
  }
  
  // Bouton relâché
  if (buttonState == HIGH) {
    wasPressed = false;
  }
}

// ========== FONCTIONS DE BASE ==========

void clearLeds() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void setAllLeds(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
}

// ========== ANIMATIONS ==========

// 🎆 ANIMATION DE DÉMARRAGE
void startupAnimation() {
  Serial.println("🎆 Animation de démarrage...");
  
  // Balayage en blanc
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    delay(15);
  }
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(15);
  }
  
  FastLED.setBrightness(100);
  Serial.println("✅ Prêt !");
}

void runAnimation(int animIndex) {
  unsigned long currentTime = millis();
  
  switch(animIndex) {
    case 0:
      // Arc-en-ciel statique
      rainbowStatic();
      break;
      
    case 1:
      // Arc-en-ciel défilant
      if (currentTime - lastAnimationUpdate > 30) {
        rainbowCycle();
        lastAnimationUpdate = currentTime;
      }
      break;
      
    case 2:
      // Respiration (fade in/out)
      if (currentTime - lastAnimationUpdate > 20) {
        breathe();
        lastAnimationUpdate = currentTime;
      }
      break;
      
    case 3:
      // Scanner (K2000)
      if (currentTime - lastAnimationUpdate > 50) {
        scanner();
        lastAnimationUpdate = currentTime;
      }
      break;
      
    case 4:
      // Clignotement multicolore
      if (currentTime - lastAnimationUpdate > 500) {
        colorBlink();
        lastAnimationUpdate = currentTime;
      }
      break;
  }
}

// Animation 0 : Arc-en-ciel statique
void rainbowStatic() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(i * 255 / NUM_LEDS, 255, 255);
  }
}

// Animation 1 : Arc-en-ciel qui défile
void rainbowCycle() {
  hueOffset++;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((i * 255 / NUM_LEDS) + hueOffset, 255, 255);
  }
}

// Animation 2 : Respiration (fade in/out)
void breathe() {
  static bool increasing = true;
  static uint8_t brightness = 0;
  
  if (increasing) {
    brightness += 5;
    if (brightness >= 250) increasing = false;
  } else {
    brightness -= 5;
    if (brightness <= 5) increasing = true;
  }
  
  fill_solid(leds, NUM_LEDS, CHSV(160, 255, brightness));
}

// Animation 3 : Scanner (K2000)
void scanner() {
  static int pos = 0;
  static bool forward = true;
  
  fadeToBlackBy(leds, NUM_LEDS, 50);
  
  leds[pos] = CRGB::Red;
  if (pos > 0) leds[pos - 1] = CRGB::DarkRed;
  if (pos < NUM_LEDS - 1) leds[pos + 1] = CRGB::DarkRed;
  
  if (forward) {
    pos++;
    if (pos >= NUM_LEDS - 1) forward = false;
  } else {
    pos--;
    if (pos <= 0) forward = true;
  }
}

// Animation 4 : Clignotement multicolore
void colorBlink() {
  static int colorIdx = 0;
  
  if (animationStep % 2 == 0) {
    setAllLeds(colors[colorIdx]);
  } else {
    clearLeds();
  }
  
  if (animationStep % 2 == 0) {
    colorIdx++;
    if (colorIdx >= NUM_COLORS) colorIdx = 0;
  }
  
  animationStep++;
}

// ========== AFFICHAGE DEBUG ==========

void printStatus() {
  Serial.println("\n--- STATUS ---");
  Serial.print("Mode : ");
  
  switch(currentMode) {
    case 0:
      Serial.println("0 - ÉTEINT");
      break;
    case 1:
      Serial.print("1 - COULEUR : ");
      printColorName(currentColorIndex);
      break;
    case 2:
      Serial.print("2 - ANIMATION : ");
      printAnimationName(currentAnimationIndex);
      break;
  }
  Serial.println("--------------\n");
}

void printColorName(int index) {
  const char* colorNames[] = {"Rouge", "Vert", "Bleu", "Jaune", "Cyan", "Magenta", "Blanc", "Orange"};
  Serial.println(colorNames[index]);
}

void printAnimationName(int index) {
  const char* animNames[] = {"Arc-en-ciel fixe", "Arc-en-ciel défilant", "Respiration", "Scanner K2000", "Clignotement"};
  Serial.println(animNames[index]);
}
