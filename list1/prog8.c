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
    int cmd_count = 1;
    // zlicz liczbe pelnych komend podanych jako argumenty
    for (int i = 1 ; i < argc; i++) {
        if (!strcmp(argv[i], ",")) {
            cmd_count++;
        }
    }
    // utworz tablice na liczbe komend
    char *commands[cmd_count];
   
    // przypisz 512B pamieci dla kazdej komendy 
    for (int i = 0; i < cmd_count; i++)
    {
        commands[i] = malloc(512);
        memset(commands[i], 0, 512);
    }

    // przypisz komendy do tablicy stringow
    int counter = 0;
    for (int i = 1 ; i < argc; i++){
        if (!strcmp(argv[i], ",")) {
            counter++;
        } else {
            strcat(commands[counter], argv[i]);
            strcat(commands[counter], " ");            
        }        
    }

    // tworzenie pipeow
    struct pipe pipes[cmd_count - 1];
    for (int i = 0; i < cmd_count - 1; i++) {
        if (pipe((int *) &pipes[i])) {
            perror("pipe error");
        }        
    }

    // zrob dzieci
    for (int i = 0; i < cmd_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
        } else if (pid == 0) {
            // kod dziecka

            if (i > 0) {
                // nie pierwszy proces – ustawiamy stdin na poprzedni potok
                dup2(pipes[i - 1].r, STDIN_FILENO);
            }
            if (i < cmd_count - 1) {
                // nie ostatni proces – ustawiamy stdout na obecny potok
                dup2(pipes[i].w, STDOUT_FILENO);
            }

            // zamykamy potoki
            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j].r);
                close(pipes[j].w);
            }

            // parsujemy komendę i uruchamiamy ją
            char** args = parse_command(commands[i]);
            execvp(args[0], args);
            perror("execvp error");
        }
    }
            // Kod rodzica
            for (int k = 0; k < cmd_count - 1; k++) {
                close(pipes[k].r);
                close(pipes[k].w);
            }

            // Oczekiwanie na zakończenie procesów potomnych
            for (int i = 0; i < cmd_count; i++) {
                wait(NULL);
            }

            return 0;
    }