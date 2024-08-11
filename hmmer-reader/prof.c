#include "prof.h"
#include "aux.h"
#include "error.h"
#include "fsm.h"
#include "hmr.h"
#include "node.h"
#include "tok.h"
#include <limits.h>
#include <stdlib.h>

void hmr_profile_dump(struct hmr_profile const *prof, FILE *restrict fp)
{
    fprintf(fp, "HEADER: %s\n", prof->header);
    fprintf(fp, "  Name: %s\n", prof->meta.name);
    fprintf(fp, "   Acc: %s\n", prof->meta.acc);
    fprintf(fp, "  Desc: %s\n", prof->meta.desc);
    fprintf(fp, "  Leng: %s\n", prof->meta.leng);
    fprintf(fp, "  Alph: %s\n", prof->meta.alph);
    fprintf(fp, "    GA: %s\n", prof->meta.ga);
    fprintf(fp, "  Name: %s\n", prof->meta.name);
    fprintf(fp, "  ");
    for (unsigned i = 0; i < prof->symbols_size; ++i)
    {
        fprintf(fp, "       %c", prof->symbols[i]);
    }
    fprintf(fp, "\n");
}

static void prof_init(struct hmr_profile *prof, char *error);

void hmr_profile_init(struct hmr_profile *prof, struct hmr *hmr)
{
    prof_init(prof, hmr->error);
}

int hmr_profile_next_node(struct hmr_profile *prof, FILE *restrict fp,
                          struct hmr_position *aux, enum hmr_state *state,
                          struct hmr_token *tok)
{
    if (*state != HMR_FSM_PAUSE)
        return hmr_error(HMR_EUSAGE, prof->error,
                         "unexpected prof_next_node call");

    hmr_aux_init(aux);
    do
    {
        int rc = HMR_OK;
        if ((rc = hmr_token_next(tok, fp))) return rc;

        *state = hmr_fsm_next(*state, tok, aux, prof);
        if (*state == HMR_FSM_ERROR) return HMR_EPARSE;
        if (*state == HMR_FSM_BEGIN)
        {
            if (hmr_profile_length(prof) != prof->node.idx)
                return hmr_error_parse(tok, "profile length mismatch");
            return HMR_END;
        }

    } while (*state != HMR_FSM_PAUSE);

    return HMR_OK;
}

int hmr_profile_next_profile(struct hmr_profile *prof, FILE *restrict fp,
                             struct hmr_position *aux, enum hmr_state *state,
                             struct hmr_token *tok)
{
    if (*state != HMR_FSM_BEGIN)
        return hmr_error(HMR_EUSAGE, prof->error,
                         "unexpected prof_next_prof call");

    prof_init(prof, tok->error);
    hmr_aux_init(aux);
    do
    {
        int rc = HMR_OK;
        if ((rc = hmr_token_next(tok, fp))) return rc;

        if ((*state = hmr_fsm_next(*state, tok, aux, prof)) == HMR_FSM_ERROR)
            return HMR_EPARSE;

    } while (*state != HMR_FSM_PAUSE && *state != HMR_FSM_END);

    if (*state == HMR_FSM_END) return HMR_EOF;

    return HMR_OK;
}

unsigned hmr_profile_length(struct hmr_profile const *prof)
{
    long v = strtol(prof->meta.leng, NULL, 10);
    if (v == LONG_MAX) return UINT_MAX;
    if (v == LONG_MIN) return 0;
    return (unsigned)v;
}

static void prof_init(struct hmr_profile *prof, char *error)
{
    prof->header[0] = '\0';
    prof->meta.name[0] = '\0';
    prof->meta.acc[0] = '\0';
    prof->meta.desc[0] = '\0';
    prof->meta.leng[0] = '\0';
    prof->meta.alph[0] = '\0';
    prof->meta.ga[0] = '\0';
    prof->symbols_size = 0;
    prof->symbols[0] = '\0';
    prof->error = error;
    hmr_node_init(&prof->node);
}
