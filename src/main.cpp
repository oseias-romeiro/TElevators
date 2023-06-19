/**
 * Concorrencia por elevatores, ha 3 elevatores e 10 andares
 * cada andar chamará um elevator e será colocado em uma fila
 * o programa deve decidir qual elevator vai até onde foi chamado de forma concorrente
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <queue>

#define NUM_ELEVATORS 2
#define NUM_USERS 10
#define FLOORS 10

using namespace std;

// global var
queue<int> callBuffer = {};


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

    id = (int *)malloc(sizeof(int));

    // elevators thread creation
    for (size_t i = 0; i < NUM_ELEVATORS; i++) {
        *id = i;
        err = pthread_create(&ele[i], NULL, elevator, (void*)id);
        if(err){
            cout << "Erro ao criar a thread" << i << endl;
            exit(1);
        }
    }
    // users thread creation
    for (size_t i = 0; i < NUM_USERS; i++) {
        *id = i;
        err = pthread_create(&usr[i], NULL, callsHandler, (void*)id);
        if(err){
            cout << "Erro ao criar a thread" << i << endl;
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
    int floor;
    while (true) {
        pthread_mutex_lock(&bufMutex);

        while (callBuffer.empty()) {// wait
            cout << "elevador " << id << " esperando" << endl;
            pthread_cond_wait(&bufCond, &bufMutex);
        }
        // get call
        floor = callBuffer.front();
        callBuffer.pop();
        pthread_mutex_unlock(&bufMutex);

        // answer
        sleep(5);
        cout << "elevador " << id << " atendeu o andar " << floor << endl;
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
        // generate random floors
        floor = drand48() * FLOORS;

        // call logic
        pthread_mutex_lock(&bufMutex);
        cout << "Chamada " << id << " no andar " << floor << endl;
        callBuffer.push(floor);

        // wakeup elevator
        pthread_cond_signal(&bufCond);

        pthread_mutex_unlock(&bufMutex);

        // wait be answered
        sleep(5);
    }
    pthread_exit(0);
}
