struct h3result;

struct h3result *h3result_new(void);
void h3result_del(struct h3result const *);

int h3result_pack(struct h3result const *, FILE *);
int h3result_unpack(struct h3result *, FILE *);

int h3result_errnum(struct h3result const *);
char const *h3result_errstr(struct h3result const *);

void h3result_print_targets(struct h3result const *, FILE *);
void h3result_print_domains(struct h3result const *, FILE *);

void h3result_print_targets_table(struct h3result const *, FILE *);
void h3result_print_domains_table(struct h3result const *, FILE *);

unsigned h3result_nhits(struct h3result const *);
char const *h3result_hit_name(struct h3result const *, unsigned idx);
char const *h3result_hit_acc(struct h3result const *, unsigned idx);
double h3result_hit_evalue_ln(struct h3result const *, unsigned idx);

char const *h3result_strerror(int rc);

FILE *fopen(char const *filename, char const *mode);
FILE *fdopen(int, char const *);
int fclose(FILE *);
