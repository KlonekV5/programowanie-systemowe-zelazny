#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int pipe_p_to_c[2];
    int pipe_c_to_p[2];
    char buffer[1024];
    pid_t pid;

    pipe(pipe_c_to_p);
    pipe(pipe_p_to_c);

    pid = fork();

    if (pid < 0){
        perror("fork fail");
    } else if (pid == 0) {
        //kod w dziecku
        close(pipe_c_to_p[0]);
        dup2(pipe_c_to_p[1], STDOUT_FILENO);

        close(pipe_p_to_c[1]);
        dup2(pipe_p_to_c[0], STDIN_FILENO);

        execl("/bin/ls", "ls", NULL);

    } else {
        //kod w rodzicu
        close(pipe_c_to_p[1]);
        close(pipe_p_to_c[0]);
        read(pipe_c_to_p[0], buffer, sizeof(buffer) - 1);
        printf("Otrzymano od dziecka:\n%s", buffer);

    }


}