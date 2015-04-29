#include <errno.h>
#include "tinyfs.h"

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

/*
   -> op_getattr.c
int tiny_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	fprintf(stderr, "[TINYFS] %s\n", __func__);

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		stbuf->st_mode = S_IFREG | 0555;
		stbuf->st_nlink = 1;
	}

	return res;
}
*/

/*
   -> op_readdir.c
int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	return 0;
}
*/

int tiny_mkdir(const char *path, mode_t mode)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}

/*
   -> op_unlink.c
int tiny_unlink(const char *path)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

int tiny_rmdir(const char *path)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}

int tiny_rename(const char *before, const char *after)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}

/*
   -> op_truncate.c
int tiny_truncate(const char *path, off_t size)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

/*
   -> op_open.c
int tiny_open(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

/*
   -> op_read.c
int tiny_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

/*
   -> op_write.c
int tiny_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

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

/*
   -> op_init.c
void *tiny_init(struct fuse_conn_info *conn)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
 */

/*
   -> op_destroy.c
void tiny_destroy(void *user_data)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
}
*/

/*
   -> op_create.c
int tiny_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
*/

int tiny_utimens(const char *path, const struct timespec tv[2])
{
	fprintf(stderr, "[TINYFS] %s\n", __func__);
	return 0;
}
