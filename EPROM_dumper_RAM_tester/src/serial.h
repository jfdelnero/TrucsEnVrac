///////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------//
//----------H----H--X----X-----CCCCC-----22222----0000-----0000-----11-------//
//---------H----H----X-X-----C--------------2---0----0---0----0---1-1--------//
//--------HHHHHH-----X------C----------22222---0----0---0----0-----1---------//
//-------H----H----X--X----C----------2-------0----0---0----0-----1----------//
//------H----H---X-----X---CCCCC-----22222----0000-----0000----11111---------//
//---------------------------------------------------------------------------//
//----- Contact: hxc2001 at hxc2001.com ----------- https://hxc2001.com -----//
//----- (c) 2021 Jean-Fran�ois DEL NERO ----------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////
// File : serial.h
// Contains: Serial port I/O
//
// Written by: Jean-Fran�ois DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

void Serial_Init(void);
void Serial_Send_Byte(unsigned char val);
unsigned char Serial_Receive_Byte(unsigned char* val);
unsigned char Serial_Receive_Byte_Fast(void);
