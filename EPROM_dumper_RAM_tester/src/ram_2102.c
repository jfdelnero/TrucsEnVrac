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
// File : ram_2102.c
// Contains: 2102 RAM tester
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

void ram_2102(void)
{
	unsigned short i;
	unsigned short crc,err_cnt;
	unsigned short checksum;

	printrom((const far rom char *)"\r\n2102 RAM tester V0.1\r\n");

	while(1)
	{
		while(PUSH_BUTTONn == 1);

		TRISDbits.RD0 = OUTPUT;

		CHIP_SELECTn = 1;
		WE_OUTPUTn = 1;

		LED_OUTPUT = 1;

		printrom((const far rom char *)"\r\nGenerating test buffer (128 bytes / 1024bits)...\r\n");

		for(i=0;i<128;i++)
		{
			ram_buffer[i] = gen_rand();
		}

		printbuf((unsigned char*)&ram_buffer,(unsigned char*)0,128);

		printrom((const far rom char *)"\r\nWriting...\r\n");

		for(i=0;i<1024;i++)
		{
			PORTA = (i & 0x3F);
			PORTE = (i>>6)&0x7;
			if(i&0x200)
				ADDRESS_A9 = 1;
			else
				ADDRESS_A9 = 0;

			nop_delay();

			if( ram_buffer[i>>3] & (0x80>>(i&7)) )
				DIN_OUTPUT = 1;
			else
				DIN_OUTPUT = 0;

			nop_delay();
			nop_delay();

			CHIP_SELECTn = 0;
			nop_delay();
			nop_delay();
			WE_OUTPUTn = 0;
			nop_delay();
			nop_delay();
			WE_OUTPUTn = 1;
			nop_delay();
			CHIP_SELECTn = 1;
			nop_delay();
			nop_delay();
		}

		printrom((const far rom char *)"\r\nReading / Checking ...\r\n");

		mswait(100, 1);

		WE_OUTPUTn = 1;
		DIN_OUTPUT = 0;

		err_cnt = 0;

		for(i=0;i<1024;i++)
		{
			PORTA = (i & 0x3F);
			PORTE = (i>>6)&0x7;
			if(i&0x200)
				ADDRESS_A9 = 1;
			else
				ADDRESS_A9 = 0;

			nop_delay();
			nop_delay();

			CHIP_SELECTn = 0;
			nop_delay();
			nop_delay();
			nop_delay();

			if(DOUT_INPUT)
			{
				if( !(ram_buffer[i>>3] & (0x80>>(i&7))) )
				{
					err_cnt++;
				}
			}
			else
			{
				if( (ram_buffer[i>>3] & (0x80>>(i&7))) )
				{
					err_cnt++;
				}
			}

			CHIP_SELECTn = 1;

			nop_delay();
			nop_delay();
		}

		if(err_cnt)
		{
			printrom((const far rom char *)"\r\nERROR ! : 0x");
			printhex_short(err_cnt);
			printrom((const far rom char *)" errors found !\r\n");
			mswait(255, 20);
		}
		else
			printrom((const far rom char *)"\r\nTest passed ! No Error found :) !\r\n");

		mswait(5, 1);

		PORTA = 0x00;
		PORTE = 0x00;
		ADDRESS_A9 = 0;

		DIN_OUTPUT = 0;
		CHIP_SELECTn = 0;
		LED_OUTPUT = 0;
		TRISDbits.RD0 = INPUT;
	}

	for(;;);
}
