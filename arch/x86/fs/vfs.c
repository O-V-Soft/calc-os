#include <stdint.h>
#include <utils.h>
#include <video.h>
#include <fat.h>
#include <vfs.h>

struct fs_driver* root_driver = 0;
vfs_fd_entry_t vfs_fd_table[32];

int vfs_register_driver(struct fs_driver* driver) {
    if (!driver) return -1;
    root_driver = driver;
    return 0;
}

int vfs_alloc_fd() {
    for (int i = 0; i < 32; i++) {
        if (!vfs_fd_table[i].used) {
            vfs_fd_table[i].used = 1;
            return i;
        }
    }
    return -1; 
}

int vfs_open(const char* path) {
    if (!root_driver || !path) return -1;

    int real_fd = root_driver->open(path);
    if (real_fd < 0) return -1;

    int fd = vfs_alloc_fd();
    if (fd < 0) {
        root_driver->close(real_fd);
        return -1;
    }

    vfs_fd_table[fd].drv = root_driver;
    vfs_fd_table[fd].real_fd = real_fd;

    return fd;
}

int vfs_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= 32 || !vfs_fd_table[fd].used) return -1;

    struct fs_driver* drv = vfs_fd_table[fd].drv;
    int real_fd = vfs_fd_table[fd].real_fd;

    return drv->read(real_fd, buf, count);
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= 32 || !vfs_fd_table[fd].used) return -1;

    struct fs_driver* drv = vfs_fd_table[fd].drv;
    int real_fd = vfs_fd_table[fd].real_fd;

    int result = drv->close(real_fd);

    vfs_fd_table[fd].used = 0;
    vfs_fd_table[fd].drv = 0;
    vfs_fd_table[fd].real_fd = -1;

    return result;
}

int vfs_create(const char* path, const uint8_t* data, size_t size) {
    if (!root_driver || !root_driver->create) return -1;
    return root_driver->create(path, data, size);
}

int vfs_list() {
    if (!root_driver || !root_driver->list) return -1;
    return root_driver->list();
}
