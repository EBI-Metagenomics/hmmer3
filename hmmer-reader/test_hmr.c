#include "aye.h"
#include "hmmer_reader.h"
#include <math.h>
#include <string.h>

static inline int nearf(float a, float b)
{
    static float const rel_tol = 1e-06f;
    if (a == b) return 1;
    if (isinf(a) || isinf(b)) return 0;
    float diff = fabsf(b - a);
    return (diff <= fabsf(rel_tol * b)) || (diff <= fabsf(rel_tol * a));
}

static inline int neard(double a, double b)
{
    static double const rel_tol = 1e-09;
    if (a == b) return 1;
    if (isinf(a) || isinf(b)) return 0;
    double diff = fabs(b - a);
    return (diff <= fabs(rel_tol * b)) || (diff <= fabs(rel_tol * a));
}

#define near(a, b) _Generic((a), float: nearf, double: neard)(a, b)

void test_hmm_3profs(void);
void test_hmm_empty(void);
void test_hmm_corrupted1(void);
void test_hmm_corrupted2(void);
void test_hmm_corrupted3(void);
void test_hmm_corrupted4(void);
void test_hmm_corrupted5(void);
void test_hmm_corrupted6(void);
void test_hmm_corrupted7(void);
void test_hmm_corrupted8(void);
void test_hmm_noacc(void);

void check_3profs0(struct hmr_profile *prof);
void check_3profs1(struct hmr_profile *prof);
void check_3profs2(struct hmr_profile *prof);

static void (*check_prof[3])(struct hmr_profile *prof) = {
    check_3profs0,
    check_3profs1,
    check_3profs2,
};

int main(void)
{
    aye_begin();
    test_hmm_3profs();
    test_hmm_empty();
    test_hmm_corrupted1();
    test_hmm_corrupted2();
    test_hmm_corrupted3();
    test_hmm_corrupted4();
    test_hmm_corrupted5();
    test_hmm_corrupted6();
    test_hmm_corrupted7();
    test_hmm_corrupted8();
    test_hmm_noacc();
    return aye_end();
}

void test_hmm_3profs(void)
{
    FILE *fp = fopen("data/three-profs.hmm", "r");
    aye(fp != NULL);
    unsigned symbol_size = 20;
    unsigned prof_size[] = {40, 235, 449};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_profile(&hmr, &prof)))
    {
        aye(prof.symbols_size == symbol_size);
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            aye(prof.node.idx == node_idx);
            check_prof[prof_idx](&prof);
            node_idx++;
        }
        aye(prof.node.idx == prof_size[prof_idx]);
        aye(hmr_profile_length(&prof) == prof_size[prof_idx]);
        prof_idx++;
    }
    aye(prof_idx == 3);

    fclose(fp);
}

void test_hmm_empty(void)
{
    FILE *fp = fopen("data/empty.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    aye(hmr_next_profile(&hmr, &prof) == HMR_EOF);
    aye(hmr_next_profile(&hmr, &prof) == HMR_EUSAGE);
    aye(!strcmp(hmr.error, "Runtime error: unexpected prof_next_prof call"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted1(void)
{
    FILE *fp = fopen("data/corrupted1.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_profile(&hmr, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        if (prof_idx == 2 && node_idx == 6)
        {
            aye(rc == HMR_EPARSE);
            aye(!strcmp(hmr.error,
                        "Parse error: unexpected end-of-file: line 563"));
        }
        if (prof_idx == 2) aye(node_idx == 6);
        prof_idx++;
    }
    aye(prof_idx == 3);
    aye(rc == HMR_EUSAGE);
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted2(void)
{
    FILE *fp = fopen("data/corrupted2.hmm", "r");
    aye(fp != NULL);
    unsigned prof_size[] = {40};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    unsigned node_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_profile(&hmr, &prof)))
    {
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        aye(prof.node.idx == prof_size[prof_idx]);
        aye(hmr_profile_length(&prof) == prof_size[prof_idx]);
        prof_idx++;
        aye(rc == HMR_END);
    }
    aye(node_idx == 41);
    aye(prof_idx == 1);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error,
                "Parse error: expected content before end-of-line: line 153"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted3(void)
{
    FILE *fp = fopen("data/corrupted3.hmm", "r");
    aye(fp != NULL);
    unsigned prof_size[] = {40};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    unsigned node_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_profile(&hmr, &prof)))
    {
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        aye(prof.node.idx == prof_size[prof_idx]);
        aye(hmr_profile_length(&prof) == 0);
        prof_idx++;
    }
    aye(node_idx == 0);
    aye(prof_idx == 0);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error, "Parse error: missing LENG field"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted4(void)
{
    FILE *fp = fopen("data/corrupted4.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_profile(&hmr, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        aye(rc == HMR_EPARSE);
        aye(!strcmp(hmr.error,
                    "Parse error: profile length mismatch: line 33"));
        aye(node_idx == 3);
        aye(prof.node.idx == 2);
        aye(hmr_profile_length(&prof) == 40);
        prof_idx++;
    }
    aye(prof_idx == 1);
    aye(rc == HMR_EOF);
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted5(void)
{
    FILE *fp = fopen("data/corrupted5.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error, "Parse error: unexpected token: line 4"));
    rc = hmr_next_node(&hmr, &prof);
    aye(rc == HMR_EUSAGE);
    aye(!strcmp(hmr.error, "Runtime error: unexpected prof_next_node call"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted6(void)
{
    FILE *fp = fopen("data/corrupted6.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_OK);
    rc = hmr_next_node(&hmr, &prof);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error,
                "Parse error: failed to parse decimal number: line 25"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted7(void)
{
    FILE *fp = fopen("data/corrupted7.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error, "Parse error: invalid header: line 1"));
    rc = hmr_next_node(&hmr, &prof);
    aye(rc == HMR_EUSAGE);
    aye(!strcmp(hmr.error, "Runtime error: unexpected prof_next_node call"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_corrupted8(void)
{
    FILE *fp = fopen("data/corrupted8.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_EPARSE);
    aye(!strcmp(hmr.error, "Parse error: expected i->i: line 9"));
    rc = hmr_next_node(&hmr, &prof);
    aye(rc == HMR_EUSAGE);
    aye(!strcmp(hmr.error, "Runtime error: unexpected prof_next_node call"));
    hmr_clear_error(&hmr);
    aye(!strcmp(hmr.error, ""));

    fclose(fp);
}

void test_hmm_noacc(void)
{
    FILE *fp = fopen("data/noacc.hmm", "r");
    aye(fp != NULL);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_OK);

    aye(hmr_profile_length(&prof) == 4);
    aye(!strcmp(prof.meta.name, "PTHR10000.orig.30.pir"));
    aye(!strcmp(prof.meta.acc, "PTHR10000.orig.30.pir"));

    rc = hmr_next_profile(&hmr, &prof);
    aye(rc == HMR_EUSAGE);

    fclose(fp);
}

void check_3profs0(struct hmr_profile *prof)
{
    if (prof->node.idx == 0)
    {
        aye(near(prof->node.compo[0], -2.29746));
        aye(near(prof->node.compo[prof->symbols_size - 1], -3.82158));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.00201));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], -0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
    if (prof->node.idx == 1)
    {
        aye(near(prof->node.match[0], -0.34643));
        aye(near(prof->node.match[prof->symbols_size - 1], -7.58384));
        aye(prof->node.excess.map == 1);
        aye(prof->node.excess.cons == 'A');
        aye(prof->node.excess.rf == '-');
        aye(prof->node.excess.mm == '-');
        aye(prof->node.excess.cs == 'H');
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.00201));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510));
    }
    if (prof->node.idx == 40)
    {
        aye(near(prof->node.match[0], -3.29199));
        aye(near(prof->node.match[prof->symbols_size - 1], -3.78781));
        aye(prof->node.excess.map == 55);
        aye(prof->node.excess.cons == 'g');
        aye(prof->node.excess.rf == '-');
        aye(prof->node.excess.mm == '-');
        aye(prof->node.excess.cs == 'T');
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.00135));
        aye(near(prof->node.trans[2], -INFINITY));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
}

void check_3profs1(struct hmr_profile *prof)
{
    if (prof->node.idx == 0)
    {
        aye(near(prof->node.compo[0], -2.47491));
        aye(near(prof->node.compo[prof->symbols_size - 1], -3.46896));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.02633));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
    if (prof->node.idx == 1)
    {
        aye(near(prof->node.match[0], -3.26601));
        aye(near(prof->node.match[prof->symbols_size - 1], -4.14252));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.02633));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510));
    }
    if (prof->node.idx == 235)
    {
        aye(near(prof->node.match[0], -2.75686));
        aye(near(prof->node.match[prof->symbols_size - 1], -3.85418));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.01780));
        aye(near(prof->node.trans[2], -INFINITY));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
}

void check_3profs2(struct hmr_profile *prof)
{
    if (prof->node.idx == 0)
    {
        aye(near(prof->node.compo[0], -2.55148));
        aye(near(prof->node.compo[prof->symbols_size - 1], -3.24305));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.01335));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
    if (prof->node.idx == 1)
    {
        aye(near(prof->node.match[0], -2.77993));
        aye(near(prof->node.match[prof->symbols_size - 1], -2.88211));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.01335));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510));
    }
    if (prof->node.idx == 449)
    {
        aye(near(prof->node.match[0], -3.39753));
        aye(near(prof->node.match[prof->symbols_size - 1], -4.58563));
        aye(near(prof->node.insert[0], -2.68618));
        aye(near(prof->node.insert[prof->symbols_size - 1], -3.61503));
        aye(near(prof->node.trans[0], -0.00900));
        aye(near(prof->node.trans[2], -INFINITY));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0));
        aye(near(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY));
    }
}
