#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

// defines macros = mapping for shared memory
#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap( (pointer), sizeof((pointer)) );}
FILE *proj2;

// semaphores
sem_t *sem_main = NULL;
sem_t *sem_cust1 = NULL;
sem_t *sem_cust2 = NULL;
sem_t *sem_cust3 = NULL;
sem_t *sem_post_off1 = NULL;
sem_t *sem_post_off2 = NULL;
sem_t *sem_post_off3 = NULL;

// global variables
int NZ;
int NU;
int TZ;
int TU;
int F;

// shared variables
int *act_counter = NULL;
int *cust_counter = NULL;
int *line_counter = NULL;
int *queue1 = NULL;
int *queue2 = NULL;
int *queue3 = NULL;
bool *post_closed;

// function to inititalize shared variables and semaphores
int init_sems_vars(){
    MMAP(act_counter);
    MMAP(cust_counter);
    MMAP(line_counter);
    MMAP(queue1);
    MMAP(queue2);
    MMAP(queue3);
    MMAP(post_closed);

    act_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ((sem_main = sem_open("/xhanco00.sem_main", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){return 1;}
    if ((sem_cust1 = sem_open("/xhanco00.sem_cust1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    if ((sem_cust2 = sem_open("/xhanco00.sem_cust2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    if ((sem_cust3 = sem_open("/xhanco00.sem_cust3", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    if ((sem_post_off1 = sem_open("/xhanco00.sem_post_off1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    if ((sem_post_off2 = sem_open("/xhanco00.sem_post_off2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    if ((sem_post_off3 = sem_open("/xhanco00.sem_post_off3", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){return 1;}
    return 0;
}

// function customer for the whole customer process and its connection to post officer
void customer(int cust_id){
    sem_wait(sem_main);
    fprintf(proj2, "%d: Z %d: started\n", ++(*line_counter), cust_id);
    fflush(proj2);
    sem_post(sem_main);

    usleep(1000*(rand()%(TZ+1)));
    
    // picks random request to deal with in the post office
    int random_req = rand() % 3 + 1;
    
    // in case the post office is opened, if statement executes
    if ((*post_closed) == false) {

        sem_wait(sem_main);
        *cust_counter += 1;
        fprintf(proj2, "%d: Z %d: entering office for a service %d\n", ++(*line_counter), cust_id, random_req);
        fflush(proj2);
        sem_post(sem_main);

        // queue counters for each queue increments if customer picks one of the 3 queues, depending on the random_req variable
        sem_wait(sem_main);
        if (random_req == 1){
            (*queue1)++;
        }
        else if(random_req == 2){
            (*queue2)++;
        }
        else if (random_req == 3){
            (*queue3)++;
        }
        sem_post(sem_main);

        // if statement to sync customers in each queue
        if (random_req == 1){
            sem_wait(sem_post_off1);
            sem_post(sem_cust1);
        }
        else if(random_req == 2){
            sem_wait(sem_post_off2);
            sem_post(sem_cust2);
        }
        else if (random_req == 3){
            sem_wait(sem_post_off3);
            sem_post(sem_cust3);
        }

        sem_wait(sem_main);
        fprintf(proj2, "%d: Z %d: called by office worker\n", ++(*line_counter), cust_id);
        fflush(proj2);
        sem_post(sem_main);

        usleep(1000 * (rand() % 11));

        sem_wait(sem_main);
        fprintf(proj2, "%d: Z %d: going home\n", ++(*line_counter), cust_id);
        fflush(proj2);
        sem_post(sem_main);
        
        exit(0);
    }
    // executes if the post office is closed
    else {
        sem_wait(sem_main);
        fprintf(proj2, "%d: Z %d: going home\n", ++(*line_counter), cust_id);
        fflush(proj2);
        sem_post(sem_main);

        exit(0);
    }
}

// function post officer for the whole post officer process and its connection to customer
void post_officer(int post_off_id){
    sem_wait(sem_main);
    fprintf(proj2, "%d: U %d: started\n", ++(*line_counter), post_off_id);
    fflush(proj2);
    sem_post(sem_main);

    // picks random queue to serve
    int random_queue = (rand() % 3) + 1;
    
    // while cycle loops until the post is not closed and until there are customers to serve
    while (!((*cust_counter) == 0 && (*post_closed))) {
        sem_wait(sem_main);
        // main if statement in this function for serving a customer
        if(((*cust_counter) != 0) && (((queue1 != 0) || (queue2 != 0) || (queue3 != 0)))){
            (*cust_counter)--;
            sem_post(sem_main);

            // counters for each queue decrements if post officer picks any non empty queue
            sem_wait(sem_main);
            if (random_queue == 1 && (*queue1) > 0) {
                (*queue1)--;
            }
            else if (random_queue == 2 && (*queue2) > 0) {
                (*queue2)--;
            }
            else if (random_queue == 3 && (*queue3) > 0) {
                (*queue3)--;
            }
            else {
                random_queue = (rand() % 3) + 1;
                *cust_counter += 1;
                sem_post(sem_main); 
                continue;
            }
            sem_post(sem_main);

            // if statements to sync customers in each queue
            if (random_queue == 1) {
                sem_post(sem_post_off1);
                sem_wait(sem_cust1);
            }
            else if (random_queue == 2) {
                sem_post(sem_post_off2);
                sem_wait(sem_cust2);
            }
            else if (random_queue == 3) {
                sem_post(sem_post_off3);
                sem_wait(sem_cust3);
            }

            sem_wait(sem_main);
            fprintf(proj2, "%d: U %d: serving a service of type %d\n", ++(*line_counter), post_off_id, random_queue);
            fflush(proj2);
            sem_post(sem_main);

            usleep(1000 * (rand() % 11));

            sem_wait(sem_main);
            fprintf(proj2, "%d: U %d: service finished\n", ++(*line_counter), post_off_id);
            fflush(proj2);
            sem_post(sem_main);
        }
        // else if executes if there are no customers left in the post office, but the post office is still opened
        else if ((*cust_counter) == 0) {
            sem_post(sem_main);

            sem_wait(sem_main);
            fprintf(proj2, "%d: U %d: taking break\n", ++(*line_counter), post_off_id);
            fflush(proj2);
            sem_post(sem_main);

            usleep(1000 * (rand() % (TU + 1)));

            sem_wait(sem_main);
            fprintf(proj2, "%d: U %d: break finished\n", ++(*line_counter), post_off_id);
            fflush(proj2);
            sem_post(sem_main);
        }
        else {
            sem_post(sem_main);
        }
    }
    sem_wait(sem_main);
    fprintf(proj2, "%d: U %d: going home\n", ++(*line_counter), post_off_id);
    fflush(proj2);
    sem_post(sem_main);
    exit(0);
}

// function clean_all() unmaps all shared variables, closes and unlinks all semaphores so no memory leaks can occur
void clean_all(){
    UNMAP(act_counter);
    UNMAP(cust_counter);
    UNMAP(line_counter);
    UNMAP(queue1);
    UNMAP(queue2);
    UNMAP(queue3);
    UNMAP(post_closed);

    sem_close(sem_main);
    sem_close(sem_cust1);
    sem_close(sem_cust2);
    sem_close(sem_cust3);
    sem_close(sem_post_off1);
    sem_close(sem_post_off2);
    sem_close(sem_post_off3);

    sem_unlink("/xhanco00.sem_main");
    sem_unlink("/xhanco00.sem_cust1");
    sem_unlink("/xhanco00.sem_cust2");
    sem_unlink("/xhanco00.sem_cust3");
    sem_unlink("/xhanco00.sem_post_off1");
    sem_unlink("/xhanco00.sem_post_off2");
    sem_unlink("/xhanco00.sem_post_off3");
}

int main(int argc, char* argv[]) {
    // checking the right amount of input parameters
    if (argc != 6 ) {
        fprintf (stderr, "Not enough arguments!\n");
        exit(1);
    }

    // checking if all of them are numbers
    if (!isdigit(*argv[1]) || !isdigit(*argv[2]) || !isdigit(*argv[3]) || 
        !isdigit(*argv[4]) || !isdigit(*argv[5])) {
        fprintf(stderr, "One of the arguments is not a number!\n");
        exit(1);
    }

    NZ = atoi(argv[1]);
    NU = atoi(argv[2]);
    TZ = atoi(argv[3]);
    TU = atoi(argv[4]);
    F = atoi(argv[5]);

    // checking if TZ, TU and F global variables are within their own intervals
    if (TZ < 0 || TZ > 10000) {
        fprintf(stderr, "TZ parameter is out of interval <0,10000>!\n");
        exit(1);
    }

    if (TU < 0 || TU > 100) {
        fprintf(stderr, "TU parameter is out of interval <0,100>!\n");
        exit(1);
    }

    if (F < 0 || F > 10000) {
        fprintf(stderr, "F parameter is out of interval <0,10000>!\n");
        exit(1);
    }

    // closes output file proj2 and exits with exit code 1 in case the output file proj2 can not be opened
    if ((proj2 = fopen("proj2.out","w")) == NULL) {
        fprintf(stderr, "Cannot open the file!\n");
        fclose(proj2);
        exit(1);
    }
    
    /*  checking the return value of a function init_sems_vars(), if it returns a value of 1, it means the error occured,
        calls clean_all() function and closes the output file proj2 and exits with exit code 1 */
    if(init_sems_vars() == 1) {
        clean_all();
        fclose(proj2);
        fprintf(stderr, "\n");
        exit(1);
    }

    // starting values for each of the shared variables
    (*act_counter) = 0;
    (*cust_counter) = 0;
    (*line_counter) = 0;
    (*queue1) = 0;
    (*queue2) = 0;
    (*queue3) = 0;
    (*post_closed) = false;

    srand(time(0));

    // for cycles for customer and post_officer processes 
    for (int i = 1; i <= NZ; i++) {
        pid_t cust_id = fork();
        if(cust_id == 0) {
            srand(getpid());
            customer(i);
            exit(0);
        }
    }

    for (int i = 1; i <= NU; i++) {
        pid_t post_off_id = fork();
        if(post_off_id == 0) {
            srand(getpid());
            post_officer(i);
            exit(0);
        }
    }

    // picks random number from inteval F/2 to F inclusive, after which the post closes
    int random_F = rand() % (F/2 + 1) + F/2;
    usleep(1000 * random_F);
    
    sem_wait(sem_main);
    (*post_closed) = true;
    fprintf(proj2, "%d: closing\n", ++(*line_counter));
    fflush(proj2);
    sem_post(sem_main);

    // while cycle waits for all child processes to terminate
    while(wait(NULL)>0);

    clean_all();
    fclose(proj2);

    return 0;
}