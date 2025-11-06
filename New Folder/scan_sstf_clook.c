#include <stdio.h>
#include <stdlib.h> // For abs() and qsort()
#include <limits.h> // For INT_MAX
#include <string.h> // For memcpy() and strcmp()
#include <math.h>   // For abs()

// A helper function for qsort()
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// ----------------------------------------------------------------
// 1. SSTF: Shortest Seek Time First
// ----------------------------------------------------------------
void sstf(int requests[], int n, int head) {
    // Create a copy of requests to avoid modifying the original
    int req_copy[n];
    memcpy(req_copy, requests, n * sizeof(int));

    int total_seek = 0;
    int serviced_count = 0;
    int current_pos = head;

    // We'll use an array to mark which requests are done
    // 0 = not serviced, 1 = serviced
    int serviced[n];
    for (int i = 0; i < n; i++) {
        serviced[i] = 0;
    }

    printf("SSTF Path: %d", current_pos);

    // Loop until all requests are serviced
    while (serviced_count < n) {
        int min_distance = INT_MAX;
        int index = -1;

        // Find the closest un-serviced request
        for (int i = 0; i < n; i++) {
            if (serviced[i] == 0) {
                int distance = abs(current_pos - req_copy[i]);
                if (distance < min_distance) {
                    min_distance = distance;
                    index = i;
                }
            }
        }

        // Service the closest request
        if (index != -1) {
            total_seek += min_distance;
            current_pos = req_copy[index];
            serviced[index] = 1; // Mark as serviced
            serviced_count++;
            printf(" -> %d", current_pos);
        }
    }
    printf("\nTotal SSTF Seek Time: %d\n\n", total_seek);
}

// ----------------------------------------------------------------
// 2. SCAN (Elevator Algorithm)
// ----------------------------------------------------------------
void scan(int requests[], int n, int head, int disk_size, char *direction) {
    // Create a copy and sort it
    int sorted_req[n];
    memcpy(sorted_req, requests, n * sizeof(int));
    qsort(sorted_req, n, sizeof(int), compare);

    int total_seek = 0;
    int current_pos = head;

    printf("SCAN Path: %d", current_pos);

    if (strcmp(direction, "right") == 0) {
        // --- Move Right (UP) ---
        // Service all requests from head to the end
        for (int i = 0; i < n; i++) {
            if (sorted_req[i] >= current_pos) {
                total_seek += abs(sorted_req[i] - current_pos);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }
        
        // Go to the very end of the disk
        printf(" -> %d", disk_size);
        total_seek += abs(disk_size - current_pos);
        current_pos = disk_size;

        // --- Move Left (DOWN) ---
        // Service remaining requests from the end downwards
        for (int i = n - 1; i >= 0; i--) {
            if (sorted_req[i] < head) {
                total_seek += abs(current_pos - sorted_req[i]);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }
    } else { // Direction is "left"
        // --- Move Left (DOWN) ---
        // Service all requests from head to the beginning
        for (int i = n - 1; i >= 0; i--) {
            if (sorted_req[i] <= current_pos) {
                total_seek += abs(current_pos - sorted_req[i]);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }

        // Go to the very beginning of the disk
        printf(" -> 0");
        total_seek += abs(current_pos - 0);
        current_pos = 0;

        // --- Move Right (UP) ---
        // Service remaining requests from 0 upwards
        for (int i = 0; i < n; i++) {
            if (sorted_req[i] > head) {
                total_seek += abs(sorted_req[i] - current_pos);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }
    }

    printf("\nTotal SCAN Seek Time: %d\n\n", total_seek);
}

// ----------------------------------------------------------------
// 3. C-LOOK (Circular-LOOK)
// ----------------------------------------------------------------
void clook(int requests[], int n, int head, char *direction) {
    // Create a copy and sort it
    int sorted_req[n];
    memcpy(sorted_req, requests, n * sizeof(int));
    qsort(sorted_req, n, sizeof(int), compare);

    int total_seek = 0;
    int current_pos = head;

    printf("C-LOOK Path: %d", current_pos);

    if (strcmp(direction, "right") == 0) {
        // --- Move Right (UP) ---
        // Service all requests from head to the *last* request
        for (int i = 0; i < n; i++) {
            if (sorted_req[i] >= current_pos) {
                total_seek += abs(sorted_req[i] - current_pos);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }

        // --- JUMP ---
        // Jump from the last request (highest) to the first (lowest)
        // Note: The jump itself is seek time!
        total_seek += abs(current_pos - sorted_req[0]);
        current_pos = sorted_req[0];
        printf(" -> %d", current_pos);

        // --- Move Right (UP) again ---
        // Service remaining requests from the beginning
        for (int i = 1; i < n; i++) {
            if (sorted_req[i] < head) {
                total_seek += abs(sorted_req[i] - current_pos);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            } else {
                // We've reached the requests we already serviced
                break;
            }
        }
    } else { // Direction is "left"
        // --- Move Left (DOWN) ---
        // Service all requests from head to the *first* request
        for (int i = n - 1; i >= 0; i--) {
            if (sorted_req[i] <= current_pos) {
                total_seek += abs(current_pos - sorted_req[i]);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            }
        }

        // --- JUMP ---
        // Jump from the first request (lowest) to the last (highest)
        total_seek += abs(sorted_req[n-1] - current_pos);
        current_pos = sorted_req[n-1];
        printf(" -> %d", current_pos);

        // --- Move Left (DOWN) again ---
        // Service remaining requests from the top
        for (int i = n - 2; i >= 0; i--) {
            if (sorted_req[i] > head) {
                total_seek += abs(current_pos - sorted_req[i]);
                current_pos = sorted_req[i];
                printf(" -> %d", current_pos);
            } else {
                break;
            }
        }
    }

    printf("\nTotal C-LOOK Seek Time: %d\n\n", total_seek);
}

// ----------------------------------------------------------------
// Main function to run everything
// ----------------------------------------------------------------
int main() {
    // Our list of "floors" (track requests)
    int requests[] = {98, 183, 37, 122, 14, 124, 65, 67};
    int n = sizeof(requests) / sizeof(requests[0]);
    
    // Our "elevator's" starting position
    int head_start = 53;
    
    // Total size of the "building" (disk)
    int disk_size = 199;
    
    // Initial direction of the elevator
    char direction[] = "right";

    // Run each algorithm
    // We pass the ORIGINAL requests array each time
    sstf(requests, n, head_start);
    scan(requests, n, head_start, disk_size, direction);
    clook(requests, n, head_start, direction);

    return 0;
}