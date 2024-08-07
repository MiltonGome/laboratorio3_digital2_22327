#include <Arduino.h>

int ledPins[] = {4, 5, 18, 19};
int ledPins2[] = {15, 2, 22, 23};
int ledind = 13;
int counter = 0;
bool isBinaryMode = true;

hw_timer_t *Timer0 = NULL;

const int incrementButtonPin = 16; 
const int decrementButtonPin = 17;
const int capacitiveSensorPin = 12;  // Pin del sensor capacitivo

void IRAM_ATTR incrementButtonISR();
void IRAM_ATTR decrementButtonISR();
void updateBinaryLEDs(int value);
void checkLedMatch();
void initTimer0();
void IRAM_ATTR Timer0_ISR();
void checkCapacitiveSensor();
void resetTimerCounter();

volatile int manualCounter = 0;  // Variable compartida para el contador manual
volatile bool timerMatch = false; // Variable para indicar coincidencia

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(ledPins2[i], OUTPUT);
  }
  pinMode(ledind, OUTPUT);
  
  pinMode(incrementButtonPin, INPUT_PULLUP);
  pinMode(decrementButtonPin, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(incrementButtonPin), incrementButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(decrementButtonPin), decrementButtonISR, FALLING);

  Serial.begin(115200);

  initTimer0();
}

void loop() {
  checkLedMatch();
  checkCapacitiveSensor();
}

void IRAM_ATTR incrementButtonISR() {
  counter++;
  if (counter > 15) {
    counter = 0;
  }
  updateBinaryLEDs(counter);
}

void IRAM_ATTR decrementButtonISR() {
  counter--;
  if (counter < 0) {
    counter = 15;
  }
  updateBinaryLEDs(counter);
}

void updateBinaryLEDs(int value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], (value >> i) & 0b0001);
  }
  manualCounter = value; // Actualizar el valor del contador manual
}

void checkLedMatch() {
  if (timerMatch) {
    digitalWrite(ledind, HIGH);
    delay(100); // Mantener el LED encendido por un breve período
    digitalWrite(ledind, LOW);
    timerMatch = false;
  }
}

void initTimer0() {
  Timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0, &Timer0_ISR, true);
  timerAlarmWrite(Timer0, 250000, true);
  timerAlarmEnable(Timer0);
}

void IRAM_ATTR Timer0_ISR() {
  static int timerCounter = 0;
  timerCounter++;
  if (timerCounter > 15) {
    timerCounter = 0;
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins2[i], (timerCounter >> i) & 0b0001);
  }
  if (timerCounter == manualCounter) {
    timerWrite(Timer0, 0); // Reiniciar el contador del timer
    timerMatch = true;
  }
}

void checkCapacitiveSensor() {
  if (touchRead(capacitiveSensorPin) < 40) { // Ajustar el valor del umbral según sea necesario
    resetTimerCounter();
    delay(200); // Pequeño retraso para evitar múltiples lecturas
  }
}

void resetTimerCounter() {
  timerWrite(Timer0, 0); // Reiniciar el contador del timer
  timerMatch = true;
}