#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 


int main() {
    int pipe_fd[2];
    char bytes = 'x';

    // tworzymy pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }
    fcntl(pipe_fd[1], F_SETFL, O_NONBLOCK);

    int bytes_written = 0;
    char buffer[1];
    int max_check = 0;
    int bytes_read_sum = 0;

    // petla zapisuajca po bajcie poki mozemy
    while (1) {
        int result = write(pipe_fd[1], &bytes, 1);
        if (result == -1) {
            // pipe jest pelny
            printf("Pipe jest pelny. Zapisano %d bajtow.\n", bytes_written);
                        
            int bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
            bytes_read_sum += bytes_read;
            printf("Odczytano z pipe, aby zwolnić miejsce: %d bajtów.\n", bytes_read);
            max_check = 1;

        } else if (max_check == 1) {
            bytes_written += 1;
            break;
        }
        else {
            printf("Zapisano %d bajtow.\n", bytes_written);
            bytes_written += 1;
        }        
    }

    printf("Odczytano %d bajtow.\n", bytes_read_sum);            
    printf("aby zapisac %d bajt.\n", bytes_written);
    
    return 0;
}
