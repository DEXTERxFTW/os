/*
 * shm_server.c
 *
 * System V Shared Memory + Semaphore IPC (Server)
 *
 * Usage: compile and run server first:
 *   gcc -o shm_server shm_server.c
 *   ./shm_server
 *
 * Then run client in another terminal:
 *   gcc -o shm_client shm_client.c
 *   ./shm_client
 *
 * Protocol:
 *  - Server writes a request string to shared memory and "posts" sem[0].
 *  - Client waits on sem[0], reads request, writes response, and "posts" sem[1].
 *  - Server waits on sem[1], reads response.
 *  - Type "exit" as request (or response) to terminate.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY_FILE "/tmp"   /* file for ftok (must exist) */
#define SHM_KEY_ID  'S'       /* id for ftok */
#define SEM_KEY_ID  'E'
#define SHM_SIZE    4096
#define MSG_SIZE    1024

/* Shared memory structure */
typedef struct {
    char request[MSG_SIZE];
    char response[MSG_SIZE];
} shmseg_t;

/* semop helpers */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/* perform semop with relative change */
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

    /* create shared memory */
    int shmid = shmget(shmkey, sizeof(shmseg_t), IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget"); exit(1); }

    shmseg_t *shmp = (shmseg_t *)shmat(shmid, NULL, 0);
    if (shmp == (void *)-1) { perror("shmat"); /* cleanup? */ exit(1); }

    /* create semaphore set with 2 semaphores */
    int semid = semget(semkey, 2, IPC_CREAT | IPC_EXCL | 0666);
    int created_new = 1;
    if (semid == -1) {
        if (errno == EEXIST) {
            /* already exists, get it */
            semid = semget(semkey, 2, 0);
            created_new = 0;
        } else {
            perror("semget");
            shmdt(shmp);
            exit(1);
        }
    }

    if (created_new) {
        union semun arg;
        unsigned short vals[2] = {0, 0}; /* sem[0]=0 (server->client), sem[1]=0 (client->server) */
        arg.array = vals;
        if (semctl(semid, 0, SETALL, arg) == -1) {
            perror("semctl SETALL");
            shmdt(shmp);
            semctl(semid, 0, IPC_RMID);
            exit(1);
        }
    }

    printf("Server: Shared memory id=%d, sem id=%d\n", shmid, semid);
    printf("Type text messages. Type \"exit\" to quit.\n");

    while (1) {
        /* get input from user - the server request */
        printf("Server -> (request): ");
        fflush(stdout);
        if (!fgets(shmp->request, MSG_SIZE, stdin)) {
            perror("fgets");
            break;
        }
        /* remove newline */
        shmp->request[strcspn(shmp->request, "\n")] = '\0';

        /* signal client that request is ready (sem[0]++) */
        if (sem_op(semid, 0, 1) == -1) {
            perror("semop signal request");
            break;
        }

        /* if server typed "exit", break after notifying client */
        if (strcmp(shmp->request, "exit") == 0) {
            printf("Server: exit sent. Waiting for client acknowledgement (optional)...\n");
            /* optionally wait for client response and then break; we'll still wait for sem1 to avoid orphaned resource */
            if (sem_op(semid, 1, -1) == -1) {
                if (errno == EINTR) continue;
                perror("semop wait response");
            } else {
                printf("Server: got final response: %s\n", shmp->response);
            }
            break;
        }

        /* wait for client's response (sem[1]--) */
        if (sem_op(semid, 1, -1) == -1) {
            perror("semop wait response");
            break;
        }

        printf("Server: got response -> %s\n", shmp->response);
    }

    /* cleanup - detach and remove shm and sem */
    if (shmdt(shmp) == -1) perror("shmdt");
    /* remove shared memory segment */
    if (shmctl(shmid, IPC_RMID, 0) == -1) perror("shmctl(IPC_RMID)");
    /* remove semaphore set */
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl(IPC_RMID)");

    printf("Server: cleaned up and exiting.\n");
    return 0;
}
