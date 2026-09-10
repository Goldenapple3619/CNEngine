#include <criterion/criterion.h>
#include <string.h>
#include "libcncore.h"

/* ============================================================
 * new_object / delete_object
 * ============================================================ */

Test(new_object, returns_non_null)
{
    Object *obj = new_object();

    cr_assert_not_null(obj);
    delete_object(obj);
}

Test(new_object, initial_state_is_clean)
{
    Object *obj = new_object();

    cr_assert_not_null(obj);
    cr_assert_null(obj->base, "fresh object should have no base");
    cr_assert_eq(obj->ref_count, 0, "fresh object should start at ref_count 0");

    delete_object(obj);
}

Test(delete_object, null_input_does_not_crash)
{
    delete_object(NULL);
}

Test(delete_object, deletes_base_chain)
{
    Object *base = new_object();
    Object *child = new_object();

    cr_assert_not_null(base);
    cr_assert_not_null(child);

    child->base = base;

    /* Passes if this doesn't double-free / segfault under ASan */
    delete_object(child);
}

static int del_called = 0;

static cn_value spy_del(Object *self, void **args)
{
    (void)self; (void)args;
    del_called = 1;
    return VALUE_NULL;
}

Test(delete_object, invokes_del_method_if_present)
{
    Object *obj = new_object();

    del_called = 0;
    cr_assert(set_method(obj, "_del", spy_del));

    delete_object(obj);

    cr_assert_eq(del_called, 1, "_del should be called before the object is freed");
}

/* ============================================================
 * set_attr / get_attr / has_attr
 * ============================================================ */

Test(attrs, set_attr_null_object_returns_false)
{
    cr_assert_not(set_attr(NULL, "name", CN_TYPE_STRING, (cnany)"x"));
}

Test(attrs, set_attr_null_name_returns_false)
{
    Object *obj = new_object();

    cr_assert_not(set_attr(obj, NULL, CN_TYPE_STRING, (cnany)"x"));
    delete_object(obj);
}

Test(attrs, set_then_get_string_attr)
{
    Object *obj = new_object();

    cr_assert(set_attr(obj, "name", CN_TYPE_STRING, (cnany)"hello"));

    cn_value *val = get_attr(obj, "name");
    cr_assert_not_null(val);
    cr_assert_eq(val->type, CN_TYPE_STRING);
    cr_assert_str_eq(val->as.str, "hello");

    delete_object(obj);
}

Test(attrs, get_attr_missing_returns_null)
{
    Object *obj = new_object();

    cr_assert_null(get_attr(obj, "nope"));
    delete_object(obj);
}

Test(attrs, has_attr_reflects_state)
{
    Object *obj = new_object();

    cr_assert_not(has_attr(obj, "name"));
    cr_assert(set_attr(obj, "name", CN_TYPE_STRING, (cnany)"x"));
    cr_assert(has_attr(obj, "name"));

    delete_object(obj);
}

Test(attrs, has_attr_null_checks)
{
    Object *obj = new_object();

    cr_assert_not(has_attr(NULL, "name"));
    cr_assert_not(has_attr(obj, NULL));

    delete_object(obj);
}

Test(attrs, set_attr_overwrites_existing_value)
{
    Object *obj = new_object();

    cr_assert(set_attr(obj, "name", CN_TYPE_STRING, (cnany)"first"));
    cr_assert(set_attr(obj, "name", CN_TYPE_STRING, (cnany)"second"));

    cn_value *val = get_attr(obj, "name");
    cr_assert_str_eq(val->as.str, "second");

    delete_object(obj);
}

Test(attrs, get_attr_walks_base_chain)
{
    Object *base = new_object();
    Object *child = new_object();

    cr_assert(set_attr(base, "inherited", CN_TYPE_STRING, (cnany)"from_base"));
    child->base = base;

    cn_value *val = get_attr(child, "inherited");
    cr_assert_not_null(val, "child should inherit attrs via base");
    cr_assert_str_eq(val->as.str, "from_base");

    delete_object(child); /* frees base too, since delete_object recurses on ->base */
}

/* ============================================================
 * set_method / get_method / get_method_holder / has_method / call_method
 * ============================================================ */

static cn_value fake_method(Object *self, void **args)
{
    (void)self; (void)args;
    cn_value v;
    v.type = CN_TYPE_STRING;
    v.as.str = "called";
    return v;
}

