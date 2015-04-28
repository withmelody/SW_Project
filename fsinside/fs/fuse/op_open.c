#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

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

	if (strcmp(path, "/") == 0) {
		return 0;
	}

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
		ret = -ENOENT;
		goto err;
	}

	/* Path indicates a directory */
	if (pDentry->type == FILE_TYPE_DIR) {
		ret = 0;
		goto err;
	}

	/* Get inode of the target */
	target = (tiny_inode*)malloc(sizeof(tiny_inode));
	ReadInode(target, pDentry->inodeNum);
	fi->fh = (uint64_t)target;

err:
	free(path_copy);

	return ret;
}
