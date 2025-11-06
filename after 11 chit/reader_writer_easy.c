// reader_writer_easy.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 2

int shared_data = 0;                // the shared resource
int readcount = 0;                  // number of active readers

pthread_mutex_t rc_mutex = PTHREAD_MUTEX_INITIALIZER;     // protects readcount
pthread_mutex_t resource_mutex = PTHREAD_MUTEX_INITIALIZER; // protects the shared resource

void *reader(void *arg) {
    int id = *(int*)arg;
    while (1) {
        // ENTRY section for reader
        pthread_mutex_lock(&rc_mutex);
        readcount++;
        if (readcount == 1)              // first reader locks the resource for writers
            pthread_mutex_lock(&resource_mutex);
        pthread_mutex_unlock(&rc_mutex);

        // CRITICAL SECTION (reading)
        printf("Reader %d: read shared_data = %d\n", id, shared_data);
        usleep(150000); // simulate read time (150 ms)

        // EXIT section for reader
        pthread_mutex_lock(&rc_mutex);
        readcount--;
        if (readcount == 0)              // last reader unlocks the resource
            pthread_mutex_unlock(&resource_mutex);
        pthread_mutex_unlock(&rc_mutex);

        usleep(200000); // wait a bit before next read
    }
    return NULL;
}

void *writer(void *arg) {
    int id = *(int*)arg;
    while (1) {
        // ENTRY + CRITICAL SECTION (writer needs exclusive access)
        pthread_mutex_lock(&resource_mutex);
        shared_data += 1;
        printf("Writer %d: updated shared_data to %d\n", id, shared_data);
        usleep(250000); // simulate write time (250 ms)
        pthread_mutex_unlock(&resource_mutex);

        usleep(500000); // wait before next write
    }
    return NULL;
}

int main() {
    pthread_t rthreads[NUM_READERS], wthreads[NUM_WRITERS];
    int rids[NUM_READERS], wids[NUM_WRITERS];

    // create reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        rids[i] = i + 1;
        if (pthread_create(&rthreads[i], NULL, reader, &rids[i]) != 0) {
            perror("Failed to create reader thread");
            exit(1);
        }
    }

    // create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        wids[i] = i + 1;
        if (pthread_create(&wthreads[i], NULL, writer, &wids[i]) != 0) {
            perror("Failed to create writer thread");
            exit(1);
        }
    }

    // join (in this demo we won't actually reach join because loops are infinite)
    for (int i = 0; i < NUM_READERS; i++) pthread_join(rthreads[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(wthreads[i], NULL);

    return 0;
}
/*
gcc -pthread reader_writer_easy.c -o reader_writer
./reader_writer
*/