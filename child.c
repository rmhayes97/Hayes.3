#include "config.h"

/* global variables */
int semID;
FILE *logFile = NULL;
FILE *cstest = NULL;

void cleanChild() {
    if (cstest != NULL){
        fclose(cstest);
    } 
    if (logFile != NULL) {
        fclose(logFile);
    } 
    exit(0);
}

/* decrement the semaphore */
void waitSemaphore() {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = 0;
    op.sem_op = -1;
    semop(semID, &op, 1);
    return;
}

/* increment the semaphore */
void postSemaphore() {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_flg = 0;
    op.sem_op = 1;
    semop(semID, &op, 1);
    return;
}

int main(int argc, char *argv[]) {

    signal(SIGTERM, cleanChild);

    int processID = atoi(argv[1]);
    int procNum = atoi(argv[2]);
    
    key_t key = ftok(".", 'a');
    if ((semID = semget(key, 1, 0)) == -1) {
        perror("parent.c: semget");
        cleanChild();
        exit(-1);
    }

    srand(processID);
    int randNum = 0;
    char *log = malloc(sizeof(char) * 11);
 
/* time that will be used in logfiles */   
    time_t currentTime;
    struct tm timeVal;
    sprintf(log, "logfile.%d", processID);

/* opening the logfiles */
    for (int i = 0; i < 5; i++) { 
        waitSemaphore();
        logFile = fopen(log, "a");   
        cstest = fopen("cstest", "a");

        currentTime = time(NULL);
        timeVal = *localtime(&currentTime);

        fprintf(logFile, "%02d:%02d:%02d    Process %d: In critical section \n", timeVal.tm_hour, timeVal.tm_min, timeVal.tm_sec, processID);

        randNum = (rand() % 5) + 1;
        sleep(randNum);
        currentTime = time(NULL);
        timeVal = *localtime(&currentTime);

        fprintf(cstest, "%02d:%02d:%02d    File modified by process number %d\n", timeVal.tm_hour, timeVal.tm_min, timeVal.tm_sec, processID);
        fprintf(logFile, "%02d:%02d:%02d    Process %d: Writing in cstest\n", timeVal.tm_hour, timeVal.tm_min, timeVal.tm_sec, processID);

        randNum = (rand() % 5) + 1;
        sleep(randNum);
        currentTime = time(NULL);
        timeVal = *localtime(&currentTime);

        fprintf(logFile, "%02d:%02d:%02d    Process %d: Exiting critical section\n",timeVal.tm_hour, timeVal.tm_min, timeVal.tm_sec, processID);
   
/* closing the logfiles */
        fclose(cstest);
        fclose(logFile);
        cstest = NULL;
        logFile = NULL;
        
        postSemaphore();
    }

    exit(0);
}
