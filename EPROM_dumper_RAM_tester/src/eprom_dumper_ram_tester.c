///////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------//
//----------H----H--X----X-----CCCCC-----22222----0000-----0000-----11-------//
//---------H----H----X-X-----C--------------2---0----0---0----0---1-1--------//
//--------HHHHHH-----X------C----------22222---0----0---0----0-----1---------//
//-------H----H----X--X----C----------2-------0----0---0----0-----1----------//
//------H----H---X-----X---CCCCC-----22222----0000-----0000----11111---------//
//---------------------------------------------------------------------------//
//----- Contact: hxc2001 at hxc2001.com ----------- https://hxc2001.com -----//
//----- (c) 2021 Jean-François DEL NERO ----------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////
// File : eprom_dumper_ram_tester.h
// Contains: EPROM dumper / RAM tester
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#include <p18f46k22.h>
#include "functionasm.h"
#include "eprom_dumper_ram_tester.h"
#include "utils.h"
#include "hardware.h"
#include "init_io.h"
#include "serial.h"

#include "pico_libc.h"
#include "print.h"

#include "ram_2102.h"
#include "ram_4007.h"
#include "eprom_1702a.h"

///////////////////////////////////////////
// PIC Configuration
///////////////////////////////////////////

#pragma config FOSC   = HSHP    // External oscillator block
#pragma config PLLCFG = ON      // Oscillator multiplied by 4
#pragma config PWRTEN = ON		// Power up timer disabled
#pragma config IESO   = OFF		// Oscillator Switchover mode enabled
#pragma config WDTEN  = SWON    // WDT is controlled by SWDTEN bit of the WDTCON register
#pragma config LVP    = OFF		// Single-Supply ICSP disabled
//#pragma config MCLRE  = INTMCLR	// RE3 input pin enabled; MCLR disabled
#pragma config STVREN = ON		// Stack full/underflow will cause Reset
#pragma config XINST  = OFF		// extended instructions off.
#pragma config PBADEN = OFF		// PORTB<5:0> pins are configured as digital I/O on Reset
#pragma config HFOFST = OFF		// HFINTOSC output and ready status are delayed by the oscillator stable status
#pragma config CPD = OFF        // Data EEPROM not code-protected
#pragma config CPB = OFF        // Boot Block Code Protection not Enabled (000000-0007FFh)
#pragma config CP0 = OFF        // Code Protection Block 0 not Enabled (000800-001FFFh)
#pragma config CP1 = OFF        // Code Protection Block 1 not Enabled (002000-003FFFh)
#pragma config CP2 = OFF        // Code Protection Block 2 not Enabled (004000-005FFFh)
#pragma config CP3 = OFF        // Code Protection Block 3 not Enabled (006000-007FFFh)
#pragma config WRT1 = OFF       // Block 1 (004000-007FFFh) not write-protected
#pragma config WRT2 = OFF       // Block 2 (008000-00BFFFh) not write-protected
#pragma config WRT3 = OFF       // Block 3 (00C000-00FFFFh) not write-protected
#pragma config WRTC = OFF       // Configuration registers (300000-3000FFh) not write-protected
#pragma config WRTD = OFF       // Data EEPROM not write-protected
//#pragma config CCP2MX = PORTB3  // CCP2 on RB3
//#pragma config CCP3MX = PORTE0  // CCP3 on RE0
#pragma config BOREN = ON      // no wait for voltage

#pragma config BORV = 285

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// interrupts vectors
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma code Reset_Vector = 0x0000
void ResetVector(void)
{
	_asm
		goto Startup
	_endasm
}
#pragma code

#pragma code high_vector_section = 0x8
void HighVector(void)
{
	_asm GOTO HighIsr _endasm
}
#pragma code

/////////////////////////////////////
// Boot loader signature
/////////////////////////////////////

#pragma code low_vector = 0x18
void LowVector(void)
{
	_asm GOTO LowIsr _endasm
}
#pragma code

#pragma udata params_page_scn
	unsigned char i, j, c, l, k;
	unsigned char byte_pice;
	unsigned char NbBytes;

	extern unsigned long rand_seed;
	extern unsigned char cnt_rand;
