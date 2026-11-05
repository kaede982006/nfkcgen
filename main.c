#include <gtk/gtk.h>
#include <string.h>

/* 고전(classic) 외부 문자 집합 — 원본 관찰 문자. 넓히려면 이 문자열만 고치면 된다. */
static const char classic_alphabet[] = "⣿㍰█";

/* 신규(modern) 외부 문자 집합 — 현재 요청 표본 기반. */
static const char modern_alphabet[] =
    "ᚤ⣵ཇ⡼ฏ⑳㌖ཀ⟁قज⣯ग⡮⯐⢀ش⣀चᚢ⢲ฒ∰ཆघ⡖ฐ⠲ฃ⌬⠔ﷺ⢷ᚣ⣏ظ⢘ሃ⧫⒇⣽دሄख⡅ف⧫ج¼ང⡝ሁཅ⟁⣣⧫صཉ⣻⨌⢨⒇⟁㏒⧫⠪ฅ⣜خሀฎ⡦ฑགﷻ⢺ع⟁⡧ؤ⢿ञ⡔㌌㍰⡭ङ⨌⢝ᚡ⒇ﷺ⠠ᚠئ㈱⠄ཁغ㎯⣳ضฆ⠁छ⧫ฌ⢟⬢कذ⟁ط⟁ሂ⡑झ⢢ح⣩";

static GtkWidget *type_dropdown;
static GtkWidget *length_spin;
static GtkWidget *input_view;
static GtkWidget *input_scroll;
static GtkWidget *output_view;
static GtkWidget *input_help;
static GtkWidget *metrics_label;
static GtkWidget *status_label;
static GtkWidget *copy_button;
static gchar *last_result;
static GPtrArray *classic_chars;
static GPtrArray *modern_chars;

static gsize
codepoint_length(const char *text)
{
    return (gsize)g_utf8_strlen(text, -1);
}

static void
append_random_external(GString *output, GPtrArray *chars, guint count)
{
    for (guint i = 0; i < count; i++) {
        guint index = (guint)g_random_int_range(0, (gint)chars->len);
        const gchar *character = g_ptr_array_index(chars, index);
        g_string_append(output, character);
    }
}

/* 알파벳 문자열을 중복 없는 코드 포인트 배열로 만든다. */
static GPtrArray *
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

static GPtrArray *
read_general_lines(void)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(contents, "\n", -1);
    GPtrArray *parts = g_ptr_array_new_with_free_func(g_free);

    for (gsize i = 0; lines[i] != NULL; i++) {
        gsize bytes = strlen(lines[i]);
        if (bytes > 0 && lines[i][bytes - 1] == '\r')
            lines[i][bytes - 1] = '\0';
        if (lines[i][0] != '\0')
            g_ptr_array_add(parts, g_strdup(lines[i]));
    }

    g_strfreev(lines);
    g_free(contents);
    return parts;
}

/* klass: 1=완전, 2=앞 혼합, 3=뒤 혼합, 4=기존 3류(문장 사이·양끝 삽입).
   chars: 고전 또는 신규 외부 문자 집합. */
static gboolean
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

static void
clear_result(void)
{
    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view)), "", -1);
    gtk_label_set_text(GTK_LABEL(metrics_label), "원문 및 NFKC 길이가 여기에 표시됩니다.");
    gtk_widget_set_sensitive(copy_button, FALSE);
    g_clear_pointer(&last_result, g_free);
}

