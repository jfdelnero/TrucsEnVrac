///////////////////////////////////////////////////////////////////////////////////
// File : lz4_header_gen.c
// Contains: LZ4 C header generator.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stdint.h>

#include "unpack_lz4.h"

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

int save_bin(unsigned char * buf, int size,char * filename)
{
	FILE * fo;

	fo = fopen(filename,"wb");
	if(fo)
	{
		fwrite(buf,size,1,fo);
		fclose(fo);

		printf("save_bin: %s saved, size : %d bytes\n",filename,size);

		return 0;
	}
	else
	{
		printf("save_bin: ERROR ! can't save to %s !\n",filename);

		return -1;
	}
}

unsigned char* load_bin(int * size,char * filename)
{
	FILE * fi;
	int filesize;
	unsigned char * mem_buf;

	mem_buf = NULL;

	fi = fopen(filename,"rb");
	if( fi )
	{
		fseek( fi, 0, SEEK_END );
		filesize = ftell(fi);
		fseek( fi, 0, SEEK_SET );

		mem_buf = malloc(filesize);

		if( !fread(mem_buf,filesize,1,fi) )
		{
			goto error;
		}

		fclose(fi);

		if(size)
			*size = filesize;

		printf("load_bin: %s loaded, size : %d bytes\n",filename,filesize);
	}
	else
	{
		goto error;
	}

	return mem_buf;

error:
	printf("load_bin: error !\n");

	if(mem_buf)
		free(mem_buf);
	if(fi)
		fclose(fi);

	return NULL;
}


int write_c_file(unsigned char * buf, int size,char * filename,char * varname,int src_size)
{
	FILE * fo;
	int i;

	fo = fopen(filename,"w");
	if(fo)
	{
		fprintf(fo,"///////////////////////////////////////////////////////////////////////////////\n");
		fprintf(fo,"// LZ4 packed data. (Size : %d Bytes, Unpacked size : %d Bytes)\n",size, src_size);
		fprintf(fo,"///////////////////////////////////////////////////////////////////////////////\n\n");

		fprintf(fo,"const unsigned char __attribute__ ((aligned (16))) %s[]=\n{\n\t",varname);

		for(i=0;i<size;i++)
		{
			if(!(i&0xF) && i)
				fprintf(fo,"\n\t");

			fprintf(fo,"0x%.2X",buf[i]);

			if( i!= (size - 1) )
				fprintf(fo,",");
		}
		fprintf(fo,"\n};\n");
		fclose(fo);

		printf("write_c_file: %s saved\n",filename);

		return 0;
	}
	else
	{
		printf("write_c_file: ERROR : Can't create %s !\n",filename);
	}

	return -1;
}

char * getfilenamebase(char * fullpath,char * filenamebase)
{
	int len,i,j;
	char separator;

	if(fullpath)
	{
		len=strlen(fullpath);

		separator = '/';

		i=0;
		if(len)
		{
			i=len-1;
			while(i &&  ( fullpath[i] != separator && fullpath[i]!=':') )
			{
				i--;
			}

			if( fullpath[i] == separator || fullpath[i]==':' )
			{
				i++;
			}

			if(i>len)
			{
				i=len;
			}
		}

		if(filenamebase)
		{
			strcpy(filenamebase,&fullpath[i]);

			j=0;
			while(filenamebase[j])
			{
				if(filenamebase[j] == '.')
					filenamebase[j] = '_';

				if(filenamebase[j] == ' ')
					filenamebase[j] = '_';

				j++;
			}
		}

		return &fullpath[i];
	}

	return 0;
}

char * strings_concat(char * dest, char * str1, char * str2)
{
	int len;
	char * nstr;

	nstr = dest;
	len = 0;

	if( dest )
		len += strlen(dest);

	if( str1 )
		len += strlen(str1);

	if( str2 )
		len += strlen(str2);

	if(len)
	{
		nstr = malloc(len + 1);
		if(nstr)
		{
			memset(nstr,0,len + 1);

			if( dest )
			{
				strcat(nstr,dest);
				free(dest);
			}

			if( str1 )
				strcat(nstr,str1);

			if( str2 )
				strcat(nstr,str2);
		}
	}

	return nstr;
}

