/*
 * lru.c
 *
 * LRU Page Replacement simulation.
 *
 * Compile:
 *   gcc -o lru lru.c
 *
 * Run:
 *   ./lru
 *
 * The program will ask for:
 *  - number of frames (>= 3)
 *  - number of pages in the reference string
 *  - the reference string (space separated integers)
 *
 * Example:
 *  frames = 3
 *  n = 12
 *  refs: 7 0 1 2 0 3 0 4 2 3 0 3
 *
 * The program prints frame contents after each reference and final stats.
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int frames;
    printf("Enter number of frames (>=3): ");
    if (scanf("%d", &frames) != 1 || frames < 3) {
        printf("Invalid frame count. Must be an integer >= 3.\n");
        return 1;
    }

    int n;
    printf("Enter number of pages in reference string: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of pages.\n");
        return 1;
    }

    int *refs = malloc(sizeof(int) * n);
    if (!refs) { perror("malloc"); return 1; }

    printf("Enter the reference string (space separated):\n");
    for (int i = 0; i < n; ++i) {
        if (scanf("%d", &refs[i]) != 1) {
            printf("Invalid input for reference string.\n");
            free(refs);
            return 1;
        }
    }

    int *frame = malloc(sizeof(int) * frames);
    int *last_used = malloc(sizeof(int) * frames); // timestamp of last access for each frame
    if (!frame || !last_used) { perror("malloc"); free(refs); free(frame); free(last_used); return 1; }

    // initialize frames to -1 (empty) and last_used to -1
    for (int i = 0; i < frames; ++i) {
        frame[i] = -1;
        last_used[i] = -1;
    }

    int time = 0;
    int page_faults = 0;
    int hits = 0;

    printf("\nStep\tPage\tFrames (left->right)\tResult\n");
    printf("----\t----\t---------------------\t------\n");

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        time++;

        // Check if page is already in a frame (hit)
        int hit_index = -1;
        for (int f = 0; f < frames; ++f) {
            if (frame[f] == page) { hit_index = f; break; }
        }

        if (hit_index != -1) {
            // Hit: update last_used timestamp
            last_used[hit_index] = time;
            hits++;
            // print state
            printf("%2d\t%4d\t", i+1, page);
            for (int f = 0; f < frames; ++f) {
                if (frame[f] == -1) printf("  - ");
                else printf("%3d ", frame[f]);
            }
            printf("\tHit\n");
            continue;
        }

        // Miss -> page fault
        page_faults++;

        // Check for empty frame first
        int empty_index = -1;
        for (int f = 0; f < frames; ++f) {
            if (frame[f] == -1) { empty_index = f; break; }
        }

        if (empty_index != -1) {
            // place in empty frame
            frame[empty_index] = page;
            last_used[empty_index] = time;
        } else {
            // find LRU frame (minimum last_used)
            int lru_index = 0;
            int min_time = last_used[0];
            for (int f = 1; f < frames; ++f) {
                if (last_used[f] < min_time) {
                    min_time = last_used[f];
                    lru_index = f;
                }
            }
            // replace LRU
            frame[lru_index] = page;
            last_used[lru_index] = time;
        }

        // print state after fault handling
        printf("%2d\t%4d\t", i+1, page);
        for (int f = 0; f < frames; ++f) {
            if (frame[f] == -1) printf("  - ");
            else printf("%3d ", frame[f]);
        }
        printf("\tFault\n");
    }

    double hit_ratio = (double)hits / n;
    double fault_ratio = (double)page_faults / n;

    printf("\nTotal references: %d\n", n);
    printf("Page hits: %d\n", hits);
    printf("Page faults: %d\n", page_faults);
    printf("Hit ratio: %.4f\n", hit_ratio);
    printf("Fault ratio: %.4f\n", fault_ratio);

    free(refs);
    free(frame);
    free(last_used);
    return 0;
}
