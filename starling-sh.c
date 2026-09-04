#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

void display_prompt() {
    char hostname[256] = "starling";
    char username[256] = "user";

    gethostname(hostname, sizeof(hostname));
    char *user = getlogin();
    if (user) {
        strcpy(username, user);
    }

    printf("\033[38;5;218m%s\033[0m", username); 
    printf("@"); 
    printf("\033[38;5;229m%s\033[0m", hostname); 
    printf(":\033[38;5;218m~$\033[0m ");

    fflush(stdout);
}

void execute_external (char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            printf("Starling Error: Command not found: %s\n", args[0]);
        }
        
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Starling Error: Fork failed");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

void starling_cd(char **args) {
    if (args[1] == NULL) {
    
        fprintf(stderr, "Starling: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("Starling cd");
        }
    }
}

void starling_touch(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Starling touch: missing file operand\n");
        return;
    }

    int fd = open(args[1], O_WRONLY | O_CREAT, 0644);
    
    if (fd == -1) {
        perror("Starling touch");
        return;
    }

    if (utime(args[1], NULL) != 0) {
        perror("Starling touch (timestamp)");
    }

    close(fd);
}

void starling_echo(char **args) {
    int i = 1;
    int newline = 1;

    if (args[i] != NULL && strcmp(args[i], "-n") == 0) {
        newline = 0;
        i++;
    }

    while (args[i] != NULL) {
        printf("%s", args[i]);
        if (args[i + 1] != NULL) {
            printf(" ");
        }
        i++;
    }

    if (newline) {
        printf("\n");
    }
}

void starling_cat(char **args) {
    int i = 1;
    int show_lines = 0;
    int show_ends = 0;

    while (args[i] != NULL && args[i][0] == '-') {
        if (strcmp(args[i], "-n") == 0) show_lines = 1;
        else if (strcmp(args[i], "-E") == 0) show_ends = 1;
        i++;
    }

    if (args[i] == NULL) {
        fprintf(stderr, "Starling cat: missing file operand\n");
        return;
    }

    while (args[i] != NULL) {
        FILE *file = fopen(args[i], "r");
        if (!file) {
            perror("Starling cat");
        } else {
            char line[1024];
            int line_num = 1;
            while (fgets(line, sizeof(line), file)) {
                if (show_lines) printf("%6d  ", line_num++);
                
                if (show_ends) {
                    line[strcspn(line, "\n")] = 0;
                    printf("%s$\n", line);
                } else {
                    printf("%s", line);
                }
            }
            fclose(file);
        }
        i++;
    }
}

void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            char *filename = args[i + 1];
            if (filename == NULL) {
                fprintf(stderr, "Starling: Redirection error: no file specified\n");
                return;
            }

            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Starling redirection");
                return;
            }

            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("Starling dup2");
                return;
            }
            close(fd);

            args[i] = NULL;
            args[i + 1] = NULL;
            break;
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    char input[1024];

    signal(SIGINT, SIG_IGN);

    while (1) {
        display_prompt();

        int saved_stdout = dup(STDOUT_FILENO);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        char *args[64];
        int i = 0;
        char *token = strtok(input, " ");
        
        while (token != NULL && i < 63) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        handle_redirection(args);

        if (args[0] == NULL) {
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            break;
        } else if (strcmp(args[0], "cd") == 0) {
            starling_cd(args);
        } else if (strcmp(args[0], "touch") == 0) {
            starling_touch(args);
        } else if (strcmp(args[0], "echo") == 0) {
            starling_echo(args);
        } else if (strcmp(args[0], "cat") == 0) {
            starling_cat(args);
        } else {
            execute_external(args);
        }

        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    
    return 0;
}