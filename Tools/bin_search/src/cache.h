/*
//
// File cache / helper layer
//
// Copyright (C) 2022-2024 Jean-François DEL NERO
//
*/

#define FILE_CACHE_SIZE (64*1024)

#ifdef _OFFSET_64BITS_SUPPORT

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

typedef int64_t foffset_t;
typedef int64_t fsize_t;

#else

typedef int32_t foffset_t;
typedef int32_t fsize_t;

#endif

typedef struct file_cache_
{
	FILE * f;
	foffset_t  current_offset;
	foffset_t  cur_page_size;
	fsize_t    file_size;
	unsigned char cache_buffer[FILE_CACHE_SIZE];
	int dirty;
}file_cache;

int open_file(file_cache * fc, char* path, fsize_t filesize,unsigned char fill);
unsigned char get_byte(file_cache * fc, foffset_t offset, int * success);
int16_t get_short(file_cache * fc, foffset_t offset, int * success);
uint16_t get_ushort(file_cache * fc, foffset_t offset, int * success);
int32_t get_long(file_cache * fc, foffset_t offset, int * success);
uint32_t get_ulong(file_cache * fc, foffset_t offset, int * success);
float get_float( file_cache * fc, foffset_t offset, int * success);
double get_double( file_cache * fc, foffset_t offset, int * success);

int set_byte(file_cache * fc, foffset_t offset, unsigned char byte);
int set_ushort(file_cache * fc, foffset_t offset, uint16_t data);
int set_ulong(file_cache * fc, foffset_t offset, uint32_t data);

void close_file(file_cache * fc);
