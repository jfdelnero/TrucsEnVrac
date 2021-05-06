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
// File : utils.c
// Contains: utils functions
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include "hardware.h"
#include <p18f46k22.h>

unsigned long rand_seed,current_rand;
unsigned char cnt_rand;

void mswait(unsigned char ms, unsigned char mul)
{
	// 16000000/256/4=15625 hz -> 1ms = 15,625
	T0CONbits.PSA=0;
	do
	{
		do
		{
			TMR0H=((65535-16)>>8);
			TMR0L=((65535-16));
			INTCONbits.TMR0IF=0;
			do
			{
			}while(!INTCONbits.TMR0IF);
			ms--;
		}while(ms);
		mul--;
	} while(mul);
}

unsigned long xorshift(unsigned long x)
{
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return x;
}

unsigned char gen_rand(void)
{
	unsigned char byte;

	if(!cnt_rand)
	{
		rand_seed = xorshift(rand_seed);
		current_rand = rand_seed;
		cnt_rand = 4;
	}

	byte = (current_rand & 0xFF);
	current_rand >>= 8;
	cnt_rand--;

	return byte;
}

void nop_delay(void)
{
_asm
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
_endasm
}
