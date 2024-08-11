#include "hmr.h"
#include "fsm.h"
#include "profile.h"
#include "token.h"
#include <string.h>

void hmr_init(struct hmr *hmr, FILE *restrict fp)
{
    hmr->fp = fp;
    hmr_fsm_init(&hmr->state);
    hmr->error[0] = '\0';
    hmr_token_init(&hmr->tok, hmr->error);
}

int hmr_next_profile(struct hmr *hmr, struct hmr_profile *prof)
{
    return hmr_profile_next_profile(prof, hmr->fp, &hmr->aux, &hmr->state,
                                    &hmr->tok);
}

int hmr_next_node(struct hmr *hmr, struct hmr_profile *prof)
{
    return hmr_profile_next_node(prof, hmr->fp, &hmr->aux, &hmr->state,
                                 &hmr->tok);
}

void hmr_clear_error(struct hmr *hmr) { hmr->error[0] = '\0'; }

int hmr_count_profiles(char const *filepath)
{
    char line[HMR_TOK_LINE_MAX] = {0};
    FILE *fp = fopen(filepath, "r");
    if (!fp) return -1;

    int count = 0;
    while (fgets(line, sizeof line, fp) != NULL)
    {
        if (!strncmp(line, "HMMER3/f", 8)) ++count;
    }

    if (!feof(fp)) count = -1;
    fclose(fp);
    return count;
}
