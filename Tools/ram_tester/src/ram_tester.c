///////////////////////////////////////////////////////////////////////////////////
// File : ram_tester.c
// Contains: System RAM tester (Windows/Linux/Posix systems).
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MIN_PAGE_SIZE (1024*1024)

//#define _32BITS_TARGET_ 1

#ifdef _32BITS_TARGET_

// 32 bits target / mode

typedef uint32_t WORD;
#define WORDHEXPRINT "08"PRIX32
#define WORDDECPRINT "%d"
#define RANDSEED 0xAF5469CD
#define BANNERTARGETMODE "(32 Bits mode)"

// xorshift 32
static inline uint32_t xorshift_rand( uint32_t seed )
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;

	return seed;
}

#else

// 64 bits target / mode

typedef uint64_t WORD;
#define WORDHEXPRINT "016"PRIX64
#define WORDDECPRINT "%ld"
#define RANDSEED 0xAF5469CD67582A93
#define BANNERTARGETMODE "(64 Bits mode)"

// xorshift 64
static inline uint64_t xorshift_rand( uint64_t seed )
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	seed ^= seed << 13;
	seed ^= seed >> 7;
	seed ^= seed << 17;

	return seed;
}

#endif


int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,sizeof(option));

	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,sizeof(option));

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':' && ( j < (sizeof(option) - 1)) )
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						argtoparam[0] = 0;

						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] && j < (512 - 1) )
							{
								argtoparam[j]=argv[param][i];
								i++;
								j++;
							}
							argtoparam[j]=0;
							return 1;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						return 1;
					}
				}
			}
		}
		param++;
	}

	return 0;
}

int main (int argc, char ** argv)
{
	char tmp_str[512];

	WORD * src_ptr;
	WORD * ptr,rnd,rd;
	WORD page_size,word_page_size,max_loop;
	WORD i,j,ecnt;

	page_size = MIN_PAGE_SIZE;
	max_loop = 0;

	printf("Ram tester v1.0 "BANNERTARGETMODE"\n");
	printf("(c) 2023 Jean-François DEL NERO\n");

	if( isOption( argc, argv,"page_size",(char*)&tmp_str) == 1 )
	{
		page_size = page_size * (WORD)atoi(tmp_str);
	}

	if( isOption( argc, argv,"max_loop",(char*)&tmp_str) == 1 )
	{
		max_loop = (WORD)atoi(tmp_str);
	}

	word_page_size = page_size/sizeof(WORD);

	if(page_size && word_page_size && max_loop)
	{
		ecnt = 0;
		src_ptr = malloc(page_size);
		if(src_ptr)
		{
			printf("Starting ram_tester... (Page size : "WORDDECPRINT" MiB, Loop(s) : "WORDDECPRINT")\n",page_size/(MIN_PAGE_SIZE),max_loop);

			for(i=0;i<max_loop;i++)
			{
				printf("Loop n°"WORDDECPRINT" (Total error(s) count : "WORDDECPRINT")...\n",i,ecnt);

				// Fill the page...
				ptr = src_ptr;
				rnd = RANDSEED + i;

				for(j=0;j<word_page_size;j++)
				{
					rnd = xorshift_rand(rnd);
					*ptr++ = rnd;
				}

				// Check the page...
				ptr = src_ptr;
				rnd = RANDSEED + i;

				for(j=0;j<word_page_size;j++)
				{
					rnd = xorshift_rand(rnd);
					rd = *ptr++;
					if( rd != rnd )
					{
						ecnt++;
						printf("Memory error n°"WORDDECPRINT" ! (At Word 0x%"WORDHEXPRINT" = Read: 0x%"WORDHEXPRINT", Should Be: 0x%"WORDHEXPRINT", Error Mask: 0x%"WORDHEXPRINT")\n",ecnt,j,rd, rnd, rd ^ rnd);
					}
				}
			}

			printf("Test done !\nTotal error(s) count : "WORDDECPRINT"\n",ecnt);

			free(src_ptr);

			if(!ecnt)
				exit(1);
			else
				exit(-2);
		}
		else
		{
			printf("ERROR : Can't allocate test memory page...\n");

			exit(-1);
		}
	}
	else
	{
		printf("Syntax : %s -page_size:[Test page size (MiB)] -max_loop:[Test loop(s) count]\n",argv[0]);
	}

	exit(0);
}