static void
on_generate_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(type_dropdown));
    GPtrArray *chars = (selected < 4) ? classic_chars : modern_chars;
    guint klass = (selected % 4) + 1;
    guint target_length = (guint)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(length_spin));
    GPtrArray *phrases = read_general_lines();
    GString *generated = g_string_new(NULL);
    gchar *error_message = NULL;

    if (!build_string(klass, chars, target_length, phrases, generated, &error_message)) {
        clear_result();
        gtk_label_set_text(GTK_LABEL(status_label), error_message);
        g_free(error_message);
        g_string_free(generated, TRUE);
        g_ptr_array_unref(phrases);
        return;
    }

    gsize raw_length = codepoint_length(generated->str);
    gchar *normalized = g_utf8_normalize(generated->str, -1, G_NORMALIZE_NFKC);
    gsize normalized_length = normalized ? codepoint_length(normalized) : raw_length;

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view)), generated->str, -1);
    gchar *metrics = g_strdup_printf(
        "원문 코드 포인트: %zu / %u    |    NFKC 후 코드 포인트: %zu    |    길이 변화: %+ld",
        raw_length, target_length, normalized_length,
        (long)normalized_length - (long)raw_length);
    gtk_label_set_text(GTK_LABEL(metrics_label), metrics);
    gtk_label_set_text(GTK_LABEL(status_label), "생성 완료. 복사 버튼은 원문 문자열을 복사합니다.");
    gtk_widget_set_sensitive(copy_button, TRUE);

    g_free(last_result);
    last_result = g_strdup(generated->str);
    g_free(metrics);
    g_free(normalized);
    g_string_free(generated, TRUE);
    g_ptr_array_unref(phrases);
}

static void
on_copy_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    if (!last_result)
        return;

    GdkDisplay *display = gtk_widget_get_display(copy_button);
    GdkClipboard *clipboard = gdk_display_get_clipboard(display);
    gdk_clipboard_set_text(clipboard, last_result);
    gtk_label_set_text(GTK_LABEL(status_label), "원문 문자열의 클립보드 복사를 요청했습니다.");
}

static void
on_type_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    (void)object;
    (void)pspec;
    (void)user_data;

    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(type_dropdown));
    guint klass = (selected % 4) + 1;
    gtk_widget_set_sensitive(input_scroll, klass != 1);

    if (klass == 1)
        gtk_label_set_text(GTK_LABEL(input_help), "1류는 외부 유니코드 문자만 생성하므로 아래 입력란을 사용하지 않습니다.");
    else if (klass == 2)
        gtk_label_set_text(GTK_LABEL(input_help), "2류: 일반 문자열 한 개를 한 줄에 입력하세요. 결과의 맨 앞에 배치됩니다.");
    else if (klass == 3)
        gtk_label_set_text(GTK_LABEL(input_help), "3류: 일반 문자열 한 개를 한 줄에 입력하세요. 결과의 맨 뒤에 배치됩니다.");
    else
        gtk_label_set_text(GTK_LABEL(input_help), "4류: 일반 문자열을 줄마다 입력하세요(두 줄 이상). 각 문장 사이와 양 끝에 외부 문자가 들어갑니다.");
}

static GtkWidget *
make_text_area(gint height, gboolean editable)
{
    GtkWidget *view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), editable);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), editable);
    gtk_widget_set_size_request(view, -1, height);
    return view;
}

