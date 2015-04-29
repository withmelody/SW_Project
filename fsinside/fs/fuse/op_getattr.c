#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

int tiny_getattr(const char *path, struct stat *stbuf)
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
		memset(stbuf, 0, sizeof(struct stat));
		switch (pDentry->type) {
		case FILE_TYPE_DIR:
			stbuf->st_mode = S_IFDIR | 0755;
			break;
		case FILE_TYPE_FILE:
			stbuf->st_mode = S_IFREG | 0664;
			break;
		}
	}

err:
	free(path_copy);

	return ret;
}
