///////////////////////////////////////////////////////////////////////////////////
// File : rom_splitter.c
// Contains: 16 / 32 bits ROM file splitter.
//
// Written by: Jean-François DEL NERO
//
// Syntax : rom_split ROMTOSPLIT.ROM [-num_of_bytes:x] [-num_of_banks:x] [-bank_word_size:x (in KB)] [-mirror:x]
//
// Example : Split a ROM file for a 16 Bits system with 2*32KB*3 EPROMs (192KB):
//
// rom_split ROMTOSPLIT.ROM -num_of_bytes:2 -num_of_banks:3 -bank_word_size:32
//
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_CACHE_SIZE (4096)

typedef struct file_cache_
{
	FILE * f;
	unsigned int  current_offset;
	unsigned int  cur_page_size;
	unsigned int  file_size;
	unsigned char cache_buffer[FILE_CACHE_SIZE];
	int dirty;
}file_cache;

int open_file(file_cache * fc, char* path, int filesize,unsigned char fill)
{
	memset(fc,0,sizeof(file_cache));

	if( filesize < 0 )
	{   // Read mode
		fc->f = fopen(path,"rb");
		if(fc->f)
		{
			if(fseek(fc->f,0,SEEK_END))
				goto error;

			fc->file_size = ftell(fc->f);

			if(fseek(fc->f,fc->current_offset,SEEK_SET))
				goto error;

			if(fc->current_offset + FILE_CACHE_SIZE > fc->file_size)
				fc->cur_page_size = ( fc->file_size - fc->current_offset);
			else
				fc->cur_page_size = FILE_CACHE_SIZE;

			memset(&fc->cache_buffer,fill,FILE_CACHE_SIZE);

			if(!fc->file_size)
				goto error;

			if( fread(&fc->cache_buffer,fc->cur_page_size,1,fc->f) != 1 )
				goto error;

			return 1;
		}
	}
	else
	{   // write mode
		fc->f = fopen(path,"wb");
		if(fc->f)
		{
			memset(&fc->cache_buffer,fill,FILE_CACHE_SIZE);

			while( fc->file_size < filesize)
			{

				if( fc->file_size + FILE_CACHE_SIZE > filesize )
				{
					if( fwrite( &fc->cache_buffer, filesize - fc->file_size, 1, fc->f ) != 1 )
						goto error;

					fc->file_size += (filesize - fc->file_size);
				}
				else
				{
					if( fwrite( &fc->cache_buffer, FILE_CACHE_SIZE, 1, fc->f ) != 1 )
						goto error;

					fc->file_size += FILE_CACHE_SIZE;
				}
			}

			fclose(fc->f);

			fc->current_offset = 0;

			fc->f = fopen(path,"r+b");
			if(fc->f)
			{
				if(fseek(fc->f,fc->current_offset,SEEK_END))
					goto error;

				fc->file_size = ftell(fc->f);

				if(fseek(fc->f,fc->current_offset,SEEK_SET))
					goto error;

				if(fc->current_offset + FILE_CACHE_SIZE > fc->file_size)
					fc->cur_page_size = ( fc->file_size - fc->current_offset);
				else
					fc->cur_page_size = FILE_CACHE_SIZE;

				if( fread(&fc->cache_buffer,fc->cur_page_size,1,fc->f) != 1 )
					goto error;

				if(fseek(fc->f,fc->current_offset,SEEK_SET))
					goto error;
			}
			else
			{
				goto error;
			}

			return 1;
		}
	}

	return 0;

error:
	if(fc->f)
		fclose(fc->f);

	fc->f = 0;

	return 0;
}

unsigned char get_byte(file_cache * fc,unsigned int offset, int * success)
{
	unsigned char byte;
	int ret;

	byte = 0xFF;
	ret = 1;

	if(fc)
	{
		if(offset < fc->file_size)
		{
			if( ( offset >= fc->current_offset ) &&
				( offset <  (fc->current_offset + FILE_CACHE_SIZE) ) )
			{
				byte = fc->cache_buffer[ offset - fc->current_offset ];
			}
			else
			{
				fc->current_offset = (offset & ~(FILE_CACHE_SIZE-1));
				if(fseek(fc->f, fc->current_offset,SEEK_SET))
				{
					if(success)
					{
						*success = 0;
					}

					return 0x00;
				}

				memset(&fc->cache_buffer,0xFF,FILE_CACHE_SIZE);

				if(fc->current_offset + FILE_CACHE_SIZE < fc->file_size)
					ret = fread(&fc->cache_buffer,FILE_CACHE_SIZE,1,fc->f);
				else
					ret = fread(&fc->cache_buffer,fc->file_size - fc->current_offset,1,fc->f);

				byte = fc->cache_buffer[ offset - fc->current_offset ];
			}
		}
	}

	if(success)
	{
		*success = ret;
	}

	return byte;
}

