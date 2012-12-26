#ifndef WORLD_LEXER_H_SENTRY
#define WORLD_LEXER_H_SENTRY

#define MAX_LEX_LENGTH 128

typedef
struct WorldLexer
{
    char * pos;
    char * text;
    long textLength;
}
WorldLexer;

WorldLexer * newWorldLexer(const char * path);

void freeWorldLexer(WorldLexer * lexer);

/* NULL at EOF. */
char * getLex(WorldLexer * lexer);

/* Exit, if not match. */
void checkNextLex(WorldLexer * lexer, const char * pattern);

#endif /* WORLD_LEXER_H_SENTRY */
