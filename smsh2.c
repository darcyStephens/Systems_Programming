#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "smsh.h"
#include <stdio.h>
#include <string.h>

#define DFL_PROMPT "> "
#define MAX_COMMANDS 1000
#define MAX_CMD_LEN 1024

int main()
{
	char	*cmdline, *prompt, **arglist;
	int	result;
	void	setup();

	prompt = DFL_PROMPT ;
	setup();

	while ( (cmdline = next_cmd(prompt, stdin)) != NULL ){
        int count = count_commands(cmdline);
        //check for pipes, execute if they exist
        if (pipe_check(cmdline) == 1) {
            execute_pipeline(cmdline);
           
        //if no pipes run execute
        } else if ( (arglist = splitline(cmdline)) != NULL  ){
			result = execute(arglist);
			freelist(arglist);
		}
		free(cmdline);
	}
	return 0;
}
  

void setup()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
    fprintf(stderr, "Error: %s,%s\n", s1, s2);
    exit(n);
}
