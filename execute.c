/* execute.c - code used by small shell to execute commands */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "smsh.h"
#include <fcntl.h>
#include <glob.h>
#define MAX_commands 1000
#define MAX_CMD_LEN 1024


char *duplicatelicate_str(const char *source)
{
	// duplicates string
	//return the duplicate

	// make space
	char *destination = malloc(strlen(source) + 1);

	if (destination)
	{
		strcpy(destination, source);
	}
	// return the new duplicated string
	return destination;
}
char **handle_redirect(char **command)
{
	//handles redirection of commands by opening and closing the appropriate i/o streams
	//returns the passed in parameter with its i/o stream modified

	// file comes after the command
	int i = 0;
	while (command[i] != NULL)
	{
		if (strcmp(command[i], ">") == 0)
		{
			// create a write only file descriptor for the file past >
			int fd = open(command[i + 1], O_CREAT | O_WRONLY, 0777);
			dup2(fd, STDOUT_FILENO);
			close(fd);
			command[i] = NULL;
			command[i + 1] = NULL;
		}
		else if (strcmp(command[i], "<") == 0)
		{
			// create a read only descriptor for the file past <
			int fd = open(command[i + 1], O_RDONLY);
			dup2(fd, STDIN_FILENO);
			close(fd);
			command[i] = NULL;
			command[i + 1] = NULL;
		}
		i++;
	}
	return command;
}

char **handle_wildcard(char **command)
{
	// modify the path to then be executed in the execute all function
	//  if no wildcard found path is duplicated and duplicate returned unmodified

	int new_argcount = 0;
	// initialise flags
	int glob_flags = 0;

	// make space for the new list
	char **new_argL = malloc(MAX_CMD_LEN * sizeof(char *));

	for (int index = 0; command[index] != NULL; index++)
	{
		// checking for wildcards
		if (strchr(command[index], '?') != NULL || strchr(command[index], '*') != NULL)
		{
			// create glob if wildcard found
			glob_t buffer;

			
			if (glob(command[index], glob_flags, NULL, &buffer) == 0)
			{
				//for the number of elements in the path
				for (int i = 0; i < buffer.gl_pathc; i++)
				{
					// adding matches to the path
					new_argL[new_argcount] = duplicatelicate_str(buffer.gl_pathv[i]);
					new_argcount++;
				}
			}
			//free up space
			globfree(&buffer);
		}
		else
		{
			//no wildcard, duplicate the command anyway, inefficent but keeps function to completing one task and saves execvp'ing and ending up in multiple process hell
			new_argL[new_argcount] = duplicatelicate_str(command[index]);
		}
		new_argcount++;
	}
	//setting last value to null
	new_argL[new_argcount] = NULL;

	return new_argL;
}

int pipe_check(char *commandLine)
{
	//checks if a pipe exists
	//returns 1 if it exists, -1 if it doesn't

	int pipeExists = -1;
	for (int i = 0; i < strlen(commandLine); i++)
	{
		if (commandLine[i] == '|')
		{
			pipeExists = 1;
		}
	}
	return pipeExists;
}

int count_commands(char *command_line)
{
	//counts the number of commands coming from input. used to debugging mainly, keeping into show my workings
	//returns the number of commmands.
	// if command line is empty
	if (command_line == NULL || strlen(command_line) == 0)
	{
		return 0;
	}

	// will be at least one command is
	int num_commands = 1;

	// flag to check if in quotes
	int in_quotes = 0;

	for (char *c = command_line; *c != '\0'; c++)
	{
		if (*c == '|')
		{
			// Pipe symbol indicates separate commands, but not within quotes
			if (!in_quotes)
			{
				num_commands++;
			}
		}
		else if (*c == '"')
		{
			// Toggle quote state
			in_quotes = !in_quotes;
		}
		else if (*c == '>' || *c == '<')
		{
			// Redirection symbols don't count as separate commands
			continue;
		}
	}

	return num_commands;
}

int execute(char *argv[])
/*
 * purpose: run a program passing it arguments
 * returns: status returned via wait, or -1 on error
 *  errors: -1 on fork() or wait() errors

 */
{
	int pid;
	int child_info = -1;

	if (argv[0] == NULL) /* nothing succeeds	*/
		return 0;

	if ((pid = fork()) == -1)
		perror("fork");
	else if (pid == 0)
	{

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execvp(argv[0], argv);
		perror("cannot execute command");
		exit(1);
	}
	else
	{
		if (wait(&child_info) == -1)
			perror("wait");
	}
	return child_info;
}

