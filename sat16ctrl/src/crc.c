///////////////////////////////////////////////////////////////////////////////////
// File : crc.c
// Contains: crc functions
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

// Size constrained table based crc16 routine.
// (c)Jean-François DEL NERO

//#define CRCARRAYGENERATOR 1

#ifdef CRCARRAYGENERATOR
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include "crc.h"


#ifdef CRCARRAYGENERATOR
unsigned int crc_array[16*2] __attribute__ ((aligned (4))) =
#else
static const unsigned int crc_array[16*2] /*__attribute__((aligned(4)))*/ =
#endif
{
//  CRC 16 - 0x1021
//  Parameters used with the generator : Init Value : 0x0000, inv_poly : 0, inv_index : 0, inv_array_index : 0, inv_array_value : 1, xchg_byte : 0, xchg_array : 1

	0x0000,0x0881,0x8440,0x8CC1,0x4220,0x4AA1,0xC660,0xCEE1,
	0x2110,0x2991,0xA550,0xADD1,0x6330,0x6BB1,0xE770,0xEFF1,

	0x0000,0x8891,0xC448,0x4CD9,0x6224,0xEAB5,0xA66C,0x2EFD,
	0x3112,0xB983,0xF55A,0x7DCB,0x5336,0xDBA7,0x977E,0x1FEF

/*
//  CCITT CRC 16 - 0x1021
//  Parameters used with the generator : inv_poly 0, inv_index 0, inv_array_index 0, xchg_array 1, inv_array_value 0.

	0x0000, 0x3112, 0x6224, 0x5336, 0xC448, 0xF55A, 0xA66C, 0x977E,
	0x8891, 0xB983, 0xEAB5, 0xDBA7, 0x4CD9, 0x7DCB, 0x2EFD, 0x1FEF,

	0x0000, 0x2110, 0x4220, 0x6330, 0x8440, 0xA550, 0xC660, 0xE770,
	0x0881, 0x2991, 0x4AA1, 0x6BB1, 0x8CC1, 0xADD1, 0xCEE1, 0xEFF1
*/
};

static const unsigned char bit_inverter[]=
{
	0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,
	0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,
	0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,
	0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,
	0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,
	0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,
	0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,
	0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
	0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,
	0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,
	0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,
	0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,
	0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,
	0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,
	0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,
	0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,
	0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};


unsigned short crc16_buf(const unsigned char * buffer,int size,unsigned short crc16)
{
	unsigned char data;

	while(size--)
	{
		data = *buffer++;

		data ^= crc16;

		crc16 = ( (crc_array[ bit_inverter[data] >> 4 ] ^ crc_array[ (bit_inverter[data] & 0x0F) + 0x10 ]) ^ (crc16 >> 8) );
	}

	return crc16;
}

