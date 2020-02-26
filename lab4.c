// FreqEFM8.c: Measure the frequency of a signal on pin T0.
//
// By:  Jesus Calvino-Fraga (c) 2008-2018
//
// The next line clears the "C51 command line options:" field when compiling with CrossIDE
//  ~C51~
  
#include <EFM8LB1.h>
#include <stdio.h>

#define SYSCLK      72000000L  // SYSCLK frequency in Hz
#define BAUDRATE      115200L  // Baud rate of UART in bps

#define LCD_RS P2_6
// #define LCD_RW Px_x // Not used in this code.  Connect to GND
#define LCD_E  P2_5
#define LCD_D4 P2_4
#define LCD_D5 P2_3
#define LCD_D6 P2_2
#define LCD_D7 P2_1
#define CHARS_PER_LINE 16

#define RESISTOR_555 	10000		// Ra=Rb=10kOhm
#define RESISTOR_555_kO 10			// To print to PuTTy
#define BASE_TO_NANO 	1000000000	// convert to nano
// #define BASE_TO_PICO	1000000000000
#define BASE_TO_MICRO	1000000

#define TRUE			1
#define FALSE			0



unsigned char overflow_count;

char _c51_external_startup (void)
{
	// Disable Watchdog with key sequence
	SFRPAGE = 0x00;
	WDTCN = 0xDE; //First key
	WDTCN = 0xAD; //Second key
  
	VDM0CN |= 0x80;
	RSTSRC = 0x02;

	#if (SYSCLK == 48000000L)	
		SFRPAGE = 0x10;
		PFE0CN  = 0x10; // SYSCLK < 50 MHz.
		SFRPAGE = 0x00;
	#elif (SYSCLK == 72000000L)
		SFRPAGE = 0x10;
		PFE0CN  = 0x20; // SYSCLK < 75 MHz.
		SFRPAGE = 0x00;
	#endif
	
	#if (SYSCLK == 12250000L)
		CLKSEL = 0x10;
		CLKSEL = 0x10;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 24500000L)
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 48000000L)	
		// Before setting clock to 48 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x07;
		CLKSEL = 0x07;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 72000000L)
		// Before setting clock to 72 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x03;
		CLKSEL = 0x03;
		while ((CLKSEL & 0x80) == 0);
	#else
		#error SYSCLK must be either 12250000L, 24500000L, 48000000L, or 72000000L
	#endif
	
	P0MDOUT |= 0x10; // Enable UART0 TX as push-pull output
	XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
	XBR1     = 0X10; // Enable T0 on P0.0
	XBR2     = 0x40; // Enable crossbar and weak pull-ups

	#if (((SYSCLK/BAUDRATE)/(2L*12L))>0xFFL)
		#error Timer 0 reload value is incorrect because (SYSCLK/BAUDRATE)/(2L*12L) > 0xFF
	#endif
	// Configure Uart 0
	SCON0 = 0x10;
	CKCON0 |= 0b_0000_0000 ; // Timer 1 uses the system clock divided by 12.
	TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready
	
	return 0;
}
 
// Uses Timer3 to delay <us> micro-seconds. 
void Timer3us(unsigned char us)
{
	unsigned char i;               // usec counter
	
	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON0:
	CKCON0|=0b_0100_0000;
	
	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
	TMR3CN0 = 0x04;                 // Sart Timer3 and clear overflow flag
	for (i = 0; i < us; i++)       // Count <us> overflows
	{
		while (!(TMR3CN0 & 0x80));  // Wait for overflow
		TMR3CN0 &= ~(0x80);         // Clear overflow indicator
		if (TF0)
		{
		   TF0=0;
		   overflow_count++;
		}
	}
	TMR3CN0 = 0 ;                   // Stop Timer3 and clear overflow flag
}

void waitms (unsigned int ms)
{
	unsigned int j;
	for(j=ms; j!=0; j--)
	{
		Timer3us(249);
		Timer3us(249);
		Timer3us(249);
		Timer3us(250);
	}
}

void LCD_pulse (void)
{
	LCD_E=1;
	Timer3us(40);
	LCD_E=0;
}

void LCD_byte (unsigned char x)
{
	// The accumulator in the C8051Fxxx is bit addressable!
	ACC=x; //Send high nible
	LCD_D7=ACC_7;
	LCD_D6=ACC_6;
	LCD_D5=ACC_5;
	LCD_D4=ACC_4;
	LCD_pulse();
	Timer3us(40);
	ACC=x; //Send low nible
	LCD_D7=ACC_3;
	LCD_D6=ACC_2;
	LCD_D5=ACC_1;
	LCD_D4=ACC_0;
	LCD_pulse();
}

void WriteData (unsigned char x)
{
	LCD_RS=1;
	LCD_byte(x);
	waitms(2);
}

void WriteCommand (unsigned char x)
{
	LCD_RS=0;
	LCD_byte(x);
	waitms(5);
}

