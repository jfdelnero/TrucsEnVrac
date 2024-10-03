///////////////////////////////////////////////////////////////////////////////////
// File : mmap.h
// Contains: memory map functions.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

typedef struct _hw_mem_map_stat
{
	int fd;

	off_t mem_base;
	off_t mem_size;

	off_t mapped_size;
	void * mapped_mem;

	volatile void * memptr;
}hw_mem_map_stat;

int  hw_mem_map(hw_mem_map_stat * state);
void hw_mem_unmap(hw_mem_map_stat * state);
