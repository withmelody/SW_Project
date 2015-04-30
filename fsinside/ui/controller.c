#include "header/project.h"
#include "messageQ/msglib.h"

int isDiskChanged;
void printCurrentEvent(FileIO_t* fio) ;

void updateInodes(InodeBitmap_t* inode_bitmap) {
	THREAD_LOCK;
	memcpy(&ibm, inode_bitmap, sizeof(InodeBitmap_t));
	THREAD_UNLOCK;
}
void updateBlocks(BlockBitmap_t* block_bitmap) {
	THREAD_LOCK;
	memcpy(&bbm, block_bitmap, sizeof(BlockBitmap_t));
	THREAD_UNLOCK;
}

void updateBlocks_for_IO(FileIO_t* fio) {
	int i = 0;
	int block_index;
	char mode = fio->flag;

	printCurrentEvent(fio);
	THREAD_LOCK;
	isDiskChanged = true;
	while(i < 8) {
		block_index = fio->inode.i_block[i];
//		mvwprintw(top_clock, 2, 2, "[%c]", mode);     
//		mvwprintw(top_clock, 1, 5 + (i * 6), "[%d]", block_index);     
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
		else if (mode == 'd') {	// removed
			blocks[block_index].isReading = false;
			blocks[block_index].isWriting = false;
			blocks[block_index].isRemoved = true;
		}
		else if (mode == 'i') {	// Idle
			blocks[block_index].isReading = false;
			blocks[block_index].isWriting = false;
		}

	}

	THREAD_UNLOCK;

	//memcpy(&bbm, block_bitmap, sizeof(BlockBitmap_t));
}
