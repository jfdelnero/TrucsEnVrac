///////////////////////////////////////////////////////////////////////////////////
// File : unzx0.c
// Contains: ZX0 unpacker
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _zx0_ctx
{
	int backtrack;
	unsigned char * in_ptr;
	unsigned char * out_ptr;
	unsigned char bit_mask;
	unsigned char last_byte;
	unsigned char last_byte_2;
}zx0_ctx;

int zx0_unpack(unsigned char * output,unsigned char * packed);

static unsigned char get_next_byte(zx0_ctx * ctx)
{
	ctx->last_byte = *ctx->in_ptr++;

	return ctx->last_byte;
}

static unsigned char get_next_bit(zx0_ctx * ctx)
{
	if (ctx->backtrack)
	{
		ctx->backtrack = 0;
		return ctx->last_byte & 1;
	}

	ctx->bit_mask >>= 1;

	if (ctx->bit_mask == 0)
	{
		ctx->bit_mask = 0x80;
		ctx->last_byte_2 = get_next_byte(ctx);
	}

	return ctx->last_byte_2 & ctx->bit_mask ? 1 : 0;
}

static void write_bytes(zx0_ctx * ctx, int offset, int len)
{
	unsigned char *src, *dst;

	dst = ctx->out_ptr;
	src = ctx->out_ptr - offset;

	ctx->out_ptr += len;

	while (len-- > 0)
	{
		*dst++ = *src++;
	}
}

static int read_interlaced_elias_gamma(zx0_ctx * ctx,int inverted)
{
	int value = 1;

	while (!get_next_bit(ctx))
	{
		value = value << 1 | (get_next_bit(ctx) ^ inverted);
	}

	return value;
}

static void copy_literals(zx0_ctx * ctx)
{
	int len;

	len = read_interlaced_elias_gamma(ctx,0);

	while (len-- > 0)
	{
		*ctx->out_ptr++ = *ctx->in_ptr++;
	}
}

static void copy_from_last_offset(zx0_ctx * ctx,int last_offset)
{
	int len;

	len = read_interlaced_elias_gamma(ctx,0);

	write_bytes(ctx, last_offset, len);
}

static int copy_from_new_offset(zx0_ctx * ctx)
{
	int len,offset;

	offset = read_interlaced_elias_gamma(ctx,1);
	if (offset == 256)
	{
		return -1;
	}

	offset = (offset<<7) - (get_next_byte(ctx)>>1);

	ctx->backtrack = 1;
	len = read_interlaced_elias_gamma(ctx,0)+1;

	write_bytes(ctx, offset, len);

	return offset;
}

//
// COPY_LITERALS
// if bit==1 -> COPY_FROM_NEW_OFFSET  -> if bit == 1 -> COPY_FROM_NEW_OFFSET else -> COPY_LITERALS
// else      -> COPY_FROM_LAST_OFFSET -> if bit == 1 -> COPY_FROM_NEW_OFFSET else -> COPY_LITERALS
//

int decompress(zx0_ctx * ctx, unsigned char * packed, unsigned char * output)
{
	int last_offset;
	unsigned char b;

	last_offset = 1; // Initial offset value;
	ctx->backtrack = 0;
	ctx->bit_mask = 0;
	ctx->in_ptr = packed;
	ctx->out_ptr = output;

	do
	{
		do
		{
			copy_literals(ctx);

			// Read next operation bit
			// 0 -> Copy from last offsets
			// 1 -> Copy from new offset
			b = get_next_bit(ctx);

			if( !b )
			{
				copy_from_last_offset(ctx, last_offset);

				// Read next operation bit
				// 0 -> Copy literals
				// 1 -> Copy from new offset
				b = get_next_bit(ctx);
			}

		} while ( !b );

		do
		{
			last_offset = copy_from_new_offset(ctx);
			if( last_offset < 0 )
			{
				return (ctx->out_ptr - output);
			}

			// Read next operation bit
			// 0 -> Copy literals
			// 1 -> Copy from new offset
			b = get_next_bit(ctx);
		}while( b );

	}while(1);
}

int main(int argc, char** argv)
{
	int in_size;
	int out_size;
	zx0_ctx ctx;
	unsigned char * out_buf;
	unsigned char * in_buf;
	FILE * f;
	FILE * f_out;

	f = NULL;
	f_out = NULL;
	in_buf = NULL;
	out_buf = NULL;
	out_size = 0;

	if( argc < 1 )
		goto error;

	f = fopen(argv[1],"rb");
	if(!f)
		goto error;

	fseek(f, 0, SEEK_END);
	in_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	f_out = fopen(argv[2],"wb");
	if(!f_out)
		goto error;

	in_buf = calloc(in_size,1);
	if(!in_buf)
		goto error;

	if( fread( in_buf, in_size, 1, f ) != 1 )
		goto error;

	out_buf = calloc(64*1024,1);
	if(!out_buf)
		goto error;

	memset(&ctx,0,sizeof(zx0_ctx));

//	for(int i=0;i<100000;i++)
		out_size = decompress(&ctx, in_buf, out_buf);


//	for(int i=0;i<100000;i++)
//		out_size = zx0_unpack(out_buf, in_buf);

	if( fwrite( out_buf, out_size, 1, f_out ) != 1 )
		goto error;

	printf("%d bytes extracted\n", out_size);

	free(in_buf);
	free(out_buf);

	fclose(f);

	fclose(f_out);

	exit(0);

	return 0;

error:
	fprintf(stderr,"Usage : %s in_file.zx0 out_file\n",argv[0]);

	if(f_out)
		fclose(f_out);

	if(f)
		fclose(f);

	free(in_buf);

	free(out_buf);

	exit(-1);

	return -1;
}
