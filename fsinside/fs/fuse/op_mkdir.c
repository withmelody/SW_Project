#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_mkdir(const char *path, mode_t mode)
{
	tiny_inode		i_tmp, parent_inode;
	tiny_dentry		*d_tmp, *child_dentry;

	char *token;
	char *path_copy;
	char *dir_name;
	char *base_name;
	int i, j;
	int ret = 0;

	if ( strlen(path) > NAME_LEN_MAX - 1/*\0*/ ) {
		return -ENAMETOOLONG;
	}

	/* Get inode of the parent directory */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	dir_name = dirname(path_copy);
	token = strtok(dir_name, "/");
	ReadInode(&i_tmp, tiny_superblk.s_rdirino);
	memcpy(&parent_inode, &i_tmp, sizeof(tiny_inode));

	while (token) {
		memcpy(&parent_inode, &i_tmp, sizeof(tiny_inode));
		d_tmp = __find_dentry(&i_tmp, token);
		if (!d_tmp || d_tmp->type == FILE_TYPE_FILE) {
			ret = -ENOTDIR;
			goto err;
		}

		ReadInode(&i_tmp, d_tmp->inodeNum);
		token = strtok(NULL, "/");
	}

	/* Get dentry of the target */
	free(path_copy);
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	base_name = basename(path_copy);
	child_dentry = __find_dentry(&i_tmp, base_name);

	/* There is no such file */
	if (child_dentry == NULL) {
		// make a directory!!
		if ( MakeDirentry(&parent_inode, base_name) == WRONG_VALUE ) {
			return -EDQUOT;
		}
		goto err;
	} else {
		// already file exists
		ret = -EEXIST;
		goto err;
	}

err:
	free(child_dentry);
	free(path_copy);

	return ret;
}
