#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"
#include "msglib/msglib.h"
extern FileSysInfo tiny_superblk;

int tiny_getattr(const char *path, struct stat *stbuf)
{
	tiny_inode		i_tmp;
	tiny_inode		target_inode;
	tiny_dentry		*pDentry;
	char *token;
	char *path_copy;
	char *dir_name;
	char *base_name;
	int ino;
	int ret = 0;
	int qid = OpenMQ(5000);

	/* Get inode of the parent directory */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	dir_name = dirname(path_copy);
	token = strtok(dir_name, "/");
	ReadInode(&i_tmp, tiny_superblk.s_rdirino);
	while (token) {
		pDentry = __find_dentry(&i_tmp, token);
		if (!pDentry || pDentry->type == FILE_TYPE_FILE) {
			ret = -ENOTDIR;
			goto err;
		}


		ReadInode(&i_tmp, pDentry->inodeNum);
		token = strtok(NULL, "/");
	}

	/* Get dentry of the target */
	free(path_copy);
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	base_name = basename(path_copy);
	if (strlen(base_name) > NAME_LEN_MAX)
		base_name[NAME_LEN_MAX - 1] = '\0';
	pDentry = __find_dentry(&i_tmp, base_name);

	/* There is no such file */
	if (pDentry == NULL) {
		if (strcmp(path, "/") != 0) {
			ret = -ENOENT;
			goto err;
		}
		stbuf->st_mode = S_IFDIR | 0755;
	} else {
		ReadInode(&target_inode, pDentry->inodeNum);
		memset(stbuf, 0, sizeof(struct stat));
		switch (pDentry->type) {
			case FILE_TYPE_DIR:
				stbuf->st_mode = S_IFDIR | 0755;
				break;
			case FILE_TYPE_FILE:
				stbuf->st_mode = S_IFREG | 0664;
				stbuf->st_size = target_inode.i_size;
				break;
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
	memcpy(&fio.inode, &target_inode, sizeof(tiny_inode));
//	memcpy(&fio.dentry, pDentry, sizeof(tiny_dentry));
	if (pDentry)
		fio.dentry.inodeNum = pDentry->inodeNum;
	else
		fio.dentry.inodeNum = -1;
	fio.flag = 'a';
	if(SendMQ(qid, MSG_FILEIO, &fio) < 0)
	{
		printf("fio send fail\n");
		return ;
	}


	return ret;
}
