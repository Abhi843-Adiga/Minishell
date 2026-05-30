#ifndef MINI_H
#define MINI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
 

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

#define JOBS        4
#define FG          5
#define BG          6

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct Single
{
    pid_t pid;
    char cmd[150];
    struct Single *link;
}slist;


void scan_input();
char *get_command(char *input_string);
void print(int mac);
int check_command_type(char *command);
void execute_internal_commands(char *input_string);
void execute_external_commands(char *input_string);
void signal_handler(int sig_num);
void insert_first(pid_t cpid);
void delete_first();
void extract_external_commands(char **external_commands);

#endif