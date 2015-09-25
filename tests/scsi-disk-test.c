/*
 * QTest testcase for SCSI disks
 * See virtio-scsi-test for more integrated tests.
 *
 * Copyright (c) 2015 SUSE Linux GmbH
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include <glib.h>
#include "libqtest.h"
#include "qapi/qmp/qnum.h"
#include "qapi/qmp/qdict.h"

static void test_scsi_disk_common(const char *type, const char *id)
{
    char *cmdline, *path;
    QDict *response;
    QNum *value;

    cmdline = g_strdup_printf(
        "-drive id=drv0,if=none,file=/dev/null,format=raw "
        "-device virtio-scsi-pci,id=scsi0 "
        "-device %s,id=%s,bus=scsi0.0,drive=drv0"
        ",wwn=0x%" PRIx64 ",port_wwn=0x%" PRIx64,
        type, id, UINT64_MAX, UINT64_C(1) << 63);
    qtest_start(cmdline);
    g_free(cmdline);

    path = g_strdup_printf("/machine/peripheral/%s", id);

    response = qmp("{ 'execute': 'qom-get',"
                   "  'arguments': { 'path': %s,"
                   "                 'property': 'wwn' } }",
                   path);
    g_assert(response);
    g_assert(qdict_haskey(response, "return"));
    value = qobject_to(QNum, qdict_get(response, "return"));
    g_assert_cmpint(qnum_get_uint(value), ==, UINT64_MAX);

    response = qmp("{ 'execute': 'qom-get',"
                   "  'arguments': { 'path': %s,"
                   "                 'property': 'port_wwn' } }",
                   path);
    g_assert(response);
    g_assert(qdict_haskey(response, "return"));
    value = qobject_to(QNum, qdict_get(response, "return"));
    g_assert_cmpint(qnum_get_uint(value), ==, UINT64_C(1) << 63);

    g_free(path);
    qtest_end();
}

static void test_scsi_disk(void)
{
    test_scsi_disk_common("scsi-disk", "disk0");
}

static void test_scsi_hd(void)
{
    test_scsi_disk_common("scsi-hd", "hd0");
}

static void test_scsi_cd(void)
{
    test_scsi_disk_common("scsi-cd", "cd0");
}

int main(int argc, char **argv)
{
    int ret;

    g_test_init(&argc, &argv, NULL);
    qtest_add_func("/scsi-disk/props", test_scsi_disk);
    qtest_add_func("/scsi-hd/props", test_scsi_hd);
    qtest_add_func("/scsi-cd/props", test_scsi_cd);

    ret = g_test_run();

    return ret;
}
