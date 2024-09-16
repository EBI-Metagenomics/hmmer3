#include "token.h"
#include "error.h"
#include "hmmer_reader.h"
#include "posix.h"
#include <string.h>

#define DELIM " \t\r"

static void add_space_before_newline(char line[HMR_TOK_LINE_MAX]);
static int next_line(FILE *restrict fp, char error[HMR_ERROR_SIZE],
                     char line[HMR_TOK_LINE_MAX]);

void hmr_token_init(struct hmr_token *tok, char *error)
{
    tok->id = HMR_TOK_NL;
    tok->value = tok->line.data;
    memset(tok->line.data, '\0', HMR_TOK_LINE_MAX);
    tok->line.number = 0;
    tok->line.consumed = true;
    tok->line.ctx = NULL;
    tok->error = error;
}

int hmr_token_next(struct hmr_token *tok, FILE *restrict fp)
{
    int rc = HMR_OK;

    if (tok->line.consumed)
    {
        if ((rc = next_line(fp, tok->error, tok->line.data)))
        {
            if (rc == HMR_EOF)
            {
                tok->value = NULL;
                tok->id = HMR_TOK_EOF;
                tok->line.data[0] = '\0';
                return HMR_OK;
            }
            return rc;
        }
        tok->value = posix_strtok_r(tok->line.data, DELIM, &tok->line.ctx);
        tok->line.number++;

        if (!tok->value) return HMR_EPARSE;
    }
    else
        tok->value = posix_strtok_r(NULL, DELIM, &tok->line.ctx);

    if (!strcmp(tok->value, "\n"))
        tok->id = HMR_TOK_NL;
    else if (!strcmp(tok->value, "//"))
        tok->id = HMR_TOK_SLASH;
    else if (!strcmp(tok->value, "HMM"))
        tok->id = HMR_TOK_HMM;
    else if (!strcmp(tok->value, "COMPO"))
        tok->id = HMR_TOK_COMPO;
    else
        tok->id = HMR_TOK_WORD;

    tok->line.consumed = tok->id == HMR_TOK_NL;

    return HMR_OK;
}

static int next_line(FILE *restrict fp, char error[HMR_ERROR_SIZE],
                     char line[HMR_TOK_LINE_MAX])
{
    if (!fgets(line, HMR_TOK_LINE_MAX - 1, fp))
    {
        if (feof(fp)) return HMR_EOF;

        return hmr_error_io(error, ferror(fp));
    }

    add_space_before_newline(line);
    return HMR_OK;
}

static void add_space_before_newline(char line[HMR_TOK_LINE_MAX])
{
    unsigned n = (unsigned)strlen(line);
    if (n > 0)
    {
        if (line[n - 1] == '\n')
        {
            line[n - 1] = ' ';
            line[n] = '\n';
            line[n + 1] = '\0';
        }
        else
        {
            line[n - 1] = '\n';
            line[n] = '\0';
        }
    }
}
