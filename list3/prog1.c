#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    const char *fifoName = "testFifo";
    char bytes = 'x';

    if (mkfifo(fifoName, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    int fifoRdFd = open(fifoName, O_RDONLY | O_NONBLOCK);
    if (fifoRdFd == -1) {
        perror("open fifo read");
        unlink(fifoName);
        exit(1);
    }

    int fifoFd = open(fifoName, O_WRONLY | O_NONBLOCK);
    if (fifoFd == -1) {
        perror("open fifo write");
        close(fifoRdFd);
        unlink(fifoName);
        exit(1);
    }

    int bytesWritten = 0;
    char buffer[1];
    int maxCheck = 0;
    int bytesReadSum = 0;

    // zapisujemy po bajcie 
    while (1) {
        int result = write(fifoFd, &bytes, 1);
        if (result == -1) {
            // potok jest pelen
            printf("FIFO jest pełne. Zapisano %d bajtów.\n", bytesWritten);

            int bytesRead = read(fifoRdFd, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                bytesReadSum += bytesRead;
                printf("Odczytano z FIFO, aby zwolnić miejsce: %d bajtów.\n", bytesRead);
            }
            maxCheck = 1;

        } else if (maxCheck == 1) {
            bytesWritten += 1;
            break;
        } else {
            bytesWritten += 1;
        }
    }

    printf("Odczytano %d bajtów.\n", bytesReadSum);
    printf("Aby zapisać %d bajt.\n", bytesWritten);

    close(fifoFd);
    close(fifoRdFd);
    unlink(fifoName);

    return 0;
}
