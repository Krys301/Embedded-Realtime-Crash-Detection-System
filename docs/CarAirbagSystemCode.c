/*
 * File:   AssignmentCode (Car Airbag System)
 * Author: Krystian Stratynski & Johnathan Nysulu
 * Student Number: 23458994 & 23477396
 * Description: this project is a real time system thats implemented on the PIC16F877A. in this project we used a potentiometer to replicate impact.
 * when a threshold is reached a buzzer and LED turns on and a counter updates in the EEprom and is retained even after power loss.
 * the system also communicates with the user via the UART so you can reset or view the counter.
 * 
 *
 * Created on 11 December 2025, 12:21
 */
#include "ee302lcd.h"           // include lcd library 
#include "I2C_EE302_2024.h"     //Include EEPROM library 
#include <xc.h>

#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#ifndef _XTAL_FREQ
        // Unless already defined assume 4MHz system frequency
        // This definition is required to calibrate the delay functions, __delay_us() and __delay_ms()
#define _XTAL_FREQ 4000000
#endif 

//  |--- Definitions ---|
#define LED1 RC0                 // bit 0 os port c assigned to led 
#define SPK  RC2                 // bit 1 of port c assigned to speaker 

#define ON 1                     //LED on defined as 1 to light led 
#define OFF 0                    // LED 0 defined as 0 to turn led off 

//TIMER definition
volatile unsigned char sample_due = 0;      //Set by timer1 every 50ms
unsigned char tick_counter = 0;             //counts timer1 ticks 
//ADC definition
unsigned char adc_value = 0;                // last measured adct value 

// threshold definition
#define IMPACT_THRESHOLD 180                //adc value that triggers impact 

//EEPROM definition
#define IMPACT_COUNT_ADDR_H  0x00           //EEprom address (high)
#define IMPACT_COUNT_ADDR_L  0x00           //EEprom address (low)
unsigned int impact_count = 0;              //total number of impacts


//  |--- Prototypes ---|
void setup(void);
void loop(void);
void lcd_startup_screen(void);
void transmit_string(const char *p);
unsigned char read_adc(void);
void adc_display_value(unsigned char value);
void lcd_print_adc(unsigned char value);
void handle_impact(unsigned char value);
void doSPK(void);
void save_impact_count(unsigned int count);
unsigned int load_impact_count(void);
char uart_read_char(void);
void handle_uart_commands(void);

//  |--- Main Loop ---|
void main(void) {
    setup();        // initialize system hardware 
    for(;;)         //infinite loop
    {
        loop();     //run main logic continously 
    }
}


void loop()
{ 
    handle_uart_commands();                 //check for uart input (c or r)
        if(sample_due)                      //runs every 50ms 
        {
            sample_due = 0;                 // clear timing flag 
            tick_counter++;                 //increment tick counter
            //--- ADC Tick ---
            adc_value = read_adc();         //read analog input 
            adc_display_value(adc_value);   // display adc value on lcd
            handle_impact(adc_value);       // check for impact detection 
           
            if(tick_counter >= 20)//every 1 second 
            {
                tick_counter=0;
                
                transmit_string("Checking \n\r");// status message on uart 
            }
        }
      
    
}

//  |--- Initialization ---|

void setup(void)
{
    //  |--- LCD SETUP ---|
    // set PORTD to make sure they are set for LCD
    TRISD = 0x00; 
    PORTD = 0x00;               // port D set as outputs
    Lcd8_Init();                // initialize lcd 
    lcd_startup_screen();       // call on function to display something on screen 
    
    
    //  |--- LED SETUP ---|
    LED1 = 0;                   //ensure led is off
    
    //  |--- BUZZER SETUP ---|
    TRISCbits.TRISC2=0;         // set RC2 as output
    SPK=0;
    
    //  |--- USART SETUP ---|
    TRISC = 0xC0;               // RC6 & RC7 configured as input to enable UART
    TXSTA = 0x24;               // Set TXEN and BRGH
    RCSTA = 0x90;               //Enable serial port by setting SPEN bit
    SPBRG = 0x19;               // Select 9600 baud rate 
  
    //  |--- TIMER1 SETUP ---|
    T1CON = 0b00100001;         //Timer 1 on , prescaler 1:4, internal clock (fosc/4)
    TMR1H = (53036 >> 8);       //preload high byte
    TMR1L = (53036 & 0xFF);     //preload low byte 
    TMR1IF = 0;                 // clear interrupt flag 
    TMR1IE = 1;                 // enable timer1 interrupt
    PEIE = 1;                   //enable peripheral interrupts
    GIE = 1;                    //enable global interrupts
    
    
    //  |--- ADC SETUP ---| 
    TRISAbits.TRISA0 = 1;                // set RA0 as input(AN0)
    ADCON0 = 0b01000001;                 //ADC on and AN0 is selected 
    ADCON1 = 0b00000010;                 // left justified 
    
    
    //  |--- I2C SETUP ---|
    i2c_init();                             // initialize I2C 
    impact_count = load_impact_count();     //restore stored impact count 
}

//  |--- Functions ---|

