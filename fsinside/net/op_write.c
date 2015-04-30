#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"
#include "msglib/msglib.h"

extern FileSysInfo tiny_superblk;

int tiny_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	int qid = OpenMQ(5000);
	int target_inodeno = (int)fi->fh;
	tiny_inode target_inode;
	Buf* pBuf;
	int blkidx_start;
	int offset_start;
	int blkidx_end;
	int offset_end;
	int nwrite = 0;
	int start, end;
	int i;


	if (offset + size > BLOCK_SIZE * TINY_N_DIRECT_BLOCKS) {
		return -EFBIG;
	}

	offset_start = offset % BLOCK_SIZE;
	blkidx_start = offset / BLOCK_SIZE;
	offset_end = (offset + size) % BLOCK_SIZE;
	if (offset_end == 0) {
		offset_end = BLOCK_SIZE;
		blkidx_end = (offset + size) / BLOCK_SIZE - 1;
	} else {
		blkidx_end = (offset + size) / BLOCK_SIZE;
	}

	ReadInode(&target_inode, target_inodeno);

	start = offset_start;
	for (i = blkidx_start; i <= blkidx_end; i++) {
		if (size > BLOCK_SIZE) {
			end = BLOCK_SIZE;
		} else {
			end = offset_end;
		}

		if (i >= target_inode.i_nblk) {
			target_inode.i_block[i] = GetFreeBlock();
			if (target_inode.i_block[i] < 0) {
				goto finalize;
			}
			SetBlockFreeToAlloc();
			target_inode.i_nblk++;
		}

		pBuf = BufRead(target_inode.i_block[i]);
		memcpy(pBuf->pMem + start, buf, end - start);
		BufDelete(pBuf);
		BufInsert(pBuf, BUF_LIST_DIRTY);
		nwrite += end - start;
		size -= end - start;

		if (target_inode.i_size < offset + nwrite) {
			target_inode.i_size = offset + nwrite;
		}

		start = 0;
	}

finalize:
	WriteInode(&target_inode, target_inodeno);
	if(qid < 0)
	{
		printf("q open fail\n");
		return ;
	}

	SuperBlk_t sb;
	sb.fsi = tiny_superblk;
	if(SendMQ(qid, MSG_SUPER_BLOCK, &sb) < 0)
	{
		printf("superblk send fail\n");
		return ;
	}

	InodeBitmap_t ibm;
	ibm.size = tiny_superblk.s_ninode / 8; /*byte*/
	memcpy(ibm.s_ibitmap_ptr, tiny_superblk.s_ibitmap_ptr, ibm.size);
	if(SendMQ(qid, MSG_INODE_BITMAP, &ibm) < 0)
	{
		printf("ibm send fail\n");
		return ;
	}

	BlockBitmap_t bbm;
	bbm.size = tiny_superblk.s_datablk_size / 8;  /*byte*/
	memcpy(bbm.s_dbitmap_ptr, tiny_superblk.s_dbitmap_ptr, bbm.size);
	if(SendMQ(qid, MSG_BLOCK_BITMAP, &bbm) < 0)
	{
		printf("bbm send fail\n");
		return ;
	}

	FileIO_t fio;
	memcpy(&fio.inode, &target_inode, sizeof(tiny_inode));
	fio.dentry.inodeNum = target_inodeno;
	fio.flag = 'w';
	fio.size = nwrite;
	if(SendMQ(qid, MSG_FILEIO, &fio) < 0)
	{
		printf("fio send fail\n");
		return ;
	}
	return nwrite;
}
