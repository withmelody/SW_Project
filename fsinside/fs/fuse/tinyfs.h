#ifndef _TINYFS_H_
#define _TINYFS_H_

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int tiny_getattr(const char *path, struct stat *stbuf);

int tiny_mkdir(const char *path, mode_t mode);

int tiny_unlink(const char *path);

int tiny_rmdir(const char *path);

int tiny_rename(const char *before, const char *after);

int tiny_truncate(const char *path, off_t size);

int tiny_open(const char *path, struct fuse_file_info *info);

int tiny_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *info);

int tiny_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *info);

int tiny_release(const char *path, struct fuse_file_info *info);

int tiny_opendir(const char *path, struct fuse_file_info *info);

int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *info);

int tiny_releasedir(const char *path, struct fuse_file_info *info);

void *tiny_init(struct fuse_conn_info *conn);

void tiny_destroy(void *user_data);

int tiny_create(const char *path, mode_t mode, struct fuse_file_info *info);

#endif /* _TINYFS_H_ */

