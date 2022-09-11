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

int get_coff_symbol_name(char * n_name, uint8_t * strings_buffer,int strings_buffer_size,char* str)
{
	int i;
	int lessthan8;
	uint32_t stringoffset;

	str[0] = 0;

	lessthan8 = 0;
	for(i=0;i<4;i++)
	{
		if(n_name[i])
		{
			lessthan8 = 1;
		}
	}

	if(lessthan8)
	{
		i = 0;
		while(n_name[i] && i < 8)
		{
			str[i] = n_name[i];
			i++;
		}

		str[i] = 0;

		return 1;
	}

	stringoffset = 0;

	for(i=0;i<4;i++)
	{
		stringoffset |= (((uint32_t)(n_name[i+4]&0xFF)) << (i*8));
	}

	i = 0;
	while( ((stringoffset + i)  < strings_buffer_size) && strings_buffer[stringoffset + i] )
	{
		str[i] = strings_buffer[stringoffset + i];
		i++;
	}

	str[i] = 0;

	return 1;
}

int set_coff_symbol_name(char * n_name, uint8_t * strings_buffer,int strings_buffer_size,char* str)
{
	int i,maxsize;
	int lessthan8;
	uint32_t stringoffset;

	lessthan8 = 0;
	for(i=0;i<4;i++)
	{
		if(n_name[i])
		{
			lessthan8 = 1;
		}
	}

	if(lessthan8)
	{
		i = 0;
		while(str[i] && i < 8)
		{
			n_name[i] = str[i];
			i++;
		}

		if( i < 8)
			n_name[i] = 0;

		return 1;
	}

	stringoffset = 0;

	for(i=0;i<4;i++)
	{
		stringoffset |= (((uint32_t)(n_name[i+4]&0xFF)) << (i*8));
	}

	i = 0;
	maxsize = 0;
	while( ((stringoffset + i)  < strings_buffer_size) && strings_buffer[stringoffset + i] )
	{
		maxsize++;
		i++;
	}

	i = 0;
	while( ((stringoffset + i)  < (stringoffset+maxsize)) && str[i] )
	{
		strings_buffer[stringoffset + i] = str[i];
		i++;
	}

	strings_buffer[stringoffset + i] = 0;

	return 1;
}

