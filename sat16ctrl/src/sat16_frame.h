///////////////////////////////////////////////////////////////////////////////////
// File : sat16_frame.c
// Contains: SAT16 relays board frame generator
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

int encode_sat16_frame(sat16ctrl_ctx * ctx,unsigned char address, unsigned char * data, int data_size, unsigned char * out_frame);