// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"

typedef struct inode {
  int refs;   // reference count
  int mode;   // permission & type
  int size;   // bytes
  int block;  // single block pointer (if max file size <= 4K)
} inode_t;

typedef struct next_block {
  int bnum; // the block index to the nextz block that stores the continuous data.
} next_block_t;


void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int next_free_inode();
int is_dir(inode_t *inode);

#endif
