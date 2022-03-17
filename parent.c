#include "config.h"

#define MAX_N 20
#define MAX_SEC 100

int semID;
int numProcesses;
int n;
int maxTime = 100;
int maxProcesses = 20;
int timer = 100;
int * pids = NULL;
FILE * logFile = NULL;


/* function for handling Ctrl+C */
void ctrlC() { 

    for (int i = 0; i < numProcesses; i++) {
        kill(pids[i], SIGTERM);
    }
    while(wait(NULL) > 0);  
    fprintf(stderr, "Ctrl+C signal caught.\n");
    if(logFile != NULL) {
        fprintf(logFile, "Ctrl+C: Closing Parent.\n");
        fclose(logFile);
    }
    semctl(semID, 0, IPC_RMID, NULL);

    exit(-1);
}

/* function for handling SIGALRM */
void outOfTime() { 

    for (int i = 0; i < numProcesses; i++) {
        kill(pids[i], SIGTERM);
    }
    while(wait(NULL) > 0);
    fprintf(stderr, "Parent ran out of time.\n");
    if(logFile != NULL) {
        fprintf(logFile, "Alarm triggered: Time ran out, closing program.\n");
        fclose(logFile);
    }
    semctl(semID, 0, IPC_RMID, NULL);

    exit(-1);
}

int main(int argc, char *argv[]) {
/* catches SIGALRM if the program runs out of time */
    signal(SIGALRM, outOfTime);
    signal(SIGINT, ctrlC);

    int opt;
    int pid;

    time_t currentTime;
    struct tm timeVal;

    currentTime = time(NULL);
    timeVal = *localtime(&currentTime);

    printf("Started Parent.\n");

    logFile = fopen("logfile.parent", "a");

    fprintf(logFile, "Started Parent.\n");

/* getopt for choosing arguments */
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            timer = atoi(optarg);
            break;
        default:
            perror("Parent: getopt\n");
            semctl(semID, 0, IPC_RMID, NULL);
            fclose(logFile);
            exit(-1);
        }
    }
   
/* makes sure the correct argument are used by the user */
    if (argc != 4)
    {
        printf("Parent called incorrectly, do ./Parent -t (time in seconds) (max number of processes)\n");
        perror("Parent: Parent called incorrectly");
        printf("\n");
        fprintf(logFile, "Parent called incorrectly. Closing program.\n");
        semctl(semID, 0, IPC_RMID, NULL);
        fclose(logFile);
        exit(0);
    } else {
        n = atoi(argv[3]);
   
        if (n > MAX_N) {
            printf("n cannot be over 20\n");
            n = MAX_N;
        }
    }

    numProcesses = n;
    pids = malloc(sizeof(int) * n);

    key_t key = ftok(".", 'a');
  
/* error checking for creating semaphore */
    if ((semID = semget(key, 1, IPC_CREAT | 666)) == -1) {
        perror("Parent: semget");
        fprintf(logFile, "Couldn't create semaphore.\n");
        semctl(semID, 0, IPC_RMID, NULL);
        fclose(logFile);
        exit(-1);
    }
/* making the semaphore */
    fprintf(logFile, "Semaphore created\n");
    semctl(semID, 0, SETVAL, 1);
    
    for (int i = 0; i < n; i++) {
        currentTime = time(NULL);
        timeVal = *localtime(&currentTime);

        pid = fork();
        fprintf(logFile, "Forking child process %d\n", i + 1);
    
        if (pid == -1) {
            perror("Parent: Fork failure\n");
            semctl(semID, 0, IPC_RMID, NULL);
            fprintf(stderr, "Fork error, killing program.\n");
            for (int i = 0; i < numProcesses; i++) {
                kill(pids[i], SIGTERM);
            }
            exit(-1);
        }

        if (pid == 0) {
            char * numprocs = malloc(sizeof(char) * 3);
             char * currentProcess = malloc(sizeof(char) * 3);
            sprintf(numprocs, "%d", n); 
            sprintf(currentProcess, "%d", i + 1);
            execl("./child", "./child", currentProcess, numprocs, NULL); 
        } else {

            pids[i] = pid;
        }
    }

    alarm(timer);

    while(wait(NULL) > 0);

/* getting rid of shared memory */
    printf("Children processes finished, deallocating shared memory.\n");
    fprintf(logFile, "Children processes finished, deallocating shared memory.\n");

    currentTime = time(NULL);
    timeVal = *localtime(&currentTime);

/* getting rid of semaphore */
    semctl(semID, 0, IPC_RMID, NULL);
    fprintf(logFile, "Sempahore destroyed. Ending.\n");
    fclose(logFile);

    exit(0);
}

