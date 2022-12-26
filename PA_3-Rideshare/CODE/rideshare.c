#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// COPYRIGHT YIGIT KARAMANLI
// I DONT KNOW WHY, BUT DURING THIS HOMEWORK I DISCOVERED THAT
// THERE IS A HIGHER PROBABILITY OF A DEADLOCK IF YOU HAVE A FASTER COMPUTER

typedef struct semaphore   // Implemented my own semaphores with the help of lecture slides
{
    int val;   // Value of the semaphore
    pthread_mutex_t lock; // lock
    pthread_cond_t c; // cond. variable
} sem_t;

typedef struct fan  // Struct to pass into the functions. Has two semaphores, one for each team
{
    char fantext; // Team of the fan
    sem_t * sem1;
    sem_t * sem2;
} fans;

pthread_barrier_t barrier; // Barrier
sem_t semA, semB; // semaphores for each team

pthread_mutex_t globlock; // global mutex lock


int sem_val(sem_t *s){ // To check the value of the semaphore
    return s->val;
}

void sem_init(sem_t *s, int val){ // Initialize the semaphore
    s->val = val;
    pthread_mutex_init(&s->lock,NULL); // Initialize the lock of the semaphore
    pthread_cond_init(&s->c,NULL);
}



void sem_lockandwait (sem_t *s, pthread_mutex_t * lock) { // sem_wait() with some extra parts
// decrement the value of s by one
// wait if the value of s is negative
pthread_mutex_lock(&s->lock);
s->val --;                      //decrement the value of the semaphore
pthread_mutex_unlock(lock);     //unlock the global lock here, otherwise another thread can also decrement the value, resulting in a deadlock
if (s->val < 0)
pthread_cond_wait(&s->c, &s->lock);
pthread_mutex_unlock(&s->lock);
}

void sem_post (sem_t *s) {  // sem_post() from lecture slides
// increment the value of s by one
// wake a sleeping thread if exists
pthread_mutex_lock(&s->lock);
s->val ++; 
pthread_cond_signal(&s->c);
pthread_mutex_unlock(&s->lock);
}

// Thread function, all threads use the same function
void *rideshare(void * args){   
    fans *fan = (fans*) args;
    char team = fan->fantext; // get the team of the thread(fan)
    printf("Thread ID: %lu, Team: %c, I am looking for a car.\n",pthread_self(),team);
    bool Cap; //bool to check if there exist a captain
    pthread_mutex_lock(&globlock); // acquire the mutex
    //printf("Thread ID: %lu controlling\n", pthread_self());  // used during debugging
    if(sem_val(fan->sem1) <= -1 && sem_val(fan->sem2) <= -2){   //check if there is 1 more thread waiting with the same team and 2 threads waiting having the rival team
        Cap = true;  // set thread as the captain 
        sem_post(fan->sem1);    
        sem_post(fan->sem2);     //wake up 2 rival threads and 1 with the same team
        sem_post(fan->sem2);
    }
    else if (sem_val(fan->sem1)== -3) // if threre are 3 more threads waiting with the same team
    {
        Cap = true;                          // set thread as captain
        sem_post(fan->sem1);
        sem_post(fan->sem1);                 //wake up the 3 threads
        sem_post(fan->sem1);
    }
    else{                   // if the conditions are not met.
        //printf("Thread ID: %lu unlocked\n", pthread_self());    // for debugging
        //printf("Thread ID: %lu zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n", pthread_self());  // for debugging 
        sem_lockandwait(fan->sem1, &globlock);    //decrement the value of the semaphore, release the global mutex and sleep.
    }

    printf("Thread ID: %lu, Team: %c, I have found a spot in a car\n", pthread_self(), team);
    pthread_barrier_wait(&barrier);    // woken threads wait after getting a spot in the car

    if(Cap){
        printf("Thread ID: %lu, Team: %C, I am the captain and driving the car\n", pthread_self(), team);
        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier,NULL,4);  // captain destroys and reinitializes the barrier
        pthread_mutex_unlock(&globlock);
    }
    return NULL;

}

int main(int argc, char *argv[]){
    int supporterA = atoi(argv[1]);
    int supporterB = atoi(argv[2]);
    if((supporterA + supporterB)%4 != 0 || supporterA%2 != 0 || supporterB%2 != 0){  // input check 
        printf("The main terminates\n");
        exit(0);
    }
    pthread_barrier_init(&barrier,NULL,4);  // initialize the barrier
    pthread_mutex_init(&globlock,NULL);      // initialize the global mutex
    int totalnum = supporterA+supporterB;    
    pthread_t threads[totalnum];      //thread array 
    sem_init(&semA,0);       // initialize the semaphores
    sem_init(&semB,0);
    fans A;
    A.fantext= 'A';
    A.sem1= &semA;              
    A.sem2= &semB;              //Fan A has semA as sem1 and semB as sem2
    fans B;                     //While Fan B has semB as sem1 and semA as sem2
    B.fantext= 'B';             //So they can use the same function without much complications
    B.sem1= &semB;
    B.sem2= &semA;

    int count;
    for(count = 0; count < supporterA; count++){   
        pthread_create(&threads[count],NULL,rideshare,&A);
    }                                                       // create Fan threads
    for(; count < supporterB+supporterA; count++){
        pthread_create(&threads[count],NULL,rideshare,&B);
    }

    for(count = 0; count < supporterA+supporterB; count++){         // wait for all the threads to terminate
        pthread_join(threads[count],NULL);
    }

    pthread_barrier_destroy(&barrier);
    printf("The main terminates\n");

    return 0;
}