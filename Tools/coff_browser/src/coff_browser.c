///////////////////////////////////////////////////////////////////////////////////
// File : coff_browser.c
// Contains: coff file browser
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "coff_format.h"
#include "coff_access.h"

int main (int argc, char ** argv)
{
	int i;
	obj_state ** objects;

	printf("COFF Browser v0.1\n");
	printf("(c) 2022 Jean-François DEL NERO\n");

	if(argc>1)
	{
		objects = malloc( sizeof(obj_state *) * (argc - 1) );
		if(objects)
		{
			memset(objects,0,sizeof(obj_state *) * (argc - 1));

			i = 0;
			while(i<(argc-1))
			{
				printf("Loading %s...\n",argv[i+1]);
				objects[i] = loadobject(argv[i+1]);
				if(!objects[i])
				{
					printf("Error while loading %s...\n",argv[i+1]);
				}
				i++;
			}

			i = 0;
			while(i<(argc-1))
			{
				print_obj_stat(objects[i]);
				i++;
			}

			i = 0;
			while(i<(argc-1))
			{
				free_obj(objects[i]);
				i++;
			}

			free(objects);

			printf("Done\n");
		}
	}
	else
	{
		printf("Syntax : %s [files]\n",argv[0]);
	}

	exit(0);
}
