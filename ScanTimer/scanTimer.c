/*********************************************************************** 
 * Author:  Alan Curley
 * Date:    11 SEP 2013
 * Program: scanTimer.C            Version:    1.0
 * Purpose:    Fire off an interrupt on a rising edge of CCP1 and Measure
 *             when the falling edge is detected (Another interrupt)
 *             <<<<Currently fires off interrupts correctly as indicated by LED>>>>>
 *      Added Serial interface as outlined in SERIALTEST.C - Jargon
 *       coming out.  UNknown why.  Need pullup resistors / XTal?
 ***********************************************************************/
#include <16F690.h>
#FUSES  NOWDT, INTRC_IO, NOMCLR, BROWNOUT, NOCPD, NOPUT, NOIESO, NOFCMEN
#use    delay(clock=8000000)
#use    rs232(baud=9600,xmit=PIN_A0,rcv=PIN_A1,parity=N,BITS=8,ERRORS)

#USE    FAST_IO(A)
#USE    FAST_IO(B)
#USE    FAST_IO(C)


/******************************************************************************
 To Measure the pulse width.
 (Taken from the Microchip document CCP & ECCP Tips and tricks)

|
|
|            |<---------W------------>|
|
|            --------------------------
|            |                        |
|            |                        |
|            |                        |
|            |                        |
|------------|                        |-----------------------------
|           t1                        t2
|
L-----------------------------------------------------------------------------

1. Configure control bits CCPxM3:CCPxM0 
(CCPxCON<3:0>) to capture every rising 
edge of the waveform.
2. Configure Timer1 prescaler so that Timer1 will 
run WMAX without overflowing.
3. Enable the CCP interrupt (CCPxIE bit).
4. When CCP interrupt occurs, save the captured 
timer value (t1) and reconfigure control bits to 
capture every falling edge.
5. When CCP interrupt occurs again, subtract 
saved value (t1) from current captured value 
(t2) – this result is the pulse width (W).
6. Reconfigure control bits to capture the next 
rising edge and start process all over again 
(repeat steps 3 through 6)

******************************************************************************/


/**
 * Declare variables 
 */
//------------------------- I/O Names ------------------------------------
#define P1_RX                    PIN_A0
#define P1_TX                    PIN_A1
#define LPC_SW1                  PIN_A3
#define LPC_DS1                  PIN_C0
#define LPC_DS2                  PIN_C1
#define LPC_DS3                  PIN_C2
#define LPC_DS4                  PIN_C3
#define CCP1_PIN                 PIN_C5

//------------------------- Local Variables ------------------------------
boolean FallingEdge;             // memory to determine if we're checking for a Rising or Falling edge
boolean ignoreInterrupt;         // Memory to set ignore interrupt after RE / FE is updated.
boolean resetInterrupts;         // A Flag to reset the interrupts- cause samples have it.

boolean LED1;                    // Boolean representing LED1`
boolean LED2;                    // Boolean representing LED2
boolean LED3;                    // Boolean representing LED3
boolean LED4;                    // Boolean representing LED4
boolean printData;               // Flag to print the data.

int timer0_re;              // timer0 at the time of the rising edge
int timer0_fe;              // timer0 at the time of the falling edge
int timer0_of;              // timer0 overflow count.
int re_to_fe;               // Resulting difference between rising and falling edge


/**
 * This is the interrupt routine that occurs when a rising edge is detected
 * in the CCP module.
 */
#INT_CCP1
void  CCP1_isr(void) 
{
   LED4= !LED4;
   if(false){//ignoreInterrupt){
      ignoreInterrupt = 0;
   }
   else{
      if(fallingEdge){
         setup_ccp1(CCP_CAPTURE_RE);               // CCP set up for rising edge capture.
         ignoreInterrupt = 1;         
         LED2 = !(LED2);
         timer0_re = CCP_1;                        // Store the timer value
         re_to_fe = timer0_re - timer0_fe;         // Calculate the Difference.         
         fallingEdge = false;
         printData = true;
      }
      else{//Rising edge.
         setup_ccp1(CCP_CAPTURE_FE);
         ignoreInterrupt = 1;
         LED3 = !(LED3);
         timer0_of = 0;                           // Reset the value of the overflow.
         timer0_re = CCP_1;                       // Store the timer value.
         fallingEdge = true;
      }
   }
    resetInterrupts = true;
}

/**
 * This is the interrupt routine that occirs when Timer0 overflows
 */
#INT_RTCC
void RTCC_isr(void)
{
   // Toggle LED1 in the Low Pin Count Demo board.
   LED1 = (timer0_of & 0X40)>0;
   timer0_of++;
   if(timer0_of>=0X80)
      timer0_of = 0;   
}


/**
 * Main Routine
 */
void main()
{

   /**
    * Setup I/O, CCP Registers Timers Etc.
    */    
   setup_adc_ports(NO_ANALOGS|VSS_VDD);      // No Analog signals
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_4);
   setup_timer_2(T2_DIV_BY_1,255,16);
   setup_ccp1(CCP_CAPTURE_RE);               // CCP set up for rising edge capture.
   enable_interrupts(INT_CCP1);              // Enable the CCP Interrupt
   enable_interrupts(INT_TIMER0);            // Enable real time clock interrupt.
   enable_interrupts(GLOBAL);                // Enable Global Interrupts
   setup_oscillator(OSC_8MHZ);               // Oscillator clock.
   
   // Set up the I/O pins in port A
   set_tris_a(0XFE);                         // Pin RA3 is the SW input in the LPC Demo board.
   port_a_pullups(TRUE);
   //port_c_pullups(TRUE);
   set_tris_c(0X20);                         // Pin RC5 Input
   
   output_low(LPC_DS1);
   output_low(LPC_DS2);
   output_low(LPC_DS3);
   output_low(LPC_DS4);
   
   // TODO: USER CODE!!
   while(1){
      if(resetInterrupts){
         disable_interrupts(GLOBAL);
         enable_interrupts(GLOBAL);
         resetInterrupts = false;
      }
      
      if(printData){
      printf("Data Collected.\r\n");
      printData = false;
      }
   
      if(LED1)
         output_high(LPC_DS1);
      else
         output_low(LPC_DS1);

      if(LED2)
         output_high(LPC_DS2);
      else
         output_low(LPC_DS2);

      if(LED3)
         output_high(LPC_DS3);
      else
         output_low(LPC_DS3);

      if(LED4)
         output_high(LPC_DS4);
      else
         output_low(LPC_DS4);
   }
}
