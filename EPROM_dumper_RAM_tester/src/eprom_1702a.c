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
// File : eprom_1702a.c
// Contains: 1702a EPROM dumper
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

#include "eprom_1702a.h"

#pragma udata params_page_scn
	unsigned char CRCCount;
	unsigned char CRCData;
	unsigned char CRCHi;
	unsigned char CRCLo;
	unsigned char CRCPolyLo;
	unsigned char CRCPolyHi;
#pragma udata


void eprom_1702a(void)
{
	unsigned short i;
	unsigned short crc;
	unsigned short checksum;

	printrom((const far rom char *)"\r\n1702A EPROM Reader V0.1\r\n");

	while(1)
	{
		while(PORTBbits.RB5 == 1);

		PORTCbits.RC0 = 1;
		PORTCbits.RC1 = 1;

		printrom((const far rom char *)"\r\nReading...\r\n");

		mswait(100, 1);

		for(i=0;i<256;i++)
		{
			PORTA = (i & 0x3F);
			PORTE = (i>>6)&0xF;

			PORTCbits.RC0 = 0;
			mswait(3, 1);
			ram_buffer[i] = PORTD;
			PORTCbits.RC0 = 1;
			mswait(1, 1);
		}

		mswait(5, 1);

		PORTA = 0x00;
		PORTE = 0x00;

		printbuf((unsigned char*)&ram_buffer,256);

		checksum = 0;
		for(i=0;i<256;i++)
		{
			checksum += ram_buffer[i];
		}

		CRCLo=0xFF;
		CRCHi=0xFF;
		CRCPolyLo=0x21;
		CRCPolyHi=0x10;

		for(i=0;i<256;i++)
		{
			CRCData = ram_buffer[i];
			CRC16BE();
		}

		printrom((const far rom char *)"\r\nChecksum : 0x");
		printhex_short(checksum);
		printrom((const far rom char *)"\r\n");
		printrom((const far rom char *)"CRC16 CCITT : 0x");
		printhex(CRCHi);
		printhex(CRCLo);
		printrom((const far rom char *)"\r\n");

		PORTCbits.RC0 = 0;
		PORTCbits.RC1 = 0;
	}

	for(;;);
}
