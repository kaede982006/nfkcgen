#define main application_main
#include "main.c"
#undef main

int main(void)
{
    g_unsetenv("GTK_IM_MODULE"); g_setenv("XMODIFIERS", "@im=ibus", TRUE);
    prefer_ibus_im_module();
    g_assert_cmpstr(g_getenv("GTK_IM_MODULE"), ==, "ibus");
    g_setenv("GTK_IM_MODULE", "fcitx", TRUE);
    prefer_ibus_im_module();
    g_assert_cmpstr(g_getenv("GTK_IM_MODULE"), ==, "fcitx");
    g_unsetenv("GTK_IM_MODULE"); g_setenv("XMODIFIERS", "@im=fcitx", TRUE);
    prefer_ibus_im_module();
    g_assert_null(g_getenv("GTK_IM_MODULE"));
    g_unsetenv("XMODIFIERS");
    gtk_init();
    classic_chars = build_charset(classic_alphabet);
    modern_chars = build_charset(modern_alphabet);
    GtkApplication *app = gtk_application_new("org.xisik.nfkcgen.test", G_APPLICATION_NON_UNIQUE);
    g_assert_true(g_application_register(G_APPLICATION(app), NULL, NULL));
    activate(app, NULL);
    on_generate_clicked(NULL, NULL);
    g_assert_nonnull(last_result);
    g_assert_cmpuint(codepoint_length(last_result), ==, 120);
    g_assert_true(gtk_widget_get_sensitive(copy_button));
    on_copy_clicked(NULL, NULL);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(length_spin), 100);
    on_generate_clicked(NULL, NULL);
    g_assert_cmpuint(codepoint_length(last_result), ==, 100);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(type_dropdown), 5);
    on_generate_clicked(NULL, NULL);
    g_assert_null(last_result);
    g_assert_false(gtk_widget_get_sensitive(copy_button));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_view));
    gtk_text_buffer_set_text(buffer, "한글🙂", -1);
    on_generate_clicked(NULL, NULL);
    g_assert_nonnull(last_result);
    g_assert_true(g_str_has_prefix(last_result, "한글🙂"));
    g_print("UI: IM module choice, generation, copy, validation passed\n");
    gtk_window_destroy(gtk_application_get_active_window(app));
    g_object_unref(app);
    g_ptr_array_unref(classic_chars); g_ptr_array_unref(modern_chars);
    g_free(last_result);
    return 0;
}
