/* SPDX-License-Identifier: MIT */
/*
 * util.c (mostly based on QEMU os-win32.c)
 *
 * Copyright (c) 2003-2008 Fabrice Bellard
 * Copyright (c) 2010-2016 Red Hat, Inc.
 *
 * QEMU library functions for win32 which are shared between QEMU and
 * the QEMU tools.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "util.h"

#include <glib.h>
#include <fcntl.h>
#include <stdint.h>

static int slirp_vsnprintf(char *str, size_t size,
                           const char *format, va_list args)
{
    int rv = g_vsnprintf(str, size, format, args);

    if (rv < 0) {
        g_error("g_vsnprintf() failed: %s", g_strerror(errno));
    }

    return rv;
}

/*
 * A snprintf()-like function that:
 * - returns the number of bytes written (excluding optional \0-ending)
 * - dies on error
 * - warn on truncation
 */
int slirp_fmt(char *str, size_t size, const char *format, ...)
{
    va_list args;
    int rv;

    va_start(args, format);
    rv = slirp_vsnprintf(str, size, format, args);
    va_end(args);

    if (rv > size) {
        g_critical("slirp_fmt() truncation");
    }

    return MIN(rv, size);
}

/*
 * A snprintf()-like function that:
 * - always \0-end (unless size == 0)
 * - returns the number of bytes actually written, including \0 ending
 * - dies on error
 * - warn on truncation
 */
int slirp_fmt0(char *str, size_t size, const char *format, ...)
{
    va_list args;
    int rv;

    va_start(args, format);
    rv = slirp_vsnprintf(str, size, format, args);
    va_end(args);

    if (rv >= size) {
        g_critical("slirp_fmt0() truncation");
        if (size > 0)
            str[size - 1] = '\0';
        rv = size;
    } else {
        rv += 1; /* include \0 */
    }

    return rv;
}
