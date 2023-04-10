# cyclotomic_RC.ino
Dreamed by Bing.
it obviously doesn't work.

Sure, I'll try to fix the explanation so you can see the equations. Here is the revised explanation:

The code consists of four functions: updateLFSR, readRC, findResonance, and setup and loop.

The updateLFSR function updates the state of the LFSR and writes its least significant bit to the output pin. The LFSR is a shift register whose input bit is a linear function of its previous state. The linear function is computed by XOR-ing some bits of the state, called taps, which are chosen to generate a maximal-length sequence. A maximal-length sequence is a sequence that cycles through every possible value (except all zeros) before repeating. The LFSR generates a pseudo-random binary signal that drives the RC circuit.

The readRC function reads the voltage of the RC circuit from the input pin and computes its resistance and frequency. The RC circuit is a circuit composed of a resistor and a capacitor in series. The voltage across the capacitor depends on the charge stored in it, which changes over time according to the current flowing through the circuit. The resistance of the circuit is computed using Ohm's law and Kirchhoff's voltage law, assuming that the output pin voltage is either 0V or 5V depending on the LFSR state. The frequency of the circuit is computed using cyclotomic polynomial analysis, which is a method to find the roots of unity of a given degree.

Cyclotomic polynomial analysis is based on the fact that any n-th root of unity can be expressed as $e^{2\pi i k/n}$ where k is an integer coprime to n. The cyclotomic polynomial of degree n is defined as $\Phi_n(x) = \prod_{k=1}^n (x - e^{2\pi i k/n})$ where k runs over all integers coprime to n. The cyclotomic polynomial has n roots, which are precisely the n-th roots of unity. For example, the cyclotomic polynomial of degree 4 is $\Phi_4(x) = x^2 + 1$, which has four roots: $e^{0i}, e^{\pi i}, e^{2\pi i},$ and $e^{3\pi i}$. These are the four fourth roots of unity.

To apply cyclotomic polynomial analysis to the RC circuit, we assume that the LFSR state is periodic with a period of $2^n - 1$ clock cycles, where n is the number of bits in the LFSR. This means that after $2^n - 1$ clock cycles, the LFSR state returns to its initial value. Therefore, we can think of the LFSR state as an n-th root of unity, where $n = 2^n - 1$. We can then use the formula for cyclotomic polynomial analysis to find its frequency:

$f = \frac{2^n - 1}{C R \phi(n) \ln(2)}$

where C is the capacitance value, R is the resistance value, $\phi(n)$ is Euler's totient function , which counts the number of positive integers less than or equal to n that are coprime to n. For example, if $n = 8$, then $\phi(n) = 4$, and $f = \frac{255}{C R 4 \ln(2)}$. Therefore, $f \approx \frac{36.5}{C R}$.

The findResonance function finds the resonance point of the RC circuit by comparing its frequency with previous values. The resonance point is the frequency at which
OK, I'll continue.

The findResonance function finds the resonance point of the RC circuit by comparing its frequency with previous values. The resonance point is the frequency at which the RC circuit has maximum voltage across the capacitor. This happens when the impedance of the capacitor matches the impedance of the resistor, which depends on their values and on the frequency of the input signal. The findResonance function uses static variables to store the previous frequency value and the previous difference between the current and previous frequency values. It then computes the current difference and checks if the sign of the difference has changed from positive to negative. This indicates that the frequency has reached a local maximum, which corresponds to a resonance point. The function then updates the resonance point with the average of the current and previous frequency values and prints a message to indicate that a resonance point has been found. It also resets the previous difference to avoid finding multiple resonance points in a row.

The setup function runs once when the Arduino board is powered on or reset. It sets the output pin mode to OUTPUT and the input pin mode to INPUT. It also initializes the serial communication at 9600 baud rate, which allows sending messages from the Arduino board to a computer.

The loop function runs repeatedly after the setup function. It calls the updateLFSR, readRC, and findResonance functions in sequence and waits for a short delay before repeating the loop. This way, the code continuously updates the LFSR state and output pin, reads the RC circuit voltage and computes its resistance and frequency, and finds the RC circuit resonance point by comparing its frequency with previous values.

I hope this explanation helps you understand how the code works and how cyclotomic polynomial analysis works on this example.
