/*
 * compare_pages.c
 *
 * Compare Page Replacement: FCFS(FIFO), LRU, Optimal
 *
 * Compile:
 *   gcc -o compare_pages compare_pages.c
 *
 * Run:
 *   ./compare_pages
 *
 * Input:
 *   - number of frames (>=1)
 *   - number of references (>0)
 *   - reference string (space separated integers)
 *
 * Output:
 *   - Page faults for each algorithm and hit ratios
 *   - (Optional) step-by-step trace can be enabled by setting verbose=1
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int find_in_frames(int *frame, int frames, int page) {
    for (int i = 0; i < frames; ++i)
        if (frame[i] == page) return i;
    return -1;
}

/* FIFO (FCFS) */
int simulate_fifo(int frames, int *refs, int n, int verbose) {
    int *frame = malloc(sizeof(int) * frames);
    for (int i = 0; i < frames; ++i) frame[i] = -1;
    int faults = 0;
    int next_replace = 0; // pointer for FIFO replacement

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        if (find_in_frames(frame, frames, page) != -1) {
            if (verbose) {
                printf("Ref %2d: %2d |", i+1, page);
                for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
                printf(" (Hit)\n");
            }
            continue; // hit
        }
        // fault
        faults++;
        frame[next_replace] = page;
        next_replace = (next_replace + 1) % frames;
        if (verbose) {
            printf("Ref %2d: %2d |", i+1, page);
            for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
            printf(" (Fault)\n");
        }
    }
    free(frame);
    return faults;
}

/* LRU */
int simulate_lru(int frames, int *refs, int n, int verbose) {
    int *frame = malloc(sizeof(int) * frames);
    int *last = malloc(sizeof(int) * frames);
    for (int i = 0; i < frames; ++i) { frame[i] = -1; last[i] = -1; }

    int time = 0, faults = 0;
    for (int i = 0; i < n; ++i) {
        int page = refs[i]; time++;
        int idx = find_in_frames(frame, frames, page);
        if (idx != -1) {
            last[idx] = time;
            if (verbose) {
                printf("Ref %2d: %2d |", i+1, page);
                for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
                printf(" (Hit)\n");
            }
            continue;
        }
        // fault
        faults++;
        int empty = -1;
        for (int j = 0; j < frames; ++j) if (frame[j] == -1) { empty = j; break; }
        if (empty != -1) {
            frame[empty] = page;
            last[empty] = time;
        } else {
            // find LRU (min last[])
            int lru = 0;
            for (int j = 1; j < frames; ++j)
                if (last[j] < last[lru]) lru = j;
            frame[lru] = page;
            last[lru] = time;
        }
        if (verbose) {
            printf("Ref %2d: %2d |", i+1, page);
            for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
            printf(" (Fault)\n");
        }
    }
    free(frame); free(last);
    return faults;
}

/* Optimal (Belady) */
int find_victim_optimal(int *frame, int frames, int *refs, int n, int current_pos) {
    int victim = -1;
    int farthest = -1;
    for (int i = 0; i < frames; ++i) {
        int page = frame[i];
        if (page == -1) return i; // empty slot
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

int simulate_optimal(int frames, int *refs, int n, int verbose) {
    int *frame = malloc(sizeof(int) * frames);
    for (int i = 0; i < frames; ++i) frame[i] = -1;
    int faults = 0;

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        if (find_in_frames(frame, frames, page) != -1) {
            if (verbose) {
                printf("Ref %2d: %2d |", i+1, page);
                for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
                printf(" (Hit)\n");
            }
            continue;
        }
        faults++;
        int empty = -1;
        for (int j = 0; j < frames; ++j) if (frame[j] == -1) { empty = j; break; }
        if (empty != -1) frame[empty] = page;
        else {
            int victim = find_victim_optimal(frame, frames, refs, n, i);
            frame[victim] = page;
        }
        if (verbose) {
            printf("Ref %2d: %2d |", i+1, page);
            for (int j = 0; j < frames; ++j) printf(" %2d", frame[j]);
            printf(" (Fault)\n");
        }
    }
    free(frame);
    return faults;
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
    printf("Enter the reference string (space separated):\n");
    for (int i = 0; i < n; ++i) {
        if (scanf("%d", &refs[i]) != 1) { printf("Invalid input\n"); free(refs); return 1; }
    }

    int verbose = 0; // change to 1 to see step-by-step traces

    printf("\nSimulating with %d frames and %d references...\n\n", frames, n);

    int faults_fifo = simulate_fifo(frames, refs, n, verbose);
    int faults_lru = simulate_lru(frames, refs, n, verbose);
    int faults_opt = simulate_optimal(frames, refs, n, verbose);

    printf("Results:\n");
    printf("FCFS (FIFO) page faults : %d (hit ratio: %.4f)\n", faults_fifo, 1.0 - (double)faults_fifo / n);
    printf("LRU page faults         : %d (hit ratio: %.4f)\n", faults_lru, 1.0 - (double)faults_lru / n);
    printf("Optimal page faults     : %d (hit ratio: %.4f)\n", faults_opt, 1.0 - (double)faults_opt / n);

    free(refs);
    return 0;
}