#pragma udata

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Entry Point
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Startup(void)
{

	_asm
	// Disable Interrupts
	bcf INTCON,7,0			// disable all interrupts

    CLRF INTCON, 0
    CLRF INTCON2, 0
    CLRF INTCON3, 0

	CLRF PIE1, 0
    CLRF PIR1, 0
	SETF IPR1, 0

	CLRF PIE2, 0
    CLRF PIR2, 0
	SETF IPR2, 0

	CLRF STKPTR,0
	CLRF TOSU,0
	CLRF TOSH,0
	CLRF TOSL,0

    CLRF TBLPTRU, 0
    CLRF TBLPTRH, 0
    CLRF TBLPTRL, 0

	CLRF TABLAT, 0

	CLRF EEDATA, 0
	CLRF EEADRH, 0
	CLRF EEADR, 0
	CLRF EECON1, 0
	CLRF EECON2, 0

	// Full SRAM RESET
	// Page 0
    MOVLW 0x00
    MOVWF FSR0H,0

    MOVLW 0x60
    MOVWF FSR0L,0

NEXT_P0:
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0

	TSTFSZ FSR0L,0
	BRA    NEXT_P0

	MOVLW  0x0F
	CPFSEQ FSR0H,0
    BRA    NEXT_P0

	// Last block
    MOVLW 0x0F
    MOVWF FSR0H,0

    MOVLW 0x38

	CLRF FSR0L,0
NEXTLASTB:
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0
	CLRF   POSTINC0,0

	CPFSEQ FSR0L,0
	BRA    NEXTLASTB

	CLRF FSR0H, 0
	CLRF FSR0L, 0
	CLRF FSR1H, 0
	CLRF FSR1L, 0
	CLRF FSR2H, 0
	CLRF FSR2L, 0

	CLRF BSR, 0

	CLRF STATUS, 0

	// Initialize the stack pointer
	lfsr 1, _stack
	lfsr 2, _stack
	clrf TBLPTRU,0			// 1st silicon doesn't do this on POR
	bcf __FPFLAGS,6,0		// Initialize rounding flag for floating point libs
	_endasm

	// Call the user's main routine
	main();
}

void Init_Timer_0(void){
	T0CONbits.TMR0ON  = 0;	// disable timer 0
	T0CONbits.T08BIT  = 0;	// 16 bit
	T0CONbits.T0CS    = 0;  // internal clock
	T0CONbits.PSA     = 0;  // prescalar enabled
	T0CONbits.T0PS2   = 1;	// set prescalar value to 256
	T0CONbits.T0PS1   = 1;
	T0CONbits.T0PS0   = 1;
	INTCONbits.TMR0IE = 0;	//disable the interrupt
	T0CONbits.TMR0ON  = 1;	// enable timer 0
}


void Init_Timer_1(void){
	IPR1bits.TMR1IP = 0;     // low priority interrupt
	T1CONbits.TMR1CS = 0;    // clock source is FOSC/4
	T1CONbits.T1CKPS = 0x03; // set prescaler to 8
	T1CONbits.T1RD16 = 1;    // enable 16 bits mode
	TMR1H = 0xEC;            // set tick to 10ms = (4*8*val)/FOSC
	TMR1L = 0x78;            //
	PIR1bits.TMR1IF = 0;     // clear flag
	PIE1bits.TMR1IE = 1;     // enable overflow interrupt
	T1CONbits.TMR1ON = 1;    // enable timer1
}

unsigned char ram_buffer[256];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main routines
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main(void)
{
	unsigned char choice;

	rand_seed = 0xA3C6F21C;
	cnt_rand = 4;

	Init_Timer_0();
	Init_Timer_1();

	////////////////////////
	// IO init
	////////////////////////

	// SLRCON = 0x00; // Slew rate

    // internal oscillator at 16MHz
	// OSCCONbits.IRCF0=1;
	// OSCCONbits.IRCF1=1;
	// OSCCONbits.IRCF2=1;

	init_io();

	Serial_Init();

	printrom((const far rom char *)"\r\n1702A EPROM Reader V0.1\r\n");

	while(1)
	{
		printrom((const far rom char *)"Menu:\r\n");
		printrom((const far rom char *)"1 - Intel 1702A EPROM Reader\r\n");
		printrom((const far rom char *)"2 - Am2102 RAM Tester\r\n");
		printrom((const far rom char *)"3 - MK4007 RAM Tester\r\n");

		choice = Serial_Receive_Byte_Fast();
		switch(choice)
		{
			case '1':
				eprom_1702a();
			break;
			case '2':
				ram_2102();
			break;
			case '3':
				ram_4007();
			break;
		}
	}
}
