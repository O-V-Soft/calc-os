#include <stdint.h>
#include <fat.h>
#include <vfs.h>
#include <utils.h>
#include <video.h>

static struct fs_driver* current_driver = 0;
static mount_t mounts[8];
static int mount_count = 0;
static vfs_fd_t vfs_fd_table[32];
static char mount_paths[8][64];

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
    const char* rel;
    struct fs_driver* drv = vfs_find_driver(path, &rel);
    if (!drv) return -1;

    int real_fd = drv->open(rel);
    if (real_fd < 0) return -1;

    int fd = vfs_alloc_fd();
    if (fd < 0) return -1;

    vfs_fd_table[fd].drv = drv;
    vfs_fd_table[fd].real_fd = real_fd;

    return fd;
}

int vfs_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= 32) return -1;
    if (!vfs_fd_table[fd].used) return -1;

    struct fs_driver* drv = vfs_fd_table[fd].drv;
    int real_fd = vfs_fd_table[fd].real_fd;

    return drv->read(real_fd, buf, count);
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= 32) return -1;
    if (!vfs_fd_table[fd].used) return -1;

    struct fs_driver* drv = vfs_fd_table[fd].drv;
    int real_fd = vfs_fd_table[fd].real_fd;

    int result = drv->close(real_fd);

    vfs_fd_table[fd].used = 0;
    vfs_fd_table[fd].drv = 0;
    vfs_fd_table[fd].real_fd = -1;

    return result;
}

int vfs_list(const char* path) {
    const char* rel;
    struct fs_driver* drv = vfs_find_driver(path, &rel);
    if (!drv || !drv->list) return -1;
    return drv->list();
}

void list_mounts() {
    for (int i = 0; i < mount_count; i++) {
        print(mounts[i].path, COLOR_WHITE);
        printk(":\n", COLOR_WHITE);

        if (mounts[i].driver->list) {
            mounts[i].driver->list_basic();   
        }
    }
}

int vfs_create(const char* path, const uint8_t* data, size_t size) {
    const char* rel;
    struct fs_driver* drv = vfs_find_driver(path, &rel);
    if (!drv || !drv->create) return -1;
    return drv->create(rel, data, size);
}

int vfs_mount(const char* path, struct fs_driver* driver) {
    if (mount_count >= 8) return -1; 

    int i = 0;
    while (path[i] != '\0' && i < 63) {
        mount_paths[mount_count][i] = path[i];
        i++;
    }
    mount_paths[mount_count][i] = '\0'; 

    mounts[mount_count].path = mount_paths[mount_count];
    mounts[mount_count].driver = driver;
    mount_count++;
    
    return 0;
}

int vfs_umount(const char* path) {
    for (int i = 0; i < mount_count; i++) {
        if (strcmp(mounts[i].path, path) == 0) {
            if (mounts[i].driver && mounts[i].driver->umount) {
                mounts[i].driver->umount();
            }

            for (int j = i; j < mount_count - 1; j++) {
                int k = 0;
                while (mount_paths[j + 1][k] != '\0' && k < 63) {
                    mount_paths[j][k] = mount_paths[j + 1][k];
                    k++;
                }
                mount_paths[j][k] = '\0';

                mounts[j].driver = mounts[j + 1].driver;
                mounts[j].path = mount_paths[j];
            }

            for (int k = 0; k < 64; k++) {
                mount_paths[mount_count - 1][k] = 0;
            }
            mounts[mount_count - 1].driver = 0;
            mounts[mount_count - 1].path = 0;

            mount_count--;
            return 0;
        }
    }
    return -1;
}

static struct fs_driver* vfs_find_driver(const char* path, const char** rel) {
    int best_match_idx = -1;
    int best_match_len = -1;

    for (int i = 0; i < mount_count; i++) {
        const char* mp = mounts[i].path;
        int len = strlen(mp);

        if (strcmp(mp, "/") == 0) {
            if (best_match_len < 1) {
                best_match_idx = i;
                best_match_len = 1;
            }
            continue;
        }

        if (strncmp(path, mp, len) == 0) {
            if (path[len] == '\0' || path[len] == '/') {
                if (len > best_match_len) {
                    best_match_idx = i;
                    best_match_len = len;
                }
            }
        }
    }

    if (best_match_idx != -1) {
        const char* mp = mounts[best_match_idx].path;
        int len = strlen(mp);
        const char* r = path + len;
        
        if (*r == '/') r++;
        *rel = r;
        
        return mounts[best_match_idx].driver;
    }

    return 0;
}
