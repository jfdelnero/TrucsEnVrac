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
// File : pico_libc.h
// Contains: pico libc
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

void * memcpy (void *dest, const void *src, int len);
void * memset (void *dest, int val, int len);
int memcmp (const void *str1, const void *str2, int count);
int strcmp (const unsigned char *p1, const unsigned char *p2);
int strncmp (const unsigned char *p1, const unsigned char *p2, int maxlen);
int nocase_strncmp (const unsigned char *p1, const unsigned char *p2, int maxlen);
int strnlen (const unsigned char *s, int maxlen);
int strlen (const unsigned char *s);
unsigned char * strncpy (unsigned char *s1, const unsigned char *s2, int n);
unsigned char * strnncat (unsigned char *s1, const unsigned char *s2, int n);
unsigned char * strcpy (unsigned char *s1, const unsigned char *s2);
int tolower(int c);
int toupper(int c);
unsigned char * strlwr (unsigned char *s);

#ifndef NULL
#define NULL 0x00000000
#endif

#define PACKED_STRUCT __attribute__ ((__packed__))
#define LONG_ALIGNED  __attribute__ ((aligned (4)))
#define SHORT_ALIGNED __attribute__ ((aligned (2)))
#define SECTOR_ALIGNED __attribute__ ((aligned (512)))