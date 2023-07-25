///////////////////////////////////////////////////////////////////////////////////
// File : unpack_lz4.c
// Contains: LZ4 block unpacker.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "unpack_lz4.h"

static void * getlen(void * buf_ptr, uint32_t * len,void * end_ptr)
{
	uint8_t b;

	if(*len == 15)
	{
		do
		{
			// More info to extract
			b = *((uint8_t*)(buf_ptr++));

			if(buf_ptr > end_ptr)
				return 0x0;

			*len += b;
		}while(b == 0xFF);
	}

	return buf_ptr;
}

static void cpdata(unsigned char * src, unsigned char * dst, uint32_t len)
{
	for(uint32_t i=0;i<len;i++)
		*dst++ = *src++;
}

int unpack_lz4(void * pack_ptr, void * dest_ptr, uint32_t dest_max_size )
{
	uint32_t pack_size, unpacked_size;
	uint32_t litteral_len, match_len;
	uint16_t match_offset;
	uint8_t  token;
	void * match_ptr;
	void * start_pack_ptr, * end_pack_ptr;
	void * start_dest_ptr, * end_dest_ptr;

	start_pack_ptr = pack_ptr;

	pack_size =  *((uint8_t*)(pack_ptr++));
	pack_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<8;
	pack_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<16;
	pack_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<24;

	unpacked_size =  *((uint8_t*)(pack_ptr++));
	unpacked_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<8;
	unpacked_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<16;
	unpacked_size |= (uint32_t)(*((uint8_t*)(pack_ptr++)))<<24;

	end_pack_ptr = pack_ptr + pack_size;

	start_dest_ptr = dest_ptr;
	end_dest_ptr = dest_ptr + dest_max_size;

	do
	{
		token = *((uint8_t*)(pack_ptr++));

		litteral_len = token >> 4;

		if(litteral_len)
		{
			pack_ptr = getlen(pack_ptr, &litteral_len, end_pack_ptr);

			if(
				(pack_ptr < start_pack_ptr  || pack_ptr >= end_pack_ptr) ||
				(dest_ptr < start_dest_ptr  || dest_ptr >= end_dest_ptr)
			)
			{
				return -1;
			}

			cpdata((unsigned char*)pack_ptr, (unsigned char *)dest_ptr, litteral_len);
			pack_ptr += litteral_len;
			dest_ptr += litteral_len;
		}

		if( pack_ptr >= end_pack_ptr - 1)
		{
			if((end_dest_ptr - start_dest_ptr) == unpacked_size )
				return unpacked_size;
		}

		// Match low byte.
        match_offset = ( (uint16_t)(*((unsigned char*)pack_ptr)) | ( ((uint16_t)(*((unsigned char*)pack_ptr+1))<<8) ) );
		match_ptr = dest_ptr - match_offset;
		pack_ptr += 2;

		match_len = token & 0xF;

		pack_ptr = getlen(pack_ptr, &match_len,end_pack_ptr);
		match_len += 4;

		if(
			(match_ptr < start_dest_ptr  || match_ptr >= end_dest_ptr) ||
			(dest_ptr < start_dest_ptr  || dest_ptr >= end_dest_ptr)
		)
		{
			return -1;
		}

		cpdata((unsigned char*)match_ptr, (unsigned char *)dest_ptr, match_len);
		dest_ptr += match_len;

	} while( pack_ptr < end_pack_ptr );

	if((end_dest_ptr - start_dest_ptr) == unpacked_size )
		return unpacked_size;

	return -1;
}