//--- LCD function ---
void lcd_startup_screen(void)
{   
    Lcd8_Clear();                           // Clear Display 
    
        
    Lcd8_Set_Cursor(1,1);
    Lcd8_Write_String("impact sensor");
    
    Lcd8_Set_Cursor(2,1);
    Lcd8_Write_String("KS+JN");
    
}

//--- UART function --- 
void transmit_string(const char *p)
{
    while(*p != '\0')         // While string doesnt equal null character 
    {
        while(!TXIF);        //wait until TXREG is empty  
        TXREG = *p;      //Load TXREG with character from string pointed to 
        p++;             //increment p 
    }
}
char uart_read_char(void)
{
    if (RCIF)              // Character received?
    {
        return RCREG;      // Read it
    }
    return 0;              // No character
}
void handle_uart_commands(void)
{
    char cmd = uart_read_char();

    if (cmd == 'c' || cmd == 'C')// if 'c' detected load counter on putty
    {
        transmit_string("\r\nImpact Count = ");

        while(!TXIF);
        TXREG = (impact_count / 100) + '0';

        while(!TXIF);
        TXREG = ((impact_count / 10) % 10) + '0';

        while(!TXIF);
        TXREG = (impact_count % 10) + '0';

        transmit_string("\r\n");
    }
    else if (cmd == 'r' || cmd == 'R') //if 'r' detected reset impact counter and then store it inside the eeprom
    {
        impact_count = 0;
        save_impact_count(impact_count);

        transmit_string("\r\nImpact counter RESET\r\n");

        Lcd8_Clear();
        Lcd8_Set_Cursor(1,1);
        Lcd8_Write_String("Count= 000");
    }
}


//--- TIMER function ---
void __interrupt() isr(void)
{
    if(TMR1IF)
    {
        TMR1H = (53036 >> 8);//high 8 bits & preloaded value
        TMR1L = (53036 & 0xFF);
        
        TMR1IF = 0;//clear timer1 intterupt flag
        sample_due = 1;
    }
}
//--- ADC function ---
unsigned char read_adc(void)
{
    __delay_us(20);
    
    GO_nDONE = 1;
    while(GO_nDONE);//waits until conversion finishes
    return ADRESH;//returns high 8 bit adc value
}
//displays this new adc value to the lcd
void adc_display_value(unsigned char value)
{
    char buffer[16];
    Lcd8_Set_Cursor(2,1);
    lcd_print_adc(adc_value);
  
}
//function for 3 digit adc value
void lcd_print_adc(unsigned char value)
{
    char b1 = '0' + (value / 100);
    char b2 = '0' + ((value / 10) % 10);
    char b3 = '0' + (value % 10);
    
   //print function for char b1-b3
    Lcd8_Set_Cursor(2,1);
    Lcd8_Write_String("Impact =");
    Lcd8_Write_Char(b1);
    Lcd8_Write_Char(b2);
    Lcd8_Write_Char(b3);
    Lcd8_Write_String("  ");
}
//--- Impact function ---
void handle_impact(unsigned char value)
{
    if(value > IMPACT_THRESHOLD)
    {
        LED1 = 1;// turn on led 
        doSPK();//turn on buzzer 
        transmit_string("impact detected!!");
        
        impact_count++;
        save_impact_count(impact_count);//saves count to eeprom
        //updates lcd to show latest count value
        Lcd8_Clear();
        Lcd8_Set_Cursor(1,1);
        Lcd8_Write_String("Count= ");
        Lcd8_Write_Char((impact_count/100)+'0');
        Lcd8_Write_Char(((impact_count/10)%10)+'0');
        Lcd8_Write_Char((impact_count%10)+'0');
    }
    else
    {
        LED1=0;
        
    }
        
}

//--- speaker function --- 
void doSPK(void)	// 1 cycle of 1khz squarewave for 250ms output
{
    for(int x = 250;x > 0; --x)
				{
					SPK = 1; //speaker pin is high/on 
                    __delay_us(500);		//500us delay
                    SPK = 0;//speaker pin is low/ off
                    __delay_us(500);		//500us delay
                    
				}
 
}

//--- EEPROM function --- 
// SAVES IMPACT TO EEPROM 
void save_impact_count(unsigned int count)
{
    unsigned char high = (count >> 8);
    unsigned char low  = (count & 0xFF);

    write_ext_eeprom(IMPACT_COUNT_ADDR_H, IMPACT_COUNT_ADDR_L, high);
    __delay_ms(10);     // mandatory EEPROM write delay

    write_ext_eeprom(IMPACT_COUNT_ADDR_H, IMPACT_COUNT_ADDR_L + 1, low);
    __delay_ms(10);
}
//LOADS/WRITES IMPACT FROM EEPROM
unsigned int load_impact_count(void)
{
    unsigned char high = read_ext_eeprom(IMPACT_COUNT_ADDR_H, IMPACT_COUNT_ADDR_L);//reads this high byte value
    unsigned char low  = read_ext_eeprom(IMPACT_COUNT_ADDR_H, IMPACT_COUNT_ADDR_L + 1);//reads low byte value

    return ((unsigned int)high << 8) | low;//brings both and combines them into a 16 bit value
}

