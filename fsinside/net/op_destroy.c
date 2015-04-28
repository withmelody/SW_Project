#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

void tiny_destroy(void *user_data)
{
	int i = 0;
	Buf* pBuf = NULL;

	pBuf = BufRead(0);
	BufWrite(pBuf, &tiny_superblk, sizeof(tiny_superblk) - sizeof(char*) * 2);

	for ( i = tiny_superblk.s_ibitmap_start ; i < tiny_superblk.s_ibitmap_start + tiny_superblk.s_ibitmap_size ; i++ )
	{
		pBuf = BufRead(i);
		BufWrite(pBuf, tiny_superblk.s_ibitmap_ptr + (i - tiny_superblk.s_ibitmap_start) * BLOCK_SIZE, 512);
	}

	for ( i = tiny_superblk.s_dbitmap_start ; i < tiny_superblk.s_dbitmap_start + tiny_superblk.s_dbitmap_size ; i++ )
	{
		pBuf = BufRead(i);
		BufWrite(pBuf, tiny_superblk.s_dbitmap_ptr + (i - tiny_superblk.s_dbitmap_start) * BLOCK_SIZE, 512);
	}
	BufSync();
}
