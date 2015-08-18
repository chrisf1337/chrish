#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
*/
int chrish_cd(char **args);
int chrish_help(char **args);
int chrish_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &chrish_cd,
    &chrish_help,
    &chrish_exit
};

int chrish_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
*/
int chrish_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "chrish: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("chrish");
        }
    }
    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
*/
int chrish_help(char **args)
{
    int i;
    printf("Stephen Brennan's CHRISH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < chrish_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
*/
int chrish_exit(char **args)
{
    return 0;
}

/**
   @brief Launch a program and wait for it to terminate.
   @param args Null terminated list of arguments (including program).
   @return Always returns 1, to continue execution.
*/
int chrish_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("chrish");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("chrish");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
*/
int chrish_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < chrish_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return chrish_launch(args);
}

#define CHRISH_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
*/
int chrish_read_line(char **line)
{
    size_t bufsize = 0;
    return getline(line, &bufsize, stdin);
}

#define CHRISH_TOK_BUFSIZE 64
#define CHRISH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
*/
char **chrish_split_line(char *line)
{
    int bufsize = CHRISH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;
    int n_tokens = 0;

    if (!tokens) {
        fprintf(stderr, "chrish: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CHRISH_TOK_DELIM);
    while (token != NULL) {
        n_tokens++;
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += CHRISH_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "chrish: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CHRISH_TOK_DELIM);
    }
    tokens[position] = NULL;
    printf("%d\n", n_tokens);
    return tokens;
}

/**
   @brief Loop getting input and executing it.
*/
void chrish_loop(void)
{
    char **args;
    int status;

    do {
        char *line = NULL;
        printf("> ");
        int ret = chrish_read_line(&line);
        printf("%d\n", ret);
        if (ret == -1)
        {
            free(line);
            free(args);
            exit(EXIT_SUCCESS);
        }
        args = chrish_split_line(line);
        status = chrish_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
*/
int main(int argc, char **argv)
{
    // Load config files, if any.

    // Run command loop.
    chrish_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}
