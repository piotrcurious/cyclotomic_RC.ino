
// Define the digital output pin that drives the RC circuit
#define OUTPUT_PIN 13

// Define the analog input pin that reads the RC circuit voltage
#define INPUT_PIN A0

// Define the known value of the capacitor in Farads
#define CAPACITOR_VALUE 0.00001

// Define the number of bits for the LFSR
#define LFSR_BITS 8

// Define the taps for the LFSR feedback function
// Make them variables instead of defines so they can be changed
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

// Function prototypes
void updateLFSR();
void readRC();
void findResonance();
void convergeLFSR();

// Define a function to update the LFSR state and output pin
void updateLFSR() {
  // Compute the feedback bit by XOR-ing the tap bits
  uint8_t feedback = ((lfsr >> tap_0) ^ (lfsr >> tap_1) ^ (lfsr >> tap_2) ^ (lfsr >> tap_3)) & 1;
  
  // Shift the LFSR state to the right by one bit and insert the feedback bit at the leftmost position
  lfsr = (lfsr >> 1) | (feedback << (LFSR_BITS - 1));
  
  // Write the least significant bit of the LFSR state to the output pin
  digitalWrite(OUTPUT_PIN, lfsr & 1);
}

// Define a function to read the RC circuit voltage and compute its resistance and frequency
void readRC() {
  // Read the analog value from the input pin and convert it to voltage
  voltage = analogRead(INPUT_PIN) * (5.0 / 1023.0);
  
  // Compute the resistance of the RC circuit using Ohm's law and Kirchhoff's voltage law
  float t = 0.01; // 10ms delay in loop
  if (voltage < 4.9 && voltage > 0.1) {
    resistance = -t / (CAPACITOR_VALUE * log(1.0 - (voltage / 5.0)));
  } else {
    resistance = 1e6; // Default to high resistance
  }
  
  // Compute the frequency of the RC circuit using cyclotomic polynomial analysis
  frequency = (pow(2, LFSR_BITS) - 1) / (CAPACITOR_VALUE * resistance * log(2));
}

// Define a function to find the RC circuit resonance point by comparing its frequency with previous values
void findResonance() {
  // Define a static variable to store the previous frequency value
  static float prevFrequency = 0;
  // Define a static variable to store the previous difference between frequency and prevFrequency
  static float prevDiff = 0;
  // State to track if we've already triggered on this peak
  static bool peakTriggered = false;
  
  // Compute the current difference between frequency and prevFrequency
  float diff = frequency - prevFrequency;
  
  // Check if the sign of diff has changed from positive to negative, indicating a local maximum
  if (diff < 0 && prevDiff > 0 && !peakTriggered) {
    // Update the resonance point with the average of frequency and prevFrequency
    resonance = (frequency + prevFrequency) / 2;
    
    // Print a message to indicate that a resonance point has been found
    Serial.print("Resonance point found at ");
    Serial.print(resonance);
    Serial.println(" Hz");
    
    peakTriggered = true; // Mark as triggered to avoid repeats on same peak
    
    // Compute the absolute resistance of the resistor
    absolute_resistance = (pow(2, LFSR_BITS) - 1) / (CAPACITOR_VALUE * resonance * log(2));
    
    // Print a message to indicate the absolute resistance value
    Serial.print("Absolute resistance of the resistor is ");
    Serial.print(absolute_resistance);
    Serial.println(" Ohms");
    
    // Converge the LFSR to the resonant frequency by adjusting its taps
    convergeLFSR();
  } else if (diff > 0) {
    peakTriggered = false; // Reset for next peak when frequency starts rising again
  }
  
  // Update prevFrequency and prevDiff with the current values
  prevFrequency = frequency;
  prevDiff = diff;
}

// Define a function to converge the LFSR to the resonant frequency by adjusting its taps
void convergeLFSR() {
  // Define variables to store the best taps so far
  uint8_t bestT0 = tap_0, bestT1 = tap_1, bestT2 = tap_2, bestT3 = tap_3;
  
  // Define a variable to store the best frequency error so far
  float bestError = abs(frequency - resonance);
  
  // Loop over all possible tap combinations for the LFSR_BITS
  for (uint8_t t0 = 0; t0 < LFSR_BITS; t0++) {
    for (uint8_t t1 = t0 + 1; t1 < LFSR_BITS; t1++) {
      for (uint8_t t2 = t1 + 1; t2 < LFSR_BITS; t2++) {
        for (uint8_t t3 = t2 + 1; t3 < LFSR_BITS; t3++) {
          // Update the global taps with the current combination
          tap_0 = t0; tap_1 = t1; tap_2 = t2; tap_3 = t3;
          
          // Re-run part of the logic to see the effect
          updateLFSR();
          readRC();
          
          // Compute the frequency error with respect to the resonance point
          float error = abs(frequency - resonance);
          
          // Check if the error is smaller than the best error so far
          if (error < bestError) {
            bestT0 = t0; bestT1 = t1; bestT2 = t2; bestT3 = t3;
            bestError = error;
          }
        }
      }
    }
  }
  
  // Restore the global taps with the best combination found
  tap_0 = bestT0; tap_1 = bestT1; tap_2 = bestT2; tap_3 = bestT3;

  // Final update with best taps
  updateLFSR();
  readRC();
  
  // Print a message to indicate that the LFSR has converged to the resonant frequency with a certain error
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
  // Set the output pin mode to OUTPUT
  pinMode(OUTPUT_PIN, OUTPUT);
  
  // Set the input pin mode to INPUT
  pinMode(INPUT_PIN, INPUT);
  
  // Initialize the serial communication at 9600 baud rate
  Serial.begin(9600);
}

// Define the loop function that runs repeatedly after the setup function
void loop() {
  // Update the LFSR state and output pin
  updateLFSR();
  
  // Read the RC circuit voltage and compute its resistance and frequency
  readRC();
  
  // Find the RC circuit resonance point by comparing its frequency with previous values
  findResonance();
  
  // Wait for a short delay before repeating the loop
  delay(10);
}
