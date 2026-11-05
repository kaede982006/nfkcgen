#include "generator.h"

/* 고전(classic) 외부 문자 집합 — 원본 관찰 문자. 넓히려면 이 문자열만 고치면 된다. */
const char classic_alphabet[] = "⣿㍰█";

/* 신규(modern) 외부 문자 집합 — 현재 요청 표본 기반. */
const char modern_alphabet[] =
    "ᚤ⣵ཇ⡼ฏ⑳㌖ཀ⟁قज⣯ग⡮⯐⢀ش⣀चᚢ⢲ฒ∰ཆघ⡖ฐ⠲ฃ⌬⠔ﷺ⢷ᚣ⣏ظ⢘ሃ⧫⒇⣽دሄख⡅ف⧫ج¼ང⡝ሁཅ⟁⣣⧫صཉ⣻⨌⢨⒇⟁㏒⧫⠪ฅ⣜خሀฎ⡦ฑགﷻ⢺ع⟁⡧ؤ⢿ञ⡔㌌㍰⡭ङ⨌⢝ᚡ⒇ﷺ⠠ᚠئ㈱⠄ཁغ㎯⣳ضฆ⠁छ⧫ฌ⢟⬢कذ⟁ط⟁ሂ⡑झ⢢ح⣩";

gsize
codepoint_length(const char *text)
{
    return (gsize)g_utf8_strlen(text, -1);
}

void
append_random_external(GString *output, GPtrArray *chars, guint count)
{
    for (guint i = 0; i < count; i++) {
        guint index = (guint)g_random_int_range(0, (gint)chars->len);
        const gchar *character = g_ptr_array_index(chars, index);
        g_string_append(output, character);
    }
}

GPtrArray *
build_charset(const char *alphabet)
{
    GPtrArray *chars = g_ptr_array_new_with_free_func(g_free);
    GHashTable *seen = g_hash_table_new(g_str_hash, g_str_equal);
    for (const gchar *p = alphabet; *p; p = g_utf8_next_char(p)) {
        gchar *character = g_strndup(p, g_utf8_next_char(p) - p);
        if (g_hash_table_contains(seen, character)) {
            g_free(character);
        } else {
            g_hash_table_add(seen, character);
            g_ptr_array_add(chars, character);
        }
    }
    g_hash_table_unref(seen);
    return chars;
}

gboolean
build_string(guint klass, GPtrArray *chars, guint target_length,
             GPtrArray *phrases, GString *output, gchar **error_message)
{
    gsize general_length = 0;
    for (guint i = 0; i < phrases->len; i++)
        general_length += codepoint_length(g_ptr_array_index(phrases, i));

    if (klass == 1) {
        append_random_external(output, chars, target_length);
        return TRUE;
    }

    if ((klass == 2 || klass == 3) && phrases->len != 1) {
        *error_message = g_strdup_printf(
            "%u류는 비어 있지 않은 일반 문자열을 정확히 한 줄 입력해야 합니다.", klass);
        return FALSE;
    }

    if (klass == 4 && phrases->len < 2) {
        *error_message = g_strdup("4류는 서로 다른 일반 문자열을 두 줄 이상 입력해야 합니다.");
        return FALSE;
    }

    if (general_length >= target_length) {
        *error_message = g_strdup_printf(
            "일반 문자열의 총 길이(%zu)가 목표 길이(%u)보다 작아야 합니다.",
            general_length, target_length);
        return FALSE;
    }

    guint external_length = target_length - (guint)general_length;

    if (klass == 2) {   /* 앞에 일반 문자열 */
        g_string_append(output, g_ptr_array_index(phrases, 0));
        append_random_external(output, chars, external_length);
        return TRUE;
    }

    if (klass == 3) {   /* 뒤에 일반 문자열 */
        append_random_external(output, chars, external_length);
        g_string_append(output, g_ptr_array_index(phrases, 0));
        return TRUE;
    }

    /* 4류: 기존 3류 알고리즘. 각 문장을 외부 문자로 감싼다. */
    guint segments = phrases->len + 1;
    if (external_length < segments) {
        *error_message = g_strdup_printf(
            "4류는 일반 문자열 %u개 사이와 양 끝에 외부 문자가 필요합니다. 현재 남은 외부 문자 수는 %u개입니다.",
            phrases->len, external_length);
        return FALSE;
    }

    guint *segment_lengths = g_new(guint, segments);
    for (guint i = 0; i < segments; i++)
        segment_lengths[i] = 1;
    for (guint i = segments; i < external_length; i++)
        segment_lengths[g_random_int_range(0, (gint)segments)]++;

    for (guint i = 0; i < phrases->len; i++) {
        append_random_external(output, chars, segment_lengths[i]);
        g_string_append(output, g_ptr_array_index(phrases, i));
    }
    append_random_external(output, chars, segment_lengths[segments - 1]);
    g_free(segment_lengths);
    return TRUE;
}
