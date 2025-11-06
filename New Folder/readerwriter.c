#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// --- Global Synchronization Variables ---

// The mutex that protects the resource from WRITERS.
// This is the "main" lock.
pthread_mutex_t resource_mutex; 

// The mutex that protects the read_count variable.
// This is a short-term lock for managing the counter.
pthread_mutex_t rw_mutex; 

int read_count = 0; // Counts how many readers are active

// --- Shared Data ---
int shared_data = 0;


/**
 * @brief The function for reader threads.
 */
void *reader(void *param) {
    int reader_id = *(int *)param;
    
    // Simulate reading multiple times
    for (int i = 0; i < 3; i++) {
        
        // --- Reader Entry Section ---
        
        // 1. Lock the counter-mutex to safely change read_count
        pthread_mutex_lock(&rw_mutex);
        
        read_count++;
        
        // 2. If this is the FIRST reader...
        if (read_count == 1) {
            // ...it must lock the resource to block any writers.
            printf("Reader %d: I am the FIRST reader. Locking resource for writers.\n", reader_id);
            pthread_mutex_lock(&resource_mutex); 
        }
        
        // 3. Unlock the counter-mutex. Other readers can now enter.
        pthread_mutex_unlock(&rw_mutex);

        
        // --- Critical Section (Reading) ---
        // Multiple readers can be in this section at once.
        printf("Reader %d: Reading data... Value = %d (Total Readers: %d)\n", 
               reader_id, shared_data, read_count);
        usleep(rand() % 500000); // Simulate reading (up to 0.5 sec)

        
        // --- Reader Exit Section ---
        
        // 1. Lock the counter-mutex to safely change read_count
        pthread_mutex_lock(&rw_mutex);
        
        read_count--;
        
        // 2. If this is the LAST reader...
        if (read_count == 0) {
            // ...it must unlock the resource so writers can enter.
            printf("Reader %d: I am the LAST reader. Unlocking resource for writers.\n", reader_id);
            pthread_mutex_unlock(&resource_mutex);
        }
        
        // 3. Unlock the counter-mutex
        pthread_mutex_unlock(&rw_mutex);

        // Simulate thinking before reading again
        usleep(rand() % 200000);
    }
    
    free(param); // Free the memory allocated for the ID
    return NULL;
}

/**
 * @brief The function for writer threads.
 */
void *writer(void *param) {
    int writer_id = *(int *)param;

    // Simulate writing multiple times
    for (int i = 0; i < 3; i++) {
        
        // --- Writer Entry Section ---
        // The writer's logic is simple: lock the resource.
        // This lock will wait until:
        // 1. No other writer is writing.
        // 2. No readers are reading (the 'last reader' has unlocked it).
        printf("Writer %d: Trying to lock resource...\n", writer_id);
        pthread_mutex_lock(&resource_mutex);

        
        // --- Critical Section (Writing) ---
        // Only one writer can be here.
        shared_data++;
        printf("Writer %d: Wrote to data. New Value = %d\n", writer_id, shared_data);
        usleep(rand() % 800000); // Simulate writing (up to 0.8 sec)

        
        // --- Writer Exit Section ---
        pthread_mutex_unlock(&resource_mutex);
        printf("Writer %d: Unlocked resource.\n", writer_id);

        // Simulate working before writing again
        usleep(rand() % 500000);
    }

    free(param); // Free the memory allocated for the ID
    return NULL;
}

/**
 * @brief Main function to create and manage threads.
 */
int main() {
    int num_readers = 5;
    int num_writers = 2;
    pthread_t readers[num_readers];
    pthread_t writers[num_writers];

    // Initialize the two mutexes
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&resource_mutex, NULL);

    // Create reader threads
    for (int i = 0; i < num_readers; i++) {
        int *reader_id = malloc(sizeof(int));
        *reader_id = i + 1;
        pthread_create(&readers[i], NULL, reader, (void *)reader_id);
    }

    // Create writer threads
    for (int i = 0; i < num_writers; i++) {
        int *writer_id = malloc(sizeof(int));
        *writer_id = i + 1;
        pthread_create(&writers[i], NULL, writer, (void *)writer_id);
    }

    // Wait for all reader threads to finish
    for (int i = 0; i < num_readers; i++) {
        pthread_join(readers[i], NULL);
    }

    // Wait for all writer threads to finish
    for (int i = 0; i < num_writers; i++) {
        pthread_join(writers[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&resource_mutex);

    printf("Main: All threads finished. Final data value: %d\n", shared_data);
    return 0;
} 
/*
gcc reader_writer.c -o reader_writer -pthread
*/