static void
activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "NFKC 연구용 문자열 생성기");
    gtk_window_set_default_size(GTK_WINDOW(window), 860, 760);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(root, 18);
    gtk_widget_set_margin_bottom(root, 18);
    gtk_widget_set_margin_start(root, 20);
    gtk_widget_set_margin_end(root, 20);
    gtk_window_set_child(GTK_WINDOW(window), root);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='x-large' weight='bold'>NFKC 연구용 문자열 생성기</span>");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(root), title);

    GtkWidget *subtitle = gtk_label_new("문자열 유형별 실험 입력을 생성하고, NFKC 정규화 전후 코드 포인트 길이를 비교합니다.");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
    gtk_box_append(GTK_BOX(root), subtitle);

    GtkWidget *settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(settings, 6);
    gtk_box_append(GTK_BOX(root), settings);

    GtkWidget *type_label = gtk_label_new("문자열 유형");
    gtk_box_append(GTK_BOX(settings), type_label);
    const char *type_names[] = {
        "고전 1류 — 완전한 NFKC 문자열",
        "고전 2류 — 앞에 일반 문자열",
        "고전 3류 — 뒤에 일반 문자열",
        "고전 4류 — 문장 사이·양끝 삽입",
        "신규 1류 — 완전한 NFKC 문자열",
        "신규 2류 — 앞에 일반 문자열",
        "신규 3류 — 뒤에 일반 문자열",
        "신규 4류 — 문장 사이·양끝 삽입",
        NULL
    };
    type_dropdown = gtk_drop_down_new_from_strings(type_names);
    gtk_widget_set_hexpand(type_dropdown, TRUE);
    gtk_box_append(GTK_BOX(settings), type_dropdown);

    GtkWidget *length_label = gtk_label_new("목표 길이 (코드 포인트)");
    gtk_box_append(GTK_BOX(settings), length_label);
    length_spin = gtk_spin_button_new_with_range(20, 120, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(length_spin), 80);
    gtk_widget_set_size_request(length_spin, 90, -1);
    gtk_box_append(GTK_BOX(settings), length_spin);

    GtkWidget *input_heading = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(input_heading), "<b>일반 문자열 입력</b>");
    gtk_label_set_xalign(GTK_LABEL(input_heading), 0.0f);
    gtk_widget_set_margin_top(input_heading, 8);
    gtk_box_append(GTK_BOX(root), input_heading);

    input_help = gtk_label_new("1류는 외부 유니코드 문자만 생성하므로 아래 입력란을 사용하지 않습니다.");
    gtk_label_set_xalign(GTK_LABEL(input_help), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(input_help), TRUE);
    gtk_box_append(GTK_BOX(root), input_help);

    input_view = make_text_area(145, TRUE);
    input_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(input_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(input_scroll), input_view);
    gtk_box_append(GTK_BOX(root), input_scroll);
    gtk_widget_set_sensitive(input_scroll, FALSE);

    GtkWidget *output_heading = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(output_heading), "<b>생성 결과 (원문)</b>");
    gtk_label_set_xalign(GTK_LABEL(output_heading), 0.0f);
    gtk_widget_set_margin_top(output_heading, 8);
    gtk_box_append(GTK_BOX(root), output_heading);

    output_view = make_text_area(150, FALSE);
    GtkWidget *output_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(output_scroll), output_view);
    gtk_widget_set_vexpand(output_scroll, TRUE);  /* 남는 세로 공간을 결과 박스가 채운다 */
    gtk_box_append(GTK_BOX(root), output_scroll);

    metrics_label = gtk_label_new("원문 및 NFKC 길이가 여기에 표시됩니다.");
    gtk_label_set_xalign(GTK_LABEL(metrics_label), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(metrics_label), TRUE);
    gtk_box_append(GTK_BOX(root), metrics_label);

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_widget_set_margin_top(buttons, 4);
    gtk_box_append(GTK_BOX(root), buttons);

    GtkWidget *generate_button = gtk_button_new_with_label("생성");
    copy_button = gtk_button_new_with_label("복사");
    gtk_box_append(GTK_BOX(buttons), generate_button);
    gtk_box_append(GTK_BOX(buttons), copy_button);
    gtk_widget_set_sensitive(copy_button, FALSE);

    status_label = gtk_label_new("준비됨");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(status_label), TRUE);
    gtk_box_append(GTK_BOX(root), status_label);

    g_signal_connect(generate_button, "clicked", G_CALLBACK(on_generate_clicked), NULL);
    g_signal_connect(copy_button, "clicked", G_CALLBACK(on_copy_clicked), NULL);
    g_signal_connect(type_dropdown, "notify::selected", G_CALLBACK(on_type_changed), NULL);

    gtk_window_present(GTK_WINDOW(window));
}

#ifndef NFKCGEN_SELFTEST
int
main(int argc, char **argv)
{
    /* fcitx 등 다국어 입력기를 GTK4가 불러오게 한다. 이미 설정돼 있으면 그대로 둔다.
       (한국어 입력에는 fcitx5-frontend-gtk4 패키지가 필요하다.) */
    g_setenv("GTK_IM_MODULE", "fcitx", FALSE);

    classic_chars = build_charset(classic_alphabet);
    modern_chars = build_charset(modern_alphabet);

    GtkApplication *app = gtk_application_new(
        "org.xisik.nfkcgen", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_clear_object(&app);
    g_clear_pointer(&last_result, g_free);
    g_ptr_array_unref(classic_chars);
    g_ptr_array_unref(modern_chars);
    return status;
}
#else  /* NFKCGEN_SELFTEST: build_string 분기 구조 검증 (make selftest) */
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
    (void)activate;  /* GUI 진입점은 self-test에서 쓰지 않는다 */
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
#endif
