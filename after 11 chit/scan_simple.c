// scan_simple.c
#include <stdio.h>
#include <stdlib.h>

int absval(int x){ return x < 0 ? -x : x; }

/* simple ascending qsort comparator */
int cmp(const void *a, const void *b){
    int x = *(int*)a;
    int y = *(int*)b;
    return (x > y) - (x < y);
}

int main(){
    int n;
    printf("Number of requests: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number\n");
        return 1;
    }

    int req[n];
    printf("Enter %d request track numbers (space separated):\n", n);
    for (int i = 0; i < n; ++i) scanf("%d", &req[i]);

    int head, disk_size;
    printf("Initial head position: ");
    scanf("%d", &head);
    printf("Disk size (number of tracks, e.g. 200 for tracks 0..199): ");
    scanf("%d", &disk_size);

    if (head < 0 || head >= disk_size) {
        printf("Head position out of disk range\n");
        return 1;
    }

    /* sort requests */
    qsort(req, n, sizeof(int), cmp);

    /* find first request >= head */
    int idx = 0;
    while (idx < n && req[idx] < head) idx++;

    int total_movement = 0;
    int cur = head;

    printf("\nService order (SCAN, moving away from spindle first):\n");
    /* 1) service requests >= head in ascending order */
    for (int i = idx; i < n; ++i){
        printf("%d -> ", req[i]);
        total_movement += absval(req[i] - cur);
        cur = req[i];
    }

    /* after finishing high-side requests, move to end of disk (highest track) */
    int end_track = disk_size - 1;
    if (cur != end_track){
        printf("%d -> ", end_track);
        total_movement += absval(end_track - cur);
        cur = end_track;
    }

    /* 2) reverse direction: service remaining requests < head in descending order */
    for (int i = idx - 1; i >= 0; --i){
        printf("%d", req[i]);
        if (i > 0) printf(" -> ");
        total_movement += absval(req[i] - cur);
        cur = req[i];
    }

    printf("\n\nTotal head movement = %d tracks\n", total_movement);
    return 0;
}
