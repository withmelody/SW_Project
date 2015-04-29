#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_truncate(const char *path, off_t size)
{
/*
 If  the file previously was larger than this size, 
 the extra data is lost.  If the file previously was shorter, 
 it is extended, and the extended part reads as null bytes ('\0').
*/
	if ( size > TINY_N_DIRECT_BLOCKS * BLOCK_SIZE ) {
		return -EFBIG;
	}

	return 0;
}
