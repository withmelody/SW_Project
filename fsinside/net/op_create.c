#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"
#include "fs.h"

extern FileSysInfo tiny_superblk;

static tiny_inode *__create_file(tiny_inode *parent_inode, const char *name)
{
	int i, j;
	tiny_inode* target = NULL;
	tiny_dirblk tmp_dirblk;
	unsigned int inode_no;

	// 1. get inode number
	inode_no = GetFreeInode();
	if ( SetInodeFreeToAlloc() == -1 ) {
		goto __create_file_finalize;
	}

	target = (tiny_inode*)calloc(sizeof(tiny_inode), 1);

	target->i_nblk = 0;
	target->i_size = 0;
	target->i_type = FILE_TYPE_FILE;

	// 2. write a inode entry into inode block
	WriteInode(target, inode_no);

	// 3. find a empty place for dentry
	for ( i = 0 ; i < parent_inode->i_nblk ; i++ ) {
		ReadDirBlock(&tmp_dirblk, parent_inode->i_block[i]);
		for ( j = 0 ; j < NUM_OF_DIRENT_IN_1BLK ; j++ ) {
			if ( strcmp(tmp_dirblk.dirEntries[j].name, "") == 0 ) {
				tmp_dirblk.dirEntries[j].inodeNum = inode_no;
				tmp_dirblk.dirEntries[j].type = FILE_TYPE_FILE;
				strncpy(tmp_dirblk.dirEntries[j].name, name, NAME_LEN_MAX);
				goto __create_file_finalize;
			}
		}
	}
	
	// error occured!
	free(target);
	target = NULL;
	SetInodeAllocToFree(inode_no);

__create_file_finalize:
	return target;
}

int tiny_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	tiny_inode		i_tmp;
	tiny_inode		*target;
	tiny_dentry		*pDentry;
	char *token;
	char *path_copy;
	char *dir_name;
	char *base_name;
	int ino;
	int ret = 0;

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
	pDentry = __find_dentry(&i_tmp, base_name);

	/* There is no such file */
	if (pDentry == NULL) {
		//TODO: create a new file
		target = __create_file(&i_tmp, base_name);
		if (!target) {
			ret = -EDQUOT;
			goto err;
		}
		fi->fh = (uint64_t)target;
	} else {
		ret = -EEXIST;
		goto err;
	}

err:
	free(path_copy);

	return ret;
}
