#include "storage.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "bitmap.h"
#include "constants.h"
#include "directory.h"
#include "inode.h"

void storage_init(const char *path) {
  blocks_init(path);
  directory_init();
}

int storage_mknod(const char *path, int mode) {
  int parent_inum = tree_lookup(get_parent_path(path));
  if (parent_inum == -1) {
    return -ENOENT;
  }

  char *entry_name = get_entry_name(path);
  inode_t *parent_dd = get_inode(parent_inum);

  // Check if a new entry can be made
  int valid_block = next_free_block() != -1;
  int valid_inode = next_free_inode() != -1;
  int has_space = parent_dd->size + sizeof(dirent_t) <= BLOCK_SIZE;
  if (!(valid_block && valid_inode && has_space)) {
    return -ENOENT;
  }

  // Allocate data block for new entry
  int new_entry_bnum = alloc_block();

  // Allocate inode for new entry
  int new_entry_inum = alloc_inode();

  inode_t *entry_node = get_inode(new_entry_inum);
  entry_node->refs = 1;
  entry_node->mode = mode;
  entry_node->size = 0;
  entry_node->block = new_entry_bnum;

  assert(directory_put(parent_dd, entry_name, new_entry_inum) != -1);

  return 0;
}

int storage_stat(const char *path, struct stat *st) {
  int inum = tree_lookup(path);
  if (inum == -1) {
    return -ENOENT;
  }

  memset(st, 0, sizeof(struct stat));
  inode_t *inode = get_inode(inum);
  st->st_mode = inode->mode;
  st->st_size = inode->size;
  st->st_uid = getuid();

  return 0;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  int file_inum = tree_lookup(path);
  if (file_inum == -1) {
    return -ENOENT;
  }

  inode_t *file_node = get_inode(file_inum);
  char *file_block = (char *)blocks_get_block(file_node->block);
  memcpy(buf, file_block + offset, size);

  return size;
}

int storage_write(const char *path, const char *buf, size_t size,
                  off_t offset) {
  int file_inum = tree_lookup(path);
  if (file_inum == -1 || size > BLOCK_SIZE) {
    return -ENOENT;
  }

  inode_t *file_node = get_inode(file_inum);
  assert(!is_dir(file_node));

  if (file_node->size < size + offset) {
    storage_truncate(path, size + offset);
  }

  char *file_block = (char *)blocks_get_block(file_node->block);
  memcpy(file_block + offset, buf, size);

  return size;
}

int storage_truncate(const char *path, off_t size) {
  assert(size >= 0);

  int inum = tree_lookup(path);
  if (inum == -1) {
    return -ENOENT;
  }

  inode_t *inode = get_inode(inum);
  inode->size = size;

  if (inode->size == 0) {
    free_block(inode->block);
  }

  return 0;
}

int storage_unlink(const char *path) {
  int inum = tree_lookup(path);
  if (inum == -1) {
    return -1;
  }

  // "Delete" entry from parent directory by renaming to ""
  inode_t *dd = get_inode(tree_lookup(get_parent_path(path)));
  assert(directory_delete(dd, get_entry_name(path)) == 0);

  inode_t *inode = get_inode(inum);
  inode->refs--;

  // If new decremented ref count reaches 0,
  // free block and inode for that entry.
  if (inode->refs == 0) {
    free_block(inode->block);
    free_inode(inum);
  }

  return 0;
}

int storage_link(const char *from, const char *to) {
  // 'from' is the old file and 'to' is the path to the new file
  int to_parent_inum = tree_lookup(get_parent_path(to));
  int from_inum = tree_lookup(from);
  if (to_parent_inum == -1 || from_inum == -1) {
    return -ENOENT;
  }

  // Get new entry name from `to`.
  char *file_name = get_entry_name(to);

  inode_t *to_dd = get_inode(to_parent_inum);
  int old_size = to_dd->size;
  assert(directory_put(to_dd, file_name, from_inum) == 0);
  assert(to_dd->size > old_size);

  // increases ref count and points 'to' to the same inode as 'from'
  inode_t *from_node = get_inode(from_inum);
  from_node->refs++;

  return 0;
}

int storage_rename(const char *from, const char *to) {
  if (tree_lookup(from) == -1 || tree_lookup(get_parent_path(to)) == -1) {
    return -ENOENT;
  }

  assert(storage_link(from, to) == 0);
  assert(storage_unlink(from) == 0);

  return 0;
}

slist_t *storage_list(const char *path) { return directory_list(path); }
