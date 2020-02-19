# Lab 4

## 555 Timer Circuit
Build a 555 timer using discrete parts, **not** the chip itself. The block diagram is:

![555 timer](https://github.com/tzchuen/elec291-lab4-2019w2/blob/master/555_block.png)

Compare the actual and theoretical frequency. **NOTE**: This circuit needs to be demoed. 

### Tips
1. 5.1kΩ or 4.7kΩ resistors are OK, as long as the voltage divider values at the `-` and `+` inputs at the 2 comparators are 2/3 V<sub>CC</sub> and 1/3 V<sub>CC</sub> respectively.
2. Use the `LM393 dual`/`LM339 quad` comparator, which need pull-up resistors from the output of each comparator to V<sub>CC</sub>.
3. Implement the SR flip-flop using either NAND `(74LS00/74HC00)` or NOR `(74LS02/74HC02)` gates. Recall that the inputs of a RS flip-flop implemented using NAND gates are inverted (`R'` and `S'`).
4. Choose the base resistor `R` so that the transistor gets in saturation when the `Q` output of the RS flip-flop is high (5V). Assume a worst case of β=50, and a maximum collector current of 50mA. Use a `2N3904` or `PN2222A` NPN transistor. 

## Capacitance Meter
When used as an astable oscillator, the frequency output of a 555 timer is inversely proportional to the capacitance used in the circuit.

If this frequency is measured using a microcomputer system, the capacitor value used in the timer circuit can be determined.

Build, using an actual `555 single timer`, `556 dual timer`, a capacitance meter that works in the 1 nF to 1μF. Use the `EFM8` board and an LCD to measure and display capacitance.

## Deliverables
- [ ] C code
- [ ] Picture of system
