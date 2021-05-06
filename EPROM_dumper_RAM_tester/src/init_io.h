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
// File : init_io.h
// Contains: io init functions
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

#define INPUT  1
#define OUTPUT 0

#define SPI_SDO_PORT     PORTCbits.RC5
#define SPI_SDO_TRIS     TRISCbits.RC5

#define SPI_SDI_PORT     PORTCbits.RC4
#define SPI_SDI_TRIS     TRISCbits.RC4

#define SPI_CLK_PORT     PORTCbits.RC3
#define SPI_CLK_TRIS     TRISCbits.RC3

void init_io(void);
