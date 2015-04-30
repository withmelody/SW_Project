#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

static int __find_dentry_and_remove(tiny_inode* out_inode, 
		tiny_inode *parent_inode, const char *entry_name)
{
	tiny_dirblk *dirblk;
	tiny_dentry *dentry;
	Buf *buf;
	int i, j;
	int isEmpty = 1;
	int find = 0;
	int dirblkno;
	int parent_inodeno;
	int target_inodeno;

	for (i = 0; i < parent_inode->i_nblk; i++) {
		buf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(buf);
		dirblkno = parent_inode->i_block[i];
		
		isEmpty = 1;
		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			dentry = &dirblk->dirEntries[j];

			if ( i == 0 && j == 0 ) {
				parent_inodeno = dentry[0].inodeNum;
			}

			if (strncmp(entry_name, dentry->name, NAME_LEN_MAX) == 0) {
				ReadInode(out_inode, dentry->inodeNum);
				strncpy(dentry->name, "", NAME_LEN_MAX);
				target_inodeno = dentry->inodeNum;

				find = 1;
			} else if (strncmp(dentry->name, "", NAME_LEN_MAX) != 0) {
				isEmpty = 0;
			}
		}

		if (find) {
			if (isEmpty) {
				SetBlockAllocToFree(dirblkno);
				parent_inode->i_block[i] =
					parent_inode->i_block[--parent_inode->i_nblk];
				WriteInode(parent_inode, parent_inodeno);
			} else {
				WriteDirBlock(dirblk, dirblkno);
			}
			return target_inodeno;
		}
	}

	return -1;
}

int tiny_unlink(const char *path)
{
	tiny_inode		i_tmp;
	tiny_inode		target;
	tiny_dentry		*pDentry;
	char *token;
	char *path_copy;
	char *dir_name;
	char *base_name;
	int target_inodeno;
	int ret = 0;
	int i, j;

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
	target_inodeno = __find_dentry_and_remove(&target, &i_tmp, base_name);
	if (target_inodeno < 0) {
		ret = -ENOENT;
		goto err;
	}

	for ( i = 0 ; i < target.i_nblk ; i++ ) {
		SetBlockAllocToFree(target.i_block[i]);
	}
	SetInodeAllocToFree(target_inodeno);

err:
	free(path_copy);

	return ret;
}
