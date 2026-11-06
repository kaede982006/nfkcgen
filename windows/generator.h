#ifndef NFKCGEN_GENERATOR_H
#define NFKCGEN_GENERATOR_H

#include <glib.h>

/* 외부 문자 집합. 넓히려면 generator.c의 해당 상수만 고치면 된다. */
extern const char classic_alphabet[];  /* 고전: 원본 관찰 문자 ⣿ ㍰ █ */
extern const char modern_alphabet[];   /* 신규: 현재 요청 표본 기반 */

gsize codepoint_length(const char *text);

/* 알파벳 문자열을 중복 없는 코드 포인트 배열로 만든다. */
GPtrArray *build_charset(const char *alphabet);

void append_random_external(GString *output, GPtrArray *chars, guint count);

/* klass: 1=완전, 2=앞 혼합, 3=뒤 혼합, 4=문장 사이·양끝 삽입(기존 3류).
   chars: 고전 또는 신규 외부 문자 집합. 실패 시 *error_message에 사유를 담는다. */
gboolean build_string(guint klass, GPtrArray *chars, guint target_length,
                      GPtrArray *phrases, GString *output, gchar **error_message);

#endif  /* NFKCGEN_GENERATOR_H */
