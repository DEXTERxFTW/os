/*
 * shm_client.c
 *
 * System V Shared Memory + Semaphore IPC (Client)
 *
 * Usage:
 *   gcc -o shm_client shm_client.c
 *   ./shm_client
 *
 * Client waits for server requests and answers them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY_FILE "/tmp"
#define SHM_KEY_ID  'S'
#define SEM_KEY_ID  'E'
#define MSG_SIZE    1024

/* shared memory structure must match server's */
typedef struct {
    char request[MSG_SIZE];
    char response[MSG_SIZE];
} shmseg_t;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int sem_op(int semid, int semnum, int op) {
    struct sembuf sops;
    sops.sem_num = semnum;
    sops.sem_op  = op;
    sops.sem_flg = 0;
    return semop(semid, &sops, 1);
}

int main(void) {
    key_t shmkey = ftok(SHM_KEY_FILE, SHM_KEY_ID);
    if (shmkey == -1) { perror("ftok shm"); exit(1); }

    key_t semkey = ftok(SHM_KEY_FILE, SEM_KEY_ID);
    if (semkey == -1) { perror("ftok sem"); exit(1); }

    /* get existing shared memory */
    int shmid = shmget(shmkey, 0, 0);
    if (shmid == -1) { perror("shmget (client)"); exit(1); }

    shmseg_t *shmp = (shmseg_t *)shmat(shmid, NULL, 0);
    if (shmp == (void *)-1) { perror("shmat (client)"); exit(1); }

    /* get semaphores */
    int semid = semget(semkey, 2, 0);
    if (semid == -1) { perror("semget (client)"); shmdt(shmp); exit(1); }

    printf("Client: attached to shm id=%d, sem id=%d\n", shmid, semid);
    printf("Client: waiting for server requests...\n");

    while (1) {
        /* wait for request: sem[0]-- */
        if (sem_op(semid, 0, -1) == -1) {
            perror("semop wait request");
            break;
        }

        /* read request from shared memory */
        printf("Client: server request -> %s\n", shmp->request);

        /* if request is "exit", optionally reply and break */
        if (strcmp(shmp->request, "exit") == 0) {
            snprintf(shmp->response, MSG_SIZE, "Client: received exit, quitting.");
            /* signal server that response is ready (sem[1]++) */
            if (sem_op(semid, 1, 1) == -1) perror("semop post response");
            break;
        }

        /* produce a response - here we just echo back with a prefix */
        snprintf(shmp->response, MSG_SIZE, "Client received: \"%s\"", shmp->request);

        /* signal the server that response is ready (sem[1]++) */
        if (sem_op(semid, 1, 1) == -1) {
            perror("semop post response");
            break;
        }
    }

    if (shmdt(shmp) == -1) perror("shmdt (client)");
    printf("Client: detached and exiting.\n");
    return 0;
}
