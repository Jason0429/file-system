#include "directory.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "blocks.h"
#include "constants.h"
#include "inode.h"
#include "slist.h"

void directory_init() {
  // Do nothing if inodes block is already allocated.
  int inodes_bnum = next_free_block();
  if (inodes_bnum != INODES_BNUM) {
    return;
  }

  // Allocate inodes block if not already allocated.
  assert(alloc_block() == INODES_BNUM);

  int next_free_bnum = next_free_block();

  // Do nothing if `ROOT_DIR_BNUM` block is already initialized.
  if (next_free_bnum != ROOT_DIR_BNUM) {
    return;
  }

  int bnum = alloc_block();
  assert(bnum == ROOT_DIR_BNUM);

  int inum = alloc_inode();
  assert(inum == ROOT_DIR_INUM);

  inode_t *root_inode = get_inode(ROOT_DIR_INUM);
  root_inode->refs = 1;
  root_inode->mode = DIR_MODE;
  root_inode->size = 0;
  root_inode->block = ROOT_DIR_BNUM;
}

int directory_lookup(inode_t *dd, const char *name) {
  assert(dd->block == ROOT_DIR_BNUM || dd->mode == DIR_MODE);
  dirent_t *dir_block = (dirent_t *)blocks_get_block(dd->block);
  dirent_t *entry = get_entry_with_name(dd, name);
  if (entry == NULL) {
    return -1;
  }

  return entry->inum;
}

int tree_lookup(const char *path) {
  if (strcmp(path, "/") == 0) {
    return ROOT_DIR_INUM;
  }

  // Splits the path into a linked list.
  // Skips first element because it is "".
  slist_t *path_list = s_explode(++path, '/');
  s_print(path_list);
  slist_t *curr = path_list;
  int inum = ROOT_DIR_INUM;
  while (curr != NULL) {
    inum = directory_lookup(get_inode(inum), curr->data);
    if (inum == -1) {
      return -1;
    }
    curr = curr->next;
  }

  return inum;
}

dirent_t *next_free_entry(inode_t *dd) {
  // Look for an entry space that was previously deleted.
  dirent_t *free_entry = get_entry_with_name(dd, "");
  if (free_entry != NULL) {
    return free_entry;
  }

  // No space available
  if (dd->size + sizeof(dirent_t) > BLOCK_SIZE) {
    return NULL;
  }

  // If next available space is at the end
  dirent_t *dir_block = (dirent_t *)blocks_get_block(dd->block);
  return (dirent_t *)((char *)(dir_block) + dd->size);
}

int directory_put(inode_t *dd, const char *name, int inum) {
  dirent_t *new_entry = next_free_entry(dd);
  if (new_entry == NULL) {
    return -1;
  }

  strncpy(new_entry->name, name, DIR_NAME_LENGTH);
  new_entry->inum = inum;
  dd->size += sizeof(dirent_t);

  return 0;
}

int directory_delete(inode_t *dd, const char *name) {
  dirent_t *entry_to_delete = get_entry_with_name(dd, name);
  if (entry_to_delete == NULL) {
    return -1;
  }

  entry_to_delete->name[0] = '\0';
  dd->size -= sizeof(dirent_t);

  return 0;
}

dirent_t *get_entry_with_name(inode_t *dd, const char *name) {
  dirent_t *dir_block = (dirent_t *)blocks_get_block(dd->block);
  for (int i = 0; i < ENTRY_COUNT; i++) {
    dirent_t *entry = get_entry(dir_block, i);
    if (entry != NULL && strcmp(entry->name, name) == 0) {
      return entry;
    }
  }

  return NULL;
}

slist_t *directory_list(const char *path) {
  int inum = tree_lookup(path);
  if (inum == -1) {
    return NULL;
  }

  inode_t *dd = get_inode(inum);
  dirent_t *dir_block = (dirent_t *)blocks_get_block(dd->block);
  slist_t *entries;

  for (int i = 0; i < ENTRY_COUNT; i++) {
    dirent_t *entry = get_entry(dir_block, i);
    // Exclude self and parent references and
    // previously removed entries (empty names)
    if (entry != NULL && strcmp(entry->name, "")) {
      entries = s_cons(entry->name, entries);
    }
  }

  return entries;
}

char *get_parent_path(const char *path) {
  if (strcmp(path, "/") == 0) {
    return "/";
  }

  int last_dir_char;
  for (int i = 0; i < strlen(path); i++) {
    if (path[i] == '/') {
      last_dir_char = i;
    }
  }

  if (last_dir_char == 0) {
    return "/";
  }

  char *path_to_parent = (char *)malloc(last_dir_char + 1);
  strncpy(path_to_parent, path, last_dir_char);
  path_to_parent[last_dir_char] = '\0';

  return path_to_parent;
}

char *get_entry_name(const char *path) {
  if (strcmp(path, "/") == 0) {
    return "";
  }

  slist_t *path_list = s_explode(path, '/');
  slist_t *curr = path_list;

  while (curr != NULL) {
    if (curr->next == NULL) {
      return curr->data;
    }
    curr = curr->next;
  }
}

void print_directory(inode_t *dd) {
  dirent_t *dir_block = (dirent_t *)blocks_get_block(dd->block);
  for (int i = 0; i < get_num_entries(dd->size); i++) {
    printf("%s\n", get_entry(dir_block, i)->name);
  }
}

dirent_t *get_entry(dirent_t *dir_block, int ith_entry) {
  return (dirent_t *)(((char *)dir_block) + (ith_entry * sizeof(dirent_t)));
}

int get_num_entries(int size) { return size / sizeof(dirent_t); }