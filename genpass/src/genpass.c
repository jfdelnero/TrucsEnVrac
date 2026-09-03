///////////////////////////////////////////////////////////////////////////////////
// File : genpass.c
// Contains: SAT16 relay board frame sender software
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////
#ifndef ANSI_FILE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define PAGESIZE (64*1024)
#define SHA3_SIZE 64

#include "sha3.h"

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

int strtoValue(char* str_return)
{
	int value;

	value = 0;

	if (str_return)
	{
		if (strlen(str_return) > 2)
		{
			if (str_return[0] == '0' && (str_return[1] == 'x' || str_return[1] == 'X'))
			{
				value = (int)strtoul(str_return, NULL, 0);
			}
			else
			{
				value = atoi(str_return);
			}
		}
		else
		{
			value = atoi(str_return);
		}
	}

	return value;
}

void printhelp(char* argv[])
{
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "  -help \t\t\t: This help\n");
	fprintf(stdout, "\n");
}

void addsha( unsigned char * sha3_2, unsigned char * sha3_1 )
{
	unsigned short acc,i;

	i = SHA3_SIZE;

	acc = 0x0000;
	while(i)
	{
		i--;
		acc = (unsigned short)sha3_2[i] + (unsigned short)sha3_1[i] + (unsigned short)(acc >> 8);

		sha3_2[i] = acc & 0xFF;
	}
}

int sha3_file(char * file, unsigned char * sha3_512)
{
	FILE * f;
	int ret,n;
	unsigned char * buf;
	unsigned char sha[SHA3_SIZE];
	unsigned char finalsha[SHA3_SIZE];

	ret = -1;

	memset(&sha,0,sizeof(sha));
	memset(&finalsha,0,sizeof(finalsha));

	f = fopen(file,"rb");
	if(f)
	{
		buf = malloc(PAGESIZE);
		if(buf)
		{
			do
			{
				n = fread(buf,1,PAGESIZE,f);

				if ( n > 0 )
				{
					sha3(buf, n, sha, sizeof(sha));
					addsha( (uint8_t *)&finalsha, (uint8_t *)&sha );
					memset(&sha,0,sizeof(SHA3_SIZE));
				}
			}while( n == PAGESIZE );

			memset(buf,0,PAGESIZE);
			ret = 0;

			free(buf);
		}

		fclose(f);
	}

	if(ret >= 0)
		memcpy(sha3_512, &finalsha,sizeof(finalsha));

	memset(&sha,0,sizeof(sha));
	memset(&finalsha,0,sizeof(finalsha));

	return ret;
}

const char * files_list[]=
{
	"/proc/cpuinfo",
	"/proc/diskstats",
	"/proc/interrupts",
	"/proc/schedstat",
	"/proc/uptime",
	"/proc/meminfo",
	"/proc/softirqs",
	"/proc/stat",
	"/proc/net/dev",
	"/proc/pressure/cpu",
	"/proc/pressure/io",
	"/proc/pressure/memory",
	0
};

void sha2pass(unsigned char * dict, int dict_size, char * pass, int pass_size, uint8_t * sha)
{
	int i;
	uint8_t hash[SHA3_SIZE];

	memset(&hash,0,SHA3_SIZE);

	for(i=0;i<SHA3_SIZE;i++)
	{
		hash[i % pass_size] += sha[i];
	}

	for(i=0;i<pass_size;i++)
	{
		pass[i] = dict[(hash[i] % dict_size)];
	}

	pass[i] = 0;
}

double string_entropy(char * str)
{
	int i,histsize;
	unsigned char hist[256];
	int where[256];
	double H;
	int len;

	memset(&where,0xFF,sizeof(where));
	memset(&hist,0,sizeof(hist));

	histsize = 0;

	// Make "packed" histogram
	i = 0;
	while(str[i])
	{
		if( where[(int)str[i]] == -1)
		{
			where[(int)str[i]] = histsize;
			histsize++;
		}
		hist[where[(int)str[i]]]++;
		i++;
	}

	len = i;
	H = 0;
	for(i=0;i<histsize;i++)
	{
		H -= (double)hist[i] / len*log2((double)hist[i]/len);
	}

	return H;
}

int generate_dictionnary(unsigned char * dict, unsigned int flags)
{
	int dict_size;
	unsigned char c;
	int i;

	dict_size = 0;

	if( flags & (0x1<<0) )
	{
		for(c='a';c<='z';c++)
		{
			dict[dict_size++] = c;
		}
	}

	if( flags & (0x1<<1) )
	{
		for(c='A';c<='Z';c++)
		{
			dict[dict_size++] = c;
		}
	}

	if( flags & (0x1<<2) )
	{
		for(i=0;i<2;i++)
		{
			for(c='0';c<='9';c++)
			{
				dict[dict_size++] = c;
			}
		}
	}

	if( flags & (0x1<<2) )
	{
		for(i=0;i<2;i++)
		{
			dict[dict_size++] = '!';
			dict[dict_size++] = '@';
			dict[dict_size++] = '#';
			dict[dict_size++] = '$';
			dict[dict_size++] = '%';
			dict[dict_size++] = '-';
			dict[dict_size++] = '=';
			dict[dict_size++] = '&';
			dict[dict_size++] = '*';
			dict[dict_size++] = '_';
		}
	}

	return dict_size;
}

