/*
 * sstf.c
 *
 * Shortest Seek Time First (SSTF) disk scheduling.
 *
 * Compile:
 *   gcc -o sstf sstf.c
 *
 * Run:
 *   ./sstf
 *
 * Input:
 *   - number of requests (n)
 *   - initial head position (an integer cylinder)
 *   - n request cylinder numbers (integers)
 *
 * Output:
 *   - service order (seek sequence)
 *   - distance moved for each step
 *   - total seek distance
 *   - average seek distance
 *
 * Notes:
 *   - Assumes integer cylinder numbers.
 *   - Ties (two requests equidistant) are broken by choosing the request
 *     with the smaller cylinder number (stable deterministic tie-breaker).
 *   - Does not assume any particular disk size; it will accept any ints.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int find_nearest(int requests[], int served[], int n, int head) {
    int best_idx = -1;
    int best_dist = INT_MAX;
    for (int i = 0; i < n; ++i) {
        if (served[i]) continue;
        int dist = requests[i] - head;
        if (dist < 0) dist = -dist;
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = i;
        } else if (dist == best_dist) {
            // tie-breaker: choose smaller cylinder value (deterministic)
            if (best_idx != -1 && requests[i] < requests[best_idx]) {
                best_idx = i;
            }
        }
    }
    return best_idx;
}

int main(void) {
    int n;
    printf("Enter number of requests: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid number of requests.\n");
        return 1;
    }

    int head;
    printf("Enter initial head position: ");
    if (scanf("%d", &head) != 1) {
        fprintf(stderr, "Invalid head position.\n");
        return 1;
    }

    int *requests = malloc(sizeof(int) * n);
    int *served = malloc(sizeof(int) * n);
    if (!requests || !served) {
        perror("malloc");
        free(requests);
        free(served);
        return 1;
    }

    for (int i = 0; i < n; ++i) {
        printf("Enter request %d cylinder: ", i + 1);
        if (scanf("%d", &requests[i]) != 1) {
            fprintf(stderr, "Invalid cylinder value.\n");
            free(requests);
            free(served);
            return 1;
        }
        served[i] = 0;
    }

    int *sequence = malloc(sizeof(int) * n); // store indices of requests in service order
    int seq_len = 0;
    long total_seek = 0;
    int cur = head;

    while (seq_len < n) {
        int idx = find_nearest(requests, served, n, cur);
        if (idx == -1) break; // all served or error
        int dist = requests[idx] - cur;
        if (dist < 0) dist = -dist;
        total_seek += dist;
        cur = requests[idx];
        sequence[seq_len++] = idx;
        served[idx] = 1;
    }

    printf("\nInitial Head Position: %d\n", head);
    printf("Seek Sequence and movements:\n");
    int cur_pos = head;
    for (int i = 0; i < seq_len; ++i) {
        int req_idx = sequence[i];
        int req_cyl = requests[req_idx];
        int move = req_cyl - cur_pos;
        if (move < 0) move = -move;
        printf("Step %2d: Move from %d -> %d  |  Distance = %d\n",
               i + 1, cur_pos, req_cyl, move);
        cur_pos = req_cyl;
    }

    double avg_seek = (double) total_seek / n;
    printf("\nTotal seek distance = %ld\n", total_seek);
    printf("Average seek distance = %.2f\n", avg_seek);

    // cleanup
    free(requests);
    free(served);
    free(sequence);
    return 0;
}

