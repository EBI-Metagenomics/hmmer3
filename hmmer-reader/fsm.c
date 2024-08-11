#include "fsm.h"
#include "error.h"
#include "hmr.h"
#include "position.h"
#include "to.h"
#include "trans.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#define INT_ERROR "failed to parse integer"
#define DEC_ERROR "failed to parse decimal number"

struct args
{
    struct hmr_token *tok;
    enum hmr_state state;
    struct hmr_position *pos;
    struct hmr_profile *prof;
};

struct trans
{
    enum hmr_state const next;
    int (*action)(struct args *a);
};

static char const arrows[HMR_TRANS_SIZE][5] = {
    [HMR_TRANS_MM] = "m->m", [HMR_TRANS_MI] = "m->i", [HMR_TRANS_MD] = "m->d",
    [HMR_TRANS_IM] = "i->m", [HMR_TRANS_II] = "i->i", [HMR_TRANS_DM] = "d->m",
    [HMR_TRANS_DD] = "d->d"};

static int nop(struct args *a)
{
    (void)a;
    return HMR_OK;
}

static int arrow(struct args *a);

static int unexpect_eof(struct args *a)
{
    return hmr_error_parse(a->tok, "unexpected end-of-file");
}

static int unexpect_eon(struct args *a)
{
    return hmr_error_parse(a->tok, "unexpected end-of-node");
}

static int unexpect_symbol(struct args *a)
{
    return hmr_error_parse(a->tok, "unexpected symbol");
}

static int unexpect_tok(struct args *a)
{
    return hmr_error_parse(a->tok, "unexpected token");
}

static int unexpect_nl(struct args *a)
{
    return hmr_error_parse(a->tok, "unexpected newline");
}

static int header(struct args *a);

static int field_name(struct args *a);

static int field_content(struct args *a);

static int hmm(struct args *a);

static int symbol(struct args *a);

static int compo(struct args *a);

static int insert(struct args *a);

static int match(struct args *a);

static int trans(struct args *a);

static int check_header(struct hmr_profile *prof);

static int check_required_metadata(struct hmr_profile *prof);

