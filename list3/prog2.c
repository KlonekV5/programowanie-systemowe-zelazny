#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int fifoFd = -1;

void send_command(const char *command) {
    if (fifoFd == -1) {
        perror("blad zapisu fifo");
    }
    if (write(fifoFd, command, strlen(command)) == -1) {
        perror("blad zapisu fifo");
    }
}

void interactive() {
    char cmd;
    while (1) {
        cmd = getchar();
        while (getchar() != '\n');

        switch (cmd) {
        case 'd':
            send_command("seek 5\n");
            break;
        case 'a':
            send_command("seek -5\n");
            break;
        case 'w':
            send_command("speed_incr 1\n");
            break;        
        case 's':
            send_command("speed_incr -1\n");
            break;        
        case 'e':
            send_command("pause\n");
            break;        
        case 'q':
            send_command("quit\n");
            return;
        default:
            printf("nierozpoznano komendy: %c\n", cmd);
        }
    }
}

int main(int argc, char *argv[]) {
    const char *fifoName = "zad2Fifo";
    unlink("zad2Fifo");

    if (mkfifo(fifoName, 0666) == -1) {
        perror("blad stworzenia mkfifo");
        exit(1);
    }

    if (fork() == 0) {
        char inputCommand[256]; 
        sprintf(inputCommand, "file=%s", fifoName); 

        // otworz mplayer
        execlp("mplayer", "mplayer", "-slave", "-input", inputCommand, "video.mp4", NULL);
        perror("blad uruchamiania mplayer");
        exit(1);
    }

    fifoFd = open(fifoName, O_WRONLY);
    if (fifoFd == -1) {
        perror("blad otwarcia fifo");
        exit(1);
    }

    write(fifoFd, "pause\n", 0);

    if (argc == 2 && strcmp(argv[1], "control") == 0) {
        interactive();
    }
}

