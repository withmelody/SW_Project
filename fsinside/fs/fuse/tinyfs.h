#ifndef _TINYFS_H_
#define _TINYFS_H_

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "disk.h"
#include "buf.h"

//#define FS_DISK_CAPACITY		(8 * 1024 * 1024) /* 8M */
//#define FS_INODE_COUNT			(128)
//#define BLOCK_SIZE				(512)
#define FS_DISK_CAPACITY		(16 * 1024 * 1024) /* 16M */
#define FS_INODE_COUNT			(256)
#define BLOCK_SIZE				(1024)
#define NUM_OF_INODE_IN_1BLK	(BLOCK_SIZE / sizeof(tiny_inode))
#define NUM_OF_DIRENT_IN_1BLK	(BLOCK_SIZE / sizeof(tiny_dentry))
#define MAX_INDEX_OF_DIRBLK		(NUM_OF_DIRENT_IN_1BLK)
#define NAME_LEN_MAX			(60)
#define TINY_N_DIRECT_BLOCKS	(24)

typedef enum { 
	MT_TYPE_UNKNOWN = 0,
	MT_TYPE_FORMAT,     // 마운트가 되는 해당 파티션은 포맷된다. 
	MT_TYPE_READWRITE,  // 마운트가 되는 해당 파티션은 그대로 유지된다. 
} MountType; 

typedef struct {
	int	s_ibitmap_size;
	int	s_inodeblk_size;
	int	s_dbitmap_size;
	int	s_datablk_size;

	int s_disk_fd; 
	int s_blksize;
	int s_ninode_in_blk;
//	int s_ndentry_in_blk;
	int s_nblk;				// 파일 시스템의 설정된 블록 크기
	int s_rdirino; 		// root inode number 저장
	int s_ndatablk;		// 디스크의 용량. 블록 개수로 저장됨
	int s_nblk_use;		// 할당된 블록 개수
	int s_nblk_free;		// 할당되지 않은 블록 개수
	int s_ninode;			// 전체 inode의 개수
	int s_ninode_use;		// 할당된 inode의 개수
	int s_ninode_free;		// 할당되지 않은 inode의 개수
	int s_ibitmap_start;	// inode bitmap이 저장된 블록의 번호
	int s_dbitmap_start;	// block bitmap이 저장된 블록 번호
	int s_inodeblk_start;		// inode list를 저장하는 영역의 시작 블록 번호
	int s_datablk_start;			// data 영역의 시작 블록 번호
	char* s_ibitmap_ptr; 	// inode bitmap 정보가 저장된 메모리 주소
	char* s_dbitmap_ptr; 	// block bitmap 정보가 저장된 메모리 주소
} FileSysInfo;

typedef enum {
    FILE_TYPE_FILE,
    FILE_TYPE_DIR,
} FileType;

typedef struct {
	int			i_size;
	FileType	i_type;
	int			i_nblk;
	int			i_block[TINY_N_DIRECT_BLOCKS];
} tiny_inode;

typedef struct {
    char 		name[NAME_LEN_MAX];
    unsigned	inodeNum;
    FileType	type;
} tiny_dentry;

typedef struct {
	tiny_dentry	dirEntries[NUM_OF_DIRENT_IN_1BLK];
} tiny_dirblk;

#define __get_dirblk_from_buf(buf)	\
	((tiny_dirblk*)((buf)->pMem))

tiny_dentry *__find_dentry(tiny_inode *dir, const char *entry_name);

int tiny_getattr(const char *path, struct stat *stbuf);
int tiny_mkdir(const char *path, mode_t mode);
int tiny_unlink(const char *path);
int tiny_rmdir(const char *path);
int tiny_rename(const char *before, const char *after);
int tiny_truncate(const char *path, off_t size);
int tiny_open(const char *path, struct fuse_file_info *info);
int tiny_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info);
int tiny_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info);
int tiny_release(const char *path, struct fuse_file_info *info);
int tiny_opendir(const char *path, struct fuse_file_info *info);
int tiny_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *info);
int tiny_releasedir(const char *path, struct fuse_file_info *info);
/*OK*/void *tiny_init(struct fuse_conn_info *conn);
void tiny_destroy(void *user_data);
int tiny_create(const char *path, mode_t mode, struct fuse_file_info *info);
int tiny_utimens(const char *path, const struct timespec tv[2]);

#endif /* _TINYFS_H_ */

