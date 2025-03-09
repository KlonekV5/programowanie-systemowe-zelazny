#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_PHILOSOPHERS 5
#define MAX_CYCLES 100000

sem_t chopsticks[NUM_PHILOSOPHERS];
pthread_t philosophers[NUM_PHILOSOPHERS];
char* states[NUM_PHILOSOPHERS];
int cycle = 0;
int meals[NUM_PHILOSOPHERS] = {0};
pthread_mutex_t state_lock;
pthread_mutex_t stick_lock;

void print_table() {
    pthread_mutex_lock(&state_lock);
    printf("\nCycle: %d\n", cycle);
    printf("+-------------------------------+\n");
    printf("| Philosopher State            |\n");
    printf("+-------------------------------+\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("| Philosopher %d: %-15s |\n", i, states[i]);
    }

    printf("+-------------------------------+\n");
    pthread_mutex_unlock(&state_lock);
}

void zzz() {
    usleep((random() % 5 + 1) * 100);
}

void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left_value;
    int right_value;

    while (1) {
        pthread_mutex_lock(&state_lock);
        if (cycle >= MAX_CYCLES) {
            pthread_mutex_unlock(&state_lock);
            break;
        }
        pthread_mutex_unlock(&state_lock);

        // thinking
        pthread_mutex_lock(&state_lock);
        states[id] = "Thinking";
        pthread_mutex_unlock(&state_lock);
        zzz();

        // hungry
        pthread_mutex_lock(&state_lock);
        states[id] = "Hungry";
        pthread_mutex_unlock(&state_lock);

        while (1) {
            // lock stick lock to not end up in a situation where another person
            // stole our chopstick while we were checking

            pthread_mutex_lock(&stick_lock);
            sem_getvalue(&chopsticks[id], &left_value);
            sem_getvalue(&chopsticks[id], &right_value);

            if (left_value > 0 && right_value > 0) {
                // pick up chopsticks (left first, then right)
                sem_wait(&chopsticks[id]);
                sem_wait(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);
                pthread_mutex_lock(&state_lock);
                states[id] = "Starting to eat";
                pthread_mutex_unlock(&state_lock);
                pthread_mutex_unlock(&stick_lock);
                break;
            }

            pthread_mutex_unlock(&stick_lock);
            usleep(10);
        }

        // eating
        pthread_mutex_lock(&state_lock);
        states[id] = "Eating";
        meals[id]++;
        pthread_mutex_unlock(&state_lock);
        zzz();

        
        // put down chopsticks
        sem_post(&chopsticks[id]);
        sem_post(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);

        pthread_mutex_lock(&state_lock);
        states[id] = "Finished eating";
        pthread_mutex_unlock(&state_lock);
        if (id == 2) {
            usleep((random() % 5 + 1) * 1000);
        } else {
        zzz();
        }
    }

    return NULL;
}

int main() {
    int ids[NUM_PHILOSOPHERS];

    // initialize semaphores and mutex
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_init(&chopsticks[i], 0, 1);
        states[i] = "Thinking";
    }

    pthread_mutex_init(&state_lock, NULL);
    pthread_mutex_init(&stick_lock, NULL);

    // create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    // main loop for printing table
    for (int i = 0; i < MAX_CYCLES; i++) {
        print_table();
        usleep(100); // print the table every second

        pthread_mutex_lock(&state_lock);
        cycle++;
        pthread_mutex_unlock(&state_lock);
    }

    // join threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // destroy semaphores and mutex
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_destroy(&chopsticks[i]);
    }

    pthread_mutex_destroy(&state_lock);
    pthread_mutex_destroy(&stick_lock);

    // print summary of meals
    printf("\nSimulation complete after %d cycles.\n", MAX_CYCLES);
    printf("\nSummary of meals:\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times.\n", i, meals[i]);
    }

    return 0;
}
