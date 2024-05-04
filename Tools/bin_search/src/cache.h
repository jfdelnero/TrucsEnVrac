/*
//
// File cache / helper layer
//
// Copyright (C) 2022-2024 Jean-François DEL NERO
//
*/

#define FILE_CACHE_SIZE (64*1024)

typedef struct file_cache_
{
	FILE * f;
	int  current_offset;
	int  cur_page_size;
	int  file_size;
	unsigned char cache_buffer[FILE_CACHE_SIZE];
	int dirty;
}file_cache;

int open_file(file_cache * fc, char* path, int filesize,unsigned char fill);
unsigned char get_byte(file_cache * fc,int offset, int * success);
int16_t get_short(file_cache * fc,int offset, int * success);
uint16_t get_ushort(file_cache * fc,int offset, int * success);
int32_t get_long(file_cache * fc,int offset, int * success);
uint32_t get_ulong(file_cache * fc,int offset, int * success);
float get_float( file_cache * fc,int offset, int * success);
double get_double( file_cache * fc,int offset, int * success);

int set_byte(file_cache * fc,unsigned int offset, unsigned char byte);
int set_ushort(file_cache * fc,unsigned int offset, uint16_t data);
int set_ulong(file_cache * fc,unsigned int offset, uint32_t data);

void close_file(file_cache * fc);
