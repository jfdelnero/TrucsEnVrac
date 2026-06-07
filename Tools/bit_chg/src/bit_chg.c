///////////////////////////////////////////////////////////////////////////////////
// File : bit_chg.c
// Contains: File bit modifier.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main (int argc, char ** argv)
{
	FILE * in_file;
	int file_size;
	uint8_t byte;
	int byte_idx;
	int bit_idx;

	in_file = NULL;

	if(argc>3)
	{
		in_file = fopen(argv[1],"r+b");
		if(!in_file)
		{
			printf("ERROR : Can't open input file %s !\n",argv[1]);
			exit(-1);
		}

		byte_idx = atoi(argv[2]);
		bit_idx = atoi(argv[3]);

		fseek(in_file,0,SEEK_END);
		file_size = ftell(in_file);
		fseek(in_file,0,SEEK_SET);

		if( ( byte_idx >= file_size ) || ( bit_idx > 7 ))
		{
			fclose(in_file);
			exit(1);
		}

		fseek(in_file,byte_idx,SEEK_SET);

		if( fread(&byte,1,1,in_file) != 1 )
		{
			fclose(in_file);
			exit(-1);
		}

		if(argv[4])
		{
			if(atoi(argv[4]))
			{
				printf("Set byte %d bit %d...\n",byte_idx,bit_idx);
				byte = byte | (0x01<<bit_idx);
			}
			else
			{
				printf("Clear byte %d bit %d...\n",byte_idx,bit_idx);
				byte = byte & (~(0x01<<bit_idx));
			}
		}
		else
		{
			printf("Toggle byte %d bit %d...\n",byte_idx,bit_idx);
			byte = byte ^ (0x01<<bit_idx);
		}

		fseek(in_file,byte_idx,SEEK_SET);

		if( fwrite(&byte,1,1,in_file) != 1 )
		{
			fclose(in_file);
			exit(-1);
		}

		fclose(in_file);

		exit(0);
	}
	else
	{
		printf("Syntax : %s in_file byte bit [0/1]\n",argv[0]);
	}
}
