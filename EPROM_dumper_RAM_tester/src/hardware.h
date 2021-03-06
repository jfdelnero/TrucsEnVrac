///////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------//
//----------H----H--X----X-----CCCCC-----22222----0000-----0000-----11-------//
//---------H----H----X-X-----C--------------2---0----0---0----0---1-1--------//
//--------HHHHHH-----X------C----------22222---0----0---0----0-----1---------//
//-------H----H----X--X----C----------2-------0----0---0----0-----1----------//
//------H----H---X-----X---CCCCC-----22222----0000-----0000----11111---------//
//---------------------------------------------------------------------------//
//----- Contact: hxc2001 at hxc2001.com ----------- https://hxc2001.com -----//
//----- (c) 2021 Jean-Fran�ois DEL NERO ----------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////
// File : hardware.h
// Contains: asm function declaration
//
// Written by: Jean-Fran�ois DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#define CHIP_SELECTn PORTCbits.RC0
#define LED_OUTPUT   PORTCbits.RC1
#define DIN_OUTPUT   PORTDbits.RD0
#define DOUT_INPUT   PORTDbits.RD1
#define WE_OUTPUTn   PORTCbits.RC2

#define RW_OUTPUT    PORTCbits.RC2

#define PUSH_BUTTONn PORTBbits.RB5

#define ADDRESS_A9   PORTCbits.RC5

#define SERIAL_TX    PORTCbits.RC6
#define SERIAL_RX    PORTCbits.RC7