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

	int modified;

}obj_state;

int coff_get_str_symbol_name(char * n_name, uint8_t * strings_buffer,int strings_buffer_size,char* str);
int coff_set_str_symbol_name(char * n_name, uint8_t * strings_buffer,int strings_buffer_size,char* str);

obj_state * coff_load_obj(char * path);
void coff_print_obj_stat(obj_state * obj);

enum{
	SYMBOL_ALL_TYPE=0,
	SYMBOL_UNRESOLVED_EXT_SYMBOL_TYPE,
	SYMBOL_UNINITIALISED_GLOBAL_VARIABLE_TYPE,
	SYMBOL_INITIALISED_GLOBAL_VARIABLE_TYPE,
	SYMBOL_INITIALISED_STATIC_VARIABLE_TYPE,
	SYMBOL_UNINITIALISED_STATIC_VARIABLE_TYPE,
	SYMBOL_FUNCTION_ENTRYPOINT_TYPE,
	SYMBOL_SECTION_TYPE
};

int coff_get_next_symbol(obj_state * obj, int type, int index);
int coff_get_symbol_name(obj_state * obj, int index, char *name);
int coff_set_symbol_name(obj_state * obj, int index, char *name);

int coff_update_obj_file(obj_state * object);

void coff_free_obj(obj_state * object);
