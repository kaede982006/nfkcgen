#include "generator.h"
#include <string.h>

static void assert_spacing(const gchar *text)
{
    g_assert_true(g_regex_match_simple(
        "^(?:[^⇿]{3,4}(?:⇿⣿|⣿⇿))*[^⇿]{0,5}$", text, 0, 0));
}

static void symbols_only(void)
{
    const gchar *alphabets[] = {classic_alphabet, modern_alphabet};
    for (guint i = 0; i < G_N_ELEMENTS(alphabets); i++) {
        gchar *normalized = g_utf8_normalize(alphabets[i], -1, G_NORMALIZE_NFKC);
        for (const gchar *p = normalized; *p; p = g_utf8_next_char(p)) {
            g_assert_false(g_unichar_isalpha(g_utf8_get_char(p)));
            g_assert_false(g_unichar_ismark(g_utf8_get_char(p)));
        }
        g_free(normalized);
    }
}

static void spacing(void)
{
    GHashTable *seen = g_hash_table_new(g_direct_hash, g_direct_equal);
    for (const gchar *p = modern_alphabet; *p; p = g_utf8_next_char(p))
        g_assert_true(g_hash_table_add(seen, GUINT_TO_POINTER(g_utf8_get_char(p))));
    g_hash_table_unref(seen);
    GPtrArray *chars = build_charset(modern_alphabet), *phrases = g_ptr_array_new();
    GString *out = g_string_new(NULL), *external = g_string_new(NULL);
    gchar *error = NULL;
    for (guint length = 20; length <= 120; length++) {
        for (guint seed = 0; seed < 100; seed++) {
            g_assert_true(build_candidate(1, chars, length, phrases, NULL, seed, out, external, &error));
            g_assert_cmpuint(codepoint_length(out->str), ==, length);
            assert_spacing(external->str);
            gchar *parent = g_strdup(external->str);
            g_assert_true(build_candidate(1, chars, length, phrases, parent, seed, out, external, &error));
            assert_spacing(external->str);
            g_free(parent);
        }
    }
    /* One available external slot must not increase the requested length. */
    g_ptr_array_add(phrases, "1234567890123456789");
    g_assert_true(build_candidate(2, chars, 20, phrases, NULL, 1, out, external, &error));
    g_assert_cmpuint(codepoint_length(out->str), ==, 20);
    assert_spacing(external->str);
    g_ptr_array_unref(chars);
    chars = build_charset("⇿");
    g_assert_false(build_candidate(1, chars, 20, phrases, NULL, 1, out, external, &error));
    g_assert_nonnull(error);
    g_free(error);
    g_ptr_array_unref(chars); g_ptr_array_unref(phrases);
    g_string_free(out, TRUE); g_string_free(external, TRUE);
}

static void candidates(void)
{
    const char *alphabets[] = {classic_alphabet, modern_alphabet};
    for (guint a = 0; a < 2; a++) {
        GPtrArray *chars = build_charset(alphabets[a]);
        GPtrArray *phrases = g_ptr_array_new();
        GString *out = g_string_new(NULL), *external = g_string_new(NULL);
        gchar *error = NULL;
        for (guint klass = 1; klass <= 4; klass++) {
            g_ptr_array_set_size(phrases, 0);
            if (klass != 1) g_ptr_array_add(phrases, "한글🙂");
            if (klass == 4) g_ptr_array_add(phrases, "두 번째");
            for (guint seed = 0; seed < 25; seed++) {
                g_assert_true(build_candidate(klass, chars, 120, phrases, NULL, seed, out, external, &error));
                g_assert_null(error);
                if (a == 1 && klass != 4) assert_spacing(external->str);
                g_assert_true(g_utf8_validate(out->str, -1, NULL));
                g_assert_cmpuint(codepoint_length(out->str), ==, 120);
                if (klass == 2) g_assert_true(g_str_has_prefix(out->str, "한글🙂"));
                if (klass == 3) g_assert_true(g_str_has_suffix(out->str, "한글🙂"));
                if (klass == 4) {
                    const gchar *first = strstr(out->str, "한글🙂"), *second = strstr(out->str, "두 번째");
                    g_assert_nonnull(first); g_assert_nonnull(second);
                    g_assert_true(first > out->str && second > first + strlen("한글🙂"));
                    g_assert_true(*(second + strlen("두 번째")) != '\0');
                    if (a == 1) {
                        gchar *left = g_strndup(out->str, first - out->str);
                        const gchar *middle_start = first + strlen("한글🙂");
                        gchar *middle = g_strndup(middle_start, second - middle_start);
                        assert_spacing(left); assert_spacing(middle);
                        assert_spacing(second + strlen("두 번째"));
                        g_free(left); g_free(middle);
                    }
                }
                gchar *parent = g_strdup(external->str);
                g_assert_true(build_candidate(klass, chars, 120, phrases, parent, seed, out, external, &error));
                if (a == 1 && klass != 4) assert_spacing(external->str);
                gchar *expected = g_strdup(out->str);
                g_assert_cmpuint(codepoint_length(out->str), ==, 120);
                g_assert_true(build_candidate(klass, chars, 120, phrases, parent, seed, out, external, &error));
                g_assert_cmpstr(out->str, ==, expected);
                g_free(expected); g_free(parent);
            }
        }
        g_assert_false(build_candidate(0, chars, 120, phrases, NULL, 1, out, external, &error));
        g_clear_pointer(&error, g_free);
        g_assert_false(build_candidate(1, chars, 121, phrases, NULL, 1, out, external, &error));
        g_clear_pointer(&error, g_free);
        g_assert_false(build_candidate(1, chars, 120, phrases, "bad", 1, out, external, &error));
        g_clear_pointer(&error, g_free);
        gchar *foreign = g_strnfill(120, 'a');
        g_assert_false(build_candidate(1, chars, 120, phrases, foreign, 1, out, external, &error));
        g_free(foreign); g_clear_pointer(&error, g_free);
        g_ptr_array_set_size(phrases, 0);
        g_assert_false(build_string(2, chars, 120, phrases, out, &error));
        g_clear_pointer(&error, g_free);
        g_ptr_array_add(phrases, "1234567890123456789");
        g_ptr_array_add(phrases, "a");
        g_assert_false(build_string(4, chars, 20, phrases, out, &error));
        g_clear_pointer(&error, g_free);
        g_assert_true(build_string(1, chars, 20, phrases, out, &error));
        g_assert_cmpuint(codepoint_length(out->str), ==, 20);
        g_string_free(out, TRUE); g_string_free(external, TRUE);
        g_ptr_array_unref(phrases); g_ptr_array_unref(chars);
    }
}
int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/generator/classes-mutation-validation", candidates);
    g_test_add_func("/generator/modern-spacing", spacing);
    g_test_add_func("/generator/symbols-only", symbols_only);
    return g_test_run();
}
