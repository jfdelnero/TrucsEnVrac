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
	int i,symbol_index;
	obj_state ** objects;
	char tmp_string[1024];

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
				symbol_index = -1;

				do
				{
					symbol_index = get_next_symbol(objects[i], SYMBOL_FUNCTION_ENTRYPOINT_TYPE, symbol_index);
					if( symbol_index >= 0 )
					{
						if(!get_symbol_name(objects[i], symbol_index, (char *)tmp_string))
						{
							printf("Entry point : %s\n",tmp_string);
							#if 0
							if(!strcmp(tmp_string,"function_name_to_modify"))
							{
								printf("Modification\n");
								set_symbol_name(objects[i], symbol_index, "func_test_hxc");
							}
							#endif
						}
					}
				}while(symbol_index >= 0);

				i++;
			}

			i = 0;
			while(i<(argc-1))
			{
				update_obj_file(objects[i]);
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