obj_state * loadobject(char * path)
{
	int i;
	FILE * in_file;
	obj_state * object;

	object = NULL;

	in_file = fopen(path,"r");
	if(!in_file)
	{
		printf("ERROR : Can't open input file %s !\n",path);
		goto fatal_error;
	}

	object = malloc(sizeof(obj_state));
	if(object)
	{
		memset(object,0,sizeof(obj_state));

		object->file_path = malloc(strlen(path) + 1);
		if(!object->file_path)
			goto fatal_error;

		strcpy(object->file_path, path);

		fseek(in_file,0,SEEK_END);
		object->obj_file_size = ftell(in_file);
		fseek(in_file,0,SEEK_SET);

		if( fread(&object->file_header,sizeof(coff_file_header),1,in_file) != 1 )
		{
			printf("ERROR : Can't read input file %s !\n",path);
			goto fatal_error;
		}

		if(object->file_header.f_magic != 0x014C &&
		   object->file_header.f_magic != 0x8664
		)
		{
			printf("not a coff/pe-i386 file !\n");
			goto fatal_error;
		}

		object->string_table_offset = object->file_header.f_symptr + ( object->file_header.f_nsyms * sizeof( coff_symbol_table ) );

		if( ( ( object->string_table_offset + sizeof(uint32_t) ) >= object->obj_file_size ) || ( object->string_table_offset < sizeof(coff_file_header) ) )
		{
			printf("invalid coff/pe-i386 file !\n");
			goto fatal_error;
		}

		fseek(in_file,object->string_table_offset,SEEK_SET);
		if( fread(&object->string_table_size,sizeof(uint32_t),1,in_file) != 1)
		{
			printf("string buffer size loading error !\n");
			goto fatal_error;
		}

		if( ( ( object->string_table_offset + object->string_table_size ) > object->obj_file_size ) || ( object->string_table_offset < sizeof(coff_file_header) ) )
		{
			printf("invalid coff/pe-i386 file (string table)!\n");
			goto fatal_error;
		}

		object->strings_buffer = malloc(object->string_table_size);
		if(!object->strings_buffer)
		{
			printf("string buffer alloc error !\n");
			goto fatal_error;
		}

		memset(object->strings_buffer,0,object->string_table_size);
		fseek(in_file,object->string_table_offset,SEEK_SET);

		if( fread(object->strings_buffer,object->string_table_size,1,in_file) != 1 )
		{
			printf("string buffer loading error !\n");
			goto fatal_error;
		}

		// Loading sections

		object->sections = malloc( sizeof( coff_section_header ) * object->file_header.f_nscns );
		if(!object->sections)
		{
			printf("sections array alloc error !\n");
			goto fatal_error;
		}

		fseek(in_file,sizeof(coff_file_header) + object->file_header.f_opthdr,SEEK_SET);

		for(i=0;i<object->file_header.f_nscns;i++)
		{
			if( fread(&object->sections[i],sizeof(coff_section_header),1,in_file) != 1)
			{
				printf("section loading error ! (%d)\n",i);
				goto fatal_error;
			}
		}

		// Loading symbols

		object->symbols = malloc( sizeof( coff_symbol_table ) * object->file_header.f_nsyms );
		if(!object->symbols)
		{
			printf("symbols array alloc error !\n");
			goto fatal_error;
		}

		fseek(in_file,object->file_header.f_symptr,SEEK_SET);

		for(i=0;i<object->file_header.f_nsyms;i++)
		{
			if( fread(&object->symbols[i],sizeof(coff_symbol_table),1,in_file) != 1)
			{
				printf("symbol loading error ! (%d)\n",i);
				goto fatal_error;
			}
		}
	}

	fclose(in_file);

	return object;

fatal_error:

	if(in_file)
		fclose(in_file);

	if(object)
	{
		if(object->file_path)
			free(object->file_path);

		if(object->strings_buffer)
			free(object->strings_buffer);

		if(object->symbols)
			free(object->symbols);

		if(object->sections)
			free(object->sections);

		free(object);
	}

	return NULL;
}

