#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

static const char *str_file1 = "My name is file1\n";
static const char *path_file1 = "/file1";
static const char *path_dir1 = "/dir1";
static const char *path_dir2 = "/dir2";

static const char *str_file11 = "My name is file11\n";
static const char *path_file11 = "/dir1/file11";
static const char *path_dir11 = "/dir1/dir11";


static int test_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);

	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, path_file1) == 0) {
		stbuf->st_mode = S_IFREG | 0555;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(str_file1);
	} else if (strcmp(path, path_dir1) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, path_dir2) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, path_file11) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(str_file11);
	} else if (strcmp(path, path_dir11) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		res = -ENOENT;
	}

	return res;
}

static int test_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, buf: %s, offset: %d\n",
			__func__, path, buf, offset);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	if (strcmp(path, "/") == 0) {
		filler(buf, basename(path_file1), NULL, 0);
		filler(buf, basename(path_dir1), NULL, 0);
		filler(buf, basename(path_dir2), NULL, 0);
	} else if (strcmp(path, path_dir1) == 0) {
		filler(buf, basename(path_file11), NULL, 0);
		filler(buf, basename(path_dir11), NULL, 0);
	}

	return 0;
}

int test_mkdir(const char *path, mode_t mode)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, mode: 0x%x\n", __func__, path, mode);
}

int test_unlink(const char *path)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n", __func__, path);
}

int test_rmdir(const char *path)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);
}

int test_rename(const char *before, const char *after)
{
	fprintf(stderr, "[TESTFS] %s - before: %s, after: %s\n",
			__func__, before, after);
}

int test_truncate(const char *path, off_t size)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, size: %d\n",
			__func__, path, size);
}

int test_open(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);

	fprintf(stderr, "[TESTFS] flags: 0x%x\n", info->flags);
	info->fh = 128;
}

int test_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, buf: %s, size: %d, offset: %d\n",
			__func__, path, buf, size, offset);
	fprintf(stderr, "[TESTFS] %s - info->fh: %lu\n",
			__func__, info->fh);
}

int test_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, buf: %s, size: %d, offset: %d\n",
			__func__, path, buf, size, offset);
}

int test_release(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);
}

int test_opendir(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);
}

int test_releasedir(const char *path, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s\n",
			__func__, path);
}

void *test_init(struct fuse_conn_info *conn)
{
	fprintf(stderr, "[TESTFS] %s\n", __func__);
	fprintf(stderr, "[TESTFS] proto_major: %u\n", conn->proto_major);
	fprintf(stderr, "[TESTFS] proto_minor: %u\n", conn->proto_minor);
	fprintf(stderr, "[TESTFS] async_read : %u\n", conn->async_read);
	fprintf(stderr, "[TESTFS] max_write  : %u\n", conn->max_write);
	fprintf(stderr, "[TESTFS] max_readahead: %u\n", conn->max_readahead);
	fprintf(stderr, "[TESTFS] capable    : %u\n", conn->capable);
	fprintf(stderr, "[TESTFS] want       : %u\n", conn->want);
	fprintf(stderr, "[TESTFS] max_background: %u\n", conn->max_background);
	fprintf(stderr, "[TESTFS] congestion_threshold: %u\n", conn->congestion_threshold);
}

void test_destroy(void *data)
{
	fprintf(stderr, "[TESTFS] %s %s\n",
			__func__, (char*)data);
}

int test_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	fprintf(stderr, "[TESTFS] %s - path: %s, mode: 0x%x\n",
			__func__, path, mode);
}

static struct fuse_operations test_oper = {
	.getattr	= test_getattr,
	.readdir	= test_readdir,
	.mkdir		= test_mkdir,
	.create		= test_create,
	.destroy	= test_destroy,
	.init		= test_init,
	.releasedir	= test_releasedir,
//	.opendir	= test_opendir,
	.release	= test_release,
	.write		= test_write,
	.read		= test_read,
	.open		= test_open,
	.truncate	= test_truncate,
	.rename		= test_rename,
	.rmdir		= test_rmdir,
	.unlink		= test_unlink,
};

const char *user_data = "this is my user data";

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &test_oper, (void*)user_data);
}
