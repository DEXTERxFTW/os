// sjf_preemptive_simple.c
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#define MAXP 50
#define MAXG 1000

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
    bool finished[MAXP] = {0};

    for (int i = 0; i < n; ++i) {
        printf("P%d Arrival time and Burst time: ", i);
        scanf("%d %d", &at[i], &bt[i]);
        rem[i] = bt[i];
        completion[i] = -1;
    }

    int time = 0;
    int completed = 0;
    GanttEntry gantt[MAXG];
    int gcount = 0;

    int last_pid = -1; // to detect context switches for gantt

    // If no process at time 0, jump to earliest arrival
    int earliest = INT_MAX;
    for (int i = 0; i < n; ++i) if (at[i] < earliest) earliest = at[i];
    if (earliest > 0) time = earliest;

    while (completed < n) {
        // find process with smallest remaining time among arrived & not finished
        int min_rem = INT_MAX;
        int cur = -1;
        for (int i = 0; i < n; ++i) {
            if (!finished[i] && at[i] <= time && rem[i] > 0) {
                if (rem[i] < min_rem) {
                    min_rem = rem[i];
                    cur = i;
                }
                // tie-breaker: lower arrival time then lower pid (optional)
                else if (rem[i] == min_rem) {
                    if (at[i] < at[cur]) cur = i;
                    else if (at[i] == at[cur] && i < cur) cur = i;
                }
            }
        }

        if (cur == -1) {
            // no process available now -> advance time to next arrival
            int nextArr = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (!finished[i] && at[i] > time && at[i] < nextArr) nextArr = at[i];
            if (nextArr == INT_MAX) break; // nothing left (shouldn't happen)
            // record idle in gantt if last was not idle (-1)
            if (last_pid != -2) {
                // start idle
                if (gcount == 0 || gantt[gcount-1].pid != -2) {
                    gantt[gcount].pid = -2; // -2 means idle
                    gantt[gcount].start = time;
                    gantt[gcount].end = nextArr;
                    gcount++;
                } else {
                    // extend last idle
                    gantt[gcount-1].end = nextArr;
                }
                last_pid = -2;
            }
            time = nextArr;
            continue;
        }

        // execute cur for 1 time unit (preemptive shortest-remaining-time)
        if (last_pid != cur) {
            // start new gantt entry for cur
            gantt[gcount].pid = cur;
            gantt[gcount].start = time;
            gantt[gcount].end = time + 1;
            gcount++;
        } else {
            // extend current gantt entry
            gantt[gcount-1].end = time + 1;
        }
        last_pid = cur;

        rem[cur] -= 1;
        time += 1;

        if (rem[cur] == 0) {
            finished[cur] = true;
            completion[cur] = time;
            completed++;
        }
    }

    // compute TAT and WT
    double total_tat = 0.0, total_wt = 0.0;
    for (int i = 0; i < n; ++i) {
        tat[i] = completion[i] - at[i];
        wt[i] = tat[i] - bt[i];
        total_tat += tat[i];
        total_wt += wt[i];
    }

    // print Gantt-like trace (compact)
    printf("\nGantt Chart (start -> Pid -> end). Idle denoted by IDLE:\n");
    for (int i = 0; i < gcount; ++i) {
        if (gantt[i].pid == -2)
            printf("| %d -> IDLE -> %d ", gantt[i].start, gantt[i].end);
        else
            printf("| %d -> P%d -> %d ", gantt[i].start, gantt[i].pid, gantt[i].end);
    }
    printf("|\n");

    // print table
    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\n");
    for (int i = 0; i < n; ++i) {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\n", i, at[i], bt[i], completion[i], tat[i], wt[i]);
    }

    printf("\nAverage Turnaround Time = %.2f\n", total_tat / n);
    printf("Average Waiting Time = %.2f\n", total_wt / n);

    return 0;
}
