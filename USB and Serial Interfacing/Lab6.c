#include <18F27K40.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#fuses NOMCLR RSTOSC_HFINTRC_64MHZ CLKOUT PUT
 
#use delay(internal = 32MHZ, clock_out)
#define Forever 1

//defining pins for FTDI and PWM
#pin_select PWM4=PIN_C4
#pin_select PWM4=PIN_C5
#pin_select U1TX=PIN_C6
#pin_select U1RX=PIN_C7
//RS232 has same values with configuration of Putty
#use RS232(uart1, baud=9600, BITS = 8, STOP = 2, Parity = N, ERRORS, STREAM = Serial_Stream)

//Buffer Definitions
#define RS232BUFFSIZE 32
char RS232_buffer[RS232BUFFSIZE];
int RS232_next_in = 0;    //input pointer
int RS232_next_out = 0;   //output pointer
char brightness[20];      //character to get string of numbers for brightness percentage

//structure
struct Structure_1 
{
   int unused_A;          //All A pins not used
   int unused_B;          //All B pins not used
   int1 LED1;             //LEDs
   int1 LED2;
   int1 LED3;
   int1 LED4;
   int unused_C:2;        //2 unused C pins
   int1 TXpin;            //Transmit Pin to FTDI
   int1 RXpin;            //Receive Pin from FTDI
} Port, Direction, Latch; //setting variable names

//setting variables to SFR addresses
#byte Port = 0xF8D       /* PORTA register in SFR */
#byte Latch = 0xF83      /* LATA register in SFR  */
#byte Direction = 0xF88  /* TRISA register in SFR */

#int_RDA //receive data available, next function is interrupt
void RS232_Interrupt_Routine()  //RS232 Receive Data Available
{  
   RS232_buffer[RS232_next_in] = fgetc(Serial_Stream);     //get character and place in buffer
   if ((RS232_next_in+1) % RS232BUFFSIZE != RS232_next_out)//if not last, keep adding
   {
      RS232_next_in = (RS232_next_in+1)%RS232BUFFSIZE;
   }
}

//function that returns a string without leading spaces
char GetCom (char * command_string) 
{
   int length = 0;
   char c;
   RS232_next_out = 0;
   
   do //ignoring leading spaces
   {
      c = RS232_buffer[RS232_next_out]; //getting character from buffer
      RS232_next_out = (RS232_next_out+1)%RS232BUFFSIZE;
   } while (c == ' ');
   
   //Entering non-space character
   command_string[length++] = c;
   
   //getting rest of command string
   while((c != 13) && (length<20))
   {
      c = RS232_buffer[RS232_next_out];
      RS232_next_out = (RS232_next_out+1)%RS232BUFFSIZE;
      command_string[length++] = c;
   }
   command_string[--length] = '\0';
   return command_string;
}//end getcom

//function to only get the numbers from brightness[value] and convert it into a float
float GetBrightness(char * brightness_input)
{
   int i;
   int length = 0;
   float level;
   
   //only takes number out of string
   while(length <= 20)
   {
      if((brightness_input[length] >= '0') && (brightness_input[length] <= '9') || (brightness_input == '.'))
      {
         brightness[i] = brightness_input[length];
         i++;
      }
      length++;
   }
   level = atof(brightness);  //converts string to float
   return level;              //return float back to receive commmand 
} //end adjusting brightness level

