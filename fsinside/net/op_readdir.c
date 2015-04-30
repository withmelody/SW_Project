#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

#include "msglib/msglib.h"

extern FileSysInfo tiny_superblk;

int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *info)
{
	int qid = OpenMQ(5000);
	tiny_inode      return_inode;
	tiny_inode		i_tmp;
	tiny_dentry		*pDentry;
	tiny_dirblk		tmp_dirblk;
	char *token;
	char *path_copy;
	int ret = 0;
	int i, j;

	/* Get inode of the parent directory */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	token = strtok(path_copy, "/");
	ReadInode(&i_tmp, tiny_superblk.s_rdirino);
	//
	memcpy(&return_inode, &i_tmp, sizeof(tiny_inode));
	//
	while (token) {
		pDentry = __find_dentry(&i_tmp, token);
		if (!pDentry || pDentry->type == FILE_TYPE_FILE) {
			ret = -ENOTDIR;
			goto err;
		}

		ReadInode(&i_tmp, pDentry->inodeNum);
		token = strtok(NULL, "/");
	}

	for ( i = 0 ; i < i_tmp.i_nblk ; i++ ) {
		ReadDirBlock(&tmp_dirblk, i_tmp.i_block[i]);
		for ( j = 0 ; j < NUM_OF_DIRENT_IN_1BLK ; j++ ) {
			if ( strcmp(tmp_dirblk.dirEntries[j].name, "") != 0 ) {
				filler(buf, tmp_dirblk.dirEntries[j].name, NULL, 0);
			}
		}
	}

err:
	free(path_copy);


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
	memcpy(&fio.inode, &return_inode, sizeof(tiny_inode));
//	fio.dentry.inodeNum = target_inodeno;
	fio.flag = 'R';
	if(SendMQ(qid, MSG_FILEIO, &fio) < 0)
	{
		printf("fio send fail\n");
		return ;
	}


	return ret;
}
