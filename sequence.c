#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX_LINE_LENGTH 256



int main()
{
    // create line variable
    char line[MAX_LINE_LENGTH];

    // read file line by line
    while (fgets(line, MAX_LINE_LENGTH, stdin))
    {
       
        int i = 0;
        char *parameters[110];
        char *token = strtok(line, " \n");

        if (token == NULL)
        {
            continue;
        }
        

        while (token != NULL && i < 110)
        {
             // tokenise
            parameters[i] = token;
            token = strtok(NULL, " \n");
            i++;
           
        }
        //setting the last value to null so execvp doesn't cry
        parameters[i] = NULL;

        // create a fork
        pid_t pid = fork();

        // launch child process
        if (pid == 0)
        {
            //execute the command
            execvp(parameters[0], parameters);
        }
        else
        {
            // wait for child to finish
            wait(NULL);
        }
    }

  
    return 0;
}