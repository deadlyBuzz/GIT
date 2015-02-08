/******************************************************************************
 * AC1 Scan
 *  Notes marked with a "<<<<"
 * In this version, only mode "1" is programmed - the response, or echo timer.
 *****************************************************************************/
//set to 1 to use a PIC's internal USB Peripheral
#define __USB_PIC_PERIF__ 1
#include <18F2550.h>

#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN
#use delay(clock=48000000)
//#use rs232(UART1, baud=9600, errors)
#include <usb_cdc.h>

#define RED_LED           PIN_A0
#define SPARE_A1          PIN_A1
#define GREEN_LED         PIN_A2
#define SPARE_A3          PIN_A3
#define BLUE_LED          PIN_A4
#define SPARE_A5          PIN_A5
#define XTAL_IN           PIN_A6       // cannot use as pin is used by xtal 
#define XTAL_OUT          PIN_A7       // cannot use as pin is used by xtal

#define SPARE_B0          PIN_B0
#define SPARE_B1          PIN_B1
#define SPARE_B2          PIN_B2
#define SPARE_B3          PIN_B3
#define SPARE_B4          PIN_B4
#define SPARE_B5          PIN_B5
#define SPARE_B6          PIN_B6
#define SPARE_B7          PIN_B7

#define PLC_OUT1          PIN_C0
#define PLC_IN1           PIN_C1        // CCP1
#define PLC_IN2           PIN_C2        // CCP2
#define VUSB              PIN_C3       // cannot use as pin is used by capacitor for vusb
#define USB_D_MINUS       PIN_C4       // cannot use as pin is used by USB Data-
#define USB_D_PLUS        PIN_C5       // cannot use as pin is used by USB Data+
#define SPARE_C6          PIN_C6
#define SPARE_C7          PIN_C7

#define SPARE_E3          PIN_E3

#include <types.h>

///////////////////////////////////////////////////////////////////////////////
// Decalare global variables here
///////////////////////////////////////////////////////////////////////////////
char  c = 0;

#byte osctune = 0xF9B // Oscillator tuning variable.
long  timer_10s = 0;
long  CCP1_Count = 0;
long  CCP2_Count = 0;
short Count_Done = 0;
long  Pulse_Time = 0;
long  Pulse_Overflow = 0;
long  T1_Overflow = 0;
short timing = 0;
long  measureCount = 0;
long  startCount = 0;
int   oscTuneSP = 0;
int   oscTuneTmp = 0;
long  mode = 2;
short opModeInverted = 0; // output matches the input.
short newTrigger = 0;   // Signal to indicate a new trigger is allowed.

