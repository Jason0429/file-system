// Disk storage manipulation.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "slist.h"

/**
 * Initializes the root directory, if not already.
 * Loads and initializes the given disk image.
 */
void storage_init(const char *path);

/**
 * Gets an object's attributes (type, permissions, size, etc.)
 * and loads it to corresponding `inode`.
 * Returns 0 on success and -ENOENT otherwise.
 */
int storage_stat(const char *path, struct stat *st);

/**
 * Reads contents of a file into given buffer.
 * Returns the length of the file on success and -ENOENT otherwise.
 */
int storage_read(const char *path, char *buf, size_t size, off_t offset);

/**
 * Handles writing data from buffer into corresponding data block.
 * Returns the length of the write on success and -ENOENT otherwise.
 */
int storage_write(const char *path, const char *buf, size_t size, off_t offset);

/**
 * Sets the size of the entry at the given path to the given `size`.
 * Will release blocks if new size becomes 0.
 * Will grow blocks if new size exceeds `BLOCK_SIZE`.
 * Returns 0 on success and -ENOENT otherwise.
 */
int storage_truncate(const char *path, off_t size);

/**
 * Creates a file or directory (determined by `mode`) at the given `path`.
 * Returns 0 on success and -ENOENT otherwise.
 */
int storage_mknod(const char *path, int mode);

/**
 * Decrements reference count of the entry specified by the given path.
 * If reference count hits 0, frees that inode and data block.
 * Returns 0 on success and -1 otherwise.
 */
int storage_unlink(const char *path);

/**
 * Renames a file or directory with the name `from` to the name `to`.
 * Returns 0 on success and -1 otherwise.
 */
int storage_rename(const char *from, const char *to);

/**
 * Increases the reference count of the `from` entry and creates the same entry at `to`.
 */
int storage_link(const char *from, const char *to);

int storage_set_time(const char *path, const struct timespec ts[2]);

/**
 * Lists the contents of the given directory specified by `path`.
 */
slist_t *storage_list(const char *path);

#endif