static struct trans const transition[][6] = {
    [HMR_FSM_BEGIN] = {[HMR_TOK_WORD] = {HMR_FSM_HEADER, &header},
                       [HMR_TOK_NL] = {HMR_FSM_BEGIN, &nop},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &header},
                       [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &header},
                       [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &header},
                       [HMR_TOK_EOF] = {HMR_FSM_END, &nop}},
    [HMR_FSM_HEADER] = {[HMR_TOK_WORD] = {HMR_FSM_HEADER, &header},
                        [HMR_TOK_NL] = {HMR_FSM_NAME, &header},
                        [HMR_TOK_HMM] = {HMR_FSM_ERROR, &header},
                        [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &header},
                        [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &header},
                        [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_NAME] = {[HMR_TOK_WORD] = {HMR_FSM_CONTENT, &field_name},
                      [HMR_TOK_NL] = {HMR_FSM_ERROR, &unexpect_nl},
                      [HMR_TOK_HMM] = {HMR_FSM_SYMBOL, &hmm},
                      [HMR_TOK_COMPO] = {HMR_FSM_PAUSE, &nop},
                      [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_tok},
                      [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_CONTENT] = {[HMR_TOK_WORD] = {HMR_FSM_CONTENT, &field_content},
                         [HMR_TOK_NL] = {HMR_FSM_NAME, &field_content},
                         [HMR_TOK_HMM] = {HMR_FSM_CONTENT, &field_content},
                         [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_SYMBOL] = {[HMR_TOK_WORD] = {HMR_FSM_SYMBOL, &symbol},
                        [HMR_TOK_NL] = {HMR_FSM_ARROW, &symbol},
                        [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_symbol},
                        [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_symbol},
                        [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_symbol},
                        [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_ARROW] = {[HMR_TOK_WORD] = {HMR_FSM_ARROW, &arrow},
                       [HMR_TOK_NL] = {HMR_FSM_PAUSE, &arrow},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_tok},
                       [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_tok},
                       [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_tok},
                       [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_COMPO] = {[HMR_TOK_WORD] = {HMR_FSM_COMPO, &compo},
                       [HMR_TOK_NL] = {HMR_FSM_INSERT, &compo},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_INSERT] = {[HMR_TOK_WORD] = {HMR_FSM_INSERT, &insert},
                        [HMR_TOK_NL] = {HMR_FSM_TRANS, &insert},
                        [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_eon},
                        [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_eon},
                        [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_eon},
                        [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_MATCH] = {[HMR_TOK_WORD] = {HMR_FSM_MATCH, &match},
                       [HMR_TOK_NL] = {HMR_FSM_INSERT, &match},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_TRANS] = {[HMR_TOK_WORD] = {HMR_FSM_TRANS, &trans},
                       [HMR_TOK_NL] = {HMR_FSM_PAUSE, &trans},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_eon},
                       [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_PAUSE] = {[HMR_TOK_WORD] = {HMR_FSM_MATCH, &match},
                       [HMR_TOK_NL] = {HMR_FSM_ERROR, &unexpect_nl},
                       [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_tok},
                       [HMR_TOK_COMPO] = {HMR_FSM_COMPO, &nop},
                       [HMR_TOK_SLASH] = {HMR_FSM_SLASHED, &nop},
                       [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
    [HMR_FSM_SLASHED] = {[HMR_TOK_WORD] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_NL] = {HMR_FSM_BEGIN, &nop},
                         [HMR_TOK_HMM] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_COMPO] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_SLASH] = {HMR_FSM_ERROR, &unexpect_tok},
                         [HMR_TOK_EOF] = {HMR_FSM_ERROR, &unexpect_eof}},
};

static char state_name[][10] = {
    [HMR_FSM_BEGIN] = "BEGIN",   [HMR_FSM_HEADER] = "HEADER",
    [HMR_FSM_NAME] = "NAME",     [HMR_FSM_CONTENT] = "CONTENT",
    [HMR_FSM_SYMBOL] = "SYMBOL", [HMR_FSM_ARROW] = "ARROW",
    [HMR_FSM_COMPO] = "COMPO",   [HMR_FSM_INSERT] = "INSERT",
    [HMR_FSM_MATCH] = "MATCH",   [HMR_FSM_TRANS] = "TRANS",
    [HMR_FSM_PAUSE] = "PAUSE",   [HMR_FSM_SLASHED] = "SLASHED",
    [HMR_FSM_END] = "END",       [HMR_FSM_ERROR] = "ERROR",
};

void hmr_fsm_init(enum hmr_state *state) { *state = HMR_FSM_BEGIN; }

enum hmr_state hmr_fsm_next(enum hmr_state state, struct hmr_token *tok,
                            struct hmr_position *pos, struct hmr_profile *prof)
{
    unsigned row = (unsigned)state;
    unsigned col = (unsigned)tok->id;
    struct trans const *const t = &transition[row][col];
    struct args args = {tok, state, pos, prof};
    if (t->action(&args)) return HMR_FSM_ERROR;
    return t->next;
}

char const *hmr_fsm_name(enum hmr_state state) { return state_name[state]; }

static int error_parse_arrow(struct hmr_token *tok, unsigned i)
{
    if (i == HMR_TRANS_MM)
        return hmr_error_parse(tok, "expected m->m");
    else if (i == HMR_TRANS_MI)
        return hmr_error_parse(tok, "expected m->i");
    else if (i == HMR_TRANS_MD)
        return hmr_error_parse(tok, "expected m->d");
    else if (i == HMR_TRANS_IM)
        return hmr_error_parse(tok, "expected i->m");
    else if (i == HMR_TRANS_II)
        return hmr_error_parse(tok, "expected i->i");
    else if (i == HMR_TRANS_DM)
        return hmr_error_parse(tok, "expected d->m");
    else if (i == HMR_TRANS_DD)
        return hmr_error_parse(tok, "expected d->d");
    assert(false);
    return HMR_OK;
}

static int arrow(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->pos->idx >= HMR_TRANS_SIZE) return unexpect_tok(a);

        if (strcmp(a->tok->value, arrows[a->pos->idx]))
            return error_parse_arrow(a->tok, a->pos->idx);
        a->pos->idx++;
    }
    else
    {
        if (a->pos->idx != HMR_TRANS_SIZE)
            return hmr_error_parse(a->tok, "unexpected end-of-line");
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int header(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->pos->prof.pos && (a->pos->prof.pos > a->pos->prof.begin + 1))
        {
            *(a->pos->prof.pos - 1) = ' ';
            a->pos->prof.pos++;
        }
        else
        {
            a->pos->prof.begin = a->prof->header;
            a->pos->prof.pos = a->pos->prof.begin + 1;
            a->pos->prof.end = a->pos->prof.begin + HMR_HEADER_MAX;
        }
        a->pos->prof.pos =
            memccpy(a->pos->prof.pos - 1, a->tok->value, '\0',
                    (unsigned long)(a->pos->prof.end - a->pos->prof.pos));
    }
    else
    {
        *(a->pos->prof.pos - 1) = '\0';
        if (check_header(a->prof))
            return hmr_error_parse(a->tok, "invalid header");
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int field_name(struct args *a)
{
    if (!strcmp(a->tok->value, "NAME"))
    {
        a->pos->prof.begin = a->prof->meta.name;
        a->pos->prof.end = a->pos->prof.begin + HMR_NAME_MAX;
    }
    else if (!strcmp(a->tok->value, "ACC"))
    {
        a->pos->prof.begin = a->prof->meta.acc;
        a->pos->prof.end = a->pos->prof.begin + HMR_ACC_MAX;
    }
    else if (!strcmp(a->tok->value, "DESC"))
    {
        a->pos->prof.begin = a->prof->meta.desc;
        a->pos->prof.end = a->pos->prof.begin + HMR_DESC_MAX;
    }
    else if (!strcmp(a->tok->value, "LENG"))
    {
        a->pos->prof.begin = a->prof->meta.leng;
        a->pos->prof.end = a->pos->prof.begin + HMR_LENG_MAX;
    }
    else if (!strcmp(a->tok->value, "ALPH"))
    {
        a->pos->prof.begin = a->prof->meta.alph;
        a->pos->prof.end = a->pos->prof.begin + HMR_ALPH_MAX;
    }
    else if (!strcmp(a->tok->value, "GA"))
    {
        a->pos->prof.begin = a->prof->meta.ga;
        a->pos->prof.end = a->pos->prof.begin + HMR_GA_MAX;
    }
    else
    {
        a->pos->prof.begin = a->prof->buff;
        a->pos->prof.end = a->pos->prof.begin + HMR_BUFF_MAX;
    }
    a->pos->prof.pos = a->pos->prof.begin + 1;
    return HMR_OK;
}

static int field_content(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_HMM ||
           a->tok->id == HMR_TOK_COMPO || a->tok->id == HMR_TOK_NL);

    if (a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_HMM ||
        a->tok->id == HMR_TOK_COMPO)
    {
        if (a->pos->prof.pos && (a->pos->prof.pos > a->pos->prof.begin + 1))
        {
            *(a->pos->prof.pos - 1) = ' ';
            a->pos->prof.pos++;
        }
        a->pos->prof.pos =
            memccpy(a->pos->prof.pos - 1, a->tok->value, '\0',
                    (unsigned long)(a->pos->prof.end - a->pos->prof.pos));
    }
    else
    {
        if (a->pos->prof.pos == a->pos->prof.begin + 1)
            return hmr_error_parse(a->tok,
                                   "expected content before end-of-line");
        *(a->pos->prof.pos - 1) = '\0';
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int hmm(struct args *a)
{
    a->pos->prof.begin = a->prof->symbols;
    a->pos->prof.end = a->pos->prof.begin + HMR_SYMBOLS_MAX;
    a->pos->prof.pos = a->pos->prof.begin + 1;
    if (a->prof->meta.acc[0] == '\0')
        strcpy(a->prof->meta.acc, a->prof->meta.name);
    return check_required_metadata(a->prof);
}

static int symbol(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        *(a->pos->prof.pos - 1) = *a->tok->value;
        a->pos->prof.pos++;
    }
    else
    {
        *(a->pos->prof.pos - 1) = '\0';
        a->prof->symbols_size = (unsigned)strlen(a->prof->symbols);
        a->prof->node.symbols_size = a->prof->symbols_size;
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int compo(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->pos->idx >= a->prof->symbols_size)
            return hmr_error_parse(a->tok, "too many compo numbers");

        if (!hmr_to_lprob(a->tok->value, a->prof->node.compo + a->pos->idx++))
            return hmr_error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->pos->idx != a->prof->symbols_size)
            return hmr_error_parse(a->tok,
                                   "compo length not equal to symbols length");
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int insert(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->pos->idx >= a->prof->symbols_size)
            return hmr_error_parse(a->tok, "too many insert numbers");

        if (!hmr_to_lprob(a->tok->value, a->prof->node.insert + a->pos->idx++))
            return hmr_error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->pos->idx != a->prof->symbols_size)
            return hmr_error_parse(a->tok,
                                   "insert length not equal to symbols length");
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static bool read_map(struct args *a);

static int read_match_excess(struct args *a)
{
    unsigned sz = a->prof->symbols_size;
    unsigned excess = MEMBER_SIZE(struct hmr_node, excess.buf) + 1;
    if (a->pos->idx >= sz + excess)
        return hmr_error_parse(a->tok, "too many match numbers");

    if (a->pos->idx == sz)
    {
        if (!read_map(a)) return hmr_error_parse(a->tok, INT_ERROR);
        return HMR_OK;
    }

    if (a->tok->value[0] == '\0' || a->tok->value[1] != '\0')
        return hmr_error_parse(a->tok, "excesses must be single character");

    a->prof->node.excess.buf[a->pos->idx++ - sz - 1] = a->tok->value[0];
    return HMR_OK;
}

static int match(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    unsigned sz = a->prof->symbols_size;
    unsigned excess = MEMBER_SIZE(struct hmr_node, excess.buf) + 1;
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->state == HMR_FSM_PAUSE)
        {
            if (!hmr_to_uint(a->tok->value, &a->prof->node.idx))
                return hmr_error_parse(a->tok, INT_ERROR);
            return HMR_OK;
        }
        if (a->pos->idx >= sz) return read_match_excess(a);

        if (!hmr_to_lprob(a->tok->value, a->prof->node.match + a->pos->idx++))
            return hmr_error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->pos->idx > sz + excess)
            return hmr_error_parse(a->tok, "too many match numbers");
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

static int trans(struct args *a)
{
    assert(a->tok->id == HMR_TOK_WORD || a->tok->id == HMR_TOK_NL);
    if (a->tok->id == HMR_TOK_WORD)
    {
        if (a->pos->idx >= HMR_TRANS_SIZE)
            return hmr_error_parse(a->tok, "too many trans numbers");

        if (!hmr_to_lprob(a->tok->value, a->prof->node.trans + a->pos->idx++))
            return hmr_error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->pos->idx != HMR_TRANS_SIZE)
            return hmr_error_parse(
                a->tok, "trans length not equal to " XSTR(HMR_TRANS_SIZE));
        hmr_position_init(a->pos);
    }
    return HMR_OK;
}

#define HEADER_EXAMPLE "HMMER3/f [3.3.1 | Jul 2020]"

static int check_header(struct hmr_profile *prof)
{
    char tmp[sizeof HEADER_EXAMPLE + 10];
    if (strlen(prof->header) >= ARRAY_SIZE(tmp)) return HMR_EPARSE;

    strcpy(tmp, prof->header);
    char *ptr = NULL;
    char *tok = NULL;

    if (!(tok = strtok_r(tmp, " ", &ptr))) return HMR_EPARSE;

    if (strcmp(tok, "HMMER3/f")) return HMR_EPARSE;

    if (!(tok = strtok_r(NULL, " ", &ptr))) return HMR_EPARSE;

    if (*tok != '[') return HMR_EPARSE;

    if (!(tok = strtok_r(NULL, " ", &ptr))) return HMR_EPARSE;

    if (*tok != '|') return HMR_EPARSE;

    /* Month */
    if (!(tok = strtok_r(NULL, " ", &ptr))) return HMR_EPARSE;

    /* Year] */
    if (!(tok = strtok_r(NULL, " ", &ptr))) return HMR_EPARSE;

    if (!(tok = strchr(tok, ']'))) return HMR_EPARSE;

    if (strtok_r(NULL, " ", &ptr)) return HMR_EPARSE;

    return HMR_OK;
}

static int check_required_metadata(struct hmr_profile *prof)
{
    int rc = HMR_EPARSE;

    if (prof->meta.acc[0] == '\0')
        return hmr_error(rc, prof->error, "missing ACC field");

    if (prof->meta.leng[0] == '\0')
        return hmr_error(rc, prof->error, "missing LENG field");

    if (prof->meta.alph[0] == '\0')
        return hmr_error(rc, prof->error, "missing ALPH field");

    return HMR_OK;
}

#define HMR_NODE_MAP_NULL UINT_MAX

static bool read_map(struct args *a)
{
    if (a->tok->value[0] == '-' && a->tok->value[1] == '\0')
        a->prof->node.excess.map = HMR_NODE_MAP_NULL;
    else
    {
        if (!hmr_to_uint(a->tok->value, &a->prof->node.excess.map))
            return false;
    }
    a->pos->idx++;
    return true;
}
