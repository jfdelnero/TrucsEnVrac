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
// File : serial.c
// Contains: Serial port I/O
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#include "serial.h"
#include <p18lf46k22.h>

// Initialise the Serial module TX1/RX1
void Serial_Init(){

    // 16 bit configuration
	BAUDCON1 = 0x08;

    // baudrate configuration
   	SPBRGH1 = 0x00;
	SPBRG1 = 34;

	// TX/RX configuration
    TRISC |= 0xC0;

	// TX enable and BRGH = 1
	TXSTA1 = 0x20;

    // Serial port enable
	RCSTA1 = 0x90;

	// timer 0 prescaler enable
	T0CONbits.PSA=0;
}

// Send a byte on Serial port TX1
void Serial_Send_Byte(unsigned char val){

	// wait for TX1IF flag high (TX ready to send)
	while((PIR1&0x10) == 0x00){};

	// write and send byte
	TXREG1 = val;
}

// Receive a byte on Serial port RX1
unsigned char Serial_Receive_Byte_Fast(){

	// wait for RC1IF flag high (Byte present in receiver buffer)
	while((PIR1&0x20) == 0x00){};

	if(RCSTA1&0x02 == 0x02){ //Overrun error
		RCSTA1 &= 0xEF; // clear CREN
		RCSTA1 |= 0x10; // set CREN
	}
	return RCREG1;
}

// Receive a byte on Serial port RX1
// function with timout
// use timer 0
// value received is returned on variable pointed by val
// return 0xFF if timeout occured
// return 0x00 if all ok
unsigned char Serial_Receive_Byte(unsigned char* val){

	unsigned char tmp;
	unsigned char RCSTA1_reg;

	for(;;){

		// Timer 0 starts for 1 second
		TMR0H = ((65535-15625)>>8);
		TMR0L = ((65535-15625));
		INTCONbits.TMR0IF = 0;

		// wait for RC1IF flag high (Byte present in receiver buffer)
		while(((PIR1&0x20) == 0x00) && (!INTCONbits.TMR0IF)){};

		if(INTCONbits.TMR0IF){ // timeout
			return 0xFF;
		}
		else{ // one byte received

			// get the register
			RCSTA1_reg = RCSTA1;

			// check the errors
			if((RCSTA1_reg&0x04) == 0x04){ // framing error
				tmp = RCREG1; // read RCREG1
			}
			else if((RCSTA1_reg&0x02) == 0x02){ // Overrun error
				RCSTA1 &= 0xEF; // clear CREN
				RCSTA1 |= 0x10; // set CREN
				*val = RCREG1;
				return 0x00;
			}
			else{ // ok
				*val = RCREG1;
				return 0x00;
			}
		}
	}
}