void print_obj_stat(obj_state * obj)
{
	int auxcnt;
	int i,j;
	char tmp_string[1024];

	if(obj)
	{
		printf("f_magic : %.4X\n",obj->file_header.f_magic);
		printf("f_nscns : %.4X\n",obj->file_header.f_nscns);
		printf("f_timdat : %.8X\n",obj->file_header.f_timdat);
		printf("f_symptr : %.8X\n",obj->file_header.f_symptr);
		printf("f_nsyms : %.8X\n",obj->file_header.f_nsyms);
		printf("f_opthdr : %.4X\n",obj->file_header.f_opthdr);
		printf("f_flags : %.4X\n",obj->file_header.f_flags);
		printf("Strings table offsets : 0x%x\n",obj->string_table_offset);
		printf("Strings table size : 0x%x\n",obj->string_table_size);

		printf("Section table :\n");
		for(i=0;i<obj->file_header.f_nscns;i++)
		{
			printf("-------------\n");

			printf("Section n°%d : ",i + 1);

			get_coff_symbol_name((char*)&obj->sections[i].s_name, obj->strings_buffer,obj->string_table_size,(char*)&tmp_string);

			printf("%s\n",tmp_string);
			printf("Physical Address : 0x%X\n",obj->sections[i].s_paddr);
			printf("Virtual Address :  0x%X\n",obj->sections[i].s_vaddr);
			printf("Section Size in Bytes : %d\n",obj->sections[i].s_size);
			printf("Section file offset : 0x%X\n",obj->sections[i].s_scnptr);
			printf("Reloc table file offset : 0x%X\n",obj->sections[i].s_relptr);
			printf("Reloc table entries : %d\n",obj->sections[i].s_nreloc);
			printf("Line number table file offset : 0x%X\n",obj->sections[i].s_lnnoptr);
			printf("Line number table entries : 0x%X\n",obj->sections[i].s_nlnno);
			printf("Flags for this section : 0x%.4X\n",obj->sections[i].s_flags);

			printf("-------------\n");
		}

		printf("Symbol table :\n");
		auxcnt = 0;
		for(i=0;i<obj->file_header.f_nsyms;i++)
		{
			if(!auxcnt)
			{
				printf("-------------\n");

				printf("Symbol n°%d : ",i);

				for(j=0;j<8;j++)
					printf("%.2X",obj->symbols[i].n_name[j]);

				printf(" - ");

				get_coff_symbol_name((char*)&obj->symbols[i].n_name, obj->strings_buffer,obj->string_table_size,(char*)&tmp_string);

				printf("%s\n",tmp_string);
				printf("n_value : 0x%X\n",obj->symbols[i].n_value);
				printf("n_scnum : %d",obj->symbols[i].n_scnum);
				if(obj->symbols[i].n_scnum && (obj->symbols[i].n_scnum < obj->file_header.f_nscns))
				{
					get_coff_symbol_name((char*)&obj->sections[obj->symbols[i].n_scnum-1].s_name, obj->strings_buffer,obj->string_table_size,(char*)&tmp_string);

					printf(" - %s\n",tmp_string);
				}
				else
				{
					printf("\n");
				}

				printf("n_type  : 0x%X\n",obj->symbols[i].n_type);
				printf("n_sclass: 0x%X\n",obj->symbols[i].n_sclass);
				printf("n_numaux: 0x%X\n",obj->symbols[i].n_numaux);

				switch( obj->symbols[i].n_sclass )
				{
					case 2:
						if(!obj->symbols[i].n_scnum)
						{
							if(!obj->symbols[i].n_value)
								printf("Type: Unresolved external Symbol\n");
							else
								printf("Type: Uninitialised global variable (not included in BSS)\n");
						}
						else
						{
							get_coff_symbol_name((char*)&obj->sections[obj->symbols[i].n_scnum-1].s_name, obj->strings_buffer,obj->string_table_size,(char*)&tmp_string);
							if(!strcmp(tmp_string,".text"))
								printf("Type: Function entry point\n");

							if(!strcmp(tmp_string,".data"))
								printf("Type: Initialised global variable\n");
						}

					break;

					case 3:
						if(!obj->symbols[i].n_value)
						{
							if(
								!strcmp(tmp_string,".text") ||
								!strcmp(tmp_string,".data") ||
								!strcmp(tmp_string,".rdata") ||
								!strcmp(tmp_string,".xdata") ||
								!strcmp(tmp_string,".pdata") ||
								!strcmp(tmp_string,".bss") )
							{
								printf("Type: Section Symbol indicating start of Section\n");
							}
						}
						else
						{
							if(
								!strcmp(tmp_string,".data") ||
								!strcmp(tmp_string,".rdata") ||
								!strcmp(tmp_string,".xdata") ||
								!strcmp(tmp_string,".pdata")
							)
							{
								printf("Type: Initialised static variable\n");
							}

							if( !strcmp(tmp_string,".bss") )
							{
								printf("Type: Unitialised static variable\n");
							}
						}

					break;
				}

				printf("-------------\n");
			}
			else
			{
				auxcnt--;
			}

			if(!auxcnt)
				auxcnt = obj->symbols[i].n_numaux;
		}
	}
}

