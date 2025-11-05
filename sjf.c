/*
 * sjf_preemptive.c
 *
 * Shortest Job First (Preemptive) a.k.a Shortest Remaining Time First (SRTF)
 * Compile: gcc -o sjf_preemptive sjf_preemptive.c
 * Run:     ./sjf_preemptive
 *
 * Input:
 *  - number of processes n
 *  - for each process i: arrival_time burst_time
 *
 * Example:
 *  n = 4
 *  P1: 0 8
 *  P2: 1 4
 *  P3: 2 9
 *  P4: 3 5
 *
 * The program will compute completion, turnaround and waiting times,
 * and print a simple Gantt chart and averages.
 */

#include <stdio.h>
#include <limits.h>

int main() {
    int n;
    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    int arrival[n], burst[n], rem[n];
    int completed[n];    // flag 1 if finished else 0
    int completion_time[n];
    int waiting[n], turnaround[n];

    for (int i = 0; i < n; ++i) {
        printf("Enter arrival time and burst time for P%d: ", i + 1);
        scanf("%d %d", &arrival[i], &burst[i]);
        if (arrival[i] < 0 || burst[i] <= 0) {
            printf("Arrival must be >= 0 and burst must be > 0.\n");
            return 1;
        }
        rem[i] = burst[i];
        completed[i] = 0;
        completion_time[i] = 0;
        waiting[i] = 0;
        turnaround[i] = 0;
    }

    int t = 0;                 // current time
    int finished = 0;          // number of processes finished
    long total_wait = 0;
    long total_tat = 0;

    // start time: minimum arrival (optional)
    int min_arr = arrival[0];
    for (int i = 1; i < n; ++i) if (arrival[i] < min_arr) min_arr = arrival[i];
    t = min_arr;

    // For Gantt chart: record each change of running process
    int gantt_proc[10000]; // sequence of process indices executed per time unit (maybe large)
    int gantt_len = 0;

    while (finished < n) {
        // find process with shortest remaining time among arrived and not completed
        int idx = -1;
        int shortest = INT_MAX;
        for (int i = 0; i < n; ++i) {
            if (!completed[i] && arrival[i] <= t && rem[i] < shortest && rem[i] > 0) {
                shortest = rem[i];
                idx = i;
            }
        }

        if (idx == -1) {
            // no process ready at time t -> idle for 1 unit
            gantt_proc[gantt_len++] = -1; // mark idle
            t++;
            continue;
        }

        // run process idx for 1 time unit (preemptive)
        rem[idx]--;
        gantt_proc[gantt_len++] = idx;
        t++; // time advances

        // if finished
        if (rem[idx] == 0) {
            completed[idx] = 1;
            finished++;
            completion_time[idx] = t; // current time is completion time
            turnaround[idx] = completion_time[idx] - arrival[idx];
            waiting[idx] = turnaround[idx] - burst[idx];

            total_wait += waiting[idx];
            total_tat += turnaround[idx];
        }
    }

    // Print results
    printf("\nProcess\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    for (int i = 0; i < n; ++i) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\n", i + 1, arrival[i], burst[i], completion_time[i], turnaround[i], waiting[i]);
    }

    double avg_wait = (double) total_wait / n;
    double avg_tat = (double) total_tat / n;
    printf("\nAverage Turnaround Time = %.2f\n", avg_tat);
    printf("Average Waiting Time    = %.2f\n", avg_wait);

    // Compact Gantt chart (merge consecutive same process)
    printf("\nGantt Chart (time units):\n");
    int pos = 0;
    while (pos < gantt_len) {
        int p = gantt_proc[pos];
        int start = pos;
        int end = pos;
        while (end + 1 < gantt_len && gantt_proc[end + 1] == p) end++;
        // print block
        if (p == -1) {
            printf("| Idle (%d-%d) ", start + min_arr, end + 1 + min_arr);
        } else {
            printf("| P%d (%d-%d) ", p + 1, start + min_arr, end + 1 + min_arr);
        }
        pos = end + 1;
    }
    printf("|\n");

    return 0;
}
