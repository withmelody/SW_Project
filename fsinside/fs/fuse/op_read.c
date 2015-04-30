#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	int target_inodeno = (int)fi->fh;
	tiny_inode target_inode;
	Buf* pBuf;
	int blkno_start = offset / BLOCK_SIZE;
	int blkno_end = (offset + size) / BLOCK_SIZE;
	int nblk_read = blkno_end - blkno_start + 1;
	
	int offset_in_blk_start = offset % BLOCK_SIZE;
	int offset_in_blk_end = (offset + size) % BLOCK_SIZE;
	int ntoread = 0;
	int nread = 0;
	int nread_cur = 0;
	int i;

	ReadInode(&target_inode, target_inodeno);

	ntoread = target_inode.i_size - offset;
	ntoread = ntoread > size ? size : ntoread;
	if (ntoread < 0) {
		return -EIO;
	}

	//TODO: read first block
	pBuf = BufRead( target_inode.i_block[blkno_start] );
	nread_cur = BLOCK_SIZE - offset_in_blk_start;
	if (nread_cur > ntoread)
		nread_cur = ntoread;
	memcpy( buf, pBuf->pMem + offset_in_blk_start, nread_cur);
	ntoread -= nread_cur;
	nread += nread_cur;
	if (ntoread == 0)
		goto read_all;

	if (nblk_read > 1) {
		//TODO: read middle blocks
		for (i = blkno_start + 1; i < blkno_end; i++) {
			pBuf = BufRead( target_inode.i_block[i] );
			nread_cur = BLOCK_SIZE;
			if (nread_cur > ntoread)
				nread_cur = ntoread;
			memcpy( buf + nread, pBuf->pMem, nread_cur);
			ntoread -= nread_cur;
			nread += nread_cur;

			if (ntoread == 0)
				goto read_all;
		}

		//TODO: read last block
		pBuf = BufRead( target_inode.i_block[blkno_end] );
		nread_cur = offset_in_blk_end;
		if (nread_cur > ntoread)
			nread_cur = ntoread;
		memcpy( buf + nread, pBuf->pMem, nread_cur);
		ntoread -= nread_cur;
		nread += nread_cur;
	}

read_all:
	return nread;
}
