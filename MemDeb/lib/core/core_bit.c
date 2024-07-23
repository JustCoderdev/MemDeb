/* Bit module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <stdio.h>
#include <stdlib.h>

void printb(n8 byte)
{
	printf("%c%c%c%c%c%c%c%c",
	       byte & 0x80 ? '1' : '0',
	       byte & 0x40 ? '1' : '0',
	       byte & 0x20 ? '1' : '0',
	       byte & 0x10 ? '1' : '0',
	       byte & 0x08 ? '1' : '0',
	       byte & 0x04 ? '1' : '0',
	       byte & 0x02 ? '1' : '0',
	       byte & 0x01 ? '1' : '0');
}

void printw(n32 word)
{
	printb(word >> 24);
	printb(word >> 16);
	printb(word >> 8);
	printb(word);
}

