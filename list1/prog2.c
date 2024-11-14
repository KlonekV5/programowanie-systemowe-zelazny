#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int pipefd[2];
    char message[] = "czesc rodzic :)";
    
    pipe(pipefd);
    pid_t pid = fork();

    if (pid < 0){
        perror("fork fail");
    } else if (pid == 0) {
        //kod w dziecku
        close(pipefd[0]);
        write(pipefd[1], message, strlen(message) + 1);
        close(pipefd[1]);
    } else {
        //kod w rodzicu
        close(pipefd[1]);
        char message_recived[100];
        read(pipefd[0], message_recived, sizeof(message_recived));
        close(pipefd[0]);
        printf("proces parent o PID %d, wiadomosc to '%s'\n", getpid(), message_recived);

    }


}