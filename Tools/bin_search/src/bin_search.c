///////////////////////////////////////////////////////////////////////////////////
// File : bin_search.c
// Contains: File binary sequence finder
//
// Copyright (C) 2024 Jean-François DEL NERO
//
// Written by: Jean-François DEL NERO (2024)
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#include "cache.h"
#include "cmd_param.h"

static unsigned char quartetchar2bin(unsigned char c)
{
	if( c >= '0' && c <= '9')
		return c - '0';

	if( c >= 'a' && c <= 'f')
		return 10 + c - 'a';

	if( c >= 'A' && c <= 'F')
		return 10 + c - 'A';

	return 0;
}

static int ishex(unsigned char c)
{
	if( c >= '0' && c <= '9')
		return 1;

	if( c >= 'a' && c <= 'f')
		return 1;

	if( c >= 'A' && c <= 'F')
		return 1;

	return 0;
}

int is_printable_char(unsigned char c)
{
	int i;
	const unsigned char specialchar[]={"&#{}()|_@=$!?;+*-"};

	if( (c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') )
	{
		return 1;
	}

	i = 0;
	while(specialchar[i])
	{
		if(specialchar[i] == c)
		{
			return 1;
		}

		i++;
	}

	return 0;
}

void printbuf(void * buf,int size, int markstart, int markend)
{
	#define PRINTBUF_HEXPERLINE 16
	#define PRINTBUF_MAXLINE_SIZE (1 + (3*PRINTBUF_HEXPERLINE)+1+PRINTBUF_HEXPERLINE+2)

	int i,j;
	unsigned char *ptr = buf;
	char tmp[8];
	char str[PRINTBUF_MAXLINE_SIZE];

	memset(str, ' ', PRINTBUF_MAXLINE_SIZE);
	str[PRINTBUF_MAXLINE_SIZE-1] = 0;

	j = 0;
	for(i=0;i<size;i++)
	{
		if(!(i&(PRINTBUF_HEXPERLINE-1)) && i)
		{
			printf("%s\n", str);
			memset(str, ' ', PRINTBUF_MAXLINE_SIZE);
			str[PRINTBUF_MAXLINE_SIZE-1] = 0;
			j = 0;
		}

		if( i == markstart)
		{
			str[(j*3)] = '[';
		}

		snprintf(tmp, sizeof(tmp), "%02X", ptr[i]);
		memcpy(&str[1+(j*3)],tmp,2);

		if( i == markend)
		{
			str[(j)*3] = ']';
		}

		if( is_printable_char(ptr[i]) )
			str[1 + 3*PRINTBUF_HEXPERLINE + 1 + j] = ptr[i];
		else
			str[1 + 3*PRINTBUF_HEXPERLINE + 1 + j] = '.';

		j++;
	}

	printf("%s\n", str);
}

int strbin2bin(char * str, unsigned char * binseq, int max_size)
{
	int i, j, q_cnt;
	unsigned char byte;

	byte = 0;
	q_cnt = 0;


	i = 0;
	j = 0;
	while( str[i] )
	{
		if( ishex(str[i]) )
		{
			byte <<= 4;

			byte |= quartetchar2bin( str[i] );

			q_cnt++;
			if(q_cnt >= 2)
			{
				if( j < max_size)
				{
					binseq[j++] = byte;
				}

				byte = 0;
				q_cnt = 0;
			}
		}

		i++;
	}

	return j;
}

int binseqsearch(char * file, char * seqfile, unsigned char * binseq, int binseqsize)
{
	file_cache bin_file;
	file_cache seq_file;
	int success;
	int bin_i,seq_i,i;
	int lastvalid;
	int cnt;
	unsigned char seqbyte;
	unsigned char prtbuf[128];

	success = 0;

	if( open_file( &bin_file, file, -1, 0 ) < 0 )
	{
		printf("File access error : %s\n",file);
		return -1;
	}

	if( seqfile )
	{
		if( open_file( &seq_file, seqfile, -1, 0 ) < 0 )
		{
			printf("Sequence File access error : %s\n",file);
			return -1;
		}

		binseqsize = bin_file.file_size;
	}

	if(!binseqsize)
	{
		close_file( &bin_file );

		if( seqfile )
			close_file( &seq_file );

		printf("I refuse to search a null sized binary sequence, sorry... ;)\n");

		return -1;
	}

	cnt = 0;
	lastvalid = 0;
	bin_i = 0;
	seq_i = 0;

	while( bin_i < bin_file.file_size )
	{
		if( seqfile )
			seqbyte = get_byte( &seq_file, seq_i, &success );
		else
			seqbyte = binseq[ seq_i ];

		if( get_byte( &bin_file, bin_i, &success ) == seqbyte )
		{
			if( !seq_i )
				lastvalid = bin_i;

			seq_i++;
			bin_i++;

			if(seq_i >= binseqsize)
			{
				// Found !
				seq_i = 0;

				printf("%s: Offset 0x%X\n", file, lastvalid);

				for(i=0;i<sizeof(prtbuf);i++)
				{
					prtbuf[i] = get_byte( &bin_file, ((lastvalid & ~0xF) - 0x20) + i, &success );
				}

				printbuf(prtbuf,sizeof(prtbuf), lastvalid - ((lastvalid & ~0xF) - 0x20), lastvalid - ((lastvalid & ~0xF) - 0x20) + binseqsize );
				printf("\n");

				bin_i = lastvalid + 1;
				cnt++;
			}
		}
		else
		{
			if( seq_i )
			{
				bin_i = lastvalid + 1;
			}

			seq_i = 0;
			bin_i++;
		}
	}

	close_file( &bin_file );

	if( seqfile )
		close_file( &seq_file );

	return cnt;
}

int main (int argc, char ** argv)
{
	char seq_file_path[PATH_MAX];
	char * seq_file_path_ptr;
	char bin_seq_str[4096];

	unsigned char bin_seq[4096];
	int  bin_seq_size;
	int i;

	printf("bin_search v1.0. -help format command line syntax.");

	if(isOption(argc, argv,"-help",NULL, NULL) )
	{
		printf("Syntax:\n");
		printf("%s -binseq:0123456789ABCDEF [files]\n",argv[0]);
		printf("%s -strseq:\"ascii string\" [files]\n",argv[0]);
		printf("%s -fileseq:file_path [files]\n",argv[0]);

		exit(0);
	}

	seq_file_path_ptr = NULL;
	bin_seq_size = 0;
	bin_seq_str[0] = '\0';

	if(isOption(argc, argv,"binseq",(char*)&bin_seq_str, NULL) )
	{
		bin_seq_size = strbin2bin(bin_seq_str, bin_seq, sizeof(bin_seq));

		printf("Binary seq to search (%d bytes) : ", bin_seq_size);
		for(i=0;i<bin_seq_size;i++)
		{
			printf("%.2X ",bin_seq[i]);
		}
		printf("\n");
	}

	if(isOption(argc, argv,"strseq",(char*)&bin_seq_str, NULL) )
	{
		i = 0;
		bin_seq_size = 0;
		while(bin_seq_str[i])
		{
			bin_seq[i] = bin_seq_str[i];
			i++;
			bin_seq_size++;
		}

		printf("str seq to search (%d bytes) : ", bin_seq_size);

		for(i=0;i<bin_seq_size;i++)
		{
			printf("%.2X ",bin_seq[i]);
		}

		 printf("\n");
	}

	if(isOption(argc, argv,"fileseq",(char*)&seq_file_path, NULL) )
	{
		printf("file seq to search : %s\n", seq_file_path);
		seq_file_path_ptr = seq_file_path;
	}

	i = 1;
	while( i < argc )
	{
		if(argv[i][0] != '-')
		{
			binseqsearch( argv[i], seq_file_path_ptr, (unsigned char*)bin_seq, bin_seq_size );
		}
		i++;
	}

	return 0;
}
