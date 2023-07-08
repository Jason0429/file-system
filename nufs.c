// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "blocks.h"
#include "constants.h"
#include "directory.h"
#include "inode.h"
#include "slist.h"
#include "storage.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  int rv = tree_lookup(path) == -1 ? -ENOENT : 0;
  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = storage_stat(path, st);
  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  struct stat st;
  int rv;

  rv = nufs_getattr(path, &st);
  assert(rv == 0);
  filler(buf, SELF_REF, &st, 0);

  int is_path_root = strcmp(path, "/") == 0;

  // We want parent reference if `path` is not root.
  if (!is_path_root) {
    rv = nufs_getattr(get_parent_path(path), &st);
    assert(rv == 0);
    filler(buf, PARENT_REF, &st, 0);
  }

  // Linked list of entry names that are not one of the following:
  // "", ".", ".."
  slist_t *curr = storage_list(path);
  s_print(curr);

  while (curr != NULL) {
    char path_to_entry[DIR_NAME_LENGTH * 5];
    strcpy(path_to_entry, path);
    if (!is_path_root) {
      strcat(path_to_entry, "/");
    }
    strcat(path_to_entry, curr->data);
    rv = nufs_getattr(path_to_entry, &st);
    filler(buf, curr->data, &st, 0);
    curr = curr->next;
  }

  printf("readdir(%s) -> %d\n", path, rv);
  return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = storage_mknod(path, mode);
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

// deletes the given path
// unlink("hello.txt")
// look the file up, check if it exists
// delete the inode entry
// reduce reference count in the inode - reference count keeps track of number
// of times used
// "/a/file.txt" still exists so you need to delete that

// 1. remove entry
// 2. ref--
// 3. if ref == 0, deallocate the block
// the data still stays in the block it's in, just removed from inode
int nufs_unlink(const char *path) {
  int rv = storage_unlink(path);
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

// link("/hello.txt", "/a/file.txt")
// hello.txt = inode 1
// /a/file.txt = inode 1
// return error if the file already exists
int nufs_link(const char *from, const char *to) {
  int rv = storage_link(from, to);
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = storage_unlink(path);
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = storage_rename(from, to);
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  int inum = tree_lookup(path);
  int rv = inum == -1 ? -1 : 0;

  if (inum != -1) {
    inode_t *inode = get_inode(inum);
    inode->mode = mode;
  }

  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// Truncate the size of the entry at the given path
int nufs_truncate(const char *path, off_t size) {
  int rv = storage_truncate(path, size);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = nufs_access(path, 0);
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = storage_read(path, buf, size, offset);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Writes data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = 0;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = 0;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);

  // should mount the block
  storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  int rv = fuse_main(argc, argv, &nufs_ops, NULL);
  blocks_free();
  return rv;
}
