/*
 * Copyright (C) 2015 Red Hat, Inc.
 * Copyright (c) 2015 SUSE Linux GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: Daniel P. Berrange <berrange@redhat.com>
 *         Andreas Färber <afaerber@suse.com>
 */

#include "qemu/osdep.h"

#include <glib.h>

#include "qapi/visitor.h"
#include "qom/object.h"
#include "qemu/module.h"


#define TYPE_DUMMY "qemu-dummy"

typedef struct DummyObject DummyObject;
typedef struct DummyObjectClass DummyObjectClass;

#define DUMMY_OBJECT(obj)                               \
    OBJECT_CHECK(DummyObject, (obj), TYPE_DUMMY)

struct DummyObject {
    Object parent_obj;

    uint64_t u64val;
};

struct DummyObjectClass {
    ObjectClass parent_class;
};

static void dummy_set_uint64(Object *obj, Visitor *v,
                             const char *name, void *opaque,
                             Error **errp)
{
    uint64_t *ptr = (uint64_t *)opaque;

    visit_type_uint64(v, name, ptr, errp);
}

static void dummy_get_uint64(Object *obj, Visitor *v,
                             const char *name, void *opaque,
                             Error **errp)
{
    uint64_t value = *(uint64_t *)opaque;

    visit_type_uint64(v, name, &value, errp);
}

static void dummy_init(Object *obj)
{
    DummyObject *dobj = DUMMY_OBJECT(obj);

    object_property_add(obj, "u64val", "uint64",
                              dummy_get_uint64,
                              dummy_set_uint64,
                              NULL, &dobj->u64val, NULL);
}


static const TypeInfo dummy_info = {
    .name          = TYPE_DUMMY,
    .parent        = TYPE_OBJECT,
    .instance_size = sizeof(DummyObject),
    .instance_init = dummy_init,
    .class_size = sizeof(DummyObjectClass),
};

static void test_dummy_uint64(void)
{
    Error *err = NULL;
    char *str;
    DummyObject *dobj = DUMMY_OBJECT(object_new(TYPE_DUMMY));

    g_assert(dobj->u64val == 0);

    str = g_strdup_printf("%" PRIu64, UINT64_MAX);
    object_property_parse(OBJECT(dobj), str, "u64val", &err);
    g_free(str);
    g_assert(!err);
    g_assert_cmpint(dobj->u64val, ==, UINT64_MAX);

    dobj->u64val = 0;
    str = g_strdup_printf("0x%" PRIx64, UINT64_MAX);
    object_property_parse(OBJECT(dobj), str, "u64val", &err);
    g_free(str);
    g_assert(!err);
    g_assert_cmpint(dobj->u64val, ==, UINT64_MAX);

    object_unref(OBJECT(dobj));
}


int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    module_call_init(MODULE_INIT_QOM);
    type_register_static(&dummy_info);

    g_test_add_func("/qom/props/uint64", test_dummy_uint64);

    return g_test_run();
}
