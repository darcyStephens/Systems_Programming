#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_commands 1000
#define MAX_CMD_LEN 1024

//executes a pipeline of commans
void execute_pipeline(char *commands[], int num_commands) {
    int pipes[num_commands - 1][2];
    //create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    //looping through each command in the plumbing
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (i != 0) {
                //input redirection setup
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            //output redirection setup
            if (i != num_commands - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Close all pipe ends in child processes except the ones it uses
            for (int j = 0; j < num_commands - 1; j++) {
                if ((i != j) && (i != j + 1)) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            // Execute the command
            char *arguments[MAX_CMD_LEN];
            char *token = strtok(commands[i], " ");
            int arg_index = 0;
            while (token != NULL) {
                arguments[arg_index++] = token;
                token = strtok(NULL, " ");
            }
            arguments[arg_index] = NULL;

            execvp(arguments[0], arguments);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipe ends in parent process
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}

int main(void) {
    char *commands[MAX_commands];
    int num_commands = 0;

    char line[MAX_CMD_LEN];
    //read in commands
    while ((fgets(line, MAX_CMD_LEN, stdin)) != NULL) {
        line[strcspn(line, "\n")] = '\0'; 
        char * cmd = malloc(strlen(line) + 1);
        if (cmd == NULL){
            fprintf(stderr, "Memory allocation cmd failed");
            exit(EXIT_FAILURE);
        }
        //copy command
        strcpy(cmd, line);
        commands[num_commands++] = cmd;  
        if (num_commands >= MAX_commands) {
            fprintf(stderr, "Max number of commands reached\n");
            break;
        }
    }

    if (num_commands > 0) {
        execute_pipeline(commands, num_commands);
    }

    // Free memory
    for (int i = 0; i < num_commands; i++) {
        free(commands[i]);
    }

    return 0;
}