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

int decode_sat16_frame(sat16ctrl_ctx * ctx, sat_16_rx_frame_state * frame_state, unsigned char rx_byte )
{
	int ret;
	unsigned short crc_value;

	ret = 0;

	switch(frame_state->frame_size)
	{
		case 0:
			// Wait Start byte
			if(rx_byte == 0xAA)
			{
				frame_state->full_frame[frame_state->frame_size++] = rx_byte;
			}
		break;
		case 1:
			// Address
			frame_state->full_frame[frame_state->frame_size++] = rx_byte;
			frame_state->address = rx_byte;
		break;
		case 2:
			// Size
			frame_state->full_frame[frame_state->frame_size++] = rx_byte;
			frame_state->data_size = rx_byte;
		break;
		default:
			if( frame_state->frame_size - 3 < frame_state->data_size ) // Data
			{
				frame_state->data[frame_state->frame_size - 3] = rx_byte;
				frame_state->full_frame[frame_state->frame_size++] = rx_byte;
			}
			else
			{
				switch((frame_state->frame_size - frame_state->data_size) - 3)
				{
					case 0: //CRC L
						frame_state->crc = rx_byte;
						frame_state->full_frame[frame_state->frame_size++] = rx_byte;
					break;
					case 1: //CRC H
						frame_state->crc |= (rx_byte<<8);
						frame_state->full_frame[frame_state->frame_size++] = rx_byte;
					break;
					case 2: //EOF - 0x04
						crc_value = crc16_buf(frame_state->full_frame, frame_state->frame_size, 0x0000);

						frame_state->full_frame[frame_state->frame_size++] = rx_byte;

						if( !crc_value && rx_byte == 0x04)
							ret = 1;
						else
							ret = -2;
					break;
					default:
						ret = -1;
					break;
				}
			}
		break;
	}

	return ret;
}
