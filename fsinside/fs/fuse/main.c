#include "tinyfs.h"

FileSysInfo tiny_superblk;

static struct fuse_operations tiny_ops = {
	.getattr        = tiny_getattr,
	.readlink       = NULL,
	.mknod          = NULL,
	.mkdir          = tiny_mkdir,
	.unlink         = tiny_unlink,
	.rmdir          = tiny_rmdir,
	.symlink        = NULL,
	.rename         = tiny_rename,
	.link           = NULL,
	.chmod          = NULL,
	.chown          = NULL,
	.truncate       = tiny_truncate,
	.open           = tiny_open,
	.read           = tiny_read,
	.write          = tiny_write,
	.statfs         = NULL,
	.flush          = NULL,
	.release		= tiny_release,
	.fsync          = NULL,
	.setxattr       = NULL,
	.getxattr       = NULL,
	.listxattr      = NULL,
	.removexattr    = NULL,
	.opendir        = tiny_open,
	.readdir        = tiny_readdir,
	.releasedir     = tiny_release,
	.fsyncdir       = NULL,
	.init			= tiny_init,
	.destroy		= tiny_destroy,
	.access         = NULL,
	.create         = tiny_create,
	.ftruncate      = NULL,
	.fgetattr       = NULL,
	.lock           = NULL,
	.utimens        = tiny_utimens,
	.bmap           = NULL,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &tiny_ops, NULL);
}
