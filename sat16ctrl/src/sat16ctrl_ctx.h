///////////////////////////////////////////////////////////////////////////////////
// File : sat16ctrl_ctx.c
// Contains: SAT16 relay board frame sender software context
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#define MAX_STR_SIZE 512
#define DEFAULT_BUFLEN 512

typedef struct sat16ctrl_ctx_
{
	char com_port_str[MAX_STR_SIZE];
	char test_name[MAX_STR_SIZE];

	int verbose;

	void* envvar;

	unsigned char current_address;
	unsigned char current_data;

	FILE * serialport;
}sat16ctrl_ctx;
