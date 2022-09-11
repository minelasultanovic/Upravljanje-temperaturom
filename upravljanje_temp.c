/*
 * File:   upravljanje_temp.c
 * Author: User
 *
 * Created on March 15, 2022, 12:05 AM
 */

#include <xc.h>
#include <stdio.h> 
#include <math.h>

#pragma config FOSC=HS,WDTE=OFF,PWRTE=OFF,MCLRE=ON,CP=OFF,CPD=OFF,BOREN=OFF,CLKOUTEN=OFF
#pragma config IESO=OFF,FCMEN=OFF,WRT=OFF,VCAPEN=OFF,PLLEN=OFF,STVREN=OFF,LVP=OFF
#define _XTAL_FREQ 8000000

#define T1 RB2      //taster za povecavanje temperature
#define T2 RB3      //taster za smanjivanje temperature
#define VENTILATOR RA2
#define GRIJAC RA3

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7



unsigned char temperatura = 0;
unsigned char zeljenaTemp;
unsigned char zeljena_stara = 0;
int pocetak = 1;

void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if (data_bit & 1)
        D4 = 1;
    else
        D4 = 0;

    if (data_bit & 2)
        D5 = 1;
    else
        D5 = 0;

    if (data_bit & 4)
        D6 = 1;
    else
        D6 = 0;

    if (data_bit & 8)
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a) {
    RS = 0;
    Lcd_SetBit(a); //Incoming Hex value
    EN = 1;
    __delay_ms(4);
    EN = 0;
}

void Lcd_Clear() {
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the cursor to first position
}

