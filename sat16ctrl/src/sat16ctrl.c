///////////////////////////////////////////////////////////////////////////////////
// File : sat16ctrl.c
// Contains: SAT16 relay board frame sender software
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////
#ifndef ANSI_FILE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef ANSI_FILE
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#endif

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

#ifdef ANSI_FILE
int fread_to(FILE * file, unsigned char * buf)
#else
int fread_to(int file, unsigned char * buf)
#endif
{
	fd_set rfds;
	struct timeval tv;
	int ret, fd;

#ifdef ANSI_FILE
	fd = fileno(file);
#else
	fd = file;
#endif
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	ret = select(fd+1, &rfds, NULL, NULL, &tv);

	if(ret == 0)
	{
		fprintf(stderr, "Poll timed out !\n");
		return -1;
	}
	else if (ret < 0)
	{
		return -2;
	}
	else if(FD_ISSET(fd, &rfds))
	{
#ifdef ANSI_FILE
		ret = fread(buf, 1, 1, file);
#else
		ret = read(file, buf, 1);
#endif

		return ret;
	}

	return 0;
}

#ifdef ANSI_FILE
FILE* open_and_cfg_serial(char *devpath)
#else
int open_and_cfg_serial(char *devpath)
#endif
{

#ifdef ANSI_FILE
	FILE * port;

	port = fopen(devpath,"wb");

#else
	struct termios options;
	int port,ret;

	port = open(app_ctx.com_port_str, O_RDWR | O_NOCTTY);
	if(port == -1)
		return -1;

	usleep(10000);

	tcflush(port, TCIOFLUSH);

	ret = tcgetattr(port, &options);
	if (ret)
	{
		close(port);
		return -1;
	}

	//Disable these flags :
	//INLCR: Translate NL to CR on input.
	//IGNCR: Ignore carriage return on input.
	//ICRNL: Translate carriage return to newline on input (unless IGNCR is set).
	//IXON:  Enable XON/XOFF flow control on output.
	//IXOFF: Enable XON/XOFF flow control on input.
	options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);

	//Disable these flags :
	//ONLCR: (XSI) Map NL to CR-NL on output.
	//OCRNL: Map CR to NL on output.
	//ONOCR: Don't output CR at column 0.
	options.c_oflag &= ~(ONLCR | OCRNL | ONOCR);

	//Disable these flags :
	//ECHO:   Echo input characters.
	//ECHONL: If ICANON is also set, echo the NL character even if ECHO is not set.
	//ICANON: Enable canonical mode (described below).
	//ISIG:   When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding signal.
	//IEXTEN: Enable implementation-defined input processing.
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	// 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	options.c_cc[VTIME] = 7;
	options.c_cc[VMIN] = 0;

	cfsetospeed(&options, B4800);
	cfsetispeed(&options, cfgetospeed(&options));

	ret = tcsetattr(port, TCSANOW, &options);
	if (ret)
	{
		close(port);
		return -1;
	}

#endif

	return port;
}

int main(int argc, char* argv[])
{
	char tmpstr[MAX_STR_SIZE];
	unsigned char tmpstr2[256];
	unsigned char tx_frame[3 + 256 + 3];
	int i,frame_size,ret;
#ifdef WAITACK
	unsigned char byte;
	sat_16_rx_frame_state rx_frame;
#endif
	ret = 0;

	fprintf(stdout, "sat16ctrl v0.4\n");

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

		app_ctx.serialport = open_and_cfg_serial(app_ctx.com_port_str);

		if(!app_ctx.serialport || app_ctx.serialport < 0)
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
			if(app_ctx.serialport > 0)
			{
				ret = 0;
#ifdef ANSI_FILE
				fwrite(&tx_frame,frame_size,1,app_ctx.serialport);
				fflush(app_ctx.serialport);
#else
				if ( write(app_ctx.serialport, &tx_frame, frame_size) != frame_size )
				{
					ret = -5;
				}
#endif
#ifdef WAITACK
				ret = -4;
				rx_frame.frame_size = 0;
				while( fread_to(app_ctx.serialport, &byte) > 0 )
				{
					ret = decode_sat16_frame(&app_ctx, &rx_frame, byte );
					if(ret == 1)
					{
						break;
					}

					if( ret < 0 )
					{
						break;
					}
				}

				if( ret == 1 )
				{
					fprintf(stderr, "Ack frame received !\n");
					ret = 0;
				}
				else
				{
					fprintf(stderr, "Ack frame error (%d) !\n",ret);
					if(!ret)
						ret = -6;
				}

				printf("Received Frame : ");
				for(i=0;i<rx_frame.frame_size;i++)
				{
					printf("%.2X ",rx_frame.full_frame[i]);
				}
				printf("\n");
#endif
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
	{
#ifdef ANSI_FILE
		fclose(app_ctx.serialport);
#else
		close(app_ctx.serialport);
#endif
	}

	return ret;
}
