/* round_robin.c
 *
 * Round Robin scheduling with different arrival times.
 * Compile: gcc -o round_robin round_robin.c
 * Run:     ./round_robin
 *
 * The program asks:
 *  - number of processes n
 *  - time quantum (integer)
 *  - arrival time and burst time for each process
 *
 * Output:
 *  - Completion, Turnaround, Waiting times per process
 *  - Average TAT and WT
 *  - Gantt chart (blocks with start-end times)
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid;
    int arrival;
    int burst;
    int rem;
    int completion;
} Process;

typedef struct {
    int pid;
    int start;
    int end;
} GanttBlock;

int main(void) {
    int n, tq;
    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of processes.\n");
        return 1;
    }
    printf("Enter time quantum: ");
    if (scanf("%d", &tq) != 1 || tq <= 0) {
        printf("Invalid time quantum.\n");
        return 1;
    }

    Process *p = malloc(sizeof(Process) * n);
    if (!p) { perror("malloc"); return 1; }

    for (int i = 0; i < n; ++i) {
        p[i].pid = i + 1;
        printf("Enter arrival time and burst time for P%d: ", p[i].pid);
        if (scanf("%d %d", &p[i].arrival, &p[i].burst) != 2) {
            printf("Invalid input.\n");
            free(p);
            return 1;
        }
        if (p[i].arrival < 0 || p[i].burst <= 0) {
            printf("Arrival must be >= 0 and burst must be > 0.\n");
            free(p);
            return 1;
        }
        p[i].rem = p[i].burst;
        p[i].completion = -1;
    }

    // Find earliest arrival to start simulation there
    int time = p[0].arrival;
    for (int i = 1; i < n; ++i) if (p[i].arrival < time) time = p[i].arrival;

    // Simple queue for ready processes (store indices)
    int qcap = n * 5; // safe size
    int *queue = malloc(sizeof(int) * qcap);
    int qhead = 0, qtail = 0;
    #define Q_EMPTY (qhead == qtail)
    #define Q_PUSH(x) (queue[qtail++ % qcap] = (x))
    #define Q_POP() (queue[qhead++ % qcap])

    // initially enqueue processes that arrive at 'time' (in increasing pid order)
    int remaining = n;
    for (int i = 0; i < n; ++i) {
        if (p[i].arrival <= time) Q_PUSH(i);
    }

    // for processes that haven't been added yet
    int *added = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) if (p[i].arrival <= time) added[i] = 1;

    // Gantt chart blocks dynamic array
    GanttBlock *gantt = malloc(sizeof(GanttBlock) * (n * 100));
    int gcount = 0;

    while (remaining > 0) {
        if (Q_EMPTY) {
            // no process ready -> jump time to next arrival
            int next_arr = -1;
            for (int i = 0; i < n; ++i) {
                if (!added[i]) {
                    if (next_arr == -1 || p[i].arrival < next_arr) next_arr = p[i].arrival;
                }
            }
            if (next_arr == -1) break; // shouldn't happen
            // idle block (optional) - we won't print idle blocks unless you want them
            time = next_arr;
            for (int i = 0; i < n; ++i) {
                if (!added[i] && p[i].arrival <= time) {
                    Q_PUSH(i);
                    added[i] = 1;
                }
            }
            continue;
        }

        int idx = Q_POP(); // index in p[]
        int run = (p[idx].rem < tq) ? p[idx].rem : tq;
        int start = time;
        int end = time + run;

        // run the process for 'run' units
        p[idx].rem -= run;
        time = end;

        // append gantt block (merge with previous if same pid and adjacent)
        if (gcount > 0 && gantt[gcount-1].pid == p[idx].pid && gantt[gcount-1].end == start) {
            gantt[gcount-1].end = end;
        } else {
            gantt[gcount].pid = p[idx].pid;
            gantt[gcount].start = start;
            gantt[gcount].end = end;
            gcount++;
        }

        // mark newly arrived processes during this time and enqueue them
        for (int i = 0; i < n; ++i) {
            if (!added[i] && p[i].arrival <= time) {
                Q_PUSH(i);
                added[i] = 1;
            }
        }

        if (p[idx].rem > 0) {
            // not finished -> requeue
            Q_PUSH(idx);
        } else {
            // finished
            p[idx].completion = time;
            remaining--;
        }
    }

    // compute turnaround and waiting times
    long total_tat = 0, total_wt = 0;
    printf("\nProcess\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; ++i) {
        int tat = p[i].completion - p[i].arrival;
        int wt = tat - p[i].burst;
        if (wt < 0) wt = 0; // safety
        total_tat += tat;
        total_wt += wt;
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\n", p[i].pid, p[i].arrival, p[i].burst, p[i].completion, tat, wt);
    }

    double avg_tat = (double) total_tat / n;
    double avg_wt = (double) total_wt / n;
    printf("\nAverage Turnaround Time = %.2f\n", avg_tat);
    printf("Average Waiting Time    = %.2f\n", avg_wt);

    // print gantt chart
    printf("\nGantt Chart:\n");
    for (int i = 0; i < gcount; ++i) {
        printf("| P%d (%d-%d) ", gantt[i].pid, gantt[i].start, gantt[i].end);
    }
    printf("|\n");

    // cleanup
    free(p);
    free(queue);
    free(added);
    free(gantt);

    return 0;
}
