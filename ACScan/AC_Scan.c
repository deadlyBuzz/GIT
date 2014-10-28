/******************************************************************************
 * AC1 Update
 *  Notes marked with a "<<<<"
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
#define PLC_IN1           PIN_C1
#define PLC_IN2           PIN_C2
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
int   oscTuneSP = 0;
int   oscTuneTmp = 0;

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
    T1_Overflow++;
}
///////////////////////////////////////////////////////////////////////////////
#INT_CCP1
void ccp1_isr() // Captures the rising edge of CCP1 pin.
{ 
    if(timing==FALSE){ // only do this on the edge, any bouncing will reset timers etc.
        set_timer1(0);
        T1_Overflow = 0;
        Pulse_Time = 0;
        timing = 1;     // Set flag to indicate timing.
        output_high(BLUE_LED);
    }
}
///////////////////////////////////////////////////////////////////////////////
#INT_CCP2
void ccp2_isr()                 
{   
    if(timing == TRUE){ // only output this when preceded by CCP1
        if(Count_Done == FALSE)
        {
            Count_Done = TRUE;
            CCP1_Count = CCP_1;
            CCP2_Count = CCP_2;        
            //Pulse_Time = (CCP2_Count - CCP1_Count); //CCP1 is irrellevant as the timer has been reset in the associated interrupt
            Pulse_time = CCP_2;
            Pulse_Overflow = T1_Overflow;
            measureCount++;     // increment the number of measures.
        }
        output_low(BLUE_LED);
        timing = FALSE;
    }
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
    enable_interrupts(INT_TIMER1);     // enable timer 1 interrupts
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
//           printf(usb_cdc_putc, "Time: %lu us\r\n", pulse_time/5 );     // divide by 5 as resolution is 5.2 nano seconds max //<<<< Needs to be corrected to "*5.2" instead of "/5"
//           printf(usb_cdc_putc, "Overflow: %lu pu\r\n", pulse_overflow);  // divide by 5 as resolution is 5.2 nano seconds max, this is (T1 overflow/5)       
//           printf(usb_cdc_putc, "CCP1:%lu CCP2:%lu\r\n", CCP1_Count, CCP2_Count);
             printf(usb_cdc_putc, "%lu , %lu ,  %lu, %x \r\n",measureCount, pulse_time, pulse_overflow, osctune);
           Count_Done = FALSE;
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
          c = usb_cdc_getc();          // get character from USB port
          if(c =='\r' || c == '\n')    // if CR or LF read from USB port
          {
            output_low(GREEN_LED);                        
            output_high(RED_LED);                    //<<<< Menu options to be updated.            
            printf(usb_cdc_putc,"Press '1' - Clear counters.\r\n");                          // <<<< output not operating.
            printf(usb_cdc_putc,"Press '+' to increment OSCTUNE by 1\r\n");                                      
            printf(usb_cdc_putc,"Press ']' to increment OSCTUNE by 4\r\n");                                      
            printf(usb_cdc_putc,"Press '[' to decrement OSCTUNE by 4\r\n");                                      
            printf(usb_cdc_putc,"Press '-' to decrement OSCTUNE by 1\r\n");                                      
            printf(usb_cdc_putc,"Press 'X' to set OSCTUNE to Max \r\n");                                      
            printf(usb_cdc_putc,"Press 'N' to set OSCTUNE to Min \r\n");                                      
//            printf(usb_cdc_putc,"3. Menu Option 3\r\n");                                      
//            printf(usb_cdc_putc,"4. Menu Option 4\r\n");                                      
            output_low(RED_LED);                        
            output_high(GREEN_LED);           
            delay_ms(250);            
          }
          else if(c=='1'){
            printf(usb_cdc_putc,"-------------------------------------------------\r\n");
            printf(usb_cdc_putc,"count,timer1,overflow,OscTune\r\n");
            measureCount = 0;
          }          
          else if(c=='+'){
            oscTuneTmp = ((oscTuneTmp&0XE0))|((oscTuneSP++)&0x1F);            
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          }
          else if(c=='-'){
            oscTuneTmp = ((oscTuneTmp&0XE0))|((oscTuneSP--)&0x1F);            
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          else if(c=='['){
            oscTuneTmp = ((oscTuneTmp&0XE0))|((oscTuneSP-4)&0x1F);            
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          }
          else if(c==']'){
            oscTuneTmp = ((oscTuneTmp&0XE0))|((oscTuneSP+4)&0x1F);
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          }
          else if(c=='N'){
            oscTuneTmp = 0X10;
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          }
          else if(c=='X'){
            oscTuneTmp = 0X0F;
            printf(usb_cdc_putc,"OSCTune set to %X",osctune);                                      
          }

         osctune = oscTuneTmp;
       }             
   }
}
///////////////////////////////////////////////////////////////////////////////