int get_next_symbol(obj_state * obj, int type, int index)
{
	int i;
	int auxcnt,ltype;
	char tmp_string[1024];

	auxcnt = 0;

	if(!obj)
		return -1;

	if(index >= obj->file_header.f_nsyms)
		return -1;

	if(index < 0)
	{
		index = 0;
	}
	else
	{
		auxcnt = obj->symbols[index].n_numaux;
		if(!auxcnt)
			index++;

		if(index >= obj->file_header.f_nsyms)
			return -1;
	}

	for(i=index;i<obj->file_header.f_nsyms;i++)
	{
		if(!auxcnt)
		{
			ltype = -1;

			switch( obj->symbols[i].n_sclass )
			{
				case 2:
					if(!obj->symbols[i].n_scnum)
					{
						if(!obj->symbols[i].n_value)
							ltype = SYMBOL_UNRESOLVED_EXT_SYMBOL_TYPE;
						else
							ltype = SYMBOL_UNINITIALISED_GLOBAL_VARIABLE_TYPE;
					}
					else
					{
						get_coff_symbol_name((char*)&obj->sections[obj->symbols[i].n_scnum-1].s_name, obj->strings_buffer,obj->string_table_size,(char*)&tmp_string);
						if(!strcmp(tmp_string,".text"))
							ltype = SYMBOL_FUNCTION_ENTRYPOINT_TYPE;

						if(!strcmp(tmp_string,".data"))
							ltype = SYMBOL_INITIALISED_GLOBAL_VARIABLE_TYPE;
					}

				break;

				case 3:
					if(!obj->symbols[i].n_value)
					{
						if(
							!strcmp(tmp_string,".text") ||
							!strcmp(tmp_string,".data") ||
							!strcmp(tmp_string,".rdata") ||
							!strcmp(tmp_string,".xdata") ||
							!strcmp(tmp_string,".pdata") ||
							!strcmp(tmp_string,".bss") )
						{
							ltype = SYMBOL_SECTION_TYPE;
						}
					}
					else
					{
						if(
							!strcmp(tmp_string,".data") ||
							!strcmp(tmp_string,".rdata") ||
							!strcmp(tmp_string,".xdata") ||
							!strcmp(tmp_string,".pdata")
						)
						{
							ltype = SYMBOL_INITIALISED_STATIC_VARIABLE_TYPE;
						}

						if( !strcmp(tmp_string,".bss") )
						{
							ltype = SYMBOL_UNINITIALISED_STATIC_VARIABLE_TYPE;
						}
					}

				break;
			}

			if( ltype == type || (type == SYMBOL_ALL_TYPE))
			{
				return i;
			}
		}
		else
		{
			auxcnt--;
		}

		if(!auxcnt)
			auxcnt = obj->symbols[i].n_numaux;
	}

	return -1;

}

int get_symbol_name(obj_state * obj, int index, char *name)
{
	if(index >= obj->file_header.f_nsyms)
		return -1;

	get_coff_symbol_name((char*)&obj->symbols[index].n_name, obj->strings_buffer,obj->string_table_size,name);

	return 0;
}

int set_symbol_name(obj_state * obj, int index, char *name)
{
	if(index >= obj->file_header.f_nsyms)
		return -1;

	set_coff_symbol_name((char*)&obj->symbols[index].n_name, obj->strings_buffer,obj->string_table_size,name);

	obj->modified = 1;

	return 0;
}

int update_obj_file(obj_state * object)
{
	FILE * out_file;

	out_file = NULL;

	if(object)
	{
		if(object->modified)
		{
			printf("Updating %s ...\n",object->file_path);

			out_file = fopen(object->file_path,"r+b");
			if(!out_file)
			{
				printf("ERROR : Can't open output file %s !\n",object->file_path);
				goto fatal_error;
			}

			if(fseek(out_file,object->file_header.f_symptr,SEEK_SET))
			{
				printf("ERROR : Error while seeking file %s !\n",object->file_path);
				goto fatal_error;
			}

			if(fwrite(object->symbols,sizeof(coff_symbol_table) * object->file_header.f_nsyms,1,out_file) != 1)
			{
				printf("ERROR : Error while writing file %s !\n",object->file_path);
				goto fatal_error;
			}

			if(fseek(out_file,object->string_table_offset,SEEK_SET))
			{
				printf("ERROR : Error while seeking file %s !\n",object->file_path);
				goto fatal_error;
			}

			if(fwrite(object->strings_buffer,object->string_table_size,1,out_file) != 1)
			{
				printf("ERROR : Error while writing file %s !\n",object->file_path);
				goto fatal_error;
			}

			fclose(out_file);

			return 1;
		}
		return 0;
	}

fatal_error:

	if(out_file)
		fclose(out_file);

	return -1;
}

void free_obj(obj_state * object)
{
	if(object)
	{
		if(object->file_path)
			free(object->file_path);

		if(object->strings_buffer)
			free(object->strings_buffer);

		if(object->symbols)
			free(object->symbols);

		if(object->sections)
			free(object->sections);

		free(object);
	}
}

