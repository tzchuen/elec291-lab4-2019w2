# Lab 4

## 555 Timer Circuit
Labelled pic of built circuit:

![Labelled](https://github.com/tzchuen/elec291-lab4-2019w2/blob/master/IMG_20200224_140058.jpg)

**NOTE**: This circuit needs to be demoed. 

### Tips vs What I've Done
1. 5.1kΩ used
2. `LM393 dual` comparator used.
3. NAND `74HC00` used for SR flip-flop
4. Choose the base resistor `R` so that the transistor gets in saturation when the `Q` output of the RS flip-flop is high (5V). Assume a worst case of β=50, and a maximum collector current of 50mA. Use a `2N3904` or `PN2222A` NPN transistor. 

## Capacitance Meter
When used as an astable oscillator, the frequency output of a 555 timer is inversely proportional to the capacitance used in the circuit.

If this frequency is measured using a microcomputer system, the capacitor value used in the timer circuit can be determined.

Build, using an actual `555 single timer`, `556 dual timer`, a capacitance meter that works in the 1 nF to 1μF. Use the `EFM8` board and an LCD to measure and display capacitance.

## To-Do
- [ ] Build circuit
  - [ ] Decide on R value for BJT
- [X] C code
- [X] Integrate to hardware
- [ ] Bonus

### Bonus Ideas
- Could try a switch statement to ask for input from PuTTy, like in `EFM8_LCD_4bit.c`, to switch from nF to uF etc.
- Use the same technique to print to LCD

## Deliverables
- [ ] C code
- [ ] Picture of system
