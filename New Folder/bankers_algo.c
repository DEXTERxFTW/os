// bankers_easy.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {
    int n, m;
    printf("Number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 1;
    printf("Number of resource types: ");
    if (scanf("%d", &m) != 1 || m <= 0) return 1;

    // allocate matrices/vectors
    int **alloc = malloc(n * sizeof(int*));
    int **max = malloc(n * sizeof(int*));
    int **need = malloc(n * sizeof(int*));
    for (int i = 0; i < n; ++i) {
        alloc[i] = malloc(m * sizeof(int));
        max[i]   = malloc(m * sizeof(int));
        need[i]  = malloc(m * sizeof(int));
    }
    int *available = malloc(m * sizeof(int));

    printf("Enter Allocation matrix (n rows, m columns):\n");
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            scanf("%d", &alloc[i][j]);

    printf("Enter Max matrix (n rows, m columns):\n");
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            scanf("%d", &max[i][j]);

    printf("Enter Available vector (%d values):\n", m);
    for (int j = 0; j < m; ++j)
        scanf("%d", &available[j]);

    // compute Need = Max - Allocation
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            need[i][j] = max[i][j] - alloc[i][j];

    // Safety algorithm
    int *work = malloc(m * sizeof(int));
    bool *finish = malloc(n * sizeof(bool));
    for (int j = 0; j < m; ++j) work[j] = available[j];
    for (int i = 0; i < n; ++i) finish[i] = false;

    int *safeSeq = malloc(n * sizeof(int));
    int count = 0;
    bool progress;

    do {
        progress = false;
        for (int i = 0; i < n; ++i) {
            if (!finish[i]) {
                bool canAllocate = true;
                for (int j = 0; j < m; ++j) {
                    if (need[i][j] > work[j]) { canAllocate = false; break; }
                }
                if (canAllocate) {
                    // pretend to allocate and finish process i
                    for (int j = 0; j < m; ++j) work[j] += alloc[i][j];
                    finish[i] = true;
                    safeSeq[count++] = i;
                    progress = true;
                }
            }
        }
    } while (progress);

    // check if all finished
    bool safe = true;
    for (int i = 0; i < n; ++i) if (!finish[i]) safe = false;

    if (safe) {
        printf("\nSystem is in a SAFE state.\nSafe sequence: ");
        for (int i = 0; i < count; ++i) {
            printf("P%d", safeSeq[i]);
            if (i < count - 1) printf(" -> ");
        }
        printf("\n");
    } else {
        printf("\nSystem is NOT in a safe state. No safe sequence.\n");
    }

    // free memory
    for (int i = 0; i < n; ++i) {
        free(alloc[i]); free(max[i]); free(need[i]);
    }
    free(alloc); free(max); free(need);
    free(available); free(work); free(finish); free(safeSeq);

    return 0;
}