void Cmd_Receive(char * string)
{
   //string functions used:                                          
   //strcmp compares 2 strings and if they are identical it returns 0
   //strupr converts a given string to uppercase                     
   //strstr uses a main string to find a desired string we want inside that main string
   
   //string definitions to be compared
   char LED1ON[] = "LED 1 ON";
   char LED1OFF[] = "LED 1 OFF";
   char LED2ON[] = "LED 2 ON";
   char LED2OFF[] = "LED 2 OFF";
   char LED3ON[] = "LED 3 ON";
   char LED3OFF[] = "LED 3 OFF";
   char LED4ON[] = "LED 4 ON";
   char LED4OFF[] = "LED 4 OFF";
   char LED1FLASH[] = "LED 1 FLASH";
   char LED2FLASH[] = "LED 2 FLASH";
   char LED3FLASH[] = "LED 3 FLASH";
   char LED4FLASH[] = "LED 4 FLASH";
   char BRIGHTNESS[] = "BRIGHTNESS";
   int flash = 0;
   float value;

   //string comparison to determine which operation to do
   if (strcmp(strupr(string), LED1ON) == 0) 
   {
      puts("OK\n");
      Port.LED1 = 0b1;
   }
   else if (strcmp(strupr(string), LED1OFF) == 0)
   {
      puts("OK\n");
      Port.LED1 = 0b0;
   }
   else if (strcmp(strupr(string), LED2ON) == 0)
   {
      puts("OK\n");
      Port.LED2 = 0b1;
   }
   else if (strcmp(strupr(string), LED2OFF) == 0)
   {
      puts("OK\n");
      Port.LED2 = 0b0;
   }
   else if (strcmp(strupr(string), LED3ON) == 0)
   {
      puts("OK\n");
      Port.LED3 = 0b1;
   }
   else if (strcmp(strupr(string), LED3OFF) == 0)
   {
      puts("OK\n");
      Port.LED3 = 0b0;
   }
   else if (strcmp(strupr(string), LED4ON) == 0)
   {
      puts("OK\n");
      Port.LED4 = 0b1;
   }
   else if (strcmp(strupr(string), LED4OFF) == 0)
   {
      puts("OK\n");
      Port.LED4 = 0b0;
   }
   else if (strcmp(strupr(string), LED1FLASH) == 0)
   {
      puts("OK\n");
      for(flash = 0; flash < 10; flash++)
      {
         Port.LED1 = ~Port.LED1;
         delay_ms(80);
      }
   }
   else if (strcmp(strupr(string), LED2FLASH) == 0)
   {
      puts("OK\n");
      for(flash = 0; flash < 10; flash++)
      {
         Port.LED2 = ~Port.LED2;
         delay_ms(80);
      }
   }
   else if (strcmp(strupr(string), LED3FLASH) == 0)
   {
      puts("OK\n");
      for(flash = 0; flash < 10; flash++)
      {
         Port.LED3 = ~Port.LED3;
         delay_ms(80);
      }
   }
   else if (strcmp(strupr(string), LED4FLASH) == 0)
   {
      puts("OK\n");
      for(flash = 0; flash < 10; flash++)
      {
         Port.LED4 = ~Port.LED4;
         delay_ms(80);
      }
   }
   else if (strstr(strupr(string), BRIGHTNESS))
   {
      puts("OK\n");
      value = GetBrightness(string)/10;
      //for a clock running at 32MHz
     //brightness is given in percent and the full scale duty cycle is 2^10 = 1024     
      int16 a = ((value*256)/100); // to convert brightness to duty cycle brightness/100 * 1024/4    
      set_pwm4_duty(a);
   }
   else
   {  //if anything other than the available commands are typed, outputs error
      puts("Invalid Input! Try Again.\n");
   }
}//end receive command

void main ()
{
   short int startup = FALSE;   //boolean for first time opening Putty
   char kbhit;                  //pressing keyboard
   char command[RS232BUFFSIZE]; //string array to place characters from buffer
   
   //Pin directions
   Direction.unused_A = 0b00000000; //all unused set as output
   Direction.unused_B = 0b00000000;
   Direction.LED1 = 0b0;            //LEDs as output
   Direction.LED2 = 0b0;
   Direction.LED3 = 0b0;
   Direction.LED4 = 0b0;
   Direction.unused_C = 0b00;
   Direction.TXPin = 0b0;           //2 pins to FTDI, TX is transmit (output from PIC)
   Direction.RXPin = 0b1;           //RX is receive (input to PIC)
   
   setup_ccp2(CCP_PWM | CCP_USE_TIMER1_AND_TIMER2);        //Configure CCP2 as PWM
   setup_pwm4(PWM_ENABLED | PWM_ACTIVE_HIGH | PWM_TIMER2); //enabling pwm
   setup_timer_2(T2_CLK_INTERNAL|T2_DIV_BY_1, 249, 1);     //prescaler, PR, postscaler
   
   enable_interrupts(INT_RDA); //Initializing System and interrupts
   if (kbhit())                
      fgetc(Serial_Stream);
   enable_interrupts(GLOBAL);

   While(Forever)
   {
      while (startup == FALSE) //occurs only once, title and explanation at top of Putty, neat and tidy
      {
         if (RS232_next_in != 0)
         {
            puts("\t\t***Group 23 EE2A Lab 6***\n\r");
            puts("Available Commands:\n");
            puts("   -Turning LEDs ON-         -Turning LEDs OFF-          -Flashing LEDs-\n");
            puts("       LED x on                  LED x off                 LED x flash\n\r");
            puts("Where x = 1,2,3 or 4\n\rlower and uppercase does not matter\n\r");
            puts("IMPORTANT: Space are required!!! Backspace does not work!\n\r");
            puts("-Changing LED Brightness-\n");
            puts("Brightness[value], where value = 0 - 99 (positive numbers)\n\r--------------------------------------------\n\r");
            startup = TRUE;
            break;
         }
      }
      if (RS232_next_in != RS232_next_out)         //as long as the loop has not restarted
      {
         kbhit = RS232_buffer[RS232_next_out];     //puts characters from buffer into single character
         putc(kbhit);                              //prints characters hit from keyboard
         RS232_next_out = (RS232_next_out+1)%RS232BUFFSIZE;
         if (kbhit == '\r')                        //when pressing enter
         {
            puts('\n');                            //new line
            strcpy(command, GetCom(RS232_buffer)); //send to command and get rid of spaces
            Cmd_Receive(command);                  //send string to operation
            RS232_next_out = 0; RS232_next_in = 0; //reset to 0 when enter
         }
      }
   }//end while
}//end main
