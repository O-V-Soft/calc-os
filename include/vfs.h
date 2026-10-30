#ifndef VFS_H
#define VFS_H
#include <stdint.h>

struct fs_driver {
    int (*open)(const char* path);
    int (*read)(int fd, void* buf, size_t count);
    int (*close)(int fd);
    int (*create)(const char* path, const uint8_t* data, size_t size);
    int (*list)(void);
};

typedef struct {
    int used;
    struct fs_driver* drv;
    int real_fd;
} vfs_fd_entry_t;

int vfs_open(const char* path);
int vfs_read(int fd, void* buf, size_t count);
int vfs_close(int fd);
int vfs_list();
int vfs_create(const char* path, const uint8_t* data, size_t size);
int vfs_register_driver(struct fs_driver* driver);

#endif
