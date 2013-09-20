/*********************************************************************** 
 * <<<< Found the issue.>>>>
 * Author:  Alan Curley
 * Date:    12 SEP 2013
 * Program: serialTest.C            Version:    1.0
 * Purpose:    This program is a simple test program that tests and outlines
 *             a program that will communicate with the PicKit2 programmer
 *             via the P1 connector using the LPC Demo board.
 *             This way, the PicKit can be left connected to the programming
 *             port for programming and serial communication testing.
 *              - Added interrupt for driving LEDs  to determine what happens
 *                to serial output.
 *              - Added a routine for outputting a string on interrupt from 
 *                port B.
 *              - Change/Add Capture compare routine to Trigger message send.
 *             <<<<  Added the RTCC Interrupts and the RS232 text came out as Jargon.
 *                   Left the data all the same but disabled the RTCC Interrupts
 *                   and the Text becomes clear again.
 *                   HMMMM >>>>
 ***********************************************************************/
#include <16F690.h>
#FUSES  NOWDT, INTRC_IO, NOMCLR, BROWNOUT, NOCPD, NOPUT, NOIESO, NOFCMEN
#use    delay(clock=8000000)
#use    rs232(baud=9600,xmit=PIN_A0,rcv=PIN_A1,parity=N,BITS=8,ERRORS)

#USE    FAST_IO(A)
#USE    FAST_IO(B)
#USE    FAST_IO(C)

/**
 * Declare Variables
 */

#define P1_RX                    PIN_A0
#define P1_TX                    PIN_A1
#define LPC_SW1                  PIN_A3
#define EXT_SW1                  PIN_B4
#define LPC_DS1                  PIN_C0
#define LPC_DS2                  PIN_C1
#define LPC_DS3                  PIN_C2
#define LPC_DS4                  PIN_C3
#define CCP1_PIN                 PIN_C5

//------------------------- Local Variables ------------------------------
 int counterVal;                 // A Counter to demonstrate the functionality
 boolean LED1;                   // Boolean representing LED1
 boolean LED2;                   // Boolean representing LED2 
 boolean LED3;                   // Boolean representing LED3  
 int ccpVal;                     // Value contained in the CCP register.
 int timer1Val;                  // Value representing Timer 01.
 int timerOflwCount;             // Number of times the timer has overflow                
 int timerOflwCount1;             // Number of times the timer has overflow                
 boolean portBInterrupt;         // Interrupt occurred at Port B
 boolean ccpInterrupt;           // Interrupt occurred for capture compare.
                                    
                                 
/** 
 * Interrupt routine on change of Port RB
 */
 
#INT_RB
void portB_ISR(void)
{
   // Clean and Simple... Toggle the LED.
   if(!portBInterrupt)
   {
      LED2= !LED2;
      portBInterrupt = true;
   }   
}

/**
 * Interrupt routine on Capture Compare detection
 */ 
#INT_CCP1
void ccp1_ISR(void)
{
   // Clean and simple to start... Toggle another LED.
   if(!ccpInterrupt)
   {
      LED3 = !LED3;
      ccpInterrupt = true;
      ccpVal = get_timer0();                        // Record the Value of the CCP Register.
   }
}

/**
 * Interrupt routine for Timer elapsing
 */
#INT_RTCC
void rtcc_ISR(void)
{
   if(timerOflwCount==32765)
   {
      timerOflwCount = 0;
      timerOflwCount1++;
   }
   else
      timerOflwCount++;
      
   if(timerOflwCount1 > 32765)
      timerOflwCount1 = 0;
}

/**
 * Main Routine
 */
void main(){

   setup_oscillator(OSC_8MHZ);               // set internal oscillator to 8Mhz
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);  // Set up Timer 1
   setup_ccp1(CCP_CAPTURE_RE);               // CCP set up for rising edge capture.
   set_tris_a(0x3E);                         // setup port a
   set_tris_b(0x10);                         // Pin B4 input (Interrupt)
   set_tris_c(0x20);                         // setup port c (C5 interrupt pin)
   setup_adc_ports(NO_ANALOGS|VSS_VDD);      // No Analog signals      
   enable_interrupts(INT_RB);
   enable_interrupts(INT_CCP1);              // Enable Capture Compare Interrupts.
//   enable_interrupts(INT_RTCC);              // Enable real time counter overflow <<<< This is causeing the RS232 issue to go apeshit.
   enable_interrupts(GLOBAL);                // Enable Global Interrupts

   port_a_pullups(TRUE);
// Setup Default values
   LED1 = false;
   LED2 = false;
   LED3 = false;
   counterVal = 0;
   ccpVal = 0;
   output_low(LPC_DS1);
   output_low(LPC_DS2);
   output_low(LPC_DS3);
   output_low(LPC_DS4);
   
   while(1){        
      counterVal++;                          // Increment CounterVal
      if((counterVal&0X80)>0)
         counterVal = 0;                     // Manage overflow.
      
      printf("Counter at %2u \r\n",counterVal);    // Print out the counter
      LED1 = !LED1;
    
      // Output the Value of LED1 to ... LED1 funnily enough.
      output_bit(LPC_DS1,LED1);
      output_bit(LPC_DS2,LED2);
      output_bit(LPC_DS3,LED3);
      
      if(portBInterrupt)
      {
         printf("Port B interrupt Detected\r\n");
         portBInterrupt = false;
      }
      if(ccpInterrupt)
      {
         printf("CCP interrupt Detected: %d\r\n",ccpVal);
         ccpInterrupt = false;
      }
      
      delay_ms(1000);                        // Delay a second.      
   }
}