double gen_pass(uint8_t * sha_seed, char * pass, int pass_size_min, int pass_size_max, unsigned int dict_flags)
{
	time_t t;
	struct tm tm;
	uint8_t sha[SHA3_SIZE];
	uint8_t sha_final[SHA3_SIZE];
	int pass_size;
	unsigned char dict[256];
	int dict_size;
	int i, sum;

	dict_size = generate_dictionnary((unsigned char*)&dict, dict_flags);

	memcpy(&sha_final,sha_seed,SHA3_SIZE);

	// current time sha
	t = time(NULL);
	tm = *localtime(&t);
	sha3(&tm, sizeof(struct tm), (uint8_t *)&sha, SHA3_SIZE);
	addsha( (uint8_t *)&sha_final, (uint8_t *)&sha );

	// "random" files sources sha
	i = 0;
	while(files_list[i])
	{
		if( sha3_file((char*)files_list[i], (uint8_t *)&sha) < 0 )
			goto error;

		addsha( (uint8_t *)&sha_final, (uint8_t *)&sha );

		i++;
	}

	sum = 0;
	for(i=0;i<SHA3_SIZE;i++)
	{
		sum += sha_final[i];
	}

	pass_size = pass_size_min + (sum % ((pass_size_max - pass_size_min)+1));

	sha2pass((unsigned char*)&dict, dict_size, pass, pass_size,  (uint8_t *)&sha_final);

	memcpy(sha_seed, &sha_final, SHA3_SIZE);
	
	return string_entropy(pass);

error:
	fprintf(stderr,"Can't open/read %s ...\n", files_list[i]);
	return -1;
}

void stats_pass(char * pass)
{
	int len,uniquecnt,i;
	unsigned char hist[256];
	double H;

	memset(&hist,0,sizeof(hist));
	len = 0;
	while(pass[len])
	{
		len++;
	}

	for(i = 0;i < len; i++)
	{
		hist[((unsigned char)pass[i])]++;
	}

	uniquecnt = 0;
	for(i = 0;i < 256; i++)
	{
		if( hist[i] )
		{
			uniquecnt++;
		}
	}

	memset(&hist,0,sizeof(hist));

	H = string_entropy(pass);

	fprintf(stderr,"%d characters, %d unique characters, Entropy : %f\n\n", len, uniquecnt, H );

	fflush(stderr);
	fflush(stdout);

	for(i=0;i<len;i++)
	{
		printf("%c",pass[i]);
		pass[i] = 0;
		fflush(stdout);
	}
	printf("\n");
}

int main(int argc, char* argv[])
{
	int ret,i,p;
	uint8_t sha[SHA3_SIZE],sha_final[SHA3_SIZE];
	char pass[SHA3_SIZE+1];
	char final_pass[SHA3_SIZE+1];
	char tmp_str[512];
	int pass_size_min;
	int pass_size_max;
	unsigned int dict_flags;
	int sum;
	double H,H_max;

	ret = 0;
	dict_flags = 0xF;
	pass_size_min = 18;
	pass_size_max = 22;

	fprintf(stderr, "genpass v0.1\n");

	// help option...
	if (isOption(argc, argv, "help", 0) > 0)
	{
		printhelp(argv);
		exit(0);
	}

	if (isOption(argc, argv, "num", (char*)tmp_str) > 0)
	{
		pass_size_min = atoi(tmp_str);
		if( pass_size_min >= 64)
			pass_size_min = 64;

		pass_size_max = pass_size_min;
	}
	
	memset(&sha_final,0x00,SHA3_SIZE);

	i = 1;
	while( argv[i] )
	{
		if( argv[i][0] != '-' )
		{
			sha3_file((char*)argv[i], (uint8_t *)&sha);
			addsha( (uint8_t *)&sha_final, (uint8_t *)&sha );
			memset(&sha,0,SHA3_SIZE);
		}
		i++;
	}

	H_max = 0;
	for(p=0;p<256;p++)
	{
		H = gen_pass((unsigned char*)&sha_final, (char*)&pass, pass_size_min, pass_size_max,dict_flags);
		if( H < 0 )
			goto error;

		if( H >= H_max)
		{
			H_max = H;
			strcpy((char*)&final_pass,(char*)&pass);
		}

		sum = 0;
		for(i=0;i<SHA3_SIZE;i++)
		{
			sum += sha_final[i];
		}

		usleep( (10*1000) + ((sum%20)*1000) );
	}

	stats_pass((char*)&final_pass);

	memset(&pass,0,sizeof(pass));
	memset(&final_pass,0,sizeof(final_pass));
	memset(&sha,0,SHA3_SIZE);
	memset(&sha_final,0,SHA3_SIZE);

	return ret;

error:
	return -1;
}
