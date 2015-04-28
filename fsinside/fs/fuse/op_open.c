#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

#define __get_dirblk_from_buf(buf)	\
	((tiny_dirblk*)((buf)->pMem))

tiny_dentry *__find_dentry(tiny_inode *dir, const char *entry_name)
{
	tiny_dirblk *dirblk;
	tiny_dentry *dentry;
	Buf *buf;
	int i, j;

	for (i = 0; i < dir->i_nblk; i++) {
		buf = BufRead(dir->i_block[i]);
		dirblk = __get_dirblk_from_buf(buf);

		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			dentry = &dirblk->dirEntries[j];
			if (strncmp(entry_name, dentry->name, NAME_LEN_MAX) == 0)
				return dentry;
		}
	}

	return NULL;
}

int tiny_open(const char *path, struct fuse_file_info *fi)
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
			//TODO: error - invalid path	
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

	/* Get inode of the target */
	target = (tiny_inode*)malloc(sizeof(tiny_inode));
	ReadInode(target, pDentry->inodeNum);
	fi->fh = (uint64_t)target;


	/*
	if (!pDentry) {
		if (fi->flags & O_CREAT) {
			//TODO: create a file
		} else {
			ret = ENOENT;
		}
	} else {
		if (fi->flags & O_CREAT) {
			if (fi->flags & O_EXCL) {
				ret = EEXIST;
			} else if (fi->flags & O_TRUNC) {
				//TODO: truncate the file
			} else {
				fi->fh = pDentry->inodeNum;
			}
		}
	}
	*/

	free(path_copy);

	return ret;
}
