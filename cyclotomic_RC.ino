
// Define the digital output pin that drives the RC circuit
#define OUTPUT_PIN 13

// Define the analog input pin that reads the RC circuit voltage
#define INPUT_PIN A0

// Define the known value of the capacitor in Farads
#define CAPACITOR_VALUE 0.00001

// Define the number of bits for the LFSR
#define LFSR_BITS 8

// Define the taps for the LFSR feedback function
#define TAP_0 0
#define TAP_1 2
#define TAP_2 3
#define TAP_3 4

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

// Function prototypes to avoid compilation issues in C++
void updateLFSR();
void readRC();
void findResonance();

// Track time and voltage for accurate R calculation
unsigned long lastUpdateUs = 0;
uint8_t lastPinState = LOW;
float v_initial = 0;

// Define a function to update the LFSR state and output pin
void updateLFSR() {
  // Capture voltage at the moment of transition
  v_initial = analogRead(INPUT_PIN) * (5.0 / 1023.0);

  // Compute the feedback bit by XOR-ing the tap bits
  uint8_t feedback = ((lfsr >> TAP_0) ^ (lfsr >> TAP_1) ^ (lfsr >> TAP_2) ^ (lfsr >> TAP_3)) & 1;
  
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
  // v(t) = v_drive + (v_initial - v_drive) * exp(-dt / RC)
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
  } else if (diff > 0) {
    peakTriggered = false;
  }
  
  prevFrequency = frequency;
  prevDiff = diff;
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
