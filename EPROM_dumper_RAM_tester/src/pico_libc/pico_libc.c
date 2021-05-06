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
// File : pico_libc.c
// Contains: pico libc
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////

int tolower(int c)
{
	if( c >= 'A' && c <= 'Z')
		return (c - 'A') + 'a';

	return c;
}

int toupper(int c)
{
	if( c >= 'a' && c <= 'z')
		return (c - 'a') + 'A';

	return c;
}

void * memcpy (void *dest, const void *src, int len)
{
	char *d = dest;
	const char *s = src;
	while (len--)
		*d++ = *s++;

	return dest;
}

void * memset (void *dest, int val, int len)
{
	unsigned char *ptr = dest;
	while (len-- > 0)
		*ptr++ = val;

	return dest;
}

int memcmp (const void *str1, const void *str2, int count)
{
	const unsigned char *s1 = str1;
	const unsigned char *s2 = str2;

	while (count-- > 0)
	{
		if (*s1++ != *s2++)
			return s1[-1] < s2[-1] ? -1 : 1;
	}
	return 0;
}

int strcmp (const unsigned char *p1, const unsigned char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do
	{
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0')
			return c1 - c2;
	}while (c1 == c2);

	return c1 - c2;
}

int strncmp (const unsigned char *p1, const unsigned char *p2, int maxlen)
{
	if ( !maxlen )
		return (0);

	do {
		if (*p1 != *p2++)
			return (*(unsigned char *)p1 - *(unsigned char *)--p2);

		if (*p1++ == 0)
			break;

	} while (--maxlen != 0);

	return (0);
}

int nocase_strncmp (const unsigned char *p1, const unsigned char *p2, int maxlen)
{
	unsigned char c1,c2;

	if ( !maxlen )
		return (0);

	do {
		c1 = tolower(*p1++);
		c2 = tolower(*p2++);

		if (c1 != c2)
			return (c1 - c2);

		if (c1 == 0)
			break;

	} while (--maxlen != 0);

	return (0);
}

int strnlen (const unsigned char *s, int maxlen)
{
	int i;

	for (i = 0; i < maxlen; i++)
	{
		if (!*s)
		{
			return i;
		}
		s++;
	}

	return i;
}

int strlen (const unsigned char *s)
{
	return strnlen(s,4096*4);
}


unsigned char * strncpy (unsigned char *s1, const unsigned char *s2, int n)
{
	int i;
	unsigned char * ret;

	ret = s1;
	i = 0;
	while(i < n)
	{
		*s1++ = *s2;

		if(*s2)
			s2++;
		else
			return ret;

		i++;
	}

	return ret;
}

unsigned char * strnncat (unsigned char *s1, const unsigned char *s2, int n)
{
	if( (strnlen(s1,4096) + strnlen(s2,4096) + 1) < n )
	{
		strncpy (&s1[strnlen(s1,4096)], s2,n);
	}

	return s1;
}

unsigned char * strcpy (unsigned char *s1, const unsigned char *s2)
{
	unsigned char * ret;

	ret = s1;
	while(*s2)
	{
		*s1++ = *s2++;
	};

	*s1 = 0;

	return ret;
}

unsigned char * strlwr (unsigned char *s)
{
	unsigned char *ucs = (unsigned char *) s;
	for ( ; *ucs != '\0'; ucs++)
	{
		*ucs = tolower(*ucs);
	}
	return s;
}

#if 0 // printf support
static void printserial(char **str, int c)
{
	extern void printchar(unsigned char c);
	if (str) {
		**str = c;
		++(*str);
	}
	else printchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

int puts(const char *s)
{
	while(*s)
	{
		
		s++;
	}
}

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printserial (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printserial (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printserial (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printserial (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, int *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char *)(*varg++);
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printserial (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	return pc;
}

/* assuming sizeof(void *) == sizeof(int) */

int printf(const char *format, ...)
{
	register int *varg = (int *)(&format);
	return print(0, varg);
}

int sprintf(char *out, const char *format, ...)
{
	register int *varg = (int *)(&format);
	return print(&out, varg);
}