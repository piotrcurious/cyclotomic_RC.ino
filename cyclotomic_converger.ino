
// Define the digital output pin that drives the RC circuit
#define OUTPUT_PIN 13

// Define the analog input pin that reads the RC circuit voltage
#define INPUT_PIN A0

// Define the known value of the capacitor in Farads
#define CAPACITOR_VALUE 0.00001

// Define the number of bits for the LFSR
#define LFSR_BITS 8

// Define the taps for the LFSR feedback function
uint8_t tap_0 = 0;
uint8_t tap_1 = 2;
uint8_t tap_2 = 3;
uint8_t tap_3 = 4;

// Define a variable to store the LFSR state
uint8_t lfsr = 0x01; // Initialize with a non-zero value

// Define a variable to store the RC circuit voltage
float voltage = 0;

// Define a variable to store the RC circuit resistance
float resistance = 0;

// Define a variable to store the RC circuit frequency
float frequency = 0;

// Define a variable to store the RC circuit resonance point
float resonance = 0;

// Define a variable to store the absolute resistance of the resistor
float absolute_resistance = 0;

// Function prototypes to avoid compilation issues in C++
void updateLFSR();
void readRC();
void findResonance();
void convergeLFSR();

// Track time and voltage for accurate R calculation
unsigned long lastUpdateUs = 0;
uint8_t lastPinState = LOW;
float v_initial = 0;

// Define a function to update the LFSR state and output pin
void updateLFSR() {
  // Capture voltage at the moment of transition
  v_initial = analogRead(INPUT_PIN) * (5.0 / 1023.0);

  // Compute the feedback bit by XOR-ing the tap bits
  uint8_t feedback = ((lfsr >> tap_0) ^ (lfsr >> tap_1) ^ (lfsr >> tap_2) ^ (lfsr >> tap_3)) & 1;
  
  // Shift the LFSR state to the right by one bit and insert the feedback bit at the leftmost position
  lfsr = (lfsr >> 1) | (feedback << (LFSR_BITS - 1));
  
  // Write the least significant bit of the LFSR state to the output pin
  lastPinState = (lfsr & 1) ? HIGH : LOW;
  digitalWrite(OUTPUT_PIN, lastPinState);
  lastUpdateUs = micros();
}

// Define a function to read the RC circuit voltage and compute its resistance and frequency
void readRC() {
  unsigned long currentUs = micros();
  float dt = (currentUs - lastUpdateUs) * 1e-6;

  // Read the analog value from the input pin and convert it to voltage
  voltage = analogRead(INPUT_PIN) * (5.0 / 1023.0);
  
  // Compute the resistance of the RC circuit using the capacitor equation
  if (dt > 0) {
    float v_drive = (lastPinState == HIGH) ? 5.0 : 0.0;
    if (abs(v_initial - v_drive) > 0.01) {
        float ratio = (voltage - v_drive) / (v_initial - v_drive);
        if (ratio > 0.001 && ratio < 0.999) {
            float tau = -dt / log(ratio);
            resistance = tau / CAPACITOR_VALUE;
        }
    }
  }
  
  // Compute the frequency of the RC circuit using cyclotomic polynomial analysis
  if (resistance > 0) {
    frequency = (pow(2, LFSR_BITS) - 1) / (CAPACITOR_VALUE * resistance * log(2));
  }
}

// Define a function to find the RC circuit resonance point by comparing its frequency with previous values
void findResonance() {
  static float prevFrequency = 0;
  static float prevDiff = 0;
  static bool peakTriggered = false;
  
  float diff = frequency - prevFrequency;
  
  if (diff < 0 && prevDiff > 0 && !peakTriggered) {
    resonance = (frequency + prevFrequency) / 2;
    Serial.print("Resonance point found at ");
    Serial.print(resonance);
    Serial.println(" Hz");
    
    peakTriggered = true;
    
    // Compute the absolute resistance of the resistor using the resonance frequency formula
    absolute_resistance = (pow(2, LFSR_BITS) - 1) / (CAPACITOR_VALUE * resonance * log(2));
    
    // Print a message to indicate the absolute resistance value
    Serial.print("Absolute resistance of the resistor is ");
    Serial.print(absolute_resistance);
    Serial.println(" Ohms");
    
    // Converge the LFSR to the resonant frequency by adjusting its taps
    convergeLFSR();
  } else if (diff > 0) {
    peakTriggered = false;
  }
  
  prevFrequency = frequency;
  prevDiff = diff;
}

// Define a function to converge the LFSR to the resonant frequency by adjusting its taps
void convergeLFSR() {
  uint8_t bestT0 = tap_0, bestT1 = tap_1, bestT2 = tap_2, bestT3 = tap_3;
  float bestError = abs(frequency - resonance);
  
  for (uint8_t t0 = 0; t0 < LFSR_BITS; t0++) {
    for (uint8_t t1 = t0 + 1; t1 < LFSR_BITS; t1++) {
      for (uint8_t t2 = t1 + 1; t2 < LFSR_BITS; t2++) {
        for (uint8_t t3 = t2 + 1; t3 < LFSR_BITS; t3++) {
          tap_0 = t0; tap_1 = t1; tap_2 = t2; tap_3 = t3;
          updateLFSR();
          delay(10);
          readRC();
          float error = abs(frequency - resonance);
          if (error < bestError) {
            bestT0 = t0; bestT1 = t1; bestT2 = t2; bestT3 = t3;
            bestError = error;
          }
        }
      }
    }
  }
  
  tap_0 = bestT0; tap_1 = bestT1; tap_2 = bestT2; tap_3 = bestT3;
  updateLFSR();
  delay(10);
  readRC();
  
  Serial.print("LFSR has converged. Best taps: ");
  Serial.print(tap_0); Serial.print(",");
  Serial.print(tap_1); Serial.print(",");
  Serial.print(tap_2); Serial.print(",");
  Serial.print(tap_3);
  Serial.print(" with an error of ");
  Serial.print(bestError);
  Serial.println(" Hz");
}

// Define the setup function that runs once when the Arduino board is powered on or reset
void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);
  Serial.begin(9600);
  lastUpdateUs = micros();
  v_initial = analogRead(INPUT_PIN) * (5.0 / 1023.0);
}

// Define the loop function that runs repeatedly after the setup function
void loop() {
  updateLFSR();
  delay(10);
  readRC();
  findResonance();
}
