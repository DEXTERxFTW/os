/* optimal.c
 *
 * Optimal Page Replacement simulator (Belady's algorithm).
 *
 * Compile:
 *   gcc -o optimal optimal.c
 *
 * Run:
 *   ./optimal
 *
 * Input:
 *   - number of frames (>=1)
 *   - number of references (>0)
 *   - the reference string (space separated integers)
 *
 * Output:
 *   - Frame contents after each reference (left->right)
 *   - Total page faults
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int find_in_frames(int *frame, int frames, int page) {
    for (int i = 0; i < frames; ++i)
        if (frame[i] == page) return i;
    return -1;
}

/* Find index in frames of the page whose next use is farthest in future.
   If a page is never used again, it is the best candidate (returns its index). */
int find_victim_optimal(int *frame, int frames, int *refs, int n, int current_pos) {
    int victim = -1;
    int farthest = -1;
    for (int i = 0; i < frames; ++i) {
        int page = frame[i];
        if (page == -1) return i; // empty slot — immediate choice (shouldn't happen in replacement phase)
        int next_use = INT_MAX;
        for (int k = current_pos + 1; k < n; ++k) {
            if (refs[k] == page) { next_use = k; break; }
        }
        if (next_use > farthest) {
            farthest = next_use;
            victim = i;
        }
    }
    return victim;
}

int main(void) {
    int frames;
    printf("Enter number of frames (>=1): ");
    if (scanf("%d", &frames) != 1 || frames < 1) {
        printf("Invalid frame count.\n");
        return 1;
    }

    int n;
    printf("Enter number of page references: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of references.\n");
        return 1;
    }

    int *refs = malloc(sizeof(int) * n);
    if (!refs) { perror("malloc"); return 1; }

    printf("Enter the reference string (space separated):\n");
    for (int i = 0; i < n; ++i) {
        if (scanf("%d", &refs[i]) != 1) { printf("Invalid input\n"); free(refs); return 1; }
    }

    int *frame = malloc(sizeof(int) * frames);
    if (!frame) { perror("malloc"); free(refs); return 1; }
    for (int i = 0; i < frames; ++i) frame[i] = -1;

    int faults = 0;

    printf("\nStep\tPage\tFrames (left->right)\tResult\n");
    printf("----\t----\t---------------------\t------\n");

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        int pos = find_in_frames(frame, frames, page);

        if (pos != -1) {
            // Hit
            printf("%2d\t%4d\t", i+1, page);
            for (int f = 0; f < frames; ++f) {
                if (frame[f] == -1) printf("  - ");
                else printf("%3d ", frame[f]);
            }
            printf("\tHit\n");
            continue;
        }

        // Fault
        faults++;

        // if empty slot exists, use it
        int empty = -1;
        for (int f = 0; f < frames; ++f) if (frame[f] == -1) { empty = f; break; }

        if (empty != -1) {
            frame[empty] = page;
        } else {
            int victim = find_victim_optimal(frame, frames, refs, n, i);
            frame[victim] = page;
        }

        printf("%2d\t%4d\t", i+1, page);
        for (int f = 0; f < frames; ++f) {
            if (frame[f] == -1) printf("  - ");
            else printf("%3d ", frame[f]);
        }
        printf("\tFault\n");
    }

    printf("\nTotal references: %d\n", n);
    printf("Total page faults: %d\n", faults);
    printf("Hit ratio: %.4f\n", (double)(n - faults) / n);

    free(refs);
    free(frame);
    return 0;
}
