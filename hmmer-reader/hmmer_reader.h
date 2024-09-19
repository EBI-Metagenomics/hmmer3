#ifndef HMMER_READER_H
#define HMMER_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

enum
{
    HMR_SYMBOLS_MAX = 32,
    HMR_TRANS_SIZE = 7,
};

enum hmr_trans
{
    HMR_TRANS_MM,
    HMR_TRANS_MI,
    HMR_TRANS_MD,
    HMR_TRANS_IM,
    HMR_TRANS_II,
    HMR_TRANS_DM,
    HMR_TRANS_DD
};

struct hmr_node
{
    unsigned symbols_size;
    unsigned idx;
    union
    {
        double compo[HMR_SYMBOLS_MAX];
        double match[HMR_SYMBOLS_MAX];
    };
    double insert[HMR_SYMBOLS_MAX];
    double trans[HMR_TRANS_SIZE];
    struct
    {
        unsigned map;
        union
        {
            struct
            {
                char cons;
                char rf;
                char mm;
                char cs;
            };
            char buf[4];
        };
    } excess;
};

#define HMR_OFFSET(field) offsetof(struct hmr_node, excess.field)
#ifndef static_assert
#define static_assert(expr, msg) _Static_assert(expr, msg)
#endif
static_assert(HMR_OFFSET(rf) - HMR_OFFSET(cons) == 1, "must be packed");
static_assert(HMR_OFFSET(mm) - HMR_OFFSET(rf) == 1, "must be packed");
static_assert(HMR_OFFSET(cs) - HMR_OFFSET(mm) == 1, "must be packed");
static_assert(HMR_OFFSET(buf) - HMR_OFFSET(cons) == 0, "must be packed");
#undef HMR_OFFSET

enum
{
    HMR_HEADER_MAX = 64,
    HMR_NAME_MAX = 64,
    HMR_ACC_MAX = 64,
    HMR_DESC_MAX = 128,
    HMR_LENG_MAX = 8,
    HMR_ALPH_MAX = 12,
    HMR_GA_MAX = 32,
    HMR_BUFF_MAX = 128,
};

struct hmr_profile
{
    char header[HMR_HEADER_MAX];
    struct
    {
        char name[HMR_NAME_MAX];
        char acc[HMR_ACC_MAX];
        char desc[HMR_DESC_MAX];
        char leng[HMR_LENG_MAX];
        char alph[HMR_ALPH_MAX];
        char ga[HMR_GA_MAX];
    } meta;
    char buff[HMR_BUFF_MAX];
    unsigned symbols_size;
    char symbols[HMR_SYMBOLS_MAX];

    struct hmr_node node;
    char *error;
};

struct hmr_position
{
    union
    {
        struct
        {
            char *begin;
            char *pos;
            char *end;
        } prof;

        struct
        {
            double *begin;
            double *pos;
            double *end;
        } node;
    };
    unsigned idx;
};

enum hmr_token_id
{
    HMR_TOK_NL,
    HMR_TOK_WORD,
    HMR_TOK_HMM,
    HMR_TOK_COMPO,
    HMR_TOK_SLASH,
    HMR_TOK_EOF,
};

enum
{
    HMR_TOK_LINE_MAX = 256,
};

struct hmr_token
{
    enum hmr_token_id id;
    char const *value;
    struct
    {
        char data[HMR_TOK_LINE_MAX];
        unsigned number;
        bool consumed;
        char *ctx;
    } line;
    char *error;
};

enum
{
    HMR_ERROR_SIZE = 128,
};

enum
{
    HMR_OK,
    HMR_END,
    HMR_EOF,
    HMR_EIO,
    HMR_EUSAGE,
    HMR_EPARSE,
};

enum hmr_state
{
    HMR_FSM_BEGIN,
    HMR_FSM_HEADER,
    HMR_FSM_NAME,
    HMR_FSM_CONTENT,
    HMR_FSM_SYMBOL,
    HMR_FSM_ARROW,
    HMR_FSM_COMPO,
    HMR_FSM_INSERT,
    HMR_FSM_MATCH,
    HMR_FSM_TRANS,
    HMR_FSM_PAUSE,
    HMR_FSM_SLASHED,
    HMR_FSM_END,
    HMR_FSM_ERROR
};

struct hmr
{
    FILE *restrict fp;
    enum hmr_state state;
    struct hmr_token tok;
    struct hmr_position pos;
    char error[HMR_ERROR_SIZE];
};

#define HMR_DECLARE(name, fp) struct hmr name; hmr_init(&name, fp)

void hmr_init(struct hmr *hmr, FILE *restrict fp);
int  hmr_next_profile(struct hmr *hmr, struct hmr_profile *prof);
int  hmr_next_node(struct hmr *hmr, struct hmr_profile *prof);
void hmr_clear_error(struct hmr *hmr);
int  hmr_count_profiles(char const *filepath);

#define HMR_PROF_DECLARE(name, hmr) struct hmr_profile name; hmr_profile_init(&name, (hmr))

void     hmr_profile_dump(struct hmr_profile const *prof, FILE *restrict fp);
void     hmr_profile_init(struct hmr_profile *prof, struct hmr *hmr);
unsigned hmr_profile_length(struct hmr_profile const *prof);

#endif
