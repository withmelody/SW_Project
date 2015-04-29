#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"
#include "fs.h"

extern FileSysInfo tiny_superblk;

static int __create_file(tiny_inode *parent_inode, const char *name)
{
	tiny_inode target;
	tiny_dirblk tmp_dirblk;
	int target_inodeno;
	int parent_inodeno;
	int i, j;

	// 1. get inode number
	target_inodeno = GetFreeInode();
	if ( SetInodeFreeToAlloc() == -1 ) {
		goto __create_file_success;
	}

	target.i_nblk = 0;
	target.i_size = 0;
	target.i_type = FILE_TYPE_FILE;

	// 2. write a inode entry into inode block
	WriteInode(&target, target_inodeno);

	// 3. find a empty place for dentry
	for ( i = 0 ; i < parent_inode->i_nblk ; i++ ) {
		ReadDirBlock(&tmp_dirblk, parent_inode->i_block[i]);
		for ( j = 0 ; j < NUM_OF_DIRENT_IN_1BLK ; j++ ) {
			if ( i == 0 && j == 0 ) {
				parent_inodeno = tmp_dirblk.dirEntries[0].inodeNum;
			}
			if ( strcmp(tmp_dirblk.dirEntries[j].name, "") == 0 ) {
				tmp_dirblk.dirEntries[j].inodeNum = target_inodeno;
				tmp_dirblk.dirEntries[j].type = FILE_TYPE_FILE;
				strncpy(tmp_dirblk.dirEntries[j].name, name, NAME_LEN_MAX - 1);
				tmp_dirblk.dirEntries[j].name[NAME_LEN_MAX - 1] = '\0';
				WriteDirBlock(&tmp_dirblk, parent_inode->i_block[i]);
				goto __create_file_success;
			}
		}
	}

	//TODO: allocate new i_block
	if ( parent_inode->i_nblk != TINY_N_DIRECT_BLOCKS ) {
		parent_inode->i_block[parent_inode->i_nblk++] = GetFreeBlock();
		SetBlockFreeToAlloc();

		ReadDirBlock(&tmp_dirblk, 
				parent_inode->i_block[parent_inode->i_nblk-1]);
		tmp_dirblk.dirEntries[0].inodeNum = target_inodeno;
		tmp_dirblk.dirEntries[0].type = FILE_TYPE_FILE;
		strncpy(tmp_dirblk.dirEntries[0].name, name, NAME_LEN_MAX - 1);
		tmp_dirblk.dirEntries[0].name[NAME_LEN_MAX - 1] = '\0';
		WriteDirBlock(&tmp_dirblk, 
				parent_inode->i_block[parent_inode->i_nblk-1]);
		WriteInode(parent_inode, parent_inodeno);

		goto __create_file_success;
	}

	// err: already using all i_block -> return NULL;
	// error occured!
__create_file_failed:
	SetInodeAllocToFree(target_inodeno);

__create_file_success:
	return target_inodeno;
}

int tiny_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	tiny_inode		i_tmp;
	int				target_inodeno;
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
	if (strlen(base_name) > NAME_LEN_MAX - 1) {
		ret = -ENAMETOOLONG;
		goto err;
	}
	pDentry = __find_dentry(&i_tmp, base_name);

	/* There is no such file */
	if (pDentry == NULL) {
		target_inodeno = __create_file(&i_tmp, base_name);
		if (target_inodeno < 0) {
			ret = -EDQUOT;
			goto err;
		}
		fi->fh = (uint64_t)target_inodeno;
	} else {
		ret = -EEXIST;
		goto err;
	}

err:
	free(path_copy);

	return ret;
}
