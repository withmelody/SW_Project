#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "buf.h"
#include "tinyfs.h"

extern FileSysInfo tiny_superblk;

static tiny_dentry *__find_dentry_from_parent(tiny_inode *parent_inode,
		const char *entry_name)
{
	tiny_dirblk *dirblk;
	tiny_dentry *dentry;
	Buf *buf;
	int i, j;

	for (i = 0; i < parent_inode->i_nblk; i++) {
		buf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(buf);

		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			dentry = &dirblk->dirEntries[j];
			if (strncmp(entry_name, dentry->name, NAME_LEN_MAX) == 0)
				return dentry;
		}
	}

	return NULL;
}

static tiny_dentry *_find_dentry_and_parent_by_path(const char *path,
		tiny_inode *parent_inode)
{
	tiny_dentry		*pDentry;
	char *path_copy;
	char *token;
	char *dir_name;
	char *base_name;

	/* Get inode of the parent directory */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	dir_name = dirname(path_copy);
	token = strtok(dir_name, "/");
	ReadInode(parent_inode, tiny_superblk.s_rdirino);
	while (token) {
		pDentry = __find_dentry_from_parent(parent_inode, token);
		if (!pDentry || pDentry->type == FILE_TYPE_FILE) {
			free(path_copy);
			return NULL;
		}
		
		ReadInode(parent_inode, pDentry->inodeNum);
		token = strtok(NULL, "/");
	}
	free(path_copy);

	/* Get dentry of the target */
	path_copy = (char*)malloc(strlen(path) + 1);
	strcpy(path_copy, path);
	base_name = basename(path_copy);
	pDentry = __find_dentry_from_parent(parent_inode, base_name);
	free(path_copy);

	return pDentry;
}


static int _overwrite_dentry(tiny_inode *parent_inode,
		tiny_dentry *target_dentry_remove, tiny_dentry *target_dentry_add)
{
	tiny_dirblk *dirblk;
	tiny_inode target_inode_remove;
	Buf* pBuf;
	int i, j;

	for (i = 0; i < parent_inode->i_nblk; i++) {
		pBuf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(pBuf);

		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			if (strncmp(dirblk->dirEntries[j].name,
						target_dentry_remove->name, NAME_LEN_MAX) == 0) {
				ReadInode(&target_inode_remove, target_dentry_remove->inodeNum);
				for ( i = 0 ; i < target_inode_remove.i_nblk ; i++ ) {
					SetBlockAllocToFree(target_inode_remove.i_block[i]);
				}
				SetInodeAllocToFree(target_dentry_remove->inodeNum);

				strncpy(target_dentry_add->name, target_dentry_remove->name, NAME_LEN_MAX);
				memcpy(&dirblk->dirEntries[j], target_dentry_add,
						sizeof(tiny_dentry));
				BufDelete(pBuf);
				BufInsert(pBuf, BUF_LIST_DIRTY);
				return 0;
			}
		}
	}
}

static int _add_dentry(tiny_inode *parent_inode, tiny_dentry *target_dentry)
{
	tiny_dirblk *dirblk;
	Buf* pBuf;
	int i, j;

	for (i = 0; i < parent_inode->i_nblk; i++) {
		pBuf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(pBuf);

		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			if (strncmp(dirblk->dirEntries[j].name, "", NAME_LEN_MAX) == 0) {
				memcpy(&dirblk->dirEntries[j], target_dentry,
						sizeof(tiny_dentry));
				return 0;
			}
		}
	}

	if (parent_inode->i_nblk != TINY_N_DIRECT_BLOCKS) {
		int nblk_new = GetFreeBlock();
		if (nblk_new < 0) {
			return -1;
		}
		parent_inode->i_block[parent_inode->i_nblk++] = nblk_new;
		SetBlockFreeToAlloc();

		pBuf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(pBuf);
		memcpy(&dirblk->dirEntries[0], target_dentry, sizeof(tiny_dentry));
		BufDelete(pBuf);
		BufInsert(pBuf, BUF_LIST_DIRTY);

		WriteInode(parent_inode, __find_inodeno_for_dir(parent_inode));
		return 0;
	}

	return -1;
}

static int _remove_dentry(tiny_inode *parent_inode, tiny_dentry *target_dentry)
{
	tiny_dirblk *dirblk;
	tiny_dentry *dentry;
	Buf *buf;
	int i, j;
	int isEmpty = 1;
	int find = 0;
	int dirblkno;
	int parent_inodeno;
	int target_inodeno;


	for (i = 0; i < parent_inode->i_nblk; i++) {
		buf = BufRead(parent_inode->i_block[i]);
		dirblk = __get_dirblk_from_buf(buf);
		dirblkno = parent_inode->i_block[i];
		
		isEmpty = 1;
		for (j = 0; j < NUM_OF_DIRENT_IN_1BLK; j++) {
			dentry = &dirblk->dirEntries[j];

			if ( i == 0 && j == 0 ) {
				parent_inodeno = dentry[0].inodeNum;
			}

			if (!find && strncmp(target_dentry->name,
						dentry->name, NAME_LEN_MAX) == 0) {
				strncpy(dentry->name, "", NAME_LEN_MAX);
				find = 1;
			} else if (strncmp(dentry->name, "", NAME_LEN_MAX) != 0) {
				isEmpty = 0;
			}
		}

		if (find) {
			if (isEmpty) {
				SetBlockAllocToFree(dirblkno);
				parent_inode->i_block[i] =
					parent_inode->i_block[--parent_inode->i_nblk];
				WriteInode(parent_inode, parent_inodeno);
			} else {
				WriteDirBlock(dirblk, dirblkno);
			}
			return target_inodeno;
		}
	}

	return -1;
}

int tiny_rename(const char *path_before, const char *path_after)
{
	tiny_dentry *dentry_before;
	tiny_inode parent_inode_before;
	tiny_dentry *dentry_after;
	tiny_inode parent_inode_after;

	if (strlen(path_after) > NAME_LEN_MAX) {
		return -ENAMETOOLONG;
	}

	dentry_before =
		_find_dentry_and_parent_by_path(path_before, &parent_inode_before);
	dentry_after =
		_find_dentry_and_parent_by_path(path_after, &parent_inode_after);

	fprintf(stderr, "[DEBUG] before: %s\n",
			dentry_before != NULL ? dentry_before->name : "NULL");
	fprintf(stderr, "[DEBUG] after: %s\n",
			dentry_after != NULL ? dentry_after->name : "NULL");
	fprintf(stderr, "[DEBUG] parent before: %d\n", parent_inode_before.i_type);
	fprintf(stderr, "[DEBUG] parent after: %d\n", parent_inode_after.i_type);

	if (dentry_after) {
		// overwrite
		_overwrite_dentry(&parent_inode_after, dentry_after, dentry_before);
	} else {
		// just add
		if (_add_dentry(&parent_inode_after, dentry_before) < 0) {
			return -1;
		}
	}

	_remove_dentry(&parent_inode_before, dentry_before);

	return 0;
}

