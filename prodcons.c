/*
 * producer_consumer.c
 *
 * Producer-Consumer using pthreads + POSIX semaphores (unnamed).
 *
 * Compile:
 *   gcc -o producer_consumer producer_consumer.c -pthread
 *
 * Run:
 *   ./producer_consumer <producers> <consumers> <buffer_size> <items_per_producer>
 *
 * Example:
 *   ./producer_consumer 2 3 5 10
 *
 * This runs 2 producers and 3 consumers with buffer size 5.
 * Each producer will produce 10 items (so total items = producers * items_per_producer).
 * Consumers exit when they have consumed all produced items.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>     // sleep
#include <time.h>

typedef int item_t;

typedef struct {
    item_t *buf;
    int capacity;
    int in;   // next insertion index
    int out;  // next removal index
} buffer_t;

buffer_t buffer;
sem_t empty;   // counts free slots
sem_t full;    // counts filled slots
pthread_mutex_t mutex;

int producers_count;
int consumers_count;
int items_per_producer;
int total_items;      // producers_count * items_per_producer
int consumed_count = 0;
pthread_mutex_t consumed_count_mutex;

void buffer_init(buffer_t *b, int capacity) {
    b->buf = malloc(sizeof(item_t) * capacity);
    b->capacity = capacity;
    b->in = 0;
    b->out = 0;
}

void buffer_destroy(buffer_t *b) {
    free(b->buf);
}

/* Add item into the circular buffer (caller must handle sem/mutex) */
void buffer_put(buffer_t *b, item_t item) {
    b->buf[b->in] = item;
    b->in = (b->in + 1) % b->capacity;
}

/* Remove item from the circular buffer (caller must handle sem/mutex) */
item_t buffer_get(buffer_t *b) {
    item_t item = b->buf[b->out];
    b->out = (b->out + 1) % b->capacity;
    return item;
}

/* Simple generator for items (could be any data) */
item_t produce_item(int producer_id, int seq) {
    // Example: encode producer_id and seq into a single int
    return (producer_id << 16) | (seq & 0xFFFF);
}

/* Decode helpers for printing */
int item_producer(item_t it) { return (it >> 16) & 0xFFFF; }
int item_seq(item_t it) { return it & 0xFFFF; }

void *producer(void *arg) {
    int id = (int)(long)arg;
    for (int i = 1; i <= items_per_producer; ++i) {
        item_t it = produce_item(id, i);

        sem_wait(&empty);               // wait for free slot
        pthread_mutex_lock(&mutex);     // enter critical section
        buffer_put(&buffer, it);
        printf("[Producer %d] produced item seq=%d, placed at slot. in=%d\n",
               id, i, buffer.in);
        pthread_mutex_unlock(&mutex);   // leave critical section
        sem_post(&full);                // signal filled slot

        // simulate variable production time
        usleep((rand() % 200 + 100) * 1000); // 100-300 ms
    }
    printf("[Producer %d] finished producing.\n", id);
    return NULL;
}

void *consumer(void *arg) {
    int id = (int)(long)arg;
    while (1) {
        // If we've consumed all items globally, break out
        pthread_mutex_lock(&consumed_count_mutex);
        if (consumed_count >= total_items) {
            pthread_mutex_unlock(&consumed_count_mutex);
            break;
        }
        pthread_mutex_unlock(&consumed_count_mutex);

        // Try to consume an item
        sem_wait(&full);                // wait for a filled slot
        pthread_mutex_lock(&mutex);     // critical section to access buffer
        // Double-check: it's possible another consumer consumed the last item
        pthread_mutex_lock(&consumed_count_mutex);
        if (consumed_count >= total_items) {
            // We shouldn't consume; rollback: release locks and post full to avoid deadlock
            pthread_mutex_unlock(&consumed_count_mutex);
            pthread_mutex_unlock(&mutex);
            sem_post(&full);
            break;
        }
        item_t it = buffer_get(&buffer);
        consumed_count++;
        int local_consumed = consumed_count;
        pthread_mutex_unlock(&consumed_count_mutex);
        printf("[Consumer %d] consumed item from producer=%d seq=%d, out=%d (total consumed=%d)\n",
               id, item_producer(it), item_seq(it), buffer.out, local_consumed);
        pthread_mutex_unlock(&mutex);   // leave critical section
        sem_post(&empty);               // signal free slot

        // simulate variable consumption time
        usleep((rand() % 200 + 150) * 1000); // 150-350 ms
    }
    printf("[Consumer %d] exiting (no more items).\n", id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <producers> <consumers> <buffer_size> <items_per_producer>\n", argv[0]);
        return 1;
    }

    srand((unsigned)time(NULL));

    producers_count = atoi(argv[1]);
    consumers_count = atoi(argv[2]);
    int buffer_size = atoi(argv[3]);
    items_per_producer = atoi(argv[4]);

    if (producers_count <= 0 || consumers_count <= 0 || buffer_size <= 0 || items_per_producer <= 0) {
        fprintf(stderr, "All arguments must be positive integers.\n");
        return 1;
    }

    total_items = producers_count * items_per_producer;

    buffer_init(&buffer, buffer_size);

    sem_init(&empty, 0, buffer_size);   // initially all slots empty
    sem_init(&full, 0, 0);              // initially no filled slots
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&consumed_count_mutex, NULL);

    pthread_t *producers = malloc(sizeof(pthread_t) * producers_count);
    pthread_t *consumers = malloc(sizeof(pthread_t) * consumers_count);

    // create producer threads
    for (long i = 0; i < producers_count; ++i) {
        if (pthread_create(&producers[i], NULL, producer, (void *)i) != 0) {
            perror("pthread_create producer");
            exit(1);
        }
    }

    // create consumer threads
    for (long i = 0; i < consumers_count; ++i) {
        if (pthread_create(&consumers[i], NULL, consumer, (void *)i) != 0) {
            perror("pthread_create consumer");
            exit(1);
        }
    }

    // wait for producers
    for (int i = 0; i < producers_count; ++i) {
        pthread_join(producers[i], NULL);
    }
    printf("All producers have finished.\n");

    // Wait for consumers to finish consuming all items
    for (int i = 0; i < consumers_count; ++i) {
        pthread_join(consumers[i], NULL);
    }
    printf("All consumers have exited. Total consumed = %d (expected %d)\n", consumed_count, total_items);

    // cleanup
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&consumed_count_mutex);
    buffer_destroy(&buffer);
    free(producers);
    free(consumers);

    return 0;
}
