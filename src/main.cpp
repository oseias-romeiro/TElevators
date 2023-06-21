/**
 * Concorrencia por elevatores, ha 3 elevatores e 10 andares
 * cada andar chamará um elevator e será colocado em uma fila
 * o programa deve decidir qual elevator vai até onde foi chamado de forma concorrente
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>

#define NUM_ELEVATORS 1
#define NUM_USERS 5
#define FLOORS 20

using namespace std;

// global var
queue<int> callBuffer = {};
int elevatorPos[NUM_ELEVATORS]; 

pthread_mutex_t bufMutex;
pthread_cond_t elevCond;
pthread_cond_t callCond;

// threads
void* elevator(void* arg);
void* callsHandler(void* arg);


int main(){
    pthread_t ele[NUM_ELEVATORS], usr[NUM_USERS];
    int *id, err;

    pthread_mutex_init(&bufMutex, NULL);
    pthread_cond_init(&elevCond, NULL);
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
    int pos = 0;
    int floor = 0;
    while (true) {
        pthread_mutex_lock(&bufMutex);

        while (callBuffer.empty()) {// wait
            printf("Elevador %d esperando\n", id);
            pthread_cond_wait(&elevCond, &bufMutex);
        }
        // get call from queue
        pos = floor;
        floor = callBuffer.front();
        callBuffer.pop();

        // wakeup callers
        if (callBuffer.size() == FLOORS-1)
            pthread_cond_broadcast(&callCond);

        pthread_mutex_unlock(&bufMutex);

        // answer
        sleep(5);
        printf("Elevador %d, partiu de %d e atendeu o andar %d\n", id, pos, floor);
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
    while (true) {
        sleep(5);
        // generate random floors
        floor = drand48() * FLOORS;

        // call logic
        pthread_mutex_lock(&bufMutex);
        while (callBuffer.size() >= FLOORS) {// wait
            printf("Chamada %d esperando no andar %d\n", id, floor);
            pthread_cond_wait(&callCond, &bufMutex);
        }
        callBuffer.push(floor);
        printf("Chamada no andar %d\n", floor);

        // wakeup elevators
        if (callBuffer.size() == 1)
            pthread_cond_broadcast(&elevCond);

        pthread_mutex_unlock(&bufMutex);
    }
    pthread_exit(0);
}
