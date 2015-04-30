#include <errno.h>
#include "tinyfs.h"

int __find_inodeno_for_dir(tiny_inode *dir) {
	tiny_dirblk *dirblk;
	tiny_dentry *dentry;
	Buf *buf;
	int i, j;

	if ( dir->i_type != FILE_TYPE_DIR ) {
		return -1;
	}

	buf = BufRead(dir->i_block[0]);
	dirblk = __get_dirblk_from_buf(buf);
	return dirblk->dirEntries[0].inodeNum;
}

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

int tiny_release(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}

int tiny_releasedir(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}

int tiny_utimens(const char *path, const struct timespec tv[2])
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
