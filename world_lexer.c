#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "world_lexer.h"
#include "utils.h"

/* TODO: rewrite as finite-state machine. */

WorldLexer * newWorldLexer(const char * path)
{
    WorldLexer * lexer = (WorldLexer *) malloc(sizeof(WorldLexer));

    lexer->text = getTextFileContent(path, &(lexer->textLength));
    lexer->pos = lexer->text;

    return lexer;
}

void freeWorldLexer(WorldLexer * lexer)
{
    free(lexer->text);
    free(lexer);
}

void skipSpaces(WorldLexer * lexer)
{
    char c = *(lexer->pos);

    while (c == ' ' || c == '\n')
    {
        ++(lexer->pos);
        c = *(lexer->pos);
    }
}

void skipLex(WorldLexer * lexer)
{
    char c = *(lexer->pos);

    while (c != ' ' && c != '\n' && c != '\0')
    {
        ++(lexer->pos);
        c = *(lexer->pos);
    }
}

void skipLine(WorldLexer * lexer)
{
    char c = *(lexer->pos);

    while (c != '\n' && c != '\0')
    {
        ++(lexer->pos);
        c = *(lexer->pos);
    }

    if (c != '\0')
    {
        ++(lexer->pos);
    }
}

char * getLex(WorldLexer * lexer)
{
    char * lex = (char *) malloc(sizeof(char) * (MAX_LEX_LENGTH + 1));
    int lexPos = 0;
    char c;

    skipSpaces(lexer);
    c = *(lexer->pos);

    while (c != ' ' && c != '\n' && c != '\0' &&
        lexPos < MAX_LEX_LENGTH)
    {
        if (c == '/' && lexer->pos[1] == '/')
        {
            skipLine(lexer);
            skipSpaces(lexer);
            c = *(lexer->pos);

            if (lexPos == 0)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if ((c == '{' || c == '}' || c == '\"' ||
            c == '[' || c == ']') && lexPos > 0)
        {
            break;
        }

        lex[lexPos] = c;
        ++lexPos;
        ++(lexer->pos);

        if ((c == '{' || c == '}' || c == '\"' ||
            c == '[' || c == ']') && lexPos == 1)
        {
            break;
        }

        c = *(lexer->pos);
    }

    if (lexPos == 0)
    {
        free(lex);
        return NULL;
    }

    lex[lexPos] = '\0';

    if (lexPos == MAX_LEX_LENGTH)
    {
        skipLex(lexer);
    }

    return lex;
}

void checkNextLex(WorldLexer * lexer, const char * pattern)
{
    char * lex = getLex(lexer);

    if (lex == NULL)
    {
        fprintf(stderr, "checkNextLex(\"%s\") error: unexpected EOF.\n",
            pattern);
        exit(EXIT_FAILURE);
    }
        
    if (! STR_EQUAL(lex, pattern))
    {
        fprintf(stderr, "checkNextLex() error: \"%s\" is not \"%s\".\n",
            lex, pattern);
        exit(EXIT_FAILURE);
    }

    free(lex);
}
