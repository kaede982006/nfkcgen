#define main application_main
#include "main.c"
#undef main

int main(void)
{
    gtk_init();
    candidates = g_ptr_array_new_with_free_func(candidate_free);
    classic_chars = build_charset(classic_alphabet);
    modern_chars = build_charset(modern_alphabet);
    GtkApplication *app = gtk_application_new("org.xisik.nfkcgen.test", G_APPLICATION_NON_UNIQUE);
    g_assert_true(g_application_register(G_APPLICATION(app), NULL, NULL));
    activate(app, NULL);
    on_generate_clicked(NULL, NULL);
    g_assert_nonnull(current);
    g_assert_cmpuint(codepoint_length(current->text), ==, 120);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(lufs_spin), -12);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_spin), 40);
    on_measure_clicked(NULL, NULL);
    Candidate *first = current;
    g_assert_true(best_candidate() == first);
    on_measure_clicked(NULL, NULL);
    g_assert_cmpuint(first->trials, ==, 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_spin), 0);
    on_measure_clicked(NULL, NULL);
    g_assert_cmpuint(first->successes, ==, 2);
    g_assert_cmpuint(first->trials, ==, 3);
    on_generate_clicked(NULL, NULL);
    g_assert_true(current != first);
    g_assert_nonnull(strstr(gtk_label_get_text(GTK_LABEL(status_label)), "우수 후보 변이"));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(lufs_spin), -10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_spin), 60);
    on_measure_clicked(NULL, NULL);
    g_assert_true(best_candidate() == current);
    on_copy_clicked(NULL, NULL);
    on_best_copy_clicked(GTK_BUTTON(best_button), NULL);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(length_spin), 100);
    on_generate_clicked(NULL, NULL);
    g_assert_cmpuint(candidates->len, ==, 1);
    g_assert_null(best_candidate());
    gtk_drop_down_set_selected(GTK_DROP_DOWN(type_dropdown), 5);
    on_generate_clicked(NULL, NULL);
    g_assert_null(current);
    g_assert_false(gtk_widget_get_sensitive(measure_button));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_view));
    gtk_text_buffer_set_text(buffer, "한글🙂", -1);
    on_generate_clicked(NULL, NULL);
    g_assert_nonnull(current);
    g_assert_true(g_str_has_prefix(current->text, "한글🙂"));
    g_print("UI: generation, scoring, repeated/failure trials, mutation, copy, reset, validation passed\n");
    gtk_window_destroy(gtk_application_get_active_window(app));
    g_object_unref(app);
    g_ptr_array_unref(candidates); g_ptr_array_unref(classic_chars); g_ptr_array_unref(modern_chars);
    g_free(last_result); g_free(search_settings);
    return 0;
}
