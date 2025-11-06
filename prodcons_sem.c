/*
 * prodcons_sem.c
 *
 * Producer-Consumer using counting semaphores + mutex (POSIX).
 *
 * Compile:
 *   gcc -o prodcons_sem prodcons_sem.c -pthread
 *
 * Run:
 *   ./prodcons_sem <producers> <consumers> <buffer_size> <items_per_producer>
 *
 * Example:
 *   ./prodcons_sem 2 3 5 10
 *
 * Behavior:
 *  - Each producer produces 'items_per_producer' integer items.
 *  - After producers finish, main inserts one poison pill (-1) per consumer.
 *  - Consumers exit when they read -1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef int item_t;

/* circular buffer */
typedef struct {
    item_t *buf;
    int capacity;
    int in;   // next insertion index
    int out;  // next removal index
} buffer_t;

buffer_t buffer;
sem_t empty;              // counts free slots
sem_t full;               // counts filled slots
pthread_mutex_t mutex;    // protects buffer access

int producers_count, consumers_count, items_per_producer;

/* initialize buffer */
void buffer_init(buffer_t *b, int capacity) {
    b->buf = malloc(sizeof(item_t) * capacity);
    b->capacity = capacity;
    b->in = 0;
    b->out = 0;
}

/* cleanup buffer */
void buffer_destroy(buffer_t *b) {
    free(b->buf);
}

/* put item (caller must have synchronized via sem/mutex) */
void buffer_put(buffer_t *b, item_t x) {
    b->buf[b->in] = x;
    b->in = (b->in + 1) % b->capacity;
}

/* get item (caller must have synchronized via sem/mutex) */
item_t buffer_get(buffer_t *b) {
    item_t x = b->buf[b->out];
    b->out = (b->out + 1) % b->capacity;
    return x;
}

void *producer(void *arg) {
    long id = (long) arg;
    for (int i = 1; i <= items_per_producer; ++i) {
        item_t it = (int)(id * 100000 + i); // encode producer id and seq for clarity

        sem_wait(&empty);                 // wait for free slot
        pthread_mutex_lock(&mutex);       // enter critical section
        buffer_put(&buffer, it);
        printf("[Producer %ld] produced %d (in=%d)\n", id, it, buffer.in);
        pthread_mutex_unlock(&mutex);     // leave critical section
        sem_post(&full);                  // signal filled slot

        // simulate work
        usleep((rand() % 200 + 50) * 1000);
    }
    printf("[Producer %ld] finished producing.\n", id);
    return NULL;
}

void *consumer(void *arg) {
    long id = (long) arg;
    while (1) {
        sem_wait(&full);                  // wait for a filled slot
        pthread_mutex_lock(&mutex);       // enter critical section
        item_t it = buffer_get(&buffer);
        pthread_mutex_unlock(&mutex);     // leave critical section
        sem_post(&empty);                 // signal free slot

        if (it == -1) {
            // poison pill: re-insert for other consumers if multiple consumers read fast
            // (not necessary here because main inserted exactly consumers_count pills)
            printf("[Consumer %ld] received poison pill, exiting.\n", id);
            break;
        }

        // process item
        int prod = it / 100000;
        int seq  = it % 100000;
        printf("[Consumer %ld] consumed item from P%d seq=%d (out=%d)\n", id, prod, seq, buffer.out);

        // simulate processing
        usleep((rand() % 200 + 50) * 1000);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <producers> <consumers> <buffer_size> <items_per_producer>\n", argv[0]);
        return 1;
    }

    producers_count = atoi(argv[1]);
    consumers_count = atoi(argv[2]);
    int buffer_size = atoi(argv[3]);
    items_per_producer = atoi(argv[4]);

    if (producers_count <= 0 || consumers_count <= 0 || buffer_size <= 0 || items_per_producer <= 0) {
        fprintf(stderr, "All arguments must be positive integers.\n");
        return 1;
    }

    srand((unsigned)time(NULL));
    buffer_init(&buffer, buffer_size);
    sem_init(&empty, 0, buffer_size);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_t *producers = malloc(sizeof(pthread_t) * producers_count);
    pthread_t *consumers = malloc(sizeof(pthread_t) * consumers_count);

    // create consumers first (they will wait on 'full')
    for (long i = 0; i < consumers_count; ++i) {
        if (pthread_create(&consumers[i], NULL, consumer, (void *)i) != 0) {
            perror("pthread_create consumer");
            exit(1);
        }
    }

    // create producers
    for (long i = 0; i < producers_count; ++i) {
        if (pthread_create(&producers[i], NULL, producer, (void *)i) != 0) {
            perror("pthread_create producer");
            exit(1);
        }
    }

    // wait for producers to finish
    for (int i = 0; i < producers_count; ++i) pthread_join(producers[i], NULL);
    printf("Main: all producers finished — inserting poison pills for consumers.\n");

    // insert one poison pill (-1) per consumer so they exit
    for (int i = 0; i < consumers_count; ++i) {
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        buffer_put(&buffer, -1);
        printf("[Main] inserted poison pill (in=%d)\n", buffer.in);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }

    // wait for consumers to finish
    for (int i = 0; i < consumers_count; ++i) pthread_join(consumers[i], NULL);

    // cleanup
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    buffer_destroy(&buffer);
    free(producers);
    free(consumers);

    printf("Main: all consumers exited. Program terminating.\n");
    return 0;
}
