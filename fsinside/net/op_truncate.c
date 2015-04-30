#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

#include "msglib/msglib.h"

extern FileSysInfo tiny_superblk;

int tiny_truncate(const char *path, off_t size)
{
	int qid = OpenMQ(5000);
	tiny_inode		parent_inode;
	tiny_inode		target_inode;
	tiny_dentry		*pDentry;
	char *token;
	char *path_copy;
	char *dir_name;
	char *base_name;
	int nwrite = 0;
	int ret = 0;

	if (size > BLOCK_SIZE * TINY_N_DIRECT_BLOCKS) {
		return -EFBIG;
	}

	/* Get inode of the parent directory */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	dir_name = dirname(path_copy);
	token = strtok(dir_name, "/");
	ReadInode(&parent_inode, tiny_superblk.s_rdirino);
	while (token) {
		pDentry = __find_dentry(&parent_inode, token);
		if (!pDentry || pDentry->type == FILE_TYPE_FILE) {
			free(path_copy);
			return -ENOTDIR;
		}

		ReadInode(&parent_inode, pDentry->inodeNum);
		token = strtok(NULL, "/");
	}
	free(path_copy);

	/* Get dentry of the target */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	base_name = basename(path_copy);
	pDentry = __find_dentry(&parent_inode, base_name);
	free(path_copy);

	/* There is no such file */
	if (pDentry == NULL) {
		return -ENOENT;
	}

	ReadInode(&target_inode, pDentry->inodeNum);


	if (target_inode.i_size > size) {
		// shrink
		int offset_after;
		int blkidx_after;
		int i;

		offset_after = size % BLOCK_SIZE;
		blkidx_after = size / BLOCK_SIZE;
		if (offset_after) {
			blkidx_after++;
		}

		for (i = target_inode.i_nblk - 1; i > blkidx_after; i--) {
			SetBlockAllocToFree(target_inode.i_block[i]);
			target_inode.i_nblk--;
		}

		target_inode.i_size = size;
		WriteInode(&target_inode, pDentry->inodeNum);
	} else if (target_inode.i_size < size) {
		// extend
		Buf* pBuf;
		int blkidx_start;
		int offset_start;
		int blkidx_end;
		int offset_end;
		int start, end;
		int i;

		fprintf(stderr, "[TEST] i_size = %d\n", target_inode.i_size);
		offset_start = target_inode.i_size % BLOCK_SIZE;
		blkidx_start = target_inode.i_size / BLOCK_SIZE;
		offset_end = size % BLOCK_SIZE;
		if (offset_end == 0) {
			offset_end = BLOCK_SIZE;
			blkidx_end = size / BLOCK_SIZE - 1;
		} else {
			blkidx_end = size / BLOCK_SIZE;
		}

		size -= target_inode.i_size;
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
					ret = -1;
					goto finalize;
				}
				SetBlockFreeToAlloc();
				target_inode.i_nblk++;
			}

			pBuf = BufRead(target_inode.i_block[i]);
			memset(pBuf->pMem + start, 0, end - start);
			BufDelete(pBuf);
			BufInsert(pBuf, BUF_LIST_DIRTY);
			nwrite += end - start;
			size -= end - start;

			fprintf(stderr, "[TEST] end - start = %d\n", end - start);
			fprintf(stderr, "[TEST] nwrite = %d\n", nwrite);

			start = 0;
		}
	}

finalize:
	target_inode.i_size += nwrite;
	WriteInode(&target_inode, pDentry->inodeNum);

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
	fio.dentry.inodeNum =  pDentry->inodeNum;
	fio.flag = 'w';
	if(SendMQ(qid, MSG_FILEIO, &fio) < 0)
	{
		printf("fio send fail\n");
		return ;
	}

	return ret;
}
