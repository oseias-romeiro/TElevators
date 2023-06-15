/**
 * Concorrencia por elevadores, ha 3 elevadores e 10 andares
 * cada andar chamará um elevador e será colocado em uma fila
 * o programa deve decidir qual elevador vai até onde foi chamado de forma concorrente
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define N_ELE 2
#define N_USR 20
#define ANDARES 10

using namespace std;

struct Chamada {
    int andar;
    bool subir; // subir/descer
};

vector<Chamada> fila = {};

pthread_mutex_t mut;
sem_t calls;

void* elevador(void* arg);
void* chamadas(void* arg);


int main(){
    pthread_t ele[N_ELE], usr[N_USR];
    int *id, err;

    pthread_mutex_init(&mut, NULL);
    sem_init(&calls, 0, ANDARES);

    id = (int *)malloc(sizeof(int));

    // elevators
    for (size_t i = 0; i < N_ELE; i++) {
        *id = i;
        err = pthread_create(&ele[i], NULL, elevador, (void*)id);
        if(err){
            cout << "Erro ao criar a thread" << i << endl;
            exit(1);
        }
    }
    // users
    for (size_t i = 0; i < N_ELE; i++) {
        *id = i;
        err = pthread_create(&usr[i], NULL, chamadas, (void*)id);
        if(err){
            cout << "Erro ao criar a thread" << i << endl;
            exit(1);
        }
    }

    pthread_join(ele[0], NULL);
    
    return 0;
}

void* elevador(void* arg){
    
    int id = *((int*) arg);
    while (1) {
        pthread_mutex_lock(&mut);
        if(fila.size() > 0){
            cout << "Elevador " << id << " atendeu o andar" << fila.begin()->andar << endl;
            fila.erase(fila.begin());
        }
        pthread_mutex_unlock(&mut);
    }
    pthread_exit(0);
}

void* chamadas(void* arg){
    int id = *((int*) arg);
    int andar;
    bool chama;
    Chamada chamada;
    while (1) {
        andar = (int)drand48() % ANDARES;
        chama = (bool) ((int)drand48() % 2);
        chamada = {andar, chama};
        sem_wait(&calls);
        cout << "Chamada " << id << " no andar " << chamada.andar  << " | subir? " << chamada.subir << endl;
        fila.push_back(chamada);
        sleep(5);
        sem_post(&calls);
    }
    pthread_exit(0);
}
