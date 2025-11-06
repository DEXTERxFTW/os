/*
 * banker.c
 *
 * Banker's Algorithm for Deadlock Avoidance.
 *
 * Compile:
 *   gcc -o banker banker.c
 *
 * Run:
 *   ./banker
 *
 * The program prompts for:
 *  - number of processes (n)
 *  - number of resource types (m)
 *  - Allocation matrix (n x m)
 *  - Max matrix (n x m)
 *  - Available vector (m)
 *
 * It prints Need matrix, whether the system is SAFE or NOT SAFE,
 * and, if safe, a safe sequence of processes.
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n, m;
    printf("Number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) { fprintf(stderr, "Invalid number of processes\n"); return 1; }
    printf("Number of resource types: ");
    if (scanf("%d", &m) != 1 || m <= 0) { fprintf(stderr, "Invalid number of resource types\n"); return 1; }

    // allocate matrices/vectors
    int **alloc = malloc(n * sizeof(int *));
    int **max = malloc(n * sizeof(int *));
    int **need = malloc(n * sizeof(int *));
    for (int i = 0; i < n; ++i) {
        alloc[i] = malloc(m * sizeof(int));
        max[i]   = malloc(m * sizeof(int));
        need[i]  = malloc(m * sizeof(int));
    }
    int *avail = malloc(m * sizeof(int));
    int *work = malloc(m * sizeof(int));
    int *finish = calloc(n, sizeof(int));
    int *safe_seq = malloc(n * sizeof(int));

    printf("\nEnter Allocation matrix (rows=processes P0..P%d, columns=resources R0..R%d)\n", n-1, m-1);
    for (int i = 0; i < n; ++i) {
        printf("Allocation for P%d: ", i);
        for (int j = 0; j < m; ++j) {
            if (scanf("%d", &alloc[i][j]) != 1) { fprintf(stderr, "Invalid input\n"); return 1; }
        }
    }

    printf("\nEnter Max matrix (rows=processes P0..P%d)\n", n-1);
    for (int i = 0; i < n; ++i) {
        printf("Max for P%d: ", i);
        for (int j = 0; j < m; ++j) {
            if (scanf("%d", &max[i][j]) != 1) { fprintf(stderr, "Invalid input\n"); return 1; }
        }
    }

    printf("\nEnter Available vector (R0..R%d): ", m-1);
    for (int j = 0; j < m; ++j) {
        if (scanf("%d", &avail[j]) != 1) { fprintf(stderr, "Invalid input\n"); return 1; }
    }

    // Compute Need = Max - Allocation
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            need[i][j] = max[i][j] - alloc[i][j];
            if (need[i][j] < 0) need[i][j] = 0; // defensive
        }
    }

    // Print matrices
    printf("\nAllocation Matrix:\n");
    for (int i = 0; i < n; ++i) {
        printf("P%-3d: ", i);
        for (int j = 0; j < m; ++j) printf("%3d ", alloc[i][j]);
        printf("\n");
    }

    printf("\nMax Matrix:\n");
    for (int i = 0; i < n; ++i) {
        printf("P%-3d: ", i);
        for (int j = 0; j < m; ++j) printf("%3d ", max[i][j]);
        printf("\n");
    }

    printf("\nNeed Matrix (Max - Allocation):\n");
    for (int i = 0; i < n; ++i) {
        printf("P%-3d: ", i);
        for (int j = 0; j < m; ++j) printf("%3d ", need[i][j]);
        printf("\n");
    }

    // Safety algorithm initialization
    for (int j = 0; j < m; ++j) work[j] = avail[j];

    int count = 0;
    while (count < n) {
        int found = 0;
        for (int i = 0; i < n; ++i) {
            if (finish[i]) continue;
            int j;
            for (j = 0; j < m; ++j) {
                if (need[i][j] > work[j]) break;
            }
            if (j == m) {
                // P_i can be satisfied
                for (int k = 0; k < m; ++k) work[k] += alloc[i][k];
                safe_seq[count++] = i;
                finish[i] = 1;
                found = 1;
            }
        }
        if (!found) break; // no further process can be satisfied
    }

    int safe = (count == n);
    if (safe) {
        printf("\nSystem is in a SAFE state.\nSafe sequence is: ");
        for (int i = 0; i < n; ++i) {
            printf("P%d", safe_seq[i]);
            if (i < n-1) printf(" -> ");
        }
        printf("\n");
    } else {
        printf("\nSystem is NOT in a safe state. No safe sequence found.\n");
    }

    // cleanup
    for (int i = 0; i < n; ++i) { free(alloc[i]); free(max[i]); free(need[i]); }
    free(alloc); free(max); free(need);
    free(avail); free(work); free(finish); free(safe_seq);

    return 0;
}
