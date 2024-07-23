/* Buffer module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* Return the index of the chr or -1 */
i64 buffer_find_chr(char chr, char *buffer, n64 len) {
	n64 i;

	for(i = 0; i < len; ++i)
	{
		if(buffer[i] == chr) return i;
	}

	return -1;
}

/* Return the index of the first char that is not a delimiter or -1 */
i64 buffer_skip_str(char *del, n64 del_len, char *buffer, n64 buff_len) {
	(void)del, del_len, buffer, buff_len;
	assert(0 && "NOT IMPLEMENTED buffer_skip_str");
	return -1;
}

n64 buffer_copy_until_chr(char delimiter,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len)
{
	n64  i, min_len = min(src_len, dest_len);
	char curr;

	for(i = 0; i < min_len - 1; ++i)
	{
		curr = src_buffer[i];
		if(curr == delimiter)
		{
			dest_buffer[i] = '\0';
			return i;
		}

		dest_buffer[i] = curr;
	}

	return 0;
}

n64 buffer_copy_until_str(char *delimiter, n64 del_len,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len)
{
	n64   i, j, min_len = min(src_len, dest_len);
	char *curr;

	for(i = 0; i < min_len - 1; ++i)
	{
		bool match = true;
		curr = src_buffer + i;

		if(src_len - i < del_len) return 0;

		/* strcmp */
		for(j = 0; j < del_len; ++j)
		{
			if(curr[j] != delimiter[j])
			{
				match = false;
				break;
			}
		}

		if(match)
		{
			dest_buffer[i] = '\0';
			return i;
		}

		dest_buffer[i] = *curr;
	}

	return 0;
}

void savebuff(FILE *file, char *buffer, n64 buff_len)
{
	n64 i;
	for(i = 0; i < buff_len; ++i)
	{
		switch(buffer[i])
		{
#if 0
			case '\r':
				putc('\\', file);
				putc('r', file);
				break;

			case '\n':
				putc('\\', file);
				putc('n', file);
				putc('\n', file);
				break;
#endif
			/* TODO: Check for errors */
			default: putc(buffer[i], file);
		}
	}
	fflush(file);
}

