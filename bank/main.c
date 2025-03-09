#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>

#define TRANSACTIONS 21
#define TRANSACTIONS_PER_USER 7
#define USERS 3
#define SEED 8


int balance_A = 10000;
int balance_B = 10000;
pthread_mutex_t lock_A;
pthread_mutex_t lock_B;
int *operations;
int *sources;
int *targets;

void generate_data() {
    srand(SEED);
    operations = (int*) malloc(TRANSACTIONS * sizeof(int));
    sources = (int*) malloc(TRANSACTIONS * sizeof(int));
    targets = (int*) malloc(TRANSACTIONS * sizeof(int));

    for (int i = 0; i < TRANSACTIONS; i++) {
        operations[i] = rand() % 500;
        sources[i] = rand() % 3;
        while(1) {
            int tmp = rand() % 3;
            if (tmp != sources[i]) {
                targets[i] = tmp;
                break;
            }
        }
    }
}


/**  
int operations[TRANSACTIONS] = { 56, 250, 500, 2500, 800, 23, 150,
    830, 320, 48, 1200, 249, 2250, 600, 190, 18 };

// 0 = user;    1=accA;  2=accB
int sources[TRANSACTIONS] = { 0, 1, 1, 2, 1, 2, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0 };

// 0 = user;    1=accA;  2=accB
int targets[TRANSACTIONS] = { 1, 2, 2, 1, 2, 1, 1,
    1, 1, 1, 2, 2, 1, 1, 1, 2 };
*/

void print_balance (int *user_id , char *action, int amount, int balance, char *account_id) {

    printf("\n+-----------------------------------------------------------------+\n");
    printf("| Client Name | Operation Type | Account | Amount | Final Balance |\n");
    printf("|-----------------------------------------------------------------|\n");
    printf("| User % 6d | %-14s |    %s    | % 6d | %13d |\n", *user_id, action, account_id, amount, balance);
    printf("+-----------------------------------------------------------------+\n");
}

void print_transfer (int *user_id , char *action, int amount, char *account_id, int balance_a, int balance_b) {

    printf("\n+-----------------------------------------------------------------------------------+\n");
    printf("| Client Name | Operation Type | Account | Amount | Source Balance | Target Balance |\n");
    printf("+-----------------------------------------------------------------------------------+\n");
    printf("| User % 6d | %-14s | %7s | % 6d | %14d | %14d |\n", *user_id, action, account_id, amount, balance_a, balance_b);
    printf("+-----------------------------------------------------------------------------------+\n");
}

void *use_atm(void *id_ptr) {
    int *id = id_ptr;
    char *action;

    printf("+ thread %d\n", *id);
    srand(SEED);

    for (int i = 0; i < TRANSACTIONS_PER_USER; i++) {

        while (true) {
            int random_sel = rand() % TRANSACTIONS;
            
            int value = operations[random_sel];
            int source = sources[random_sel];
            int target = targets[random_sel];
            
            // critical section
            if (source == 1 || target == 1) {
                pthread_mutex_lock(&lock_A);
            }
            if (source == 2 || target == 2) {
                pthread_mutex_lock(&lock_B);
            }
            
            if (operations[random_sel] != 0) { // check if operation was done before
                
                usleep((rand() % 49 + 1) * 10000);

                if (source == 0 && target != 0) {
                    action = "Deposit";

                    if (target == 1) {
                        balance_A += value;
                        print_balance(id, action, value, balance_A, "A");                        
                    } else {
                        balance_B += value;
                        print_balance(id, action, value, balance_B, "B"); 
                    }
                    
                } else if (source != 0 && target == 0) {
                    action = "Withdrawal";

                    if (source == 1) {
                        balance_A -= value;
                        print_balance(id, action, value, balance_A, "A");                        
                    } else {
                        balance_B -= value;
                        print_balance(id, action, value, balance_B, "B"); 
                    }
                 
                } else if (source != 0 && target != 0 && target != source) {
                    action = "Transfer";

                    if (target == 1) {
                        balance_A += value;
                        balance_B -= value;
                        print_transfer(id, action, value, "B -> A", balance_B, balance_A);                        
                    } else {
                        balance_B += value;
                        balance_A -= value;
                        print_transfer(id, action, value, "A -> B", balance_A, balance_B);     
                    }
                } else {
                    printf("\nUser %d tried invalid operation type\n", *id);
                }

                if (source == 1 || target == 1) {
                    pthread_mutex_unlock(&lock_A);
                }
                if (source == 2 || target == 2) {
                    pthread_mutex_unlock(&lock_B);
                }
                operations[random_sel] = 0; // mark operation as done
                break;
            }

            if (source == 1 || target == 1) {
                pthread_mutex_unlock(&lock_A);
            }
            if (source == 2 || target == 2) {
                pthread_mutex_unlock(&lock_B);
            }
        }
    }

    printf("- thread %d\n", *id);
    return NULL;
}

int main() {
    generate_data();
    pthread_t thread[USERS];
    int user_ids[USERS];


    pthread_mutex_init(&lock_A, NULL);
    pthread_mutex_init(&lock_B, NULL);

    for (int i = 0; i < USERS; i++) {
        user_ids[i] = i + 1;
        pthread_create(&thread[i], NULL, use_atm, &user_ids[i]);
    }

    for (int i = 0; i < USERS; i++) {
        pthread_join(thread[i], NULL);
    }

    pthread_mutex_destroy(&lock_A);
    pthread_mutex_destroy(&lock_B);
}

