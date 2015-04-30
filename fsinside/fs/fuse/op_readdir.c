#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *info)
{
	tiny_inode		i_tmp;
	tiny_dentry		*pDentry;
	tiny_dirblk		tmp_dirblk;
	char *token;
	char *path_copy;
	int ret = 0;
	int i, j;


	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	token = strtok(path_copy, "/");
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
	return ret;
}