Test(methods, set_method_null_checks)
{
    Object *obj = new_object();

    cr_assert_not(set_method(NULL, "foo", fake_method));
    cr_assert_not(set_method(obj, NULL, fake_method));
    cr_assert_not(set_method(obj, "foo", NULL));

    delete_object(obj);
}

Test(methods, set_and_has_method)
{
    Object *obj = new_object();

    cr_assert_not(has_method(obj, "foo"));
    cr_assert(set_method(obj, "foo", fake_method));
    cr_assert(has_method(obj, "foo"));

    delete_object(obj);
}

Test(methods, call_method_invokes_function)
{
    Object *obj = new_object();

    cr_assert(set_method(obj, "foo", fake_method));

    cn_value result = call_method(obj, "foo", NULL);
    cr_assert_eq(result.type, CN_TYPE_STRING);
    cr_assert_str_eq(result.as.str, "called");

    delete_object(obj);
}

Test(methods, call_method_missing_returns_value_null)
{
    Object *obj = new_object();

    cn_value result = call_method(obj, "nope", NULL);
    cr_assert_eq(result.type, (VALUE_NULL.type));

    delete_object(obj);
}

Test(methods, get_method_walks_base_chain)
{
    Object *base = new_object();
    Object *child = new_object();

    cr_assert(set_method(base, "foo", fake_method));
    child->base = base;

    cr_assert(has_method(child, "foo"));

    cn_value result = call_method(child, "foo", NULL);
    cr_assert_str_eq(result.as.str, "called");

    delete_object(child);
}

Test(methods, get_method_holder_does_not_walk_base_chain)
{
    Object *base = new_object();
    Object *child = new_object();

    cr_assert(set_method(base, "foo", fake_method));
    child->base = base;

    /* unlike get_method / has_method, get_method_holder only checks
     * the object's OWN methods table, not object->base */
    cr_assert_null(get_method_holder(child, "foo"));
    cr_assert_not_null(get_method_holder(base, "foo"));

    delete_object(child);
}

/* ============================================================
 * create_default_object
 * ============================================================ */

Test(default_object, has_name_attr)
{
    Object *obj = create_default_object();

    cn_value *name = get_attr(obj, "name");
    cr_assert_not_null(name);
    cr_assert_str_eq(name->as.str, "object");

    delete_object(obj);
}

Test(default_object, has_lifecycle_methods)
{
    Object *obj = create_default_object();

    cr_assert(has_method(obj, "_init"));
    cr_assert(has_method(obj, "_str"));
    cr_assert(has_method(obj, "_del"));

    delete_object(obj);
}

/* ============================================================
 * print_object
 * ============================================================ */

Test(print_object, null_object_does_not_crash)
{
    print_object(NULL);
}

Test(print_object, object_without_str_method_does_not_crash)
{
    Object *obj = new_object(); /* no _str registered */

    print_object(obj);

    delete_object(obj);
}

Test(print_object, default_object_does_not_crash)
{
    Object *obj = create_default_object();

    print_object(obj);

    delete_object(obj);
}

/* ============================================================
 * build_object
 * ============================================================ */

static cn_value init_ok(Object *self, void **args)
{
    (void)self; (void)args;
    return VALUE_OK;
}

static cn_value init_fail(Object *self, void **args)
{
    (void)self; (void)args;
    return VALUE_ERR;
}

Test(build_object, null_object_returns_null)
{
    cr_assert_null(build_object(NULL, NULL));
}

Test(build_object, succeeds_when_init_returns_ok)
{
    Object *obj = new_object();
    cr_assert(set_method(obj, "_init", init_ok));

    Object *result = build_object(obj, NULL);
    cr_assert_eq(result, obj);

    delete_object(result);
}

Test(build_object, fails_and_frees_object_when_init_returns_error)
{
    Object *obj = new_object();
    cr_assert(set_method(obj, "_init", init_fail));

    Object *result = build_object(obj, NULL);
    cr_assert_null(result);
    /* obj is already freed here by build_object — don't touch it */
}

Test(build_object, default_object_init_is_rejected)
{
    /* create_default_object's stock _init returns VALUE_NULL, and
     * build_object treats a NULL-typed return as invalid. Worth
     * confirming this is actually the intended contract. */
    Object *obj = create_default_object();

    Object *result = build_object(obj, NULL);
    cr_assert_null(result);
}