///////////////////////////////////////////////////////////////////////////////////
// File : align_file_size.c
// Contains: Align a file size.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define WORK_BUFFER_SIZE 128*1024

int main (int argc, char ** argv)
{
	FILE * in_file;
	unsigned char * work_buffer;
	int file_size;
	int left_bytes;
	int cur_block_size;
	int ret;
	int final_size;
	int align_file_size;
	int fill_byte;

	in_file = NULL;
	work_buffer = NULL;
	ret = 0;

	if(argc>3)
	{
		align_file_size = (int)strtol(argv[2], NULL, 0);

		if( align_file_size < 1 )
		{
			ret = -1;
			printf("ERROR : Bad align value (%d) !\n",align_file_size);
			goto general_exit;
		}

		fill_byte = (int)strtol(argv[3], NULL, 0);

		work_buffer = malloc(WORK_BUFFER_SIZE);
		if(!work_buffer)
		{
			printf("ERROR : Allocation error !\n");
			goto general_exit;
		}

		in_file = fopen(argv[1],"a+");
		if(!in_file)
		{
			ret = -1;
			printf("ERROR : Can't open input file %s !\n",argv[1]);
			goto general_exit;
		}

		fseek(in_file,0,SEEK_END);
		file_size = ftell(in_file);
		fseek(in_file,0,SEEK_SET);

		if( !(file_size % align_file_size) )
		{
			ret = 0;
			printf("File size already aligned (%d bytes - %d bytes aligned)\nNothing to do.\n",file_size,align_file_size);
			goto general_exit;
		}

		left_bytes = align_file_size - ( ( file_size % align_file_size ) );

		final_size = file_size + left_bytes;

		memset(work_buffer,fill_byte,WORK_BUFFER_SIZE);

		while(left_bytes)
		{
			if(left_bytes >= WORK_BUFFER_SIZE)
				cur_block_size = WORK_BUFFER_SIZE;
			else
				cur_block_size = left_bytes;

			ret = fwrite(work_buffer,cur_block_size,1,in_file);
			if(ret != 1)
			{
				ret = -1;
				printf("ERROR : output file write error !\n");
				goto general_exit;
			}

			left_bytes -= cur_block_size;
		}

		printf("%d bytes added.\n", final_size - file_size );
	}
	else
	{
		printf("Syntax : %s [file] [align_file_size] [fill_byte]\n",argv[0]);
	}

general_exit:
	if(work_buffer)
		free(work_buffer);

	if(in_file)
		fclose(in_file);

	exit(ret);
}