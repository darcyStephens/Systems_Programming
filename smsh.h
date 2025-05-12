
#ifndef SMSH_H
#define SMSH_H
#define YES 1
#define NO 0
// your functions
char *next_cmd();
char **splitline(char *);
void freelist(char **);
void *emalloc(size_t);
void *erealloc(void *, size_t);
int execute(char **);
void fatal(char *, char *, int);

// my functions

// time savers + debugging
char *duplicatelicate_str(const char *);
int pipe_check(char *);
int count_commands(char *);

// handlers
char **handle_redirect(char **);
char **handle_wildcard(char **);

// executors
int execute_pipeline(char *);
int execute_pipeline_redirect(char *);
int execute_all(char *);

#endif
