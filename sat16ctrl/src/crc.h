#ifndef _INC_CRC_H
#define _INC_CRC_H

unsigned short crc16( unsigned char data, unsigned short crc16 );
unsigned short crc16_buf(const unsigned char * buffer, int size, \
			 unsigned short crc16);

#ifdef CRCARRAYGENERATOR
void test_crc_loop(unsigned short polynome);
#endif

#endif /* _INC_CRC_H */
