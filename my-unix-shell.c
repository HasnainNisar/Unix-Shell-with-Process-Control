/*
 * my-unix-shell.c
 *
 * Author: Hasnain Ali
 * NED University of Engineering & Technology
 * 2026
 *
 * A Unix-like shell supporting process control,
 * piping, I/O redirection, and command history.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAXLINE 1024
#define LIMIT 256

// Store last command for history feature
char history[MAXLINE];

// Signal handler for Ctrl+C
void signalHandler_int(int p) {
    printf("\nUse 'exit' to quit the shell.\n");
}

// Print shell prompt
void shellPrompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("hasnain-shell %s > ", cwd);
    fflush(stdout);
}

// Change directory
void changeDirectory(char *args[]) {
    if (args[1] == NULL) {
        chdir(getenv("HOME"));
    } else {
        if (chdir(args[1]) == -1) {
            printf("cd: no such directory: %s\n", args[1]);
        }
    }
}

// Handle pipes
void pipeHandler(char *args[]) {
    int filedes[2];
    int i = 0;
    char *cmd1[LIMIT];
    char *cmd2[LIMIT];
    int j = 0;

    // Split commands by '|'
    while (args[i] != NULL && strcmp(args[i], "|") != 0) {
        cmd1[i] = args[i];
        i++;
    }
    cmd1[i] = NULL;
    i++; // skip '|'

    while (args[i] != NULL) {
        cmd2[j] = args[i];
        i++;
        j++;
    }
    cmd2[j] = NULL;

    // Create pipe
    if (pipe(filedes) == -1) {
        printf("Pipe creation failed\n");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // First child: write to pipe
        dup2(filedes[1], STDOUT_FILENO);
        close(filedes[0]);
        close(filedes[1]);
        execvp(cmd1[0], cmd1);
        printf("Command not found: %s\n", cmd1[0]);
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Second child: read from pipe
        dup2(filedes[0], STDIN_FILENO);
        close(filedes[1]);
        close(filedes[0]);
        execvp(cmd2[0], cmd2);
        printf("Command not found: %s\n", cmd2[0]);
        exit(1);
    }

    // Parent closes pipe and waits
    close(filedes[0]);
    close(filedes[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Handle output redirection
void redirectOutput(char *args[], char *outputFile) {
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execvp(args[0], args);
        printf("Command not found: %s\n", args[0]);
        exit(1);
    }
    waitpid(pid, NULL, 0);
}

// Launch a program
void launchProg(char *args[], int background) {
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        execvp(args[0], args);
        printf("Command not found: %s\n", args[0]);
        exit(1);
    }
    if (background == 0) {
        waitpid(pid, NULL, 0);
    } else {
        printf("Process running in background with PID: %d\n", pid);
    }
}

// Handle commands
void commandHandler(char *args[]) {
    if (args[0] == NULL) return;

    // exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    // pwd
    else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    }
    // clear
    else if (strcmp(args[0], "clear") == 0) {
        system("clear");
    }
    // cd
    else if (strcmp(args[0], "cd") == 0) {
        changeDirectory(args);
    }
    // help
    else if (strcmp(args[0], "help") == 0) {
        printf("\n=== Hasnain Shell - Available Commands ===\n");
        printf("  exit          - Exit the shell\n");
        printf("  pwd           - Print current directory\n");
        printf("  cd <dir>      - Change directory\n");
        printf("  clear         - Clear the screen\n");
        printf("  !!            - Repeat last command\n");
        printf("  <cmd> &       - Run command in background\n");
        printf("  <cmd> > <file>- Redirect output to file\n");
        printf("  <cmd1>|<cmd2> - Pipe commands\n");
        printf("==========================================\n\n");
    }
    // check for pipe
    else {
        int i = 0;
        int background = 0;
        char *args_aux[LIMIT];
        int j = 0;

        while (args[i] != NULL) {
            if (strcmp(args[i], "|") == 0) {
                pipeHandler(args);
                return;
            }
            if (strcmp(args[i], ">") == 0) {
                if (args[i+1] != NULL) {
                    args_aux[j] = NULL;
                    redirectOutput(args_aux, args[i+1]);
                    return;
                }
            }
            if (strcmp(args[i], "&") == 0) {
                background = 1;
                break;
            }
            args_aux[j] = args[i];
            i++;
            j++;
        }
        args_aux[j] = NULL;
        launchProg(args_aux, background);
    }
}

int main() {
    char line[MAXLINE];
    char *tokens[LIMIT];
    int numTokens;

    memset(history, '\0', MAXLINE);

    signal(SIGINT, signalHandler_int);

    printf("\n==========================================\n");
    printf("         Hasnain Shell v1.0\n");
    printf("         NED University 2026\n");
    printf("==========================================\n\n");

    while (1) {
        shellPrompt();
        memset(line, '\0', MAXLINE);
        fgets(line, MAXLINE, stdin);

        // Handle !! history
        if (strncmp(line, "!!", 2) == 0) {
            if (strlen(history) == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(line, history);
            printf("%s", line);
        } else {
            strcpy(history, line);
        }

        // Tokenize input
        if ((tokens[0] = strtok(line, " \n\t")) == NULL) continue;
        numTokens = 1;
        while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;

        commandHandler(tokens);
    }

    return 0;
}
