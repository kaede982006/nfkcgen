#include <gtk/gtk.h>
#include <string.h>

#include "generator.h"

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

/* 텍스트뷰를 스크롤 창에 담아 반환한다. 최소 높이는 안쪽 뷰가 아니라 스크롤 창에
   지정해야 스크롤바 슬라이더 최소 크기가 음수로 계산되지 않는다. word-wrap이므로
   가로 스크롤바는 두지 않는다. *out_view로 내부 텍스트뷰를 돌려준다. */
static GtkWidget *
make_text_area(gint min_height, gboolean editable, GtkWidget **out_view)
{
    GtkWidget *view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), editable);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), editable);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), min_height);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), view);
    *out_view = view;
    return scroll;
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

    input_scroll = make_text_area(145, TRUE, &input_view);
    gtk_box_append(GTK_BOX(root), input_scroll);
    gtk_widget_set_sensitive(input_scroll, FALSE);

    GtkWidget *output_heading = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(output_heading), "<b>생성 결과 (원문)</b>");
    gtk_label_set_xalign(GTK_LABEL(output_heading), 0.0f);
    gtk_widget_set_margin_top(output_heading, 8);
    gtk_box_append(GTK_BOX(root), output_heading);

    GtkWidget *output_scroll = make_text_area(150, FALSE, &output_view);
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
