///////////////////////////////////////////////////////////////////////////////////
// File : sat16ctrl.c
// Contains: SAT16 relay board frame sender software
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sat16ctrl_ctx.h"
#include "sat16_frame.h"

sat16ctrl_ctx app_ctx;

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,sizeof(option));

	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,sizeof(option));

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':' && ( j < (sizeof(option) - 1)) )
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						argtoparam[0] = 0;

						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] && j < (512 - 1) )
							{
								argtoparam[j]=argv[param][i];
								i++;
								j++;
							}
							argtoparam[j]=0;
							return 1;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						return 1;
					}
				}
			}
		}
		param++;
	}

	return 0;
}
int strtoValue(char* str_return)
{
	int value;

	value = 0;

	if (str_return)
	{
		if (strlen(str_return) > 2)
		{
			if (str_return[0] == '0' && (str_return[1] == 'x' || str_return[1] == 'X'))
			{
				value = (int)strtoul(str_return, NULL, 0);
			}
			else
			{
				value = atoi(str_return);
			}
		}
		else
		{
			value = atoi(str_return);
		}
	}

	return value;
}

void printhelp(sat16ctrl_ctx * app_ctx, char* argv[])
{
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "  -help \t\t\t: This help\n");
	fprintf(stdout, "  -comport:[comport]\t\t: Set the control serial port to use\n");
	fprintf(stdout, "  -address:[hex value]\t\t: Set the address\n");
	fprintf(stdout, "  -write:[hex value]\t\t: Write operation\n");
	fprintf(stdout, "\n");
}

int main(int argc, char* argv[])
{
	char tmpstr[MAX_STR_SIZE];
	unsigned char tmpstr2[256];
	unsigned char tx_frame[3 + 256 + 3];
	int i,frame_size;

	fprintf(stdout, "sat16ctrl v0.2\n");

	memset(&app_ctx, 0, sizeof(sat16ctrl_ctx));

	app_ctx.current_address = 0x20; // First network address is 0x20.

	// help option...
	if (isOption(argc, argv, "help", 0) > 0)
	{
		printhelp(&app_ctx,argv);
	}

	if (isOption(argc, argv, "comport", (char*)&app_ctx.com_port_str) > 0)
	{
		fprintf(stdout, "Serial port : %s\n", app_ctx.com_port_str);

		app_ctx.serialport = fopen(app_ctx.com_port_str,"wb");

		if(!app_ctx.serialport)
		{
			fprintf(stderr, "Serial port access error !\n");
			exit(-1);
		}
	}

	if (isOption(argc, argv, "address", (char*)&tmpstr) > 0)
	{
		app_ctx.current_address = strtoValue(tmpstr);
	}

	if (isOption(argc, argv, "write", (char*)&tmpstr) > 0)
	{
		char hex[3];

		i = 0;
		while(1)
		{
			hex[0] = tmpstr[i];
			hex[1] = tmpstr[i+1];
			hex[2] = 0;

			if(!hex[0])
				break;

			tmpstr2[i/2] = strtoul( (char*)&hex, NULL, 16 );

			i += 2;

			if(!hex[1])
				break;
		}

		fprintf(stdout, "Write at 0x%.8X\n", app_ctx.current_address);

		frame_size = encode_sat16_frame(&app_ctx, app_ctx.current_address, (unsigned char*)&tmpstr2,i/2, (unsigned char*)&tx_frame);

		if(frame_size > 0)
		{
			if(app_ctx.serialport)
			{
				fwrite(&tx_frame,frame_size,1,app_ctx.serialport);
				fflush(app_ctx.serialport);
			}
			else
			{
				printf("Frame:");
				for(i=0;i<frame_size;i++)
				{
					printf(" %.2X",tx_frame[i]);
				}
				printf("\n");
			}
		}
	}

	if ((isOption(argc, argv, "help", 0) <= 0) &&
		(isOption(argc, argv, "comport", 0) <= 0) &&
		(isOption(argc, argv, "address", 0) <= 0) &&
		(isOption(argc, argv, "write", 0) <= 0)
		)
	{
		printhelp(&app_ctx,argv);
	}

	if( app_ctx.serialport )
		fclose(app_ctx.serialport);

	return 0;
}
