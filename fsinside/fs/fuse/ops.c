#include <errno.h>
#include "tinyfs.h"

int tiny_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	fprintf(stderr, "[TINYFS] %s", __func__);

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else
		res = -ENOENT;

	return res;
}

int tiny_mkdir(const char *path, mode_t mode)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_unlink(const char *path)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_rmdir(const char *path)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_rename(const char *before, const char *after)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_truncate(const char *path, off_t size)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

/*
   -> op_open.c
int tiny_open(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}
*/

int tiny_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

int tiny_release(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

/*
   -> op_opendir.c
int tiny_opendir(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}
*/

int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	return 0;
}

int tiny_releasedir(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}

/*
   -> op_init.c
void *tiny_init(struct fuse_conn_info *conn)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}
 */

void tiny_destroy(void *user_data)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
}

int tiny_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	fprintf(stderr, "[TINYFS] %s", __func__);
	return 0;
}
