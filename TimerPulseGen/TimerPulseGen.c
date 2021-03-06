/*********************************************************************** 
 * Author:  Alan Curley
 * Date:    21 APR 2014
 * Program: TimerPulseGen.C            Version:    1.0
 * Purpose:    This program is a simple test program that will generate
 * 			a Pulse for a given time based on the value of the POT.
 * 			This will be initially based on the LPC board but upgraded as the
 * 			LPC Board will be needed for the measurement.
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
 boolean LED1;                   // Boolean representing LED1
 boolean LED2;                   // Boolean representing LED2 
 boolean LED3;                   // Boolean representing LED3  
 boolean LED4;                   // Boolean representing LED4
 int ccpVal;                     // Value contained in the CCP register.
 int timer1Val;                  // Value representing Timer 01.
 int timerOflwCount;             // Number of times the timer has overflow                
 int timerOflwCount1;            // Number of times the timer has overflow                
 boolean ccpInterrupt;           // Interrupt occurred for capture compare.
 boolean risingEdge;             // Monitoring the Rising edge or Falling edge of CCP?
                                    
                                 
/** 
 * Interrupt routine on change of Port RB
 */
 
/**   
 * Interrupt routine on Capture Compare detection
 */ 
#INT_CCP1
void ccp1_ISR(void)
{
   LED1= !LED1; 
   // Add the additional maintenance routine here.
   if(risingEdge)                               // Monitoring the rising edge
   {                                            
      set_timer0(0);                            // Reset the timer to Zero
      timerOflwCount = 0;                       // as well as the overflows.
      timerOflwCount1= 0;
      setup_ccp1(CCP_CAPTURE_FE);               // CCP set up for falling edge capture.
      risingEdge = false;                        // Start monitoring the falling edge
   }
   else                                         // Monitoring the Falling edge.
   {
      ccpInterrupt = true;                      // Main routine to print the results.
      setup_ccp1(CCP_CAPTURE_RE);               // Start monitoring the Rising Edge again.
      //disable_interrupts(INT_RTCC);             // Disabling the Timer RTCC - No adjust overflows.
      risingEdge = true;                        // Start monitoring the rising edge
      ccpVal = get_timer0();                    // Record the Value of the CCP Register.
   }
   printf("CCP interrupt Detected: %d - %d - %u\r\n",timerOflwCount1,timerOflwCount,ccpVal);
   output_bit(LPC_DS1,LED1);
   output_bit(LPC_DS2,LED2);
   output_bit(LPC_DS3,LED3);

}

/**
 * Interrupt routine for Timer elapsing
 */
#INT_RTCC
void rtcc_ISR(void)
{
   LED2 != LED2;
   if(timerOflwCount>=127)
   {
      LED3 != LED3;
      timerOflwCount = 0;
      timerOflwCount1++;
   }
   else
      timerOflwCount++;
      
   if(timerOflwCount1 >= 127){
      timerOflwCount1 = 0;
      LED4 != LED4;
   }
   output_bit(LPC_DS2,LED2);
   output_bit(LPC_DS3,LED3);
   output_bit(LPC_DS4,LED4);

}

/**
 * Main Routine
 */
void main(){

   setup_oscillator(OSC_8MHZ);               // set internal oscillator to 8Mhz
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);  // Set up Timer 1
   setup_ccp1(CCP_CAPTURE_RE);               // CCP set up for rising edge capture.
   set_tris_a(0x3E);                         // setup port a
   //set_tris_b(0x10);                         // Pin B4 input (Interrupt)
   set_tris_c(0x20);                         // setup port c (C5 interrupt pin)
   setup_adc_ports(NO_ANALOGS|VSS_VDD);      // No Analog signals      
   //enable_interrupts(INT_RB);
   enable_interrupts(INT_CCP1);              // Enable Capture Compare Interrupts.
   enable_interrupts(INT_RTCC);              // Enable real time counter overflow <<<< This is causeing the RS232 issue to go apeshit.
   enable_interrupts(GLOBAL);                // Enable Global Interrupts

   port_a_pullups(TRUE);

// Setup Default values
   LED1 = false;
   LED2 = false;
   LED3 = false;
   LED4 = false;
   counterVal = 0;
   ccpVal = 0;
   output_low(LPC_DS1);
   output_low(LPC_DS2);
   output_low(LPC_DS3);
   output_low(LPC_DS4);
         
   while(1){        

/*
      disable_interrupts(INT_RTCC);          // Ensure the RTCC Interrupt is disabled.
      counterVal++;                          // Increment CounterVal
      if((counterVal&0X80)>0)
         counterVal = 0;                     // Manage overflow.
      
      printf("Counter at %2u \r\n",counterVal);    // Print out the counter
      LED1 = !LED1;
    
      // Output the Value of LED1 to LED4.
      output_bit(LPC_DS1,LED1);
      output_bit(LPC_DS2,LED2);
      output_bit(LPC_DS3,LED3);
      output_bit(LPC_DS4,LED4);   //*/
   
   /*
      if(portBInterrupt)
      {
         printf("Port B interrupt Detected\r\n");
         portBInterrupt = false;
      } //*/

   /*
      if(ccpInterrupt)
      {
         printf("CCP interrupt Detected: %d - %d - %u\r\n",timerOflwCount1,timerOflwCount,ccpVal);
         ccpInterrupt = false;
      }      
      
      enable_interrupts(INT_RTCC);           // Re-enable the Timer interrupt */
      
      delay_ms(1000);                        // Delay a second.      
   }
}