void Lcd_Set_Cursor(char a, char b) {
    char temp, z, y;
    if (a == 1) {
        temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp >> 4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    } else if (a == 2) {
        temp = 0xC0 + b - 1;
        z = temp >> 4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start() {
    Lcd_SetBit(0x00);
    for (int i = 1065244; i <= 0; i--) NOP();
    Lcd_Cmd(0x03);
    __delay_ms(5);
    Lcd_Cmd(0x03);
    __delay_ms(11);
    Lcd_Cmd(0x03);
    Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
    Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
    Lcd_Cmd(0x08); //Select Row 1
    Lcd_Cmd(0x00); //Clear Row 1 Display
    Lcd_Cmd(0x0C); //Select Row 2
    Lcd_Cmd(0x00); //Clear Row 2 Display
    Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data) //Send 8-bits through 4-bit mode
{
    char Lower_Nibble, Upper_Nibble;
    Lower_Nibble = data & 0x0F;
    Upper_Nibble = data & 0xF0;
    RS = 1; // => RS = 1
    Lcd_SetBit(Upper_Nibble >> 4); //Send upper half by shifting by 4
    EN = 1;
    for (int i = 2130483; i <= 0; i--) NOP();
    EN = 0;
    Lcd_SetBit(Lower_Nibble); //Send Lower half
    EN = 1;
    for (int i = 2130483; i <= 0; i--) NOP();
    EN = 0;
}

void Lcd_Print_String(char *a) {
    int i;
    for (i = 0; a[i] != '\0'; i++)
        Lcd_Print_Char(a[i]); //Split the string using pointers and call the Char function 
}

void printToLCD(int  Zeljena_temperatura, int Temp) {
    char text[30];
    Lcd_Clear();

    // First line
    sprintf(text, "Tren temp: %2d ", Temp);         //ispis trenutne temperature
    Lcd_Set_Cursor(1, 1);
    Lcd_Print_String(text);

     //Second line
    sprintf(text, "Zeljena temp: %2d", Zeljena_temperatura);       //ispis zeljene temperature
    Lcd_Set_Cursor(2, 1);
    Lcd_Print_String(text);
    
    __delay_ms(2000);
}

void printToLCD_trenutna(int  Temp) {
    char text[30];
    Lcd_Clear();

    // First line
    sprintf(text, "Tren temp: %2d ", Temp);
    Lcd_Set_Cursor(1, 1);
    Lcd_Print_String(text);

    __delay_ms(2000);
}

void printToLCD_ispis(void) {           //ispis po?etne poruke na lcd
    char text[30];
    Lcd_Clear();

    // First line
    sprintf(text, "Namjesti");
    Lcd_Set_Cursor(1, 1);
    Lcd_Print_String(text);

     //Second line
    sprintf(text, "zeljenu temp");
    Lcd_Set_Cursor(2, 1);
    Lcd_Print_String(text);
    
    __delay_ms(2000);
}

void initLCD() {
    Lcd_Start();
}


void __interrupt() fun(void) {
    zeljenaTemp = zeljena_stara;
    if(IOCIE && IOCBF2) {           //pritisnut taster za pove?avanje ?eljene temperature
        IOCBF2=0; 
        //IOCBF = 0;
        zeljenaTemp++;
        printToLCD(zeljenaTemp, temperatura);
    } 
    else if(IOCIE && IOCBF3){       //pritisnut taster za smanjivanje ?eljene temperature
        IOCBF3=0;
        //IOCBF = 0;
        zeljenaTemp--;
        printToLCD(zeljenaTemp, temperatura);
    }
    zeljena_stara = zeljenaTemp;      
}



void inicijalizacija(void) {
    TRISB = 0xCF;
    ANSELB = 0x00;
    PORTB = 0;
    
    TRISD = 0x03;
    ANSELD = 0x00;
    PORTD = 0x00;
    
    // Inicijalizacija AD konverzije
    TRISA = 0xE3; 
    ANSELA = 0x03; 
    
    // Lijevo poravnanje
    ADCON1bits.ADFM=0;
    
    // Interni RC oscilator za ADC
    ADCON1bits.ADCS2=1;
    ADCON1bits.ADCS1=1;
    ADCON1bits.ADCS0=1;
    
    // Vss za Vref-, Vdd za Vref+
    ADCON1bits.ADNREF=0;
    ADCON1bits.ADPREF1=0;
    ADCON1bits.ADPREF0=0;
    
    // Ukljucivanje ADC
    ADCON0bits.ADON=1;
    
    // Izbor AN0
    ADCON0bits.CHS3=0;
    ADCON0bits.CHS2=0;
    ADCON0bits.CHS1=0;
    ADCON0bits.CHS0=0; 
    
    // Inicijalizacija prekida
    IOCBP2 = 1;
    IOCBP3 = 1;
    IOCIE = 1;
    GIE=1;
}

void main(void) {
    inicijalizacija(); // Inicijalizacija cjelokupnog sistema
    initLCD();
    printToLCD_ispis();
    
    int vrijednost = 0;
    float napon = 0; // napon od senzora
    
    while(1){
        
        ADCON0bits.ADGO=1;
        while(ADCON0bits.ADGO)
            vrijednost =  ADRESH;

        napon = vrijednost * 5.0 / 256;
        temperatura = napon * 100;
        
        if(pocetak){
            pocetak = 0;
            continue;
        }
        else 
            printToLCD_trenutna(temperatura);
     
        do{
            
            if(temperatura>zeljenaTemp){     
                LATA = 0x04;  //upaljen ventil
                temperatura--; 
            }
            else if (temperatura <zeljenaTemp){
                LATA=0x08;  //upaljen grijac
                temperatura++;
            }
            else                      
                LATA=0x00;   //oboje iskljuceno
          
        
            ADCON0bits.ADGO=1;
            while(ADCON0bits.ADGO)
                vrijednost =  ADRESH;

            napon = vrijednost * 5.0 / 256;
            temperatura = napon * 100;
        
            if(pocetak){
                pocetak = 0;
                continue;
            }
            else
                printToLCD(zeljenaTemp, temperatura);
       
        }while(temperatura!=zeljenaTemp);
        
        printToLCD(zeljenaTemp, temperatura);
    }
    
    return;
}

