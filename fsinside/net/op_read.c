#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"
#include "msglib/msglib.h"

extern FileSysInfo tiny_superblk;

int tiny_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	int qid = OpenMQ(5000);
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
	fio.flag = 'r';
	fio.size = nread;
	if(SendMQ(qid, MSG_FILEIO, &fio) < 0)
	{
		printf("fio send fail\n");
		return ;
	}
	return nread;
}
