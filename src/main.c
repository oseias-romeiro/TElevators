/**
 * Concorrencia por elevatores, ha 3 elevatores e 10 andares
 * cada andar chamará um elevator e será colocado em uma fila
 * o programa deve decidir qual elevator vai até onde foi chamado de forma concorrente
*/

// TODO: dividir o código em módulos

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ELEVATORS 1
#define NUM_USERS 5
#define NUM_FLOORS 20
#define TRUE 1
#define FALSE 1


// global vars
int callBuffer[NUM_FLOORS], trackBuffer[NUM_FLOORS];
int countCbuf = 0, countTbuf = -1;
int elevatorPos[NUM_ELEVATORS]; 

pthread_mutex_t callMutex, trackMutex;
pthread_cond_t elevCond, deciCond, callCond;

// threads
void* elevator(void* arg);
void* callsHandler(void* arg);
void* decider(void* arg);


int main(){
    pthread_t ele[NUM_ELEVATORS], usr[NUM_USERS], deci;
    int *id, err;

    // init buffers
    for (size_t i = 0; i < NUM_FLOORS; i++){
        callBuffer[i] = -1;
        trackBuffer[i] = -1;
    }
    
    // mutex & conds
    pthread_mutex_init(&callMutex, NULL);
    pthread_mutex_init(&trackMutex, NULL);
    pthread_cond_init(&elevCond, NULL);
    pthread_cond_init(&deciCond, NULL);
    pthread_cond_init(&callCond, NULL);

    // init elevators position (floor 0)
    for (size_t i = 0; i < NUM_ELEVATORS; i++)
        elevatorPos[i] = 0;

    id = (int *)malloc(sizeof(int));

    // elevators thread creation
    for (int i = 0; i < NUM_ELEVATORS; i++) {
        *id = i;
        err = pthread_create(&ele[i], NULL, elevator, (void*)id);
        if(err){
            printf("Erro ao criar a thread %d\n", i);
            exit(1);
        }
    }
    // users thread creation
    for (int i = 0; i < NUM_USERS; i++) {
        *id = i;
        err = pthread_create(&usr[i], NULL, callsHandler, (void*)id);
        if(err){
            printf("Erro ao criar a thread %d\n", i);
            exit(1);
        }
    }
    pthread_create(&deci, NULL, decider, NULL);
    // init threads
    pthread_join(ele[0], NULL);
    
    return 0;
}

void* elevator(void* arg){
    /**
     * get calls in buffer
     * answer calls
     * movement logic
    */
    int id = *((int*) arg);
    int pos = 0, floor = 0;
    while (TRUE) {
        pthread_mutex_lock(&trackMutex);

        while (countTbuf == -1) {// wait
            printf("Elevador %d esperando\n", id);
            pthread_cond_wait(&elevCond, &trackMutex);
        }
        for (int i = 0; i < countTbuf+1; i++) {
            pos = floor; // origin
            floor = trackBuffer[i]; // enqueue
            trackBuffer[i] = -1; // pop
            printf("Elevador %d, partiu de %d e atendeu o andar %d\n", id, pos, floor);
        }
        countTbuf = -1;// clean queue
        pthread_mutex_unlock(&trackMutex);
    }
    pthread_exit(0);
}


void* callsHandler(void* arg){
    /**
     * generate a random target floor to go
     * decide which elevator would traited call
     * send solicitation to elevator
    */
    int id = *((int*) arg);
    int floor;
    while (TRUE) {
        sleep(5);
        // generate random floors
        floor = drand48() * NUM_FLOORS;

        // call logic
        pthread_mutex_lock(&callMutex);
        while (countCbuf >= NUM_FLOORS) {// wait
            printf("Chamada %d esperando no andar %d\n", id, floor);
            pthread_cond_wait(&callCond, &callMutex);
        }
        // unique insert
        int contains = 0;
        while (TRUE) {
            contains = 0;
            for (int i = 0; i < NUM_FLOORS; i++) {
                if (callBuffer[i]== -1) break;
                if (callBuffer[i] == floor) contains = 1;
            }
            if (contains) floor = drand48() * NUM_FLOORS;
            else break;
        }
        callBuffer[countCbuf] = floor;
        printf("Chamada no andar %d\n", floor);

        // wakeup decider
        if (countCbuf == 1)
            pthread_cond_signal(&deciCond);
 
        countCbuf++;
        pthread_mutex_unlock(&callMutex);
    }
    pthread_exit(0);
}

void* decider(void* args){
    /**
     * take floors from caller buffer
     * use SCAN algorithm to decide best track
     * feed track buffer for elevators
    */
    while (TRUE) {
        pthread_mutex_lock(&callMutex);
        // wait
        while (countCbuf == 0) {
            printf("Decisor esperando\n");
            pthread_cond_wait(&deciCond, &callMutex);
        }
        // TODO: usar o SCAN para calcular a track

        pthread_mutex_lock(&trackMutex);

        
        // feed track to elevators
        for (int i = 0; i < NUM_FLOORS; i++) {
            if(callBuffer[i] > -1)
                countTbuf++;
            trackBuffer[i] = callBuffer[i];
            // cleaning callBuffer
            callBuffer[i] = -1;
        }
        countCbuf = 0;
        
        // wakeup elevators
        pthread_cond_broadcast(&elevCond);
        // wakeup callers
        pthread_cond_broadcast(&callCond);

        pthread_mutex_unlock(&trackMutex);
        pthread_mutex_unlock(&callMutex);
    }
    pthread_exit(0);
}

