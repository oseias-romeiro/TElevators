/**
 * Concorrencia por elevatores, ha 3 elevatores e 10 andares
 * cada andar chamará um elevator e será colocado em uma fila
 * o programa deve decidir qual elevator vai até onde foi chamado de forma concorrente
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <set>

#define NUM_ELEVATORS 2
#define NUM_USERS 10
#define FLOORS 10

using namespace std;

// global var
set<int> callBuffer = {};
int elevatorPos[NUM_ELEVATORS]; 

pthread_mutex_t bufMutex;
pthread_cond_t bufCond;

// threads
void* elevator(void* arg);
void* callsHandler(void* arg);


int main(){
    pthread_t ele[NUM_ELEVATORS], usr[NUM_USERS];
    int *id, err;

    pthread_mutex_init(&bufMutex, NULL);
    pthread_cond_init(&bufCond, NULL);

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
    int floor, distance;
    set<int>::iterator itr;
    while (true) {
        pthread_mutex_lock(&bufMutex);

        while (callBuffer.empty()) {// wait
            printf("Elevador %d esperando\n", id);
            pthread_cond_wait(&bufCond, &bufMutex);
        }
        // get closest call
        int dist;
        distance = FLOORS+1;// impossible distance
        for (itr = callBuffer.begin(); itr != callBuffer.end(); itr++) {
            dist = abs(elevatorPos[id] - *itr);
            if(dist < distance){
                floor = *itr;
                distance = dist;
            }
        }
        // TODO: pass floor by floor checking if there no are calls to it. if yes, answer call too
        callBuffer.erase(floor);
        elevatorPos[id] = floor;// update elevator position
        pthread_mutex_unlock(&bufMutex);

        // answer
        sleep(1);
        printf("Elevador %d atendeu o andar %d\n", id, floor);
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
        sleep(10);
        // generate random floors
        floor = drand48() * FLOORS;

        // call logic
        pthread_mutex_lock(&bufMutex);
        printf("Chamada %d no andar %d\n", id, floor);
        callBuffer.insert(floor);

        pthread_cond_signal(&bufCond);// wakeup elevator
        pthread_mutex_unlock(&bufMutex);
    }
    pthread_exit(0);
}
