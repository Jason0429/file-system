# Project 2: File System

This is the starter code for [Project 2](https://course.ccs.neu.edu/cs3650f22/p2.html). The following contents are provided:

- [Makefile](Makefile)   - Targets are explained in the assignment text
- [README.md](README.md) - This README
- [helpers](helpers)     - Helper code implementing access to bitmaps and blocks
- [hints](hints)         - Incomplete bits and pieces that you might want to use as inspiration
- [nufs.c](nufs.c)       - The main file of the file system driver
- [test.pl](test.pl)     - Tests to exercise the file system

## Running the tests

You might need install an additional package to run the provided tests:

```
$ sudo apt-get install libtest-simple-perl
```

Then using `make test` will run the provided tests.

# TODO:
- [ ] Double check `tree_lookup`.
- [ ] In `directory_init`, use `directory_put` to add parent and self references.
- [ ] Fix `root_dir_init`.
- [ ] Include ".." and "." `dirent_t` references.
- [ ] `nufs_access`
- [ ] `nufs_readdir`
- [ ] `nufs_read`
- [ ] `nufs_write`
- [ ] `nufs_mknod`
- [ ] `nufs_mkdir`
- [ ] `nufs_unlink`
- [ ] `nufs_rmdir`
- [ ] `nufs_truncate`
- [ ] `nufs_rename`
- [ ] `nufs_chmod`

# Visual Help:
```
BLOCKS:
[ [bitmaps], [inodes], [root dir data], [file data], [file data], [dir data], [dir data], [file data] ]
     0           1             2             3           4            5            6           7
```

# HElp
Blocks:
[[b/i], I, D, D, a, b, a, a, b, ..., a]
^ used two blocks to store inodes
^ block(3) is root dir
^ CHECK `sizeof(inode_t)`, if 32 bytes, we only really need 1 block (4096/32 = 128 files)

`block_init` will allocate the first block for bitmaps

Block 0:
32 bytes for bitmap to store blocks
32 bytes for bitmap to store inodes

I. Split the image
1. inode table - static -> 2 * allocate a block for inodes
2. data blocks - dynamic
3. directories - dynamic

II. Create the root dir
1. Allcoate inode
2. Allocate data block

Mounting and unmounting:
- express everything in block/inode indicies, not pointer. Because after
program ends, pointer addresses are useless.

Directory Delete:
- `inode_t *dd, const char *name`
- MARK A FILE AS EMPTY BY SETTING IT'S NAME TO "". Next time do an iteration to check for "" or new.
- If file wants to be deleted, do we just decrease reference count, and if the new
reference count = 0, mark that inode bitmap as not in use.
- If directory wants to be deleted, we only delete if it's empty. But how would we know if it's
empty if some file inodes are still marked as in-use only because it's referenced somewhere else
(i.e. the file associated with that inode in this directory was deleted but not in the other location that was linked to this).

*Whenever we create a directory,
we need to create two `dirent_t`, ".." and ".", `inum` will point to parent and itself.

nufs_access:
called when you try and access a directory/file (cd, vim, cat, etc)

nufs_read:
copy all the content over to the buffer
use `memcpy`

Renaming:
dirent_t *dir = get_block_of_src_directory
get the directory entry of the source, e.g., dir[12]
get the directory block of the destination
find the first empty dir entry in destination, e.g. dir2[15]
dir2[15] = dir[2];
strcpy(dir[15].name, "newname.txt");
delete dir[12];

truncate(new size):
if you are shrinking:
     set the size to new size (release block if size 0 now). 
     no need to modify actual data
if you are growing:
     extend previous file and extend with null bytes ('\0')

They will test:
100 entires

OPTIONAL:
int rv = fuse_main
blocks_free()
return rv;

DONT NEED TO IMPLEMENT LINKS

UNLINK:
deletes that entry, drops ref count.
once ref count = 0, file should be deleted.

PRIORITIZE:
leave rename files to last

WRITING:
if a file does not exist, create it and write to it.

TESTS:
28-31 extra credit