int main(int argc, char *argv[])
{
	char * lz4outputfilename;
	char * syscmdline;
	char * varname;

	char outputfilename[512];
	char inputfilename[512];
	char filenamebase[512];

	int filesize,pack_filesize,src_filesize;
	unsigned char *lz4_buf;
	unsigned char *src_buf;
	unsigned char *tst_buf;
	int ret;

	lz4outputfilename = NULL;
	syscmdline = NULL;
	src_buf = NULL;

	printf("-------------------------------------------------------------------\n");
	printf("--           LZ4 packed data header generator  v2.1              --\n");
	printf("--       (c) 2019-2023 Jean-François DEL NERO / HxC2001          --\n");
	printf("-------------------------------------------------------------------\n");

	outputfilename[0] = 0;
	if( isOption( argc, argv,"o",(char*)&outputfilename) != 1 )
	{
		goto printsyntaxandexit;
	}

	inputfilename[0] = 0;
	if( isOption( argc, argv,"i",(char*)&inputfilename) != 1 )
	{
		goto printsyntaxandexit;
	}

	/////////////////////////////////////////////////////////////////

	getfilenamebase(inputfilename,filenamebase);

	lz4outputfilename = strings_concat(NULL, "lz4_gen_out_", filenamebase);
	lz4outputfilename = strings_concat(lz4outputfilename, ".lz4", NULL);
	if(!lz4outputfilename)
		goto error;

	remove(lz4outputfilename);

	src_buf = load_bin(&src_filesize,inputfilename);
	if(!src_buf)
	{
		printf("ERROR : Can't open %s !\n",inputfilename);
		goto error;
	}

	syscmdline = strings_concat(NULL, "lz4 -9 -z -f ", inputfilename);
	syscmdline = strings_concat(syscmdline, " ", lz4outputfilename);
	if(!syscmdline)
		goto error;

	ret = system(syscmdline);
	if(ret)
	{
		printf("ERROR : The command line \"%s\" returned %d. Is LZ4 installed ?\n",syscmdline,ret);
		goto error;
	}

	free(syscmdline);
	syscmdline = NULL;

	lz4_buf = load_bin(&filesize,lz4outputfilename);
	if(lz4_buf)
	{
		uint32_t packed_size;

		packed_size = (lz4_buf[0xA]*256*256*256) + (lz4_buf[9]*256*256) + (lz4_buf[8]*256) + lz4_buf[7];

		// Block format :
		// LZ4 Block size (32 bits LE)
		// Source size    (32 bits LE)
		// LZ4 Block      ("Packed size")
		// Unpacked CRC32 (TODO ! / Reserved)
		// Block CRC32    (TODO ! / Reserved)

		// Generate header
		lz4_buf[0] =  packed_size & 0xFF; // LZ4 Block size
		lz4_buf[1] = (packed_size>>8) & 0xFF;
		lz4_buf[2] = (packed_size>>16) & 0xFF;
		lz4_buf[3] = (packed_size>>24) & 0xFF;
		lz4_buf[4] = src_filesize & 0xFF; // Unpacked size
		lz4_buf[5] = (src_filesize>>8) & 0xFF;
		lz4_buf[6] = (src_filesize>>16) & 0xFF;
		lz4_buf[7] = (src_filesize>>24) & 0xFF;

		memcpy(&lz4_buf[8],&lz4_buf[0xB],packed_size);

		pack_filesize = packed_size + 8;

		//save_bin(lz4_buf, pack_filesize,"lz4_gen_out.bin");

		getfilenamebase(inputfilename,filenamebase);

		varname = strings_concat(NULL, "lz4_payload_", filenamebase);
		if(!varname)
		{
			free(lz4_buf);
			goto error;
		}

		printf("Testing packed data...\n");

		tst_buf = malloc(src_filesize);
		if( tst_buf )
		{
			memset(tst_buf,0xA5,src_filesize);

			ret = unpack_lz4( lz4_buf, tst_buf, src_filesize );
			if( ret > 0 )
			{
				printf("%d bytes unpacked...\n",ret);

				if(!memcmp(tst_buf,src_buf,src_filesize))
				{
					printf("Valid !\n");

					write_c_file(lz4_buf, pack_filesize,outputfilename,varname,src_filesize);
					ret = 0;
				}
				else
				{
					ret = -1;
					printf("Bad compression !\n");
					save_bin(tst_buf, src_filesize,"dbg_tst.bin");
				}
			}
			free(tst_buf);
		}

		free(varname);
		free(lz4_buf);
	}
	else
	{
		printf("ERROR : Can't open %s !\n",lz4outputfilename);
		goto error;
	}

	if(syscmdline)
		free(syscmdline);

	if(lz4outputfilename)
		free(lz4outputfilename);

	if(src_buf)
		free(src_buf);

	return ret;

printsyntaxandexit:

	printf("Syntax : %s -i:[infile.bin] -o:[outfile.h]\n",argv[0]);

	return 0;

error:
	if(syscmdline)
		free(syscmdline);

	if(lz4outputfilename)
		free(lz4outputfilename);

	if(src_buf)
		free(src_buf);

	return -1;
}
