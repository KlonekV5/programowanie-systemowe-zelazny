#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define MAX_CYCLES 5

sem_t chopsticks[NUM_PHILOSOPHERS];
pthread_t philosophers[NUM_PHILOSOPHERS];
char* states[NUM_PHILOSOPHERS];
int cycle = 0;
int meals[NUM_PHILOSOPHERS] = {0};
pthread_mutex_t state_lock;

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

void* philosopher(void* arg) {
    int id = *(int*)arg;

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
        sleep(1);

        // hungry
        pthread_mutex_lock(&state_lock);
        states[id] = "Hungry";
        pthread_mutex_unlock(&state_lock);

        // pick up chopsticks (left first, then right)
        sem_wait(&chopsticks[id]);

        pthread_mutex_lock(&state_lock);
        states[id] = "Holding left, waiting for right";
        pthread_mutex_unlock(&state_lock);
        sleep(3);

        sem_wait(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);

        // eating
        pthread_mutex_lock(&state_lock);
        states[id] = "Eating";
        meals[id]++;
        pthread_mutex_unlock(&state_lock);
        sleep(2);

        // put down chopsticks
        sem_post(&chopsticks[id]);
        sem_post(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);

        pthread_mutex_lock(&state_lock);
        states[id] = "Finished eating";
        pthread_mutex_unlock(&state_lock);

        sleep(1); // pause after eating
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

    // create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    // main loop for printing table
    for (int i = 0; i < MAX_CYCLES; i++) {
        print_table();
        sleep(1); // print the table every second

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

    // print summary of meals
    printf("\nSimulation complete after %d cycles.\n", MAX_CYCLES);
    printf("\nSummary of meals:\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times.\n", i, meals[i]);
    }

    return 0;
}
