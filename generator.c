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

/* NFKC로 한 문자가 늘어나는 코드 포인트 수(최소 1). 텍스트 확장 휴리스틱이며 실제 음량·발화 시간은 아니다. */
static guint
nfkc_expansion(const gchar *character)
{
    gchar *normalized = g_utf8_normalize(character, -1, G_NORMALIZE_NFKC);
    glong length = normalized ? g_utf8_strlen(normalized, -1) : 1;
    g_free(normalized);
    return length > 0 ? (guint)length : 1;
}

GPtrArray *
build_charset(const char *alphabet)
{
    GPtrArray *chars = g_ptr_array_new_with_free_func(g_free);
    if (!alphabet || !g_utf8_validate(alphabet, -1, NULL))
        return chars;
    GHashTable *seen = g_hash_table_new(g_str_hash, g_str_equal);
    for (const gchar *p = alphabet; *p; p = g_utf8_next_char(p)) {
        gchar *character = g_strndup(p, g_utf8_next_char(p) - p);
        if (g_hash_table_contains(seen, character)) {
            g_free(character);
        } else {
            g_hash_table_add(seen, character);
            /* 확장 가중치는 탐색 초기값일 뿐, 음향 효과는 실측으로 평가한다. */
            guint copies = nfkc_expansion(character);
            g_ptr_array_add(chars, character);
            for (guint k = 1; k < copies; k++)
                g_ptr_array_add(chars, g_strdup(character));
        }
    }
    g_hash_table_unref(seen);
    return chars;
}

static void
append_slice(GString *output, const gchar **cursor, guint count)
{
    const gchar *end = g_utf8_offset_to_pointer(*cursor, count);
    g_string_append_len(output, *cursor, end - *cursor);
    *cursor = end;
}

gboolean
build_candidate(guint klass, GPtrArray *chars, guint target_length,
                GPtrArray *phrases, const gchar *parent, guint32 seed,
                GString *output, GString *external, gchar **error_message)
{
    *error_message = NULL;
    g_string_truncate(output, 0);
    g_string_truncate(external, 0);
    if (klass < 1 || klass > 4 || !chars || !chars->len || !phrases ||
        target_length < 20 || target_length > 120) {
        *error_message = g_strdup("유형(1~4), 문자 집합, 목표 길이(20~120)를 확인하세요.");
        return FALSE;
    }
    gsize general_length = 0;
    if (klass != 1) {
        for (guint i = 0; i < phrases->len; i++) {
            const gchar *phrase = g_ptr_array_index(phrases, i);
            if (!phrase || !*phrase || !g_utf8_validate(phrase, -1, NULL)) {
                *error_message = g_strdup("일반 문자열은 비어 있지 않은 UTF-8이어야 합니다.");
                return FALSE;
            }
            general_length += codepoint_length(phrase);
        }
        if (((klass == 2 || klass == 3) && phrases->len != 1) ||
            (klass == 4 && phrases->len < 2)) {
            *error_message = g_strdup("2·3류는 한 줄, 4류는 두 줄 이상의 일반 문자열이 필요합니다.");
            return FALSE;
        }
        if (general_length >= target_length ||
            (klass == 4 && target_length - general_length < phrases->len + 1)) {
            *error_message = g_strdup("일반 문자열 사이와 양 끝에 외부 문자를 넣을 길이가 부족합니다.");
            return FALSE;
        }
    }
    guint count = target_length - general_length;
    if (parent && (!g_utf8_validate(parent, -1, NULL) || codepoint_length(parent) != count)) {
        *error_message = g_strdup("부모 후보의 외부 문자 길이가 현재 설정과 다릅니다.");
        return FALSE;
    }
    if (parent) {
        for (const gchar *c = parent; *c; c = g_utf8_next_char(c)) {
            gboolean found = FALSE;
            for (guint j = 0; j < chars->len; j++)
                if (g_utf8_get_char(c) == g_utf8_get_char(g_ptr_array_index(chars, j))) {
                    found = TRUE;
                    break;
                }
            if (!found) {
                *error_message = g_strdup("부모 후보에 현재 문자 집합 밖의 문자가 있습니다.");
                return FALSE;
            }
        }
    }
    GRand *rng = g_rand_new_with_seed(seed);
    const gchar *c = parent;
    for (guint i = 0; i < count; i++) {
        /* shortcut: 20% independent mutation; tune only after controlled repeat trials. */
        if (c && g_rand_int_range(rng, 0, 5) != 0)
            g_string_append_len(external, c, g_utf8_next_char(c) - c);
        else
            g_string_append(external, g_ptr_array_index(chars, g_rand_int_range(rng, 0, chars->len)));
        if (c)
            c = g_utf8_next_char(c);
    }
    const gchar *cursor = external->str;
    if (klass == 1)
        g_string_append(output, external->str);
    else if (klass == 2) {
        g_string_append(output, g_ptr_array_index(phrases, 0));
        g_string_append(output, external->str);
    } else if (klass == 3) {
        g_string_append(output, external->str);
        g_string_append(output, g_ptr_array_index(phrases, 0));
    } else {
        guint segments = phrases->len + 1;
        guint *lengths = g_new(guint, segments);
        for (guint i = 0; i < segments; i++)
            lengths[i] = 1;
        for (guint i = segments; i < count; i++)
            lengths[g_rand_int_range(rng, 0, segments)]++;
        for (guint i = 0; i < phrases->len; i++) {
            append_slice(output, &cursor, lengths[i]);
            g_string_append(output, g_ptr_array_index(phrases, i));
        }
        append_slice(output, &cursor, lengths[segments - 1]);
        g_free(lengths);
    }
    g_rand_free(rng);
    return TRUE;
}

gboolean
build_string(guint klass, GPtrArray *chars, guint target_length,
             GPtrArray *phrases, GString *output, gchar **error_message)
{
    GString *external = g_string_new(NULL);
    gboolean ok = build_candidate(klass, chars, target_length, phrases, NULL,
                                  g_random_int(), output, external, error_message);
    g_string_free(external, TRUE);
    return ok;
}
