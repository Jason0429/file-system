// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} dirent_t;

/**
 * Allocates a new data block for the root directory, if not allocated already.
 * Should only run once at the beginning of the program.
 */
void directory_init();

/**
 * Given a directory inode `dd`, return the inum associated with
 * an entry with the given `name`.
 * Returns -1 if such a name cannot be found.
 */
int directory_lookup(inode_t *dd, const char *name);

/**
 * Given a path, returns the `inum` of that entry.
 * Returns -1 on error.
 */
int tree_lookup(const char *path);

/**
 * Returns a pointer to the first free `dirent_t` in the given data directory
 * block associated with the given directory inode `dd`.
 * Returns NULL if none are available.
 */
dirent_t *next_free_entry(inode_t *dd);

/**
 * Adds a new entry to the given directory inode `dd`.
 * The new entry is specified by `name` and `inum`.
 * Does not allocate data block for the new entry.
 * Returns 0 on success and -1 on error.
 */
int directory_put(inode_t *dd, const char *name, int inum);

/**
 * Deletes an entry with the given `name` from the parent `dd` by
 * marking its name as "".
 * If the entry is a file, that file gets deleted.
 * If the entry is a directory, that directory will only be deleted if empty.
 * Returns 0 on success and -1 on error.
 */
int directory_delete(inode_t *dd, const char *name);

/**
 * Given a path to a directory, return a linked list of the
 * children entries in that directory AND itself.
 * The self dirent is at the END of the returned linked list.
 * Returns NULL if path is not valid.
 */
slist_t *directory_list(const char *path);

/**
 * Given a directory inode `dd`, prints the directory name followed by all the
 * entry names within that directory.
 */
void print_directory(inode_t *dd);

/**
 * Returns the `ith_entry` in in the given `dir_block`.
 */
dirent_t *get_entry(dirent_t *dir_block, int ith_entry);

/**
 * Returns the number of entries given the total size in a directory inode.
 */
int get_num_entries(int size);

/**
 * Given a path, return the path as a string
 * up to the latest parent directory.
 *
 * Ex: "/" -> "/"
 * Ex: "/dir1" -> "/"
 * Ex: "/dir1/dir2/file.txt" -> "/dir1/dir2"
 */
char *get_parent_path(const char *path);

/**
 * Returns the entry name given a `path`.
 * If the given `path` is "/", returns "".
 */
char *get_entry_name(const char *path);

/**
 * Returns the entry in the given directory inode with the specified name.
 */
dirent_t *get_entry_with_name(inode_t *dd, const char *name);

#endif
