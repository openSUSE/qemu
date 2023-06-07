/*
 * 9p utilities
 *
 * Copyright IBM, Corp. 2017
 *
 * Authors:
 *  Greg Kurz <groug@kaod.org>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef QEMU_9P_UTIL_H
#define QEMU_9P_UTIL_H

#include "qemu/error-report.h"

static inline void close_preserve_errno(int fd)
{
    int serrno = errno;
    close(fd);
    errno = serrno;
}

/**
 * close_if_special_file() - Close @fd if neither regular file nor directory.
 *
 * @fd: file descriptor of open file
 * Return: 0 on regular file or directory, -1 otherwise
 *
 * CVE-2023-2861: Prohibit opening any special file directly on host
 * (especially device files), as a compromised client could potentially gain
 * access outside exported tree under certain, unsafe setups. We expect
 * client to handle I/O on special files exclusively on guest side.
 */
static inline int close_if_special_file(int fd)
{
    struct stat stbuf;

    if (fstat(fd, &stbuf) < 0) {
        close_preserve_errno(fd);
        return -1;
    }
    if (!S_ISREG(stbuf.st_mode) && !S_ISDIR(stbuf.st_mode)) {
        error_report_once(
            "9p: broken or compromised client detected; attempt to open "
            "special file (i.e. neither regular file, nor directory)"
        );
        close(fd);
        errno = ENXIO;
        return -1;
    }

    return 0;
}

static inline int openat_dir(int dirfd, const char *name)
{
#ifdef O_PATH
#define OPENAT_DIR_O_PATH O_PATH
#else
#define OPENAT_DIR_O_PATH 0
#endif
    return openat(dirfd, name,
                  O_DIRECTORY | O_RDONLY | O_NOFOLLOW | OPENAT_DIR_O_PATH);
}

static inline int openat_file(int dirfd, const char *name, int flags,
                              mode_t mode)
{
    int fd, serrno, ret;

    fd = openat(dirfd, name, flags | O_NOFOLLOW | O_NOCTTY | O_NONBLOCK,
                mode);
    if (fd == -1) {
        return -1;
    }

    if (close_if_special_file(fd) < 0) {
        return -1;
    }

    serrno = errno;
    /* O_NONBLOCK was only needed to open the file. Let's drop it. */
    ret = fcntl(fd, F_SETFL, flags);
    assert(!ret);
    errno = serrno;
    return fd;
}

int relative_openat_nofollow(int dirfd, const char *path, int flags,
                             mode_t mode);
ssize_t fgetxattrat_nofollow(int dirfd, const char *path, const char *name,
                             void *value, size_t size);
int fsetxattrat_nofollow(int dirfd, const char *path, const char *name,
                         void *value, size_t size, int flags);

#endif