int set_byte(file_cache * fc,unsigned int offset, unsigned char byte)
{
	if(fc)
	{
		if(offset < fc->file_size)
		{
			if( ( offset >= fc->current_offset ) &&
				( offset <  (fc->current_offset + FILE_CACHE_SIZE) ) )
			{
				fc->cache_buffer[ offset - fc->current_offset ] = byte;
				fc->dirty = 1;
			}
			else
			{
				if( fc->dirty )
				{
					if(fseek(fc->f, fc->current_offset, SEEK_SET))
						goto error;

					if( fwrite( &fc->cache_buffer, fc->cur_page_size, 1, fc->f ) != 1 )
						goto error;

					fc->dirty = 0;
				}

				fc->current_offset = (offset & ~(FILE_CACHE_SIZE-1));
				if(fc->current_offset + FILE_CACHE_SIZE > fc->file_size)
					fc->cur_page_size = ( fc->file_size - fc->current_offset );
				else
					fc->cur_page_size = FILE_CACHE_SIZE;

				if(fseek(fc->f, fc->current_offset,SEEK_SET))
					goto error;

				memset(&fc->cache_buffer,0xFF,FILE_CACHE_SIZE);

				if( fread(&fc->cache_buffer,fc->cur_page_size,1,fc->f) != 1 )
					goto error;

				fc->cache_buffer[ offset - fc->current_offset ] = byte;

				fc->dirty = 1;
			}
		}
	}

	return 1;

error:
	return 0;
}

void close_file(file_cache * fc)
{
	if(fc)
	{
		if(fc->f)
		{
			if( fc->dirty )
			{
				if(fseek(fc->f, fc->current_offset, SEEK_SET))
					goto error;

				if( fwrite( &fc->cache_buffer, fc->cur_page_size, 1, fc->f ) != 1 )
					goto error;

				fc->dirty = 0;
			}

			fclose(fc->f);

			fc->f = NULL;
		}
	}

	return;

error:
	fclose(fc->f);

	return;
}


int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,512);
	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,512);

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':')
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] )
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

