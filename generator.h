#ifndef NFKCGEN_GENERATOR_H
#define NFKCGEN_GENERATOR_H
#include <glib.h>
extern const char classic_alphabet[];
extern const char modern_alphabet[];
gsize codepoint_length(const char *text);
/* NFKC expansion is a text heuristic, never a predicted acoustic measurement. */
GPtrArray *build_charset(const char *alphabet);
/* Classes 1..4: external only, prefix, suffix, interleaved phrases. */
gboolean build_string(guint klass, GPtrArray *chars, guint target_length,
                      GPtrArray *phrases, GString *output, gchar **error_message);
/* Same seed + settings + parent external genome reproduce the same candidate. */
gboolean build_candidate(guint klass, GPtrArray *chars, guint target_length,
                         GPtrArray *phrases, const gchar *parent, guint32 seed,
                         GString *output, GString *external, gchar **error_message);
#endif
