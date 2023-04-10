
// Define the digital output pin that drives the RC circuit
#define OUTPUT_PIN 13

// Define the analog input pin that reads the RC circuit voltage
#define INPUT_PIN A0

// Define the known value of the capacitor in Farads
#define CAPACITOR_VALUE 0.00001

// Define the number of bits for the LFSR
#define LFSR_BITS 8

// Define the taps for the LFSR feedback function
// This example uses [0,2,3,4] which generates a maximal-length sequence for 8 bits
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

// Define a function to update the LFSR state and output pin
void updateLFSR() {
  // Compute the feedback bit by XOR-ing the tap bits
  uint8_t feedback = ((lfsr >> TAP_0) ^ (lfsr >> TAP_1) ^ (lfsr >> TAP_2) ^ (lfsr >> TAP_3)) & 1;
  
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
  // Assume that the output pin voltage is either 0V or 5V depending on the LFSR state
  resistance = (5.0 - voltage) / (voltage / CAPACITOR_VALUE);
  
  // Compute the frequency of the RC circuit using cyclotomic polynomial analysis
  // Assume that the LFSR state is periodic with a period of (2^LFSR_BITS - 1) clock cycles
  // The formula for cyclotomic polynomial analysis is derived from https://en.wikipedia.org/wiki/Cyclotomic_polynomial#Cyclotomic_fields_and_Gauss_sums
  frequency = (pow(2, LFSR_BITS) - 1) / (CAPACITOR_VALUE * resistance * log(2));
}

// Define a function to find the RC circuit resonance point by comparing its frequency with previous values
void findResonance() {
  // Define a static variable to store the previous frequency value
  static float prevFrequency = 0;
  
  // Define a static variable to store the previous difference between frequency and prevFrequency
  static float prevDiff = 0;
  
  // Compute the current difference between frequency and prevFrequency
  float diff = frequency - prevFrequency;
  
  // Check if the sign of diff has changed from positive to negative, indicating a local maximum
  if (diff < 0 && prevDiff > 0) {
    // Update the resonance point with the average of frequency and prevFrequency
    resonance = (frequency + prevFrequency) / 2;
    
    // Print a message to indicate that a resonance point has been found
    Serial.print("Resonance point found at ");
    Serial.print(resonance);
    Serial.println(" Hz");
    
    // Reset prevDiff to avoid finding multiple resonance points in a row
    prevDiff = diff;
    
    return; // Exit the function early
  }
  
  // Update prevFrequency and prevDiff with the current values
  prevFrequency = frequency;
  prevDiff = diff;
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
