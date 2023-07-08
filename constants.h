#include "blocks.h"
#include "inode.h"

#define DIR_MODE 040775

#define INODE_COUNT BLOCK_SIZE / sizeof(inode_t)
#define ENTRY_COUNT BLOCK_SIZE / sizeof(dirent_t)

#define ROOT_DIR_INUM 0

#define INODES_BNUM 1
#define ROOT_DIR_BNUM 2

#define SELF_REF "."
#define PARENT_REF ".."