 /**
 * Author: Kyle Bendickson
 * 
 * A minishell program that I wrote to get a feel for fork(), exec and wait.
 * It will execute any command that you pass it using execvp.
 * execvp will search for the command passed in using the system's path or you can
 * pass in the path to the command (e.g. /bin/ls).
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>

#ifdef ARG_MAX
#define MAX_ARGS ARG_MAX
#else
#define MAX_ARGS 256
#endif

#define MAX_LINE_LEN 2048
#define MAX_ARG_LEN 2048
// #define DEBUG 1           /* Uncomment to enable argument debugging */

void read_input(char*);
void check_for_EOF();

int main(int argc, char** argv)
{

	char* command;
	char* arg;
	char* command_argv[MAX_ARGS];
	char* token;
	char  line[MAX_LINE_LEN];		/* Buffer for line read in */
	int   argument_count;
	int   i;

	/* Initialize the char* array used for commands arguments */
	for(i = 0; i < MAX_ARGS; ++i)
		command_argv[i] = malloc(MAX_ARG_LEN);

	while(1)
	{
		argument_count = 0;

		printf("%c ", '$');
		check_for_EOF();
		read_input(line);

		token  = strtok(line, " ");
		command = token;

/* Output command read in when in DEBUG mode */
#ifdef DEBUG 
		printf("%s\n", command);
#endif							

		/* By convention, the first value in the argv array must match
		 * the command given, per gnu website.
		 **/
		command_argv[0] = command;
		argument_count++;

		while(token != NULL)
		{
			token = strtok(NULL, " ");
			arg = token;
			command_argv[argument_count] = arg;
			++argument_count;
		}

#ifdef DEBUG
		for(i = 0; i < argument_count; ++i)
			printf("arg[%d]: %s\n", i, command_argv[i]);
#endif

		 if(fork() == 0)
		 { 	
		 	if(execvp(command, command_argv) == -1) 
		 	{
		 		printf("%s: command not found\n", command);
		 	}
		 }
		 else
		 {
		 	wait(NULL);
		 }
	}

	return 0;
}

void read_input(char *line) 
{
    fgets(line, MAX_LINE_LEN, stdin);
    line[strlen(line)-1] = '\0';
}

/* Exits if EOF is the next character in stdin */
void check_for_EOF()
{
	int c;
	c = fgetc(stdin);
	if(c == EOF)
	{
		exit(0);
	}
	else
	{
		ungetc(c, stdin);
	}	
}
