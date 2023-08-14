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
// File : print.c
// Contains: Minimal print functions
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#include "pico_libc.h"

#include "print.h"
#include <p18lf46k22.h>

void printchar(unsigned char c)
{
	// wait for TX1IF flag high (TX ready to send)
	while((PIR1&0x10) == 0x00){};

	// write and send byte
	TXREG1 = c;
}

void printhex(unsigned char c)
{
	unsigned char c1;

	c1=c>>4;

	if(c1<10)
	{
		printchar('0'+c1);
	}
	else
	{
		printchar('A'+(c1-10));
	}

	c1=c&0xF;

	if(c1<10)
	{
		printchar('0'+c1);
	}
	else
	{
		printchar('A'+(c1-10));
	}
}

void printhex_long(unsigned long c)
{
	printhex((c>>24)&0xFF);
	printhex((c>>16)&0xFF);
	printhex((c>>8)&0xFF);
	printhex(c&0xFF);
}

void printhex_short(unsigned short c)
{
	printhex((c>>8)&0xFF);
	printhex(c&0xFF);
}

void print(const unsigned char *pucBuffer)
{
	int i;
	//
	// Loop while there are more characters to send.
	//
	if(!pucBuffer)
	{
//      print((unsigned char*)"<NULL STR>");
		return;
	}
	i=0;
	while(pucBuffer[i])
	{
		printchar(pucBuffer[i]);
		i++;
	}
}

void printrom(const rom far char *pucBuffer)
{
	int i;
	//
	// Loop while there are more characters to send.
	//
	if(!pucBuffer)
	{
//      print((unsigned char*)"<NULL STR>");
		return;
	}
	i=0;
	while(pucBuffer[i])
	{
		printchar(pucBuffer[i]);
		i++;
	}
}

void dbg_printf(const unsigned char *pucBuffer, unsigned int arg1,unsigned int arg2,unsigned int arg3)
{
	int i,argpos;
	unsigned int arglist[4];

	arglist[0] = arg1;
	arglist[1] = arg2;
	arglist[2] = arg3;
	arglist[3] = 0;

	//
	// Loop while there are more characters to send.
	//
	if(!pucBuffer)
	{
//      print((unsigned char*)"<NULL STR>");
		return;
	}

	argpos = 0;

	i=0;
	while(pucBuffer[i])
	{
		if(pucBuffer[i] == '%')
		{
			switch(pucBuffer[i+1])
			{
				case 'X':
					printhex_long(arglist[argpos]);
					argpos = (argpos + 1)&3;
					i = i + 2;
				break;
				case 's':
					print((unsigned char*)arglist[argpos]);
					argpos = (argpos + 1)&3;
					i = i + 2;
				break;
				default:
					printchar(pucBuffer[i]);
					i++;
				break;
			}
		}
		else
		{
			printchar(pucBuffer[i]);
			i++;
		}
	}
}


void printdec(unsigned char c)
{
	unsigned char c1,c2,c3;

	c1=c/100;
	printchar('0'+c1);
	c2=(c/10) - (c1*10);
	printchar('0'+c2);
	c3= c - ( (c1*100) + (c2*10) );
	printchar('0'+c3);
}

void printbuf(unsigned char * buf,unsigned char * comp,unsigned short  size)
{
	unsigned short  i;

	printchar('\r');
	printchar('\n');

	for(i=0;i<size;i++)
	{
		if(!(i&0xF))
		{
			printchar('\r');
			printchar('\n');

			if(comp)
			{
				if(buf[i] != comp[i])
					printchar('>');
				else
					printchar(' ');
			}
			else
			{
				printchar(' ');
			}
		}
		else
		{
			if(comp)
			{
				if(buf[i] != comp[i])
				{
					printchar('>');
				}
				else
				{
					printchar(' ');
				}
			}
			else
			{
				printchar(' ');
			}
		}

		printhex(buf[i]);

	}

	printchar('\r');
	printchar('\n');
}

void printbuflong(unsigned long * buf,unsigned short  size)
{
	unsigned short  i;

	printchar('\r');
	printchar('\n');

	for(i=0;i<size;i++)
	{
		if(!(i&0x3))
		{
			printchar('\r');
			printchar('\n');
		}

		printhex_long(buf[i]);
		printchar(' ');
	}

	printchar('\r');
	printchar('\n');
}

unsigned char * str_printhex(unsigned char * buf,unsigned char c)
{
	unsigned char c1;

	c1=c>>4;

	if(c1<10)
	{
		*buf++ = '0'+c1;
	}
	else
	{
		*buf++ = 'A'+(c1-10);
	}

	c1=c&0xF;

	if(c1<10)
	{
		*buf++ = '0'+c1;
	}
	else
	{
		*buf++ = 'A'+(c1-10);
	}

	return buf;
}

unsigned char * str_printhex_long(unsigned char * buf, unsigned long c,int nb_bytes)
{
	do
	{
		nb_bytes--;

		buf = str_printhex(buf, (c>>(8*nb_bytes)) & 0xFF );

	}while(nb_bytes);

	return buf;
}

unsigned char str_hex_to_byte(unsigned char * buf)
{
	unsigned char c,i;

	c = 0x00;

	for(i=0;i<2;i++)
	{
		c = c << 4 ;

		if( *buf >= '0' && *buf <= '9' )
		{
			c |= (*buf - '0');
		}
		else
		{
			if( *buf >= 'A' && *buf <= 'F' )
			{
				c |= (*buf - 'A');
			}
		}

		buf++;
	}

	return c;
}