unsigned short crc16( unsigned char data, unsigned short crc16 )
{
	data ^= crc16;

	return ( (crc_array[ bit_inverter[data] >> 4 ] ^ crc_array[ (bit_inverter[data] & 0x0F) + 0x10 ] ) ^ (crc16 >> 8) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CRCARRAYGENERATOR

// Code to generate the above const arry:

static unsigned short getCRC16entry( const unsigned short index, const unsigned short poly )
{
	int i;
	unsigned short entry;

	entry = index;

	for( i = 0; i < 16; i++ ) {
		if( entry & 0x8000 )
			entry = (unsigned short)((entry << 1) ^ poly);
		else
			entry = (unsigned short)(entry << 1);
	}

	return entry;
}

static unsigned short invert_word(unsigned short w, int invert)
{
	if(invert)
		return ( bit_inverter[w&0xFF]<<8 ) | ( bit_inverter[w>>8] );
	else
		return w;
}

static unsigned short xchg_byte_word(unsigned short w, int invert)
{
	if(invert)
		return ( (w>>8) & 0xFF ) | ( (w<<8) & 0xFF00 );
	else
		return w;
}

unsigned short invert_index(unsigned short v, int invert)
{
	if(invert)
	{
		return bit_inverter[v]>>4;
	}
	else
	{
		return v;
	}
}

void generate_crc_array( unsigned short poly, int inv_poly, int inv_index, int inv_array_index, int inv_array_value, int xchg_byte, int xchg_array, unsigned int crcarray[32])
{
	int i;
	unsigned short crc16v;
	unsigned short crcarrayA[16];
	unsigned short crcarrayB[16];

	for(i=0;i<16;i++)
	{
		crc16v  = getCRC16entry( invert_index(i,inv_index), invert_word(poly,inv_poly) );

		if(xchg_array)
			crcarrayB[invert_index(i,inv_array_index)] = xchg_byte_word(invert_word(crc16v,inv_array_value), xchg_byte);
		else
			crcarrayA[invert_index(i,inv_array_index)] = xchg_byte_word(invert_word(crc16v,inv_array_value), xchg_byte);
	}

	for(i=0;i<16;i++)
	{
		crc16v  = getCRC16entry( invert_index(i,inv_index)<<4, invert_word(poly,inv_poly) );

		if(xchg_array)
			crcarrayA[invert_index(i,inv_array_index)] = xchg_byte_word(invert_word(crc16v,inv_array_value), xchg_byte);
		else
			crcarrayB[invert_index(i,inv_array_index)] = xchg_byte_word(invert_word(crc16v,inv_array_value), xchg_byte);
	}

	printf("\nArray A\n");
	for(i=0;i<16;i++)
	{
		crcarray[i] = crcarrayA[i];
		printf("0x%.4X,\n",crcarrayA[i]);
	}

	printf("\nArray B\n");
	for(i=0;i<16;i++)
	{
		crcarray[i+16] = crcarrayB[i];
		printf("0x%.4X,\n",crcarrayB[i]);
	}
}

const unsigned char test_frame1[]=
{
	0xAA,0x20,0x01,0x4B,0xCE,0x85,0x04
};

const unsigned char test_frame2[]={
	0x41,0x20,0x0A,0x63,0x80,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x30,0xE5,0x04
};

void test_crc_loop(unsigned short polynome)
{
    unsigned short crc,crc1,crc2;
    int i,j;
    int inv_poly,inv_index,inv_array_index,inv_array_value,xchg_byte,xchg_array;

    for(i=0;i<0x3F;i++)
    {
        inv_poly = 0;
        inv_index = 0;
        inv_array_index = 0;
        inv_array_value = 0;
        xchg_byte = 0;
        xchg_array = 0;

        if(i&0x1)
            inv_poly = 1;

        if(i&0x2)
            inv_index = 1;

        if(i&0x4)
            inv_array_index = 1;

        if(i&0x8)
            inv_array_value = 1;

        if(i&0x10)
            xchg_byte = 1;


        if(i&0x20)
            xchg_array = 1;

        generate_crc_array( polynome, inv_poly, inv_index, inv_array_index, inv_array_value, xchg_byte, xchg_array, (unsigned int *)&crc_array);


        crc = 0xFFFF;
        crc1 = crc16_buf((unsigned char*)&test_frame1[0],sizeof(test_frame1)-1, crc);
        crc2 = crc16_buf((unsigned char*)&test_frame2[0],sizeof(test_frame2)-1, crc);
        if( !crc1 && !crc2 )
        {
            printf("Init Value : 0xFFFF, inv_poly : %d, inv_index : %d, inv_array_index : %d, inv_array_value : %d, xchg_byte : %d, xchg_array : %d\n",inv_poly,inv_index,inv_array_index,inv_array_value,xchg_byte,xchg_array);
            for(j=0;j<32;j++)
            {
                printf("0x%.4X,",crc_array[j]);
            }
            printf("\n");

        }

        crc = 0x0000;
        crc1 = crc16_buf((unsigned char*)&test_frame1[0],sizeof(test_frame1)-1, crc);
        crc2 = crc16_buf((unsigned char*)&test_frame2[0],sizeof(test_frame2)-1, crc);
        if( !crc1 && !crc2 )
        {
            printf("Init Value : 0x0000, inv_poly : %d, inv_index : %d, inv_array_index : %d, inv_array_value : %d, xchg_byte : %d, xchg_array : %d\n",inv_poly,inv_index,inv_array_index,inv_array_value,xchg_byte,xchg_array);
            for(j=0;j<32;j++)
            {
                printf("0x%.4X,",crc_array[j]);
            }
            printf("\n");
        }
    }
}

#endif