void LCD_4BIT (void)
{
	LCD_E=0; // Resting state of LCD's enable is zero
	// LCD_RW=0; // We are only writing to the LCD in this program
	waitms(20);
	// First make sure the LCD is in 8-bit mode and then change to 4-bit mode
	WriteCommand(0x33);
	WriteCommand(0x33);
	WriteCommand(0x32); // Change to 4-bit mode

	// Configure the LCD
	WriteCommand(0x28);
	WriteCommand(0x0c);
	WriteCommand(0x01); // Clear screen command (takes some time)
	waitms(20); // Wait for clear screen command to finsih.
}

void LCDprint(char * string, unsigned char line, bit clear)
{
	int j;

	WriteCommand(line==2?0xc0:0x80);
	waitms(5);
	for(j=0; string[j]!=0; j++)	WriteData(string[j]);// Write the message
	if(clear) for(; j<CHARS_PER_LINE; j++) WriteData(' '); // Clear the rest of the line
}

int getsn (char * buff, int len)
{
	int j;
	char c;
	
	for(j=0; j<(len-1); j++)
	{
		c=getchar();
		if ( (c=='\n') || (c=='\r') )
		{
			buff[j]=0;
			return j;
		}
		else
		{
			buff[j]=c;
		}
	}
	buff[j]=0;
	return len;
}

void TIMER0_Init(void)
{
	TMOD&=0b_1111_0000; // Set the bits of Timer/Counter 0 to zero
	TMOD|=0b_0000_0101; // Timer/Counter 0 used as a 16-bit counter
	TR0=0; // Stop Timer/Counter 0
}

void main (void) 
{
	unsigned long frequency;	// same as in FreqEFM8.c
	double denom;
	double capacitance;
	// double capacitance_n;
	// double capacitance_p;
	// double capacitance_micro;

	char freq_char[17];				// string for frequency
	char cap_char[17];				// string for capacitance
	char unit_choice[2];			// stores the string for unit selection
	
	TIMER0_Init();			// initialize timer0
	LCD_4BIT();				// initialize LCD
		    //1234567890123456
	LCDprint("Welcome to L4!", 1, 1);
	waitms(1500); // Give PuTTY a chance to start and display welcome message
	//1234567890123456
	LCDprint(" ", 1, 1);
	LCDprint("F:", 1, 0);
	LCDprint("C:", 2, 0);
	printf("\x1b[2J"); // ANSI escape sequence: \x = hexadecimal, 1b = ESC, [2J = sequence

	// printf goes to PuTTy
	printf ("ELEC 291 Lab 4: 555 Timer and Capacitor Meter\n"
	        "Author(s): Ryan Acapulco, Zhi Chuen Tan\n"
			"Lab Section: L2B (M/W 12-3pm)\n"
			"Term: 2019W2\n\n"
			"R_A = R_B = %ikOhm\n"
			"Compiled: %s, %s\n\n",
	        RESISTOR_555_kO, __DATE__, __TIME__);

	printf ("Please select unit for capacitance:\n"
			"1: nF (nano Farads)\n"
			"2: uF (micro Farads)\n");
	
	getsn(unit_choice, sizeof(unit_choice));

	/***************************************************
	*		   1234567890123456
	*  prints "              nF" and
	*		  "              Hz"
	*  on LCD
	*  I'm doing it this way so it won't overlap when 
	*  I print the actual values to LCD
	****************************************************/
	// LCDprint(FREQ_LCD_MSG, 1, 1);
	// LCDprint(CAP_LCD_MSG, 2, 0);

	while(1)
	{
		// timer setup, idk how this works
		TL0=0;
		TH0=0;
		overflow_count=0;
		TF0=0;
		TR0=1; // Start Timer/Counter 0
		waitms(1000);
		TR0=0; // Stop Timer/Counter 0
		frequency=overflow_count*0x10000L+TH0*0x100L+TL0;
		
		// explicit cast frequency to type double, otherwise value will truncate
		denom = (double) frequency * (3 * RESISTOR_555); // denom=denominator of C=1.44/((3R)*F)
		capacitance = (1.44/denom);		 // capacitance should be 10^-11 or smt, makes it in nF

		switch(unit_choice[0]) {
				case '1':
					capacitance *= BASE_TO_NANO;
					LCDprint("              Hz", 1, 0);
					LCDprint("              nF", 2, 0);
					printf("\rF=%.3luHz, C=%.3fnF", frequency, capacitance);
					break;
				
				case '2':
					capacitance *= BASE_TO_MICRO;
					LCDprint("              Hz", 1, 0);
					LCDprint("              uF", 2, 0);
					printf("\rF=%.3luHz, C=%.3fuF", frequency, capacitance);
					break;
				
				default:
					printf("\rDEFAULT CASE: ERROR\n");
		}
						 
		// converts these to strings, so LCD can print them
		sprintf(freq_char, "       %lu", frequency);		// %lu is type specifier for unsigned long	
		sprintf(cap_char,  "       %.3f", capacitance);		// use %f instead of %lf otherwise code fucks itself

		// prints freq/capacitance value to LCD
		LCDprint(freq_char, 1, 0);	
		LCDprint(cap_char, 2, 0);
				//1234567890123456
		
		LCDprint("F:", 1, 0);
		LCDprint("C:", 2, 0);

		// printf("\rF=%.3luHz, C=%.3fnF", frequency, capacitance);	// this goes to PuTTy
		printf("\x1b[0K"); // ANSI: Clear from cursor to end of line.
	}
	
}
