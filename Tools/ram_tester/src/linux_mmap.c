///////////////////////////////////////////////////////////////////////////////////
// File : linux_mmap.c
// Contains: Linux mmap functions.
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef WIN32

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "mmap.h"

int hw_mem_map(hw_mem_map_stat * state)
{
	size_t pagesize;
	off_t page_base;
	off_t page_offset;

	if( !state )
		return -1;

	printf("mmap 0x%.8lX (0x%.8lX) ...\n", state->mem_base, state->mem_size);

	state->fd = open("/dev/mem",  O_RDWR | O_SYNC);
	if( state->fd >= 0)
	{
		// Truncate offset to a multiple of the page size, or mmap will fail.
		pagesize = sysconf(_SC_PAGE_SIZE);

		printf("System page size : 0x%.8lX\n", pagesize);

		page_base = (state->mem_base / pagesize) * pagesize;
		page_offset = state->mem_base - page_base;

		printf("Page base : 0x%.8lX, Page offset : 0x%.8lX\n", page_base,page_offset);

		state->mapped_size = page_offset + state->mem_size;

		if( state->mapped_size & (pagesize-1) )
		{
			state->mapped_size = (state->mapped_size & ~(pagesize-1)) + pagesize;
		}

		state->mapped_mem = (void *)mmap(NULL, state->mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, state->fd, page_base);
		if(state->mapped_mem == MAP_FAILED)
		{
			state->mapped_size = 0;
			printf("ERROR : Can't mmap the target address !\n");
			close(state->fd);
			return -1;
		}

		state->memptr = state->mapped_mem + page_offset;

		return 1;
	}
	else
	{
		state->mapped_size = 0;
		printf("ERROR : Can't open /dev/mem !\n");
		return -1;
	}
}

void hw_mem_unmap(hw_mem_map_stat * state)
{
	if( state )
	{
		if( state->mapped_size )
		{
			munmap((void*)state->mapped_mem, state->mapped_size);
			close(state->fd);
		}
	}
}

#else

#include "mmap.h"

int hw_mem_map(hw_mem_map_stat * state)
{
	printf("ERROR : Currently unsupported !\n");

	return -1;
}

void hw_mem_unmap(hw_mem_map_stat * state)
{
	return;
}

#endif