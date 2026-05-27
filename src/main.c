#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int mish_cd(char **args);
int mish_help(char **args);
int mish_exit(char **args);


typedef struct {
    const char *name;
    int (*func)(char **args);
    const char *description;
}BuiltinCommands;

BuiltinCommands builtins[] = {
    {"cd", mish_cd, "Change the current directory"},
    {"help", mish_help, "Display this help message"},
    {"exit", mish_exit, "Exit the shell"}
};


int mish_cd(char **args) {
    if(args[1] == NULL) {
        char *home = getenv("HOME");
        if(home == NULL) {
            fprintf(stderr, "mish: cd: HOME not set\n");
        } else {
            if(chdir(home) == -1) {
                perror("mish: cd");
            }
        }
    } else {
        if(chdir(args[1]) == -1) {
            perror("mish: cd");
        }
    }
    return 1;
}

int mish_help(char **args) {
    printf("Mish: A simple shell implementation in C\n");
    printf("Built-in commands:\n");
    for(int i = 0; i < sizeof(builtins) / sizeof(BuiltinCommands); i++) {
        printf("  %s: %s\n", builtins[i].name, builtins[i].description);
    }
    return 1;
}

int mish_exit(char **args) {
    return 0;
}




char *mish_read_line() {
    #ifdef MISH_USE_STD_GETLINE
        char *line = NULL;
        size_t bufsize = 0; 
        if(getline(&line, &bufsize, stdin) == -1) {
            if(feof(stdin)) {
                exit(EXIT_SUCCESS); 
            } else {
                perror("mish: getline");
                exit(EXIT_FAILURE);
            }
        }
            return line;
        #else
        #define MISH_READ_CHUNK_SIZE 1024
        int capacity = MISH_READ_CHUNK_SIZE;
        int position = 0;
        char *buffer= malloc(sizeof(char) * capacity);
        int ch;

        if(!buffer) {
            fprintf(stderr, "mish: allocation error\n");
            exit(EXIT_FAILURE);
        }
        while(1) {
            ch = fgetc(stdin);
            if(ch == EOF || ch == '\n') {
                buffer[position] = '\0';
                exit(EXIT_SUCCESS);
            } else {
                buffer[position] = ch;
                position++;
            }

            if(position >= capacity) {
                capacity += MISH_READ_CHUNK_SIZE;
                buffer = realloc(buffer, sizeof(char) * capacity);                                                                                                                                                                                                                                                                  
                if(!buffer) {
                    fprintf(stderr, "mish: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        #endif

}
#define MISH_TOKEN_BUFSIZE 64
#define MISH_TOKEN_DELIMITERS " \t\r\n\a"

char **mish_tokenize_line(char *line) {
    int capacity = MISH_TOKEN_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char *) * capacity);
    char *token;

    if(!tokens) {
        fprintf(stderr, "mish: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, MISH_TOKEN_DELIMITERS);

    while(token != NULL) {
        if(position >= capacity) {
            capacity += MISH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char *) * capacity);
        
            if(!tokens) {
                fprintf(stderr, "mish: allocation error\n");
                exit(EXIT_FAILURE);
            }

        }
        tokens[position] = token;
        position++;

        token = strtok(NULL, MISH_TOKEN_DELIMITERS);

    }
    tokens[position] = NULL;
    return tokens;
}
int mish_execute_command(char **args) {
    if(args[0] == NULL) {
        return 1;
    }
    for(int i = 0; i < sizeof(builtins) / sizeof(BuiltinCommands); i++) {
        if(strcmp(args[0], builtins[i].name) == 0) {
            return builtins[i].func(args);
        }
    }
    pid_t pid = fork();
    if(pid < 0) {
        perror("mish: fork");
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("mish: execvp");
        }
        exit(EXIT_FAILURE);
    }
    waitpid(pid, NULL, 0);
    return 1;
}





int main() {
    char *line;
    char **args;
    int status;

    while(1) {
        printf("> ");
        line = mish_read_line();
        args = mish_tokenize_line(line);
        status = mish_execute_command(args);

        free(line);
        free(args);

        if(status == 0) {
            break;
        }
    }
    return EXIT_SUCCESS;
}