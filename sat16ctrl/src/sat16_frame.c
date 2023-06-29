///////////////////////////////////////////////////////////////////////////////////
// File : sat16_frame.c
// Contains: SAT16 relays board frame generator
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "sat16ctrl_ctx.h"
#include "sat16_frame.h"

#include "crc.h"

int encode_sat16_frame(sat16ctrl_ctx * ctx,unsigned char address, unsigned char * data, int data_size, unsigned char * out_frame)
{
	int i,j;
	unsigned short crc_value;

	i = 0;

	out_frame[i++] = 0xAA;
	out_frame[i++] = address & 0xFF;
	out_frame[i++] = data_size & 0xFF;

	for(j = 0; j < (data_size & 0xFF); j++)
	{
		out_frame[i++] = data[j];
	}

	crc_value = crc16_buf(out_frame, i, 0x0000);

	out_frame[i++] = (crc_value & 0xFF);
	out_frame[i++] = (crc_value >> 8);

	out_frame[i++] = 0x04;

	return i;
}
