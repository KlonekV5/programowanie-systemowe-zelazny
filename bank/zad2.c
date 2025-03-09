#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>

#define TRANSACTIONS 16
#define TRANSACTIONS_PER_USER 4
#define USERS 4


int balance = 0;
pthread_mutex_t lock;

int operations[TRANSACTIONS] = { 56, 250, 500, 2500, -800, -23, -150,
    830, -320, 48, -1200, -249, -2250, 600, 190, 18 }; 


void print_balance (int *id , char *action, int amount) {
    if (amount < 0) {
        amount = -amount;
    }
    printf("\n+-------------------------------------------------------+\n");
    printf("| Client Name | Operation Type | Amount | Final Balance |\n");
    printf("|-------------------------------------------------------|\n");
    printf("| User % 6d | %-14s | % 6d | %13d |\n", *id, action, amount, balance);
    printf("+-------------------------------------------------------+\n");
}


void *use_atm(void *id_ptr) {
    int *id = id_ptr;
    char *action;

    printf("+ thread %d\n", *id);
    srand(time(NULL));
    sleep(1);

    for (int i = 0; i < TRANSACTIONS_PER_USER; i++) {

        while (true) {
            int random_sel = rand() % TRANSACTIONS;
            
            pthread_mutex_lock(&lock);
            // critical section
            int op = operations[random_sel];
          
            if (op > 0) {
                action = "Deposit";
            } else {
                action = "Withdrawal";
            }

            if (balance + op < 0) {
                printf("\n? thread %d cannot %s %d\n", *id, action, -op);
                usleep((rand() % 49 + 1) * 10000);
            }
            if (balance + op >= 0 && op != 0) { // check if transaction can be done and if was made before and
                
                // simulate operation sleep from 10 to 500 ms.
                usleep((rand() % 49 + 1) * 10000);

                balance += op;
                operations[random_sel] = 0;
                print_balance(id, action, op);
                
                pthread_mutex_unlock(&lock);
                break;
            }
            pthread_mutex_unlock(&lock);
        }
    }

    printf("- thread %d\n", *id);
    return NULL;
}

int main() {
    pthread_t thread[USERS];
    int ids[USERS];



    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < USERS; i++) {
        ids[i] = i + 1;
        pthread_create(&thread[i], NULL, use_atm, &ids[i]);
    }

    for (int i = 0; i < USERS; i++) {
        pthread_join(thread[i], NULL);
    }

    pthread_mutex_destroy(&lock);
}

