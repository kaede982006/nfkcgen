#include "generator.h"
#include <math.h>
#include <string.h>

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
                g_assert_true(g_utf8_validate(out->str, -1, NULL));
                g_assert_cmpuint(codepoint_length(out->str), ==, 120);
                if (klass == 2) g_assert_true(g_str_has_prefix(out->str, "한글🙂"));
                if (klass == 3) g_assert_true(g_str_has_suffix(out->str, "한글🙂"));
                if (klass == 4) {
                    const gchar *first = strstr(out->str, "한글🙂"), *second = strstr(out->str, "두 번째");
                    g_assert_nonnull(first); g_assert_nonnull(second);
                    g_assert_true(first > out->str && second > first + strlen("한글🙂"));
                    g_assert_true(*(second + strlen("두 번째")) != '\0');
                }
                gchar *parent = g_strdup(external->str);
                g_assert_true(build_candidate(klass, chars, 120, phrases, parent, seed, out, external, &error));
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
static void scoring(void)
{
    g_assert_cmpfloat_with_epsilon(trial_value(-20, 10), .1, 1e-12);
    g_assert_cmpfloat_with_epsilon(trial_value(-20, 20), 2 * trial_value(-20, 10), 1e-12);
    g_assert_cmpfloat_with_epsilon(trial_value(-10, 10), 10 * trial_value(-20, 10), 1e-12);
    g_assert_cmpfloat(trial_value(-20, 0), ==, 0);
    g_assert_cmpfloat(trial_value(NAN, 10), ==, -1);
    g_assert_cmpfloat(trial_value(-20, INFINITY), ==, -1);
    g_assert_cmpfloat(trial_value(-101, 10), ==, -1);
    g_assert_cmpfloat(trial_value(-20, -1), ==, -1);
}
int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/generator/classes-mutation-validation", candidates);
    g_test_add_func("/research/scoring", scoring);
    return g_test_run();
}