int execute_pipeline(char *commandLine)
{
	//extension of execute, can handle piping commands into one another
	//returns 0 if successful

	//intialise the variables needed
	char *commands[10];
	char *token;
	int numcommands = 0;
	// intialise previous and current pipes
	int prevPipe[2];
	int currentPipe[2];

	// tokenise the commandline around pipes
	token = strtok(commandLine, "|");
	while (token != NULL && numcommands < 10)
	{
		commands[numcommands] = token;
		numcommands++;
		token = strtok(NULL, "|");
	}
	commands[numcommands] = NULL;

	

	for (int i = 0; i < numcommands; i++)
	{

		
		if (i < numcommands - 1)
		{ //create pipe
			if (pipe(currentPipe) == -1)
			{
				perror("pipe error");
				exit(EXIT_FAILURE);
			}
		}

		// fork
		int pid = fork();

		// error handling fork
		if (pid < 0)
		{
			perror("fork failed");
			exit(EXIT_FAILURE);
		}

		// fork for child
		if (pid == 0)
		{
			// Redirecting input from the last command's output
			if (i != 0)
			{
				close(prevPipe[1]);
				dup2(prevPipe[0], STDIN_FILENO);
				close(prevPipe[0]);
				
			}

			// Redirecting output to the next command's input
			if (i < numcommands - 1)
			{
				close(currentPipe[0]);
				dup2(currentPipe[1], STDOUT_FILENO);
				close(currentPipe[1]);
			}

			// execute command
			char **cmd = splitline(commands[i]);

			if (execvp(cmd[0], cmd) == -1)
			{
				//handling if the call to exec fails
				perror("execvp failed");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			// Parent process

			// close previoous pipe
			if (i != 0)
			{
				close(prevPipe[0]);
				close(prevPipe[1]);
			}

			// set previous pipe to be current pipe
			if (i < numcommands - 1)
			{
				prevPipe[0] = currentPipe[0];
				prevPipe[1] = currentPipe[1];
			}

			// wait
			wait(NULL);
		}
	}

	return 0;
}

int execute_pipeline_redirect(char *commandLine)
{
	//extends execute_pipeling to include redirection. redirection is handled in it's own function and executed here
	//return 0 if successful

	//intialise the variables needed
	char *commands[10];
	char *token;
	int numcommands = 0;
	// intialise previous and current pipes
	int prevPipe[2];
	int currentPipe[2];

	// tokenise the commandline around pipes
	token = strtok(commandLine, "|");
	while (token != NULL && numcommands < 10)
	{
		commands[numcommands] = token;
		numcommands++;
		token = strtok(NULL, "|");
	}
	commands[numcommands] = NULL;

	

	for (int i = 0; i < numcommands; i++)
	{

		
		if (i < numcommands - 1)
		{ //create pipe
			if (pipe(currentPipe) == -1)
			{
				perror("pipe error");
				exit(EXIT_FAILURE);
			}
		}

		// fork
		int pid = fork();

		// error handling fork
		if (pid < 0)
		{
			perror("fork failed");
			exit(EXIT_FAILURE);
		}

		// fork for child
		if (pid == 0)
		{
			// Redirecting input from the last command's output
			if (i != 0)
			{
				close(prevPipe[1]);
				dup2(prevPipe[0], STDIN_FILENO);
				close(prevPipe[0]);
				
			}

			// Redirecting output to the next command's input
			if (i < numcommands - 1)
			{
				close(currentPipe[0]);
				dup2(currentPipe[1], STDOUT_FILENO);
				close(currentPipe[1]);
			}

			// execute command
			char **cmd = splitline(commands[i]);
			cmd = handle_redirect(cmd);
		

			if (execvp(cmd[0], cmd) == -1)
			{
				perror("execvp failed");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			// Parent process

			// close prev pipe
			if (i != 0)
			{
				close(prevPipe[0]);
				close(prevPipe[1]);
			}

			// set previous pipe to be current pipe
			if (i < numcommands - 1)
			{
				prevPipe[0] = currentPipe[0];
				prevPipe[1] = currentPipe[1];
			}

			// wait
			wait(NULL);
		}
	}

	return 0;
}

int execute_all(char *commandLine)
{
	//final extension, executes wildcards. wild cards are handled in their own function.
	//returns 0 if successful
	//intialise the variables needed
	char *commands[10];
	char *token;
	int numcommands = 0;
	// intialise previous and current pipes
	int prevPipe[2];
	int currentPipe[2];

	// tokenise the commandline around pipes
	token = strtok(commandLine, "|");
	while (token != NULL && numcommands < 10)
	{
		commands[numcommands] = token;
		numcommands++;
		token = strtok(NULL, "|");
	}
	commands[numcommands] = NULL;

	

	for (int i = 0; i < numcommands; i++)
	{

		
		if (i < numcommands - 1)
		{ //create pipe
			if (pipe(currentPipe) == -1)
			{
				perror("pipe error");
				exit(EXIT_FAILURE);
			}
		}

		// fork
		int pid = fork();

		// error handling fork
		if (pid < 0)
		{
			perror("fork failed");
			exit(EXIT_FAILURE);
		}

		// fork for child
		if (pid == 0)
		{
			// Redirecting input from the last command's output
			if (i != 0)
			{
				close(prevPipe[1]);
				dup2(prevPipe[0], STDIN_FILENO);
				close(prevPipe[0]);
				
			}

			// Redirecting output to the next command's input
			if (i < numcommands - 1)
			{
				close(currentPipe[0]);
				dup2(currentPipe[1], STDOUT_FILENO);
				close(currentPipe[1]);
			}


			// execute command
			char **cmd = splitline(commands[i]);
			cmd = handle_redirect(cmd);
			cmd = handle_wildcard(cmd);

			if (execvp(cmd[0], cmd) == -1)
			{
				perror("execvp failed");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			// Parent process

			// close prev pipe
			if (i != 0)
			{
				close(prevPipe[0]);
				close(prevPipe[1]);
			}

			// set previous pipe to be current pipe
			if (i < numcommands - 1)
			{
				prevPipe[0] = currentPipe[0];
				prevPipe[1] = currentPipe[1];
			}

			// wait
			wait(NULL);
		}
	}

	return 0;
}
