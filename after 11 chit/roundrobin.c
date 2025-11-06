// rr_simple.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXP 50
#define MAXG 500  // max number of Gantt entries

typedef struct {
    int pid;
    int start;
    int end;
} GanttEntry;

int main() {
    int n;
    printf("Number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAXP) return 1;

    int at[MAXP], bt[MAXP], rem[MAXP];
    int completion[MAXP], tat[MAXP], wt[MAXP];
    bool finished[MAXP];
    for (int i = 0; i < n; ++i) {
        printf("P%d Arrival time and Burst time: ", i);
        scanf("%d %d", &at[i], &bt[i]);
        rem[i] = bt[i];
        finished[i] = false;
    }

    int quantum;
    printf("Time Quantum: ");
    scanf("%d", &quantum);
    if (quantum <= 0) {
        printf("Quantum must be > 0\n");
        return 1;
    }

    // ready queue (simple circular queue)
    int q[MAXP+5], front = 0, rear = 0;
    bool inqueue[MAXP];
    for (int i = 0; i < n; ++i) inqueue[i] = false;

    // sort by arrival time (stable simple bubble for readability; n small usually)
    for (int i = 0; i < n-1; ++i)
      for (int j = 0; j < n-1-i; ++j)
        if (at[j] > at[j+1]) {
          int t;
          t = at[j]; at[j] = at[j+1]; at[j+1] = t;
          t = bt[j]; bt[j] = bt[j+1]; bt[j+1] = t;
          t = rem[j]; rem[j] = rem[j+1]; rem[j+1] = t;
        }

    int time = 0;
    int completed = 0;
    GanttEntry gantt[MAXG];
    int gcount = 0;

    // enqueue processes that arrive at time 0
    for (int i = 0; i < n; ++i) {
        if (at[i] <= time && !inqueue[i]) {
            q[rear++] = i; inqueue[i] = true;
        }
    }

    while (completed < n) {
        if (front == rear) {
            // ready queue empty -> jump to next arrival
            int nextArr = 1e9;
            int idx = -1;
            for (int i = 0; i < n; ++i) {
                if (!finished[i] && at[i] < nextArr) { nextArr = at[i]; idx = i; }
            }
            if (idx == -1) break; // shouldn't happen
            time = at[idx];
            // enqueue all that have arrived at this time
            for (int i = 0; i < n; ++i)
                if (at[i] <= time && !inqueue[i]) { q[rear++] = i; inqueue[i] = true; }
            continue;
        }

        int p = q[front++]; // dequeue
        if (front > MAXP) { front = 0; rear %= (MAXP+5); } // avoid overflow (simple)
        int exec = (rem[p] < quantum) ? rem[p] : quantum;

        // record gantt
        gantt[gcount].pid = p;
        gantt[gcount].start = time;
        time += exec;
        gantt[gcount].end = time;
        gcount++;

        rem[p] -= exec;

        // enqueue newly arrived processes during this time slice
        for (int i = 0; i < n; ++i) {
            if (!inqueue[i] && !finished[i] && at[i] <= time) {
                q[rear++] = i; inqueue[i] = true;
            }
        }

        if (rem[p] == 0) {
            finished[p] = true;
            completion[p] = time;
            completed++;
            // compute TAT and WT later (need original burst). We'll do it below.
        } else {
            // put it back to queue tail
            q[rear++] = p;
        }
    }

    // compute turnaround and waiting times
    double total_wt = 0, total_tat = 0;
    // but we sorted processes earlier; user may expect P0..Pn-1 as entered. To be simple,
    // we will print results in the current order (sorted by arrival). That's fine for learning.
    printf("\nGantt Chart:\n");
    // print timeline with simple format: | t0 Pid t1 | ...
    for (int i = 0; i < gcount; ++i) {
        printf("| %d -> P%d -> %d ", gantt[i].start, gantt[i].pid, gantt[i].end);
    }
    printf("|\n");

    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\n");
    for (int i = 0; i < n; ++i) {
        tat[i] = completion[i] - at[i];
        wt[i]  = tat[i] - bt[i];
        total_tat += tat[i];
        total_wt += wt[i];
        printf("P%d\t%d\t%d\t%d\t%d\t%d\n", i, at[i], bt[i], completion[i], tat[i], wt[i]);
    }

    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);
    printf("Average Waiting Time = %.2f\n", total_wt / n);

    return 0;
}
