#include <18F27K40.h>
#fuses NOMCLR RSTOSC_HFINTRC_64MHZ CLKOUT NOWDT NOPROTECT NOCPD
#use delay(internal=32MHZ, clock_out)
#define Forever 1

//using specific pins from PIC18F27K40
#pin_select PWM4 = PIN_A4  
#pin_select SDO2 = PIN_B4
#pin_select SCK2 = PIN_B5
#use SPI(SPI2, MASTER, MODE=0, BITS=8) //allows us to use spi

int LUT_Index;  //Look-up table counter
//Look-up Tables from Matlab
const int16 SineWave_OneKHZ[32]= {0x1800,0x198F,0x1B0F,0x1C71,0x1DA7,0x1EA6,0x1F63,0x1FD8,0x1FFF,
                                 0x1FD8,0x1F63,0x1EA6,0x1DA7,0x1C71,0x1B0F,0x198F,0x1800,0x1670,
                                 0x14F0,0x138E,0x1258,0x1159,0x109C,0x1027,0x1000,0x1027,0x109C,
                                 0x1159,0x1258,0x138E,0x14F0,0x1670};
const int16 SineWave_TwoKHZ[32]= {0x1800,0x1B0F,0x1DA7,0x1F63,0x1FFF,0x1F63,0x1DA7,0x1B0F,0x1800,
                                 0x14F0,0x1258,0x109C,0x1000,0x109C,0x1258,0x14F0,0x1800,0x1B0F,
                                 0x1DA7,0x1F63,0x1FFF,0x1F63,0x1DA7,0x1B0F,0x1800,0x14F0,0x1258,
                                 0x109C,0x1000,0x109C,0x1258,0x14F0};
const int16 Combined_SinWave[32]={0x1800,0x1A4F,0x1C5B,0x1DEA,0x1ED3,0x1F05,0x1E85,0x1D73,0x1BFF,
                                 0x1A64,0x18DD,0x17A1,0x16D4,0x1686,0x16B3,0x173F,0x1800,0x18C0,
                                 0x194C,0x1979,0x192B,0x185E,0x1722,0x159B,0x1400,0x128C,0x117A,
                                 0x10FA,0x112C,0x1215,0x13A4,0x15B0};

struct First_Structure {
   int RA_switches: 4;            //4 RA switches
   int1 Pulse_Width_Modulator;    //PWM Signal to MCP
   int1 Chip_Select;              //CS Signal to MCP
   int1 Clock_Output;             //Clock at RA6
   int1 Debug_Signal;             //Debug Test Signal 
   int RB_switches: 4;            //4 RB switches (unused)
   int MSSP2: 2;                  //SDO and SCK Signal 
   int unused_B: 2;               //unused B pins
   int unused_C;                  //all C pins unused
} IO_Port_One, IO_Direction_One, IO_Latch_One;

#byte IO_Port_One = 0xF8D        // PORTA register in Special Function Register (SFR)
#byte IO_Direction_One = 0xF88   // LATA register
#byte IO_Latch_One = 0xF83       // TRISA register

#int_timer2
void Timer_Two_Interrupt_Routine() {
   IO_Port_One.Debug_Signal = 0b1;  //Testing interrupt using one of the pins
   IO_Port_One.Chip_Select = 0b0;  //Chip Select is Low when connecting to converter
   switch(IO_Port_One.RA_switches)
   {
        case 0b0000: //1k SineWave
        {
            spi_write2(SineWave_OneKHZ[LUT_Index]>>8);       // High byte
            spi_write2(SineWave_OneKHZ[LUT_Index] & 0x00FF); // low byte
            LUT_Index = ++LUT_Index % 32;
            break;
        }
        case 0b0001: //2k SineWave
        {
            spi_write2(SineWave_TwoKHZ[LUT_Index]>>8);       // High byte
            spi_write2(SineWave_TwoKHZ[LUT_Index] & 0x00FF); // low byte
            LUT_Index = ++LUT_Index % 32;
            break;
        }
        case 0b0011: //1k + 2k SineWave
        {
            spi_write2(Combined_SinWave[LUT_Index]>>8);       // High byte
            spi_write2(Combined_SinWave[LUT_Index] & 0x00FF); // low byte
            LUT_Index = ++LUT_Index % 32;
            break;
        }
        default:
        {
            LUT_Index = 0;
        }
   }
   IO_Port_One.Chip_Select = 0b1;  //CS becomes High 
   IO_Port_One.Debug_Signal = 0b0;
}

void main () {
   IO_Direction_One.RA_switches = 0b1111;           //set as inputs because switches
   IO_Direction_One.Pulse_Width_Modulator= 0b0;     //pwm signal (output)
   IO_Direction_One.Chip_Select = 0b0;              //cs signal as output
   IO_Direction_One.Clock_Output = 0b0;             //clock output
   IO_Direction_One.Debug_Signal = 0b0;             //debug signal test as output
   IO_Direction_One.RB_switches = 0b1111;           //RB switches as inputs
   IO_Direction_One.MSSP2 = 0b00;                   //output to MCP4821 SDO and SCK
   IO_Direction_One.unused_B = 0b00;                //remaining B pins as output
   IO_Direction_One.unused_C = 0b00000000;          //all C as output
   
   port_a_pullups(0xFF);                            //weak pull-up resistor for switches in RA
   
   //setting PWM pulse
   setup_ccp2(CCP_PWM|CCP_USE_TIMER1_AND_TIMER2);       // Configure CCP2 as a PWM, //CCP2 is paired with Timer 2
   setup_pwm4(PWM_ENABLED|PWM_ACTIVE_HIGH|PWM_TIMER2);  // enabling pwm
   set_pwm4_duty(968);                                  // setting pwm ccprx twin register value
   
   //frequency 
   setup_timer_2(T2_CLK_INTERNAL | T2_DIV_BY_1, 249, 1); //prescaler, PR, postscaler
   
   //interrupts that might be enabled later
   enable_interrupts(INT_TIMER2); // Timer 2 interrupt enable
   enable_interrupts(GLOBAL); // 'Global' interrupt enable
   
   //setting spi taking signals when rising (Low to High)
   setup_spi(SPI_Master | SPI_L_TO_H);
   
   while(Forever)
   {
      //stays on, FOREVER...
   }
}


