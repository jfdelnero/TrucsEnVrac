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
// File : init_io.c
// Contains: io init functions
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#include <p18f46k22.h>
#include "init_io.h"

void init_io(void){

	// Configure AN pins as digital
	ANSELA = 0x00;
	ANSELB = 0x00;
	ANSELC = 0x00;
	ANSELD = 0x00;
	ANSELE = 0x00;

	PORTCbits.RC0 = 1;
	TRISCbits.RC0 = OUTPUT;

	TRISD = 0xFF;
	PORTD = 0x00;

	TRISA = 0xC0; 
	PORTA = 0x00;

	TRISE = 0xF0; 
	PORTE = 0x00;

	// disable pull up on port B
	INTCON2bits.RBPU = 1;
	WPUB = 0x00;

	PORTCbits.RC6 = 0;
	TRISCbits.RC6 = INPUT;

	PORTCbits.RC7 = 0;
	TRISCbits.RC7 = INPUT;

	PORTCbits.RC1 = 0;
	TRISCbits.RC1 = OUTPUT;

	PORTCbits.RC2 = 0;
	TRISCbits.RC2 = OUTPUT;

	PORTCbits.RC5 = 0;
	TRISCbits.RC5 = OUTPUT;
	
	TRISBbits.RB5 = INPUT;
}