#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();
    if (pid < 0){
        perror("fork fail");
    } else if (pid == 0) {
        printf("proces potomny o PID %d, oraz PPID %d\n", getpid(), getppid());
    } else {
        printf("proces parent o PID %d, oraz PPID %d\n", getpid(), getppid());
    }


}