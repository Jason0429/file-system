#include "../directory.h"

#include <assert.h>
#include <string.h>

int main() {
    char *path1 = "/";
    char *path2 = "/dir1";
    char *path3 = "/dir1/dir2";

    assert(strcmp(get_parent_path(path1), "/") == 0);
    assert(strcmp(get_parent_path(path2), "/") == 0);
    assert(strcmp(get_parent_path(path3), "/dir1") == 0);

    assert(strcmp(get_entry_name(path1), "") == 0);
    assert(strcmp(get_entry_name(path2), "dir1") == 0);
    assert(strcmp(get_entry_name(path3), "dir2") == 0);

    return 0;
}