///////////////////////////////////////////////////////////////////////////////
#INT_RTCC                              // 1 interrupt every 1ms
void clock_isr(void)
{
  set_rtcc(47);                     // 48,000,000/4/256/47 = 100.160 Hz // <<<<Assuming 0-467 is incorrect?
     
  if(timer_10s)                        // if timer has value 
    timer_10s--;                       // decrement timer
    
}
///////////////////////////////////////////////////////////////////////////////
#INT_TIMER1
void isr()
{
    T1_Overflow++; // record the number of overflows taken place.
}
///////////////////////////////////////////////////////////////////////////////
#INT_CCP1
void ccp1_isr() // Captures the rising edge of CCP1 pin.
{ 
    switch(mode){
        case 1: // Capture whatever signal has been configured.
            disable_interrupts(INT_CCP1); //Disable the interrupt to stop this from occurring again until triggered
            pulse_overflow = T1_Overflow;
            pulse_time = CCP_1; // record the timer value in CCP1.        
            count_done = TRUE; // Flag to the Main routine that there is a measurement available.
            measureCount++; // increment the measurement count.
            output_low(BLUE_LED); // Flag to the board that the response has taken place by resetting the BLUE LED.
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////
#INT_CCP2
void ccp2_isr()                 
{   
    // Do nothing until the program is developed.
}
///////////////////////////////////////////////////////////////////////////////
void main(void) 
{                    
    set_tris_a(0b11101010);            // setup port a
    set_tris_b(0b11111111);            // setup port b
    set_tris_c(0b10111110);            // setup port c
    set_tris_e(0b00001000);            // setup port e      
    setup_adc_ports(ADC_OFF);          // set analog A0,A1,A2 to on
    setup_adc(ADC_CLOCK_INTERNAL);     // set analog to digital converter to internal clock frequency
    setup_vref(FALSE);                 // switch off vref for comparator   
    output_a(0);
    output_b(0);    
    output_c(0);
    output_e(0);            
    setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256);// set timer 0 rtcc prescaler
    setup_timer_1(T1_INTERNAL);        // Start timer 1
    setup_timer_2(T2_DISABLED,127,1);  // disable timer 2
    setup_timer_3(T3_DISABLED);        // disable timer 3
    setup_ccp1(CCP_CAPTURE_RE);        // Configure CCP1 to capture rising edge
    setup_ccp2(CCP_CAPTURE_RE);        // Configure CCP2 to capture rising edge
    enable_interrupts(INT_CCP1);       // Setup interrupt on rising edge
    enable_interrupts(INT_CCP2);       // Setup interrupt on rising edge
    enable_interrupts(INT_TIMER0);     // enable real time clock interrupt      
    enable_interrupts(INT_TIMER1);     // enable timer 1 interrupts`x
    enable_interrupts(GLOBAL);         // enable global interrupts                  
    set_rtcc(0);                       // clear rtcc to 0
    delay_ms(1500);
    output_high(RED_LED);
    delay_ms(200);
    output_low(RED_LED);    
    output_high(GREEN_LED);
    delay_ms(200);
    output_low(GREEN_LED);    
    output_high(BLUE_LED);
    delay_ms(200);
    output_low(BLUE_LED); 
    usb_init_cs();            
    
    while(TRUE)                        // do forever while connected
    {       
       usb_task(); // keep usb alive    //<<<< Has to be done every cycle
       if(Count_Done == TRUE)
       {
            printf(usb_cdc_putc, "%lu , %lu , %lu , %lu ,  %lu \r\n",mode, startCount, measureCount, pulse_time, pulse_overflow);
            newTrigger = 1; // signal to the main routine that a new trigger can be issued.
            Count_Done = FALSE; // Clear the lock to allow a new count.
       }
//       if((usb_state == USB_STATE_ATTACHED)&&(!UCON_SE0))             
       if(usb_state == USB_STATE_CONFIGURED)            
       {
         output_high(GREEN_LED);                
         output_low(RED_LED);                         
       }
       else
       {
         output_low(GREEN_LED);              
         output_high(RED_LED);                         
       }       
       if(usb_cdc_kbhit())             // if any character received from USB port //<<<< KeyBoard HIT
       {
           startCount = 0;
           measureCount = 0;
          c = usb_cdc_getc();          // get character from USB port
          if(c =='\r' || c == '\n')    // if CR or LF read from USB port
          {
            output_low(GREEN_LED);                        
            output_high(RED_LED);                    //<<<< Menu options to be updated.            
            printf(usb_cdc_putc,"Press 'C' - Clear counters.\r\n");                          // <<<< output not operating.
            printf(usb_cdc_putc,"Press '1' - Cycle response to Toggle.\r\n");                          // <<<< output not operating.
            printf(usb_cdc_putc,"Press '2' - Select RE on CCP1 to FE on CCP1.\r\n");
            printf(usb_cdc_putc,"Press '3' - Select RE on Input 1 to RE on input 2.\r\n");
            output_low(RED_LED);
            output_high(GREEN_LED);           
            delay_ms(250);            
          }
          else if(c=='C'){
            printf(usb_cdc_putc,"-------------------------------------------------\r\n");
            printf(usb_cdc_putc,"mode,startCount,measureCount,timer1,overflow,OscTune\r\n");
            measureCount = 0;
            startCount = 0;
          }          
          else if(c=='1'){ // Response mode.
            printf(usb_cdc_putc,"Cycle response to Toggle mode Selected.\r\n");
            mode = 1;                   
          }
          else if(c=='2'){ // CCP1 RE to FE
            printf(usb_cdc_putc,"CCP1 Rising Edge to Falling Edge timing Selected.\r\n");
            mode = 2;
            setup_ccp1(CCP_CAPTURE_RE); // start with a rising edge.
          }
          else if(c=='3'){ // CCP1 RE to CCP2 RE        
            printf(usb_cdc_putc,"CCP1 Rising Edge to CCP2 Rising Edge timing Selected.\r\n");
            mode = 3;
            setup_ccp1(CCP_CAPTURE_RE);                        
            setup_ccp2(CCP_CAPTURE_RE);                        
          }
       }
       if(mode==1){ // manage the I/O capture.
            if(newTrigger == 1){
                newTrigger = 0; // Clear the new trigger request.
                set_timer1(0); // Reset the Timer.
                T1_Overflow = 0; // Clear the overflow counts for the timer.
                pulse_time = 0; // Clear the current value.
                output_toggle(PLC_OUT1); // Toggle the output pin.
                output_high(BLUE_LED); // Flag to the board that the OP toggle has taken place and we're waiting for a response.
                enable_interrupts(INT_CCP1); // Re-enable the CCP1 Interrupts.
                if(input(PLC_IN1)){
                    setup_ccp1(CCP_CAPTURE_FE); // input is currently high - trigger to capture the falling edge.
                }else{
                    setup_ccp1(CCP_CAPTURE_RE); // input is currently low  - trigger to capture the rising edge.
                }
            }
       }
   }
}
///////////////////////////////////////////////////////////////////////////////
