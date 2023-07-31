///////////////////////////////////////////////////////////////////////////////////
// File : sat16_frame.c
// Contains: SAT16 relays board frame generator
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

typedef struct sat_16_rx_frame_state_
{
	unsigned char  full_frame[3 + 256 + 3];
	int            frame_size;

	unsigned char  address;
	unsigned char  data[256];
	unsigned int   data_size;
	unsigned short crc;
}sat_16_rx_frame_state;

int encode_sat16_frame(sat16ctrl_ctx * ctx,unsigned char address, unsigned char * data, int data_size, unsigned char * out_frame);

int decode_sat16_frame(sat16ctrl_ctx * ctx, sat_16_rx_frame_state * frame_state, unsigned char rx_byte );
