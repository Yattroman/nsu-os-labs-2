#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_DEF_LENGTH 256
#define STATUS_SUCCESS 0
#define STATUS_FAIL -1
#define WAIT_ERROR -1

#define SEM_FIRST_NAME "/semFirst"
#define SEM_SECOND_NAME "/semSecond"
#define CHILD_RETURN_CODE 0
#define ITERATIONS_NUM 10

char errorBuffer[BUFFER_DEF_LENGTH];

int waitForChildProcess(){
    int currentStatus = 0;
    pid_t statusWait = wait(&currentStatus);

    printf("\n");

    if(statusWait == WAIT_ERROR){
        perror("waitForChildProcess. There are problems with wait");
        return STATUS_FAIL;
    }

    if(WIFSIGNALED(currentStatus)){
        int signalInfo = WTERMSIG(currentStatus);
        printf("Child process terminated with a signal: %d\n", signalInfo);
        if(WCOREDUMP(currentStatus)){
            printf("Also core file has been produced.");
        }
    } else if(WIFEXITED(currentStatus)){
        int exitStatus = WEXITSTATUS(currentStatus);
        printf("Child process exited with status: %d\n", exitStatus);
    }

    fflush(stdout);

    return STATUS_SUCCESS;
}

void freeResources() {
    sem_unlink(SEM_FIRST_NAME);
    sem_unlink(SEM_SECOND_NAME);
}

void writeStringsParent(const char * message, sem_t *semFirst, sem_t *semSecond) {

    for (int i = 0; i < ITERATIONS_NUM; ++i) {
        sem_wait(semFirst);

        fprintf(stdout, "%s: %d\n", message, i);
        fflush(stdout);

        sem_post(semSecond);
    }

}

void writeStringsChild(const char * message, sem_t *semFirst, sem_t *semSecond) {

    for (int i = 0; i < ITERATIONS_NUM; ++i) {
        sem_wait(semSecond);

        fprintf(stdout, "%s: %d\n", message, i);
        fflush(stdout);

        sem_post(semFirst);
    }

}

int main(){
    pid_t pid = fork();

    if (pid < STATUS_SUCCESS){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    sem_t *semFirst = sem_open(SEM_FIRST_NAME, O_CREAT, 0600, 1);
    sem_t *semSecond = sem_open(SEM_SECOND_NAME, O_CREAT, 0600, 0);

    if(pid == CHILD_RETURN_CODE){
        writeStringsChild("Child message", semFirst, semSecond);
    } else {
        writeStringsParent("Parent message", semFirst, semSecond);

        int returnStatus = waitForChildProcess();
        if(returnStatus < STATUS_SUCCESS){
            fprintf(stderr, "waitForChildProcess error.\n");
            exit(EXIT_FAILURE);
        }

    }

    freeResources();

    exit(STATUS_SUCCESS);
}