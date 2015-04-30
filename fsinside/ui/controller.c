#include "header/project.h"
#include "messageQ/msglib.h"
//#include "tinyfs.h"

void updateInodes(InodeBitmap_t* inode_bitmap) {
	memcpy(&ibm, inode_bitmap, sizeof(InodeBitmap_t));
}
void updateBlocks(BlockBitmap_t* block_bitmap) {
	memcpy(&bbm, block_bitmap, sizeof(BlockBitmap_t));
}

void updateBlocks_for_IO(FileIO_t* fio) {
	int i = 0;
	int block_index;
	char mode = fio->flag;
	
	THREAD_LOCK;

	while(i < 8) {
		block_index = fio->inode.i_block[i];
		mvwprintw(top_clock, 2, 2, "[%c]", mode);     
		mvwprintw(top_clock, 1, 5 + (i * 6), "[%d]", block_index);     
		if (block_index < 0)
			break;
		i++;
		if (mode == 'r') {		// Reading
			blocks[block_index].isReading = true;
			blocks[block_index].isWriting = false;
		}
		else if (mode == 'w') {	// Writing
			blocks[block_index].isReading = false;
			blocks[block_index].isWriting = true;
		}
		else if (mode == 'i') {	// Idle
			blocks[block_index].isReading = false;
			blocks[block_index].isWriting = false;
		}
	}

	THREAD_UNLOCK;

	//memcpy(&bbm, block_bitmap, sizeof(BlockBitmap_t));
}