int main(int argc, char* argv[])
{
	file_cache in_file;
	file_cache * out_files;

	unsigned char byte;
	unsigned short checksum,total_checkum;
	unsigned short total_checkum_in;
	char filename[32];
	char temp[512];
	int i,j,k,m,success;
	int num_of_copy,num_of_bytes,num_of_banks,bank_word_size;
	unsigned int total_size;

	printf("rom_splitter V1.5\n(c)Jean-François DEL NERO / HxC2001\n\n");

	if(argc>=2)
	{
		num_of_bytes = 2;
		num_of_banks = 1;
		bank_word_size = 128*1024;
		num_of_copy = 1;

		if(isOption(argc,argv,"num_of_bytes",(char*)&temp)>0)
		{
			num_of_bytes = atoi(temp);
		}

		if(isOption(argc,argv,"num_of_banks",(char*)&temp)>0)
		{
			num_of_banks = atoi(temp);
		}

		if(isOption(argc,argv,"bank_word_size",(char*)&temp)>0)
		{
			bank_word_size = atoi(temp)*1024;
		}

		if(isOption(argc,argv,"mirror",(char*)&temp)>0)
		{
			num_of_copy = atoi(temp);
		}

		printf("Configuration :\n- %d byte(s) per word\n- %d bank(s)\n- %d words per bank\n- Total size: %d bytes\n- Duplication : %d\n\n",num_of_bytes,num_of_banks,bank_word_size,num_of_bytes*bank_word_size*num_of_banks,num_of_copy);

		printf("Opening %s ...\n",argv[1]);

		if( open_file(&in_file, argv[1],-1,0xFF) == 1 )
		{
			printf("File size : %d\n",in_file.file_size);

			out_files = malloc( sizeof(file_cache) * num_of_banks * num_of_bytes );
			if(out_files)
			{
				memset(out_files,0,sizeof(file_cache) * num_of_banks * num_of_bytes );
				for(j=0;j<num_of_banks;j++)
				{
					for(i=0;i<num_of_bytes;i++)
					{
						sprintf(filename,"BY%d_BK%d.ROM",i,j);
						if( open_file(&out_files[(num_of_bytes*j) + i], filename,bank_word_size*num_of_copy,0xFF) != 1 )
						{
							printf("ERROR : Can't create output rom...\n");
							goto f_error;
						}
					}
				}
			}
			else
				goto m_error;

			for(m=0;m<num_of_copy;m++)
			{
				for(j=0;j<num_of_banks;j++)
				{
					for(i=0;i<num_of_bytes;i++)
					{
						for(k=0;k<bank_word_size;k++)
						{
							byte = get_byte(&in_file, (j * bank_word_size * num_of_bytes) + ( (num_of_bytes*k) + i ), &success );
							if(success!=1)
								goto f_error;

							if(!set_byte(&out_files[(num_of_bytes*j) + i],k + (m * bank_word_size), byte))
								goto f_error;
						}
					}
				}
			}

			for(j=0;j<num_of_banks;j++)
			{
				for(i=0;i<num_of_bytes;i++)
				{
					close_file(&out_files[(num_of_bytes*j) + i]);
				}
			}

			total_checkum = 0x0000;
			total_size = 0;

			memset(out_files,0,sizeof(file_cache) * num_of_banks * num_of_bytes );
			for(j=0;j<num_of_banks;j++)
			{
				for(i=0;i<num_of_bytes;i++)
				{
					sprintf(filename,"BY%d_BK%d.ROM",i,j);
					if( open_file(&out_files[0], filename,-1,0xFF) == 1 )
					{
						total_size += out_files[0].file_size;

						printf("%s, Size : %d bytes, ",filename,out_files[0].file_size);

						checksum = 0x0000;
						for(k=0;k<out_files[0].file_size;k++)
						{
							checksum += get_byte(&out_files[0], k, &success );
							if(success!=1)
								goto f_error;
						}

						printf("Checksum : 0x%.4X\n",checksum);

						total_checkum += checksum;

						close_file(&out_files[0]);
					}
					else
					{
						printf("ERROR : Can't check output rom...\n");
						goto f_error;
					}
				}
			}

			printf("Total Checksum : 0x%.4X, Total size : %d\n",total_checkum,total_size);

			total_checkum_in = 0x0000;
			for(k=0;k<in_file.file_size;k++)
			{
				total_checkum_in += get_byte(&in_file, k, &success );
				if(success!=1)
					goto f_error;

			}

			printf("Input file Checksum : 0x%.4X, Size : %d\n",total_checkum_in,in_file.file_size);

			if(in_file.file_size >= total_size)
			{
				printf("Warning : %d byte(s) ignored !\n",in_file.file_size - total_size);
			}

			total_checkum_in = 0x0000;
			for(k=0;k<total_size;k++)
			{
				total_checkum_in += get_byte(&in_file, k, &success );
				if(success!=1)
					goto f_error;
			}

			printf("Padded Input file Checksum : 0x%.4X, Size : %d\n",total_checkum_in,total_size);

			close_file(&in_file);

			free(out_files);
		}
		else
		{
			printf( "Error while opening %s !!!\n", argv[1] );
		}
	}
	else
	{
		printf( "Syntax : rom_split ROMTOSPLIT.ROM [-num_of_bytes:x] [-num_of_banks:x] [-bank_word_size:x (in KB)] [-mirror:x]\n" );
		printf( "\nExample : Split a ROM file for a 16 Bits system with 2*32KB*3 EPROMs (192KB):\n          rom_split ROMTOSPLIT.ROM -num_of_bytes:2 -num_of_banks:3 -bank_word_size:32\n" );
	}

	return 0;

f_error:

	printf( "Fatal error : file access error !\n" );
	goto terminate;

m_error:

	printf( "Fatal error : memory allocation error !\n" );
	goto terminate;

terminate:
	close_file(&in_file);

	if(out_files)
	{
		for(j=0;j<num_of_banks;j++)
		{
			for(i=0;i<num_of_bytes;i++)
			{
				close_file(&out_files[(num_of_bytes*j) + i]);
			}
		}

		free(out_files);
	}

	return -1;
}

