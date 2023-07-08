#include "inode.h"

#include <assert.h>

#include "bitmap.h"
#include "blocks.h"
#include "constants.h"

/**
 * Prints the details of the given `inode`.
 */
void print_inode(inode_t *node) {
  printf("refs: %d\n", node->refs);
  printf("mode: %d\n", node->mode);
  printf("size: %d\n", node->size);
  printf("block: %d\n", node->block);
}

/**
 * Returns the inode at the given 'inum'.
 */
inode_t *get_inode(int inum) {
  assert(0 <= inum && inum < INODE_COUNT);
  void *ibm = get_inode_bitmap();
  inode_t *inode_block = (inode_t *)blocks_get_block(INODES_BNUM);
  return inode_block + (sizeof(inode_t *) * inum);
}

/**
 * Allocates the next available inode spot and returns the index.
 * Returns -1 if a new inode cannot be allocated.
 */
int alloc_inode() {
  int inum = next_free_inode();

  if (inum == -1) {
    return -1;
  }

  void *ibm = get_inode_bitmap();
  bitmap_put(ibm, inum, 1);
  printf("+ alloc_inode() -> %d\n", inum);
  return inum;
}

/**
 * Returns the inum of the next available inode, without allocating.
 * Returns -1 if nothing is free.
 */
int next_free_inode() {
  void *ibm = get_inode_bitmap();

  for (int i = 0; i < INODE_COUNT; i++) {
    if (!bitmap_get(ibm, i)) {
      return i;
    }
  }

  return -1;
}

/**
 * Frees the inode at the given index
 */
void free_inode(int inum) {
  void *ibm = get_inode_bitmap();
  bitmap_put(ibm, inum, 0);
  printf("+ free_inode(%d)\n", inum);
}

/**
 * Determines if the given `inode` is a directory type.
 */
int is_dir(inode_t *inode) {
  assert(inode != NULL);
  return inode->mode == DIR_MODE;
}

/**
 * Returns the inode of the block that holds the partial data from size.
 * Ex. inode->size = 5000 -> Returns the 2nd block of data
*/
 int bnum_from_size(inode_t *inode, int size) {
  inode_t *ret = inode;
  if (size - BLOCK_SIZE < 0) {
    return ret->block;
  }

  while (size >= 0) {
    size -= BLOCK_SIZE;
    next_block_t *next_block = (next_block_t *) (((char *)ret) + BLOCK_SIZE - sizeof(next_block_t));
    ret = get_inode(next_block->bnum);
  }

  return ret->block;
}