///////////////////////////////////////////////////////////////////////////////////
// File : coff_access.h
// Contains: coff file browser
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

typedef struct _obj_state
{
	unsigned int obj_file_size;

	uint8_t * strings_buffer;
	uint32_t string_table_offset;
	uint32_t string_table_size;

	coff_file_header file_header;
	coff_symbol_table * symbols;
	coff_section_header * sections;

	char * file_path;
}obj_state;

int get_symbol_name(char * n_name, uint8_t * strings_buffer,int strings_buffer_size,char* str);
obj_state * loadobject(char * path);
void print_obj_stat(obj_state * obj);
void free_obj(obj_state * object);
