/* build_string 분기 구조 검증 (GTK UI 없이 glib만으로 실행). make selftest */
#include "generator.h"
#include <string.h>

static GPtrArray *
phrases_of(const char *a, const char *b)
{
    GPtrArray *p = g_ptr_array_new_with_free_func(g_free);
    if (a) g_ptr_array_add(p, g_strdup(a));
    if (b) g_ptr_array_add(p, g_strdup(b));
    return p;
}

static gchar *
gen(guint klass, GPtrArray *chars, guint target, GPtrArray *phrases)
{
    GString *out = g_string_new(NULL);
    gchar *err = NULL;
    g_assert_true(build_string(klass, chars, target, phrases, out, &err));
    g_assert_null(err);
    return g_string_free(out, FALSE);
}

int
main(void)
{
    GPtrArray *cls = build_charset(classic_alphabet);
    GPtrArray *mod = build_charset(modern_alphabet);
    g_assert_cmpuint(cls->len, ==, 3);  /* ⣿ ㍰ █ */

    /* 1류: 외부 문자만, 길이 == target */
    GPtrArray *none = phrases_of(NULL, NULL);
    gchar *s1 = gen(1, cls, 40, none);
    g_assert_cmpuint(g_utf8_strlen(s1, -1), ==, 40);

    /* 2류: 앞에 phrase, 길이 == target */
    GPtrArray *one = phrases_of("안녕", NULL);
    gchar *s2 = gen(2, mod, 30, one);
    g_assert_true(g_str_has_prefix(s2, "안녕"));
    g_assert_cmpuint(g_utf8_strlen(s2, -1), ==, 30);

    /* 3류: 뒤에 phrase, 길이 == target */
    gchar *s3 = gen(3, mod, 30, one);
    g_assert_true(g_str_has_suffix(s3, "안녕"));
    g_assert_cmpuint(g_utf8_strlen(s3, -1), ==, 30);

    /* 4류: 두 phrase가 순서대로 들어가고 외부 문자로 감싸임, 길이 == target */
    GPtrArray *two = phrases_of("가", "나");
    gchar *s4 = gen(4, cls, 20, two);
    g_assert_cmpuint(g_utf8_strlen(s4, -1), ==, 20);
    const char *pa = strstr(s4, "가"), *pb = strstr(s4, "나");
    g_assert_nonnull(pa); g_assert_nonnull(pb); g_assert_true(pa < pb);
    g_assert_false(g_str_has_prefix(s4, "가"));   /* 앞이 외부 문자로 감싸임 */
    g_assert_false(g_str_has_suffix(s4, "나"));   /* 뒤가 외부 문자로 감싸임 */

    /* 오류 경로 */
    GString *o = g_string_new(NULL); gchar *e = NULL;
    g_assert_false(build_string(2, mod, 30, none, o, &e)); g_clear_pointer(&e, g_free);
    g_assert_false(build_string(4, mod, 30, one, o, &e)); g_clear_pointer(&e, g_free);
    g_assert_false(build_string(2, mod, 1, one, o, &e));  g_clear_pointer(&e, g_free);
    g_string_free(o, TRUE);

    g_free(s1); g_free(s2); g_free(s3); g_free(s4);
    g_ptr_array_unref(none); g_ptr_array_unref(one); g_ptr_array_unref(two);
    g_ptr_array_unref(cls); g_ptr_array_unref(mod);
    g_print("selftest OK\n");
    return 0;
}
