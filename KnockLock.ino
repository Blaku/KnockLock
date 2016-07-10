/*
  License information: This code is released by Thomas Höglund (thomas.hoeglund@gmail.com) 
  under Creative Commons Attribution 4.0 International

  Summary:
  This Arduino code recognizes melodies using a digital input. The index of the recognized melody is printed out.
  
  - Print out index of recognized melody
  - Blink quick error blinks if not recognized or long blink if recognized
  - Uncomment //printMelodyFloat(normalizedMelody); to read what you input so you can copy to new melodies
*/

#define	MELODY_LENGTH 8
#define MELODIES 3
#define TIMEOUT 10000
#define ALLOWED_ERROR 1200
#define DEBOUNCE_TIME 120
#define LED_PIN 13
#define INPUT_PIN 2
#define PULSE_OUT_PIN 3
#define MINIMUM_PULSE_LENGTH 100 //index 0=100ms pulse, 1=200ms and so on

float melodies[MELODIES][MELODY_LENGTH] = {
  {1974.49,1757.65,857.14,951.53,1859.69,882.65,895.41,821.43},        //0: Bää bää vita lamm
  {662.76,812.32,1598.24,1756.60,1744.87,1929.62,788.86,706.74},       //1: SOS
  {1512.76,1510.05,1692.02,1222.16,651.82,1385.12,1330.80,695.27},     //2: Imperial March
};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(PULSE_OUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);
  Serial.begin(19200);
  /*
    Serial.println("Please store melody 0.");
    storeMelody(0);
    delay(2000);//Don't start recording the next melody immediately
  */

  //Serial.println("Initialized.");
}

//Record and recognize melody
void loop() {
  unsigned long rawMelody[MELODY_LENGTH];
  if (recordMelody(rawMelody)) {
    //printMelodyUnsignedLong(rawMelody);
    float normalizedMelody[MELODY_LENGTH];
    normalizeMelody(rawMelody, normalizedMelody);
    //printMelodyFloat(normalizedMelody);
    //compare to stored melodies
    float errors[MELODIES];
    for (byte i = 0; i < MELODIES; i++) {
      float mel[MELODY_LENGTH];
      for (byte j = 0; j < MELODY_LENGTH; j++) {
        mel[j] = melodies[i][j];
      }
      errors[i] = compareMelody(normalizedMelody, mel);
    }
    byte indexOfSmallestError = 0;
    float smallestError = 10000;
    for (byte i = 0; i < MELODIES; i++) {
      //Serial.println("Total error for melody " + String(i) + ": " + String(errors[i]));
      if (errors[i] < smallestError) {
        smallestError = errors[i];
        indexOfSmallestError = i;
      }
    }
    if (smallestError <= ALLOWED_ERROR) {
      //Transmit information about recognized melody
      Serial.println("Recognized melody: " + String(indexOfSmallestError));
      digitalWrite(LED_PIN, HIGH);
      delay(2000);//Don't start recording the next melody immediately
      digitalWrite(LED_PIN, LOW);
      //pulseOut(i);
    }
    else { //indicate failure
      //Serial.println("Failed to recognize melody.");
      blinkFast();
    }
  }
  else {
    blinkFast();
    //Serial.println("Timed out");

  }
}

void blinkFast() {
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
}

void printMelodyUnsignedLong(unsigned long mel[]) {
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    if (i == MELODY_LENGTH - 1)
      Serial.println(mel[i]);
    else
      Serial.print(String(mel[i]) + ",");
  }
}

void printMelodyFloat(float mel[]) {
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    if (i == MELODY_LENGTH - 1)
      Serial.println(mel[i]);
    else
      Serial.print(String(mel[i]) + ",");
  }
}

float compareMelody(float normMel1[], float normMel2[]) {
  float error = 0; //absolute difference
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    float diff = normMel1[i] - normMel2[i];
    //Serial.print(diff);
    //Serial.print(";");
    error += abs(diff);
  }
  //Serial.println("Total absolute difference: " + String(error));
  return error;
}

void blinkMelody(byte melodyIndex) {
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(20);
    digitalWrite(LED_PIN, LOW);
    delay(melodies[melodyIndex][i] - 20);
  }
}

bool recordMelody(unsigned long rawMelody[]) {
  unsigned long startTime;
  unsigned long lastTime;
  boolean success = false;

  while (digitalRead(INPUT_PIN) == LOW) {} //Wait for digital input pulse (e.g. knock or door handle input)
  startTime = millis();
  lastTime = startTime;
  //Serial.println("Started recording.");
  for (byte i = 0; i < MELODY_LENGTH; i++) { //record melody
    while (digitalRead(INPUT_PIN) == HIGH && millis() - startTime < TIMEOUT) {} //wait for pulse to go LOW
    if (millis() - startTime >= TIMEOUT)
      return false;
    delay(DEBOUNCE_TIME);//debounce falling edge
    while (digitalRead(INPUT_PIN) == LOW && millis() - startTime < TIMEOUT) {} //wait for next pulse to go HIGH
    if (millis() - startTime >= TIMEOUT)
      return false;
    rawMelody[i] = millis() - lastTime;
    //Serial.println(String(i) + ": " + String(rawMelody[i]));
    if (i == MELODY_LENGTH) //if whole melody successfully recorded
      return true;
    lastTime = millis();
    delay(DEBOUNCE_TIME);//debounce rising edge
  }
}

void normalizeMelody(unsigned long rawMelody[], float normalizedMelody[]) {
  //normalize melody length to TIMEOUT
  unsigned long totalLength = 0;
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    totalLength += rawMelody[i];
  }
  //Serial.println("Raw length (ms): " + String(totalLength));
  float ratio = TIMEOUT / (float)totalLength;
  //Serial.println("Ratio: " + String(ratio));
  for (byte i = 0; i < MELODY_LENGTH; i++) {
    normalizedMelody[i] = ratio * rawMelody[i];
  }
}

bool storeMelody(byte melodyIndex) {
  unsigned long rawMelody[MELODY_LENGTH];
  if (recordMelody(rawMelody)) {
    float normalizedMelody[MELODY_LENGTH];
    normalizeMelody(rawMelody, normalizedMelody);
    Serial.print("Stored: ");
    printMelodyFloat(normalizedMelody);
    for (byte i = 0; i < MELODY_LENGTH; i++) {
      melodies[melodyIndex][i] = normalizedMelody[i];
    }
    return true;
  }
  return false;
}

void pulseOut(byte index) {
  digitalWrite(PULSE_OUT_PIN, HIGH);
  delay((index + 1)*MINIMUM_PULSE_LENGTH);
  digitalWrite(PULSE_OUT_PIN, LOW);
}

