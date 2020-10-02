#include <18F27K40.h>
#fuses NOMCLR RSTOSC_HFINTRC_1MHZ CLKOUT NOWDT NOPROTECT NOCPD NOPUT //operational fuses and configuration bits
#use delay(internal=16MHZ, clock_out) //changing clock speed to 16MHz, outputting 4MHz (f/4) at pin RA6
#define Forever 1 //defining Forever as true

struct IOPort_Definition1 {                      // 1st structure
   int ExperimentSelect:4;  // Pins RA0..3 for selection
   int unused_A:4;          // remaining RA pins
   int FourBitInput:4;      // 4 switches from Port B (RB0..3)
   int unused_B:4;          // unused B pins
   int MultiBitOutput:4;    // Pins RC0..3 for 4 bit outputs
   int unused_C:4;          // Remaining bits of Port C;
} IO_Port_One, IO_Direction_One, IO_Latch_One;        // naming variables of 1st structure

struct IOPort_Definition2 {     // 2nd structure
   int ExperimentSelect:4;  // Pins RA0..3 for selection
   int unused_A1:2;         // unused A pins
   int1 Clock_Output;       // Clock output for reading frequency at pin RA6
   int1 unused_A2;          // unused pin RA7
   int1 Set;                // Set switch at pin RB0
   int1 Reset;              // Reset switch at pin RB1
   int unused_B:6;          // remaining RB pins	
   int SixBitOutput:6;      // pins RC0..5 for 6 bit led output;
   int unused_C:2;          // remaining RC pins
} IO_Port_Two, IO_Direction_Two, IO_Latch_Two;       //naming variables of 2nd structure 

struct IOPort_Definition3 {                       // 3rd strucutre
   int ExperimentSelect:4;  // Pins RA0..3 for selection
   int unused_A:4;          // remaining RA pins
   int Logic_Input_One:2;   // bitwise logic input in RB0..1
   int Logic_Input_Two:2;   // bitwise logic input in RB2..3   
   int unused_B:4;          // remaining RB pins
   int ThreeBitOutput_One:3;// LED output pin RC0..2
   int ThreeBitOutput_Two:3;// LED output pin RC3..5 
   int unused_C:2;          // unused RC6..7
} IO_Port_Three, IO_Direction_Three, IO_Latch_Three; //naming variables of 3rd structure

#byte IO_Port_One = 0xF8D       // PORTA register in Special Function Register (SFR)
#byte IO_Direction_One = 0xF88  // TRISA register
#byte IO_Latch_One = 0xF83      // LATA register

#byte IO_Port_Two = 0xF8D       // PORTA register
#byte IO_Direction_Two = 0xF88  // TRISA register
#byte IO_Latch_Two = 0xF83      // LATA register

#byte IO_Port_Three = 0xF8D       // PORTA register
#byte IO_Direction_Three = 0xF88  // TRISA register
#byte IO_Latch_Three = 0xF83      // LATA register

void main () {
   //Setting directions of all IO pins
   IO_Direction_One.ExperimentSelect = 0b1111; // Main switch at pins RA0..3 for experiment selection
   IO_Direction_One.unused_A = 0b0000;         // unused RA pins set as output
   IO_Direction_One.FourBitInput= 0b1111;      // Input switches at pins RB0..3 for logic operation and input look up table
   IO_Direction_One.unused_B = 0b0000;         // unused RB pins set as output
   IO_Direction_Two.SixBitOutput = 0b000000;   // largest group specifying all 6 LEDs as output at pins RC0..5
   IO_Direction_Two.unused_C = 0b00;           // unused RC pins set as output 
   
   //Look up Tables
   int LUT[16] = {0x00, 0x01, 0x03, 0x02, 0x06, 0x07, 0x05, 0x04, 0x0C, 0x0D, 0x0F, 0x0E, 0x0A, 0x0B, 0x09, 0x08};  //Graycode
   int samplingrates[16] = {50, 90, 120, 400, 500, 800, 940, 1000, 1350, 1800, 2000, 3000, 4000, 6000, 8000, 10000};//Non-linear sampling rate
   int FSMLab_One[10] = {0x1E, 0x04, 0x05, 0x0F, 0x0D, 0x02, 0x03, 0x0B, 0x0C, 0x01};                               //FSM Lab 1
   int RunningLight [7] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20};                                               //Running light
   int counter = 0;
   
   //Enabling buffer for pullup resistor
   port_a_pullups(0xFF);
   port_b_pullups(0xFF);
   port_c_pullups(0x00);
    
   //Active Code, never going to sleep
   while(Forever) {
      switch (IO_Port_One.ExperimentSelect) { //switch case block for switching between experiments
         case 0b0000: //4Mhz frequency at pin RA6
         {
            IO_Port_One.MultiBitOutput = 0b0000;
            break;
         }
         case 0b0001: //single led toggling
         {
            IO_Port_One.MultiBitOutput = 0b0001;  // Toggle single LED
            delay_ms(500); //delays for 1 second before restarting loop
            IO_Port_One.MultiBitOutput = 0b0000;
            delay_ms(500);
            break;
         }
         case 0b0011: //4bit pure binary up counter
         {
            IO_Port_One.MultiBitOutput++;
            delay_ms(1000);
            break;
         }
         case 0b0100: //4bit pure binary down counter
         {
            IO_Port_One.MultiBitOutput--;
            delay_ms(1000);
            break;
         }
         case 0b0101: //4-bit Graycode up counter
         {
            IO_Port_One.MultiBitOutput = LUT[++counter&0b00001111];
            delay_ms(1000);
            break;
         }
         case 0b0110: //3-bit pure binary up-counter with 3-bit gray-code up-counter
         {
            IO_Port_Three.ThreeBitOutput_One++;
            IO_Port_Three.ThreeBitOutput_Two = LUT[++counter&0b00000111];
            delay_ms(1000);
            break;
         }
         case 0b0111: //bitwise AND operation
         {
            IO_Port_One.MultiBitOutput = IO_Port_Three.Logic_Input_One & IO_Port_Three.Logic_Input_Two;
            break;
         }
         case 0b1000: //bitwise OR operation
         {
            IO_Port_One.MultiBitOutput = IO_Port_Three.Logic_Input_One | IO_Port_Three.Logic_Input_Two;
            break;
         }
         case 0b1001: //bitwise NOT operation
         {
            IO_Port_One.MultiBitOutput = ~IO_Port_One.FourBitInput;
            break;
         }
         case 0b1010: //bitwise XOR operation
         {
            IO_Port_One.MultiBitOutput = IO_Port_Three.Logic_Input_One ^ IO_Port_Three.Logic_Input_Two;
            break;
         }
         case 0b1011: //set-reset flip flop
         {
            IO_Port_Three.ThreeBitOutput_One = (IO_Port_Two.Set | (~IO_Port_Two.Reset & IO_Port_Three.ThreeBitOutput_One));
            break;
         }
         case 0b1100: //Input Look-up-table Demonstration using switches RB0..3
         {
            IO_Port_Two.SixBitOutput = RunningLight[++counter%7];
            delay_ms(samplingrates[IO_Port_One.FourBitInput]);
            break;
         }
         case 0b1101: //Lab 1 FSM using Look-up Table
         {
            IO_Port_Two.SixBitOutput = FSMLab_One[++counter%10];
            delay_ms(2000);
            break;
         }
         default:     //Every other option not in the specification turns off all LEDs
         {
            IO_Port_Two.SixBitOutput = 0b000000;
            counter = 0;
         }
      }   
   }
}

