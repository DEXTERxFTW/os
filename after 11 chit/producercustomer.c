#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Define the size of the shared buffer
#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 8

// --- Global Variables ---

// The shared buffer (a circular queue)
int buffer[BUFFER_SIZE];
int in = 0;  // Index for the producer to write to
int out = 0; // Index for the consumer to read from

// The three semaphores
sem_t full;  // Counts items in the buffer
sem_t empty; // Counts empty slots
sem_t mutex; // Binary semaphore for mutual exclusion

/**
 * @brief The producer thread's function.
 */
void *producer(void *param) {
    int producer_id = *(int *)param;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        // Produce a new item (e.g., a random number)
        int item = (producer_id * 100) + i; // Unique item for demo
        
        // --- Entry Section ---
        
        // 1. Wait for an empty slot.
        // If buffer is full (empty == 0), this blocks.
        sem_wait(&empty);
        
        // 2. Wait for the mutex to get exclusive access to the buffer.
        sem_wait(&mutex);
        
        // --- Critical Section ---
        // We are guaranteed to have an empty slot and exclusive access.
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        printf("P-%d: Produced item %d. (Buffer slots used: %d)\n", 
               producer_id, item, (in - out + BUFFER_SIZE) % BUFFER_SIZE);
        // --- End Critical Section ---
        
        // 3. Release the mutex.
        sem_post(&mutex);
        
        // 4. Signal that a new 'full' slot is available.
        sem_post(&full);
        
        // Simulate time taken to produce
        usleep(rand() % 100000); 
    }
    
    printf("P-%d: Finished producing.\n", producer_id);
    free(param);
    return NULL;
}

/**
 * @brief The consumer thread's function.
 */
void *consumer(void *param) {
    int consumer_id = *(int *)param;
    // Each consumer will consume half of the total items
    int items_to_consume = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        
        // --- Entry Section ---
        
        // 1. Wait for a 'full' slot.
        // If buffer is empty (full == 0), this blocks.
        sem_wait(&full);
        
        // 2. Wait for the mutex to get exclusive access.
        sem_wait(&mutex);
        
        // --- Critical Section ---
        // We are guaranteed to have an item and exclusive access.
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        printf("C-%d: Consumed item %d. (Buffer slots used: %d)\n", 
               consumer_id, item, (in - out + BUFFER_SIZE) % BUFFER_SIZE);
        // --- End Critical Section ---
        
        // 3. Release the mutex.
        sem_post(&mutex);
        
        // 4. Signal that a new 'empty' slot is available.
        sem_post(&empty);
        
        // Simulate time taken to consume
        usleep(rand() % 200000); 
    }

    printf("C-%d: Finished consuming.\n", consumer_id);
    free(param);
    return NULL;
}

/**
 * @brief Main function to set up and run threads.
 */
int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    // --- Initialization ---
    
    // sem_init(semaphore, pshared, initial_value)
    // pshared = 0 means the semaphore is shared between threads of this process.
    
    sem_init(&full, 0, 0);             // Initially, 0 full slots
    sem_init(&empty, 0, BUFFER_SIZE);  // Initially, BUFFER_SIZE empty slots
    sem_init(&mutex, 0, 1);            // Initially, mutex is unlocked (value 1)

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        int *producer_id = malloc(sizeof(int));
        *producer_id = i + 1;
        pthread_create(&producers[i], NULL, producer, (void *)producer_id);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int *consumer_id = malloc(sizeof(int));
        *consumer_id = i + 1;
        pthread_create(&consumers[i], NULL, consumer, (void *)consumer_id);
    }

    // --- Wait for all threads to finish ---
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    // --- Cleanup ---
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);

    printf("Main: All producers and consumers have finished.\n");
    return 0;
}
/*
gcc producer_consumer.c -o producer_consumer -pthread
*/