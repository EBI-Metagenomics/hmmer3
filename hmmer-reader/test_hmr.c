#include "hmr.h"
#include "hope.h"

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

void check_3profs0(struct hmr_prof *prof);
void check_3profs1(struct hmr_prof *prof);
void check_3profs2(struct hmr_prof *prof);

static void (*check_prof[3])(struct hmr_prof *prof) = {
    check_3profs0,
    check_3profs1,
    check_3profs2,
};

int main(void)
{
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
    return hope_status();
}

void test_hmm_3profs(void)
{
    FILE *fp = fopen("three-profs.hmm", "r");
    NOTNULL(fp);
    unsigned symbol_size = 20;
    unsigned prof_size[] = {40, 235, 449};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_prof(&hmr, &prof)))
    {
        EQ(prof.symbols_size, symbol_size);
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            EQ(prof.node.idx, node_idx);
            check_prof[prof_idx](&prof);
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(hmr_prof_length(&prof), prof_size[prof_idx]);
        prof_idx++;
    }
    EQ(prof_idx, 3);

    fclose(fp);
}

void test_hmm_empty(void)
{
    FILE *fp = fopen("empty.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    EQ(hmr_next_prof(&hmr, &prof), HMR_EOF);
    EQ(hmr_next_prof(&hmr, &prof), HMR_EUSAGE);
    EQ(hmr.error, "Runtime error: unexpected prof_next_prof call");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted1(void)
{
    FILE *fp = fopen("corrupted1.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_prof(&hmr, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        if (prof_idx == 2 && node_idx == 6)
        {
            EQ(rc, HMR_EPARSE);
            EQ(hmr.error, "Parse error: unexpected end-of-file: line 563");
        }
        if (prof_idx == 2) EQ(node_idx, 6);
        prof_idx++;
    }
    EQ(prof_idx, 3);
    EQ(rc, HMR_EUSAGE);
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted2(void)
{
    FILE *fp = fopen("corrupted2.hmm", "r");
    NOTNULL(fp);
    unsigned prof_size[] = {40};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    unsigned node_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_prof(&hmr, &prof)))
    {
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(hmr_prof_length(&prof), prof_size[prof_idx]);
        prof_idx++;
        EQ(rc, HMR_END);
    }
    EQ(node_idx, 41);
    EQ(prof_idx, 1);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: expected content before end-of-line: line 153");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted3(void)
{
    FILE *fp = fopen("corrupted3.hmm", "r");
    NOTNULL(fp);
    unsigned prof_size[] = {40};

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    unsigned node_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_prof(&hmr, &prof)))
    {
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        EQ(prof.node.idx, prof_size[prof_idx]);
        EQ(hmr_prof_length(&prof), 0);
        prof_idx++;
    }
    EQ(node_idx, 0);
    EQ(prof_idx, 0);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: missing LENG field");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted4(void)
{
    FILE *fp = fopen("corrupted4.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    unsigned prof_idx = 0;
    int rc = HMR_OK;
    while (!(rc = hmr_next_prof(&hmr, &prof)))
    {
        unsigned node_idx = 0;
        while (!(rc = hmr_next_node(&hmr, &prof)))
        {
            node_idx++;
        }
        EQ(rc, HMR_EPARSE);
        EQ(hmr.error, "Parse error: profile length mismatch: line 33");
        EQ(node_idx, 3);
        EQ(prof.node.idx, 2);
        EQ(hmr_prof_length(&prof), 40);
        prof_idx++;
    }
    EQ(prof_idx, 1);
    EQ(rc, HMR_EOF);
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted5(void)
{
    FILE *fp = fopen("corrupted5.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: unexpected token: line 4");
    rc = hmr_next_node(&hmr, &prof);
    EQ(rc, HMR_EUSAGE);
    EQ(hmr.error, "Runtime error: unexpected prof_next_node call");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted6(void)
{
    FILE *fp = fopen("corrupted6.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_OK);
    rc = hmr_next_node(&hmr, &prof);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: failed to parse decimal number: line 25");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted7(void)
{
    FILE *fp = fopen("corrupted7.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: invalid header: line 1");
    rc = hmr_next_node(&hmr, &prof);
    EQ(rc, HMR_EUSAGE);
    EQ(hmr.error, "Runtime error: unexpected prof_next_node call");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_corrupted8(void)
{
    FILE *fp = fopen("corrupted8.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_EPARSE);
    EQ(hmr.error, "Parse error: expected i->i: line 9");
    rc = hmr_next_node(&hmr, &prof);
    EQ(rc, HMR_EUSAGE);
    EQ(hmr.error, "Runtime error: unexpected prof_next_node call");
    hmr_clear_error(&hmr);
    EQ(hmr.error, "");

    fclose(fp);
}

void test_hmm_noacc(void)
{
    FILE *fp = fopen("noacc.hmm", "r");
    NOTNULL(fp);

    HMR_DECLARE(hmr, fp);

    HMR_PROF_DECLARE(prof, &hmr);

    int rc = HMR_OK;
    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_OK);

    EQ(hmr_prof_length(&prof), 4);
    EQ(prof.meta.name, "PTHR10000.orig.30.pir");
    EQ(prof.meta.acc, "PTHR10000.orig.30.pir");

    rc = hmr_next_prof(&hmr, &prof);
    EQ(rc, HMR_EUSAGE);

    fclose(fp);
}

void check_3profs0(struct hmr_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.29746);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.82158);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00201);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], -0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -0.34643);
        CLOSE(prof->node.match[prof->symbols_size - 1], -7.58384);
        EQ(prof->node.excess.map, 1);
        EQ(prof->node.excess.cons, 'A');
        EQ(prof->node.excess.rf, '-');
        EQ(prof->node.excess.mm, '-');
        EQ(prof->node.excess.cs, 'H');
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00201);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 40)
    {
        CLOSE(prof->node.match[0], -3.29199);
        CLOSE(prof->node.match[prof->symbols_size - 1], -3.78781);
        EQ(prof->node.excess.map, 55);
        EQ(prof->node.excess.cons, 'g');
        EQ(prof->node.excess.rf, '-');
        EQ(prof->node.excess.mm, '-');
        EQ(prof->node.excess.cs, 'T');
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00135);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
}

void check_3profs1(struct hmr_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.47491);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.46896);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.02633);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -3.26601);
        CLOSE(prof->node.match[prof->symbols_size - 1], -4.14252);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.02633);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 235)
    {
        CLOSE(prof->node.match[0], -2.75686);
        CLOSE(prof->node.match[prof->symbols_size - 1], -3.85418);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01780);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
}

void check_3profs2(struct hmr_prof *prof)
{
    if (prof->node.idx == 0)
    {
        CLOSE(prof->node.compo[0], -2.55148);
        CLOSE(prof->node.compo[prof->symbols_size - 1], -3.24305);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01335);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
    if (prof->node.idx == 1)
    {
        CLOSE(prof->node.match[0], -2.77993);
        CLOSE(prof->node.match[prof->symbols_size - 1], -2.88211);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.01335);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -0.95510);
    }
    if (prof->node.idx == 449)
    {
        CLOSE(prof->node.match[0], -3.39753);
        CLOSE(prof->node.match[prof->symbols_size - 1], -4.58563);
        CLOSE(prof->node.insert[0], -2.68618);
        CLOSE(prof->node.insert[prof->symbols_size - 1], -3.61503);
        CLOSE(prof->node.trans[0], -0.00900);
        CLOSE(prof->node.trans[2], -INFINITY);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 2], 0.0);
        CLOSE(prof->node.trans[HMR_TRANS_SIZE - 1], -INFINITY);
    }
}
