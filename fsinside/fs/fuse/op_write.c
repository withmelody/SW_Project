#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	int target_inodeno = (int)fi->fh;
	tiny_inode target_inode;
	Buf* pBuf;
	int blkno_start = 0;
	int blkno_end = 0;
	int nblk_write = 0;
	int offset_in_blk_start = 0;
	int offset_in_blk_end = 0;
	int ntowrite = 0;
	int nwrite = 0;
	int nwrite_cur = 0;
	int i;

	ReadInode(&target_inode, target_inodeno);

	blkno_start = offset / BLOCK_SIZE;
	blkno_end = (offset + size) / BLOCK_SIZE;
	if (blkno_end > TINY_N_DIRECT_BLOCKS - 1) 
		blkno_end = TINY_N_DIRECT_BLOCKS - 1;
	nblk_write = blkno_end - blkno_start + 1;
	
	offset_in_blk_start = offset % BLOCK_SIZE;
	if (offset + size > BLOCK_SIZE * TINY_N_DIRECT_BLOCKS) {
		offset_in_blk_end = BLOCK_SIZE;
	} else {
		offset_in_blk_end = (offset + size) % BLOCK_SIZE;
	}

	ntowrite = BLOCK_SIZE * TINY_N_DIRECT_BLOCKS - offset;
	ntowrite = ntowrite > size ? size : ntowrite;
	if (ntowrite < 0) {
		return -EIO;
	}

	for (i = target_inode.i_nblk; i <= blkno_end ; i++) {
		target_inode.i_block[i] = GetFreeBlock();
		if (target_inode.i_block[i] < 0) {
			blkno_end = i - 1;
			break;
		}
		SetBlockFreeToAlloc();
		target_inode.i_nblk++;
	}

	//TODO: write first block
	pBuf = BufRead( target_inode.i_block[blkno_start] );
	nwrite_cur = BLOCK_SIZE - offset_in_blk_start;
	if (nwrite_cur > ntowrite)
		nwrite_cur = ntowrite;
	memcpy( pBuf->pMem + offset_in_blk_start, buf, nwrite_cur);
	BufDelete(pBuf);
	BufInsert(pBuf, BUF_LIST_DIRTY);
	ntowrite -= nwrite_cur;
	nwrite += nwrite_cur;
	if (ntowrite == 0)
		goto write_all;

	if (nblk_write > 1) {
		//TODO: write middle blocks
		for (i = blkno_start + 1; i < blkno_end; i++) {
			pBuf = BufRead( target_inode.i_block[i] );
			nwrite_cur = BLOCK_SIZE;
			if (nwrite_cur > ntowrite)
				nwrite_cur = ntowrite;
			memcpy( pBuf->pMem, buf + nwrite, nwrite_cur);
			BufDelete(pBuf);
			BufInsert(pBuf, BUF_LIST_DIRTY);
			ntowrite -= nwrite_cur;
			nwrite += nwrite_cur;
			if (ntowrite == 0)
				goto write_all;
		}

		//TODO: write last block
		pBuf = BufRead( target_inode.i_block[blkno_end] );
		nwrite_cur = offset_in_blk_end;
		if (nwrite_cur > ntowrite)
			nwrite_cur = ntowrite;
		memcpy( pBuf->pMem, buf + nwrite, nwrite_cur);
		BufDelete(pBuf);
		BufInsert(pBuf, BUF_LIST_DIRTY);
		ntowrite -= nwrite_cur;
		nwrite += nwrite_cur;
	}

write_all:
	if( offset + nwrite > target_inode.i_size ) {
		target_inode.i_size = offset + nwrite;
	}

	WriteInode(&target_inode, target_inodeno);

	return nwrite;
}
