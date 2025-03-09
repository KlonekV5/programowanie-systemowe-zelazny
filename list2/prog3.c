#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

struct pipe
{
    int r;
    int w;
};

// funkcja parsujaca komendy na tablice
char** parse_command(char* command) {
    char** args = malloc(20 * sizeof(char*)); // max 20 argumentow
    int arg_count = 0;

    char* token = strtok(command, " "); // podzial po spacji
    while (token != NULL) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL; // koniec tablicy
    return args;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file1.c,file2.c,...> <flags1,flags2,...>\n", argv[0]);
        return 1;
    }

    char *files_str = argv[1]; // lista plikow z kodem
    char *flags_str = argv[2]; // lista flag

    // dzielenie plikow na tablice
    char *files[20];
    int file_count = 0;
    char *file = strtok(files_str, ",");
    while (file != NULL) {
        files[file_count++] = file;
        file = strtok(NULL, ",");
    }

    // dzielenie flag na tablice
    char *flags[20];
    int flag_count = 0;
    char *flag = strtok(flags_str, ",");
    while (flag != NULL) {
        flags[flag_count++] = flag;
        flag = strtok(NULL, ",");
    }

    // utworzenie tablicy dla komend
    int cmd_count = file_count;
    char *commands[cmd_count];

    for (int i = 0; i < cmd_count; i++) {
        commands[i] = malloc(512);
        memset(commands[i], 0, 512);
        // przygotowanie komendy kompilacji
        strcat(commands[i], "gcc ");
        strcat(commands[i], files[i]);
        for (int j = 0; j < flag_count; j++) {
            strcat(commands[i], " ");
            strcat(commands[i], flags[j]);
        }
    }

    // tworzenie potokow
    struct pipe pipes[cmd_count - 1];
    for (int i = 0; i < cmd_count - 1; i++) {
        if (pipe((int *) &pipes[i])) {
            perror("pipe error");
        }
    }

    // lista plikow z bledami kompilacji
    char *failed_files[cmd_count];
    int failed_count = 0;

    // zrobienie procesow potomnych
    for (int i = 0; i < cmd_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
        } else if (pid == 0) {
            // kod dziecka

            if (i > 0) {
                dup2(pipes[i - 1].r, STDIN_FILENO);
            }
            if (i < cmd_count - 1) {
                dup2(pipes[i].w, STDOUT_FILENO);
            }

            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j].r);
                close(pipes[j].w);
            }

            // parsowanie komendy
            char **args = parse_command(commands[i]);
            execvp(args[0], args);
            perror("execvp error");
        }
    }

    // kod rodzica
    for (int k = 0; k < cmd_count - 1; k++) {
        close(pipes[k].r);
        close(pipes[k].w);
    }

    // oczekiwanie na zakonczenie procesow potomnych
    for (int i = 0; i < cmd_count; i++) {
        int status;
        pid_t pid = wait(&status);
        if (pid > 0 && WEXITSTATUS(status) != 0) {
            failed_files[failed_count++] = files[i];
        }
    }

    // wyswietlenie bledow kompilacji
    if (failed_count > 0) {
        printf("Bledy kompilacji:\n");
        for (int i = 0; i < failed_count; i++) {
            printf("%s\n", failed_files[i]);
        }
    }

    return 0;
}
