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
// File : print.h
// Contains: Minimal print functions
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

void printchar(unsigned char c);
void print(const unsigned char *pucBuffer);
void printrom(const far rom char *pucBuffer);

void printdec(unsigned char c);
void printhex(unsigned char c);
void printhex_long(unsigned long c);
void printbuf(unsigned char * buf,unsigned short size);
void printbuflong(unsigned long * buf,unsigned short  size);
void printhex_short(unsigned short c);

void dbg_printf(const unsigned char *pucBuffer, unsigned int arg1,unsigned int arg2,unsigned int arg3);

unsigned char * str_printhex_long(unsigned char * buf, unsigned long c,int nb_bytes);
unsigned char str_hex_to_byte(unsigned char * buf);
