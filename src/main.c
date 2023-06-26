/**
 * Concorrencia por elevatores
 * Cada andar poerá chamar um elevator e será colocado em uma fila
 * O programa deve decidir qual o melhor trajeto para atender as chamadas de forma mais produtiva
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ELEVATORS 2
#define NUM_USERS 15
#define NUM_FLOORS 20
#define TRUE 1
#define FALSE 0
#define DOWN 0
#define UP 1


// variaveis globais
int callBuffer[NUM_FLOORS], trackBuffer[NUM_FLOORS];
int countCbuf = 0;
int elevatorPos[NUM_ELEVATORS];
int directionState = UP;

pthread_mutex_t callMutex, trackMutex;
pthread_cond_t elevCond, deciCond, callCond;

void* elevator(void* arg);
void* callsHandler(void* arg);
void* decider(void* arg);
void SCAN(int head, int direction);


int main(){
    pthread_t ele[NUM_ELEVATORS], usr[NUM_USERS], deci;
    int *id, err;

    // inicia buffers com valores improvaveis
    for (size_t i = 0; i < NUM_FLOORS; i++){
        callBuffer[i] = NUM_FLOORS;
        trackBuffer[i] = NUM_FLOORS;
    }
    
    pthread_mutex_init(&callMutex, NULL);
    pthread_mutex_init(&trackMutex, NULL);
    pthread_cond_init(&elevCond, NULL);
    pthread_cond_init(&deciCond, NULL);
    pthread_cond_init(&callCond, NULL);

    // inicia elevadores na posição 0
    for (size_t i = 0; i < NUM_ELEVATORS; i++)
        elevatorPos[i] = 0;

    // threads
    id = (int *)malloc(sizeof(int));
    for (int i = 0; i < NUM_ELEVATORS; i++) {
        *id = i;
        err = pthread_create(&ele[i], NULL, elevator, (void*)id);
        if(err){
            printf("Erro ao criar a thread %d\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < NUM_USERS; i++) {
        *id = i;
        err = pthread_create(&usr[i], NULL, callsHandler, (void*)id);
        if(err){
            printf("Erro ao criar a thread %d\n", i);
            exit(1);
        }
    }
    pthread_create(&deci, NULL, decider, NULL);

    pthread_join(ele[0], NULL);
    
    return 0;
}

void* elevator(void* arg){
    /**
     * Caso não haja chamadas no trackBuffer, deve esperar;
     * Elevador segue o trajeto que o decider colocou no trackBuffer;
     * Consome o trackBuffer, atendendo a cada solicitação na ordem, como uma fila.
    */
    int id = *((int*) arg);
    int floor = 0;
    while (TRUE) {
        pthread_mutex_lock(&trackMutex);

        while (trackBuffer[0] == NUM_FLOORS) {// wait
            printf("Elevador %d esperando\n", id);
            pthread_cond_wait(&elevCond, &trackMutex);
        }
        for (int i = 0; i < NUM_FLOORS; i++) {
            if(trackBuffer[i] < NUM_FLOORS){
                elevatorPos[id] = floor; // origin
                floor = trackBuffer[i]; // enqueue
                trackBuffer[i] = NUM_FLOORS; // pop
                printf("Elevador %d, partiu de %d e atendeu o andar %d\n", id, elevatorPos[id], floor);
            }
        }
        pthread_mutex_unlock(&trackMutex);
    }
    pthread_exit(0);
}


void* callsHandler(void* arg){
    /**
     * Caso o callBuffer esteja cheio, aguarde...
     * Gera chamadas para andares de forma randomica e armazena em callBuffer;
     * Garante que a chamada é unica, ou seja, não tem mais de uma chamada por andar (callBuffer segue como se fosse um Set);
     * Caso seja a primeira chamada, acxorda o decider, que esta esperando por novas chamadas.
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
        // unique floor insert
        int contains = FALSE;
        while (TRUE) {
            contains = FALSE;
            for (int i = 0; i < NUM_FLOORS; i++) {
                if (callBuffer[i] == NUM_FLOORS) break;
                if (callBuffer[i] == floor) contains = TRUE;
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
     * Se callBuffer estiver vazio, espera...
     * Utiliza o algoritmo de SCAN para decidir a rota que deve ser seguida pelo elevador que for atender
     * Acorda os elevadores e chamadores
    */
    int sumElevPos = 0;
    while (TRUE) {
        sleep(10);
        pthread_mutex_lock(&callMutex);
        // wait
        while (countCbuf == 0) {
            printf("Decisor esperando\n");
            pthread_cond_wait(&deciCond, &callMutex);
        }

        pthread_mutex_lock(&trackMutex);

        // SCAN

        // somatorio das posições dos elevadores
        for (int i = 0; i < NUM_ELEVATORS; i++){
            sumElevPos += elevatorPos[i];
        }
        
        SCAN((sumElevPos/NUM_ELEVATORS), directionState);
        directionState = !directionState; // swipe direction
        // cleaning callBuffer
        for (int i = 0; i < NUM_FLOORS; i++) {
            if(callBuffer[i] == NUM_FLOORS) break;
            // trackBuffer[i] = callBuffer[i]; // copy
            callBuffer[i] = NUM_FLOORS;
        }
        countCbuf = 0;

        // wakeup callers
        pthread_cond_broadcast(&callCond);
        pthread_mutex_unlock(&callMutex);
        
        // wakeup elevators
        pthread_cond_broadcast(&elevCond);
        pthread_mutex_unlock(&trackMutex);
    }
    pthread_exit(0);
}

void SCAN(int head, int direction)
{
	int seek_count = 0;
	int distance, cur_track;
	int* down = malloc(sizeof(int) * NUM_FLOORS);
    int* up = malloc(sizeof(int) * NUM_FLOORS);
    int down_count = 0;
    int up_count = 0;

	// setup up and down arrays
	if (direction == DOWN)
		down[down_count++] = -1;
	else if (direction == UP)
		up[up_count++] = NUM_FLOORS;

	for (int i = 0; i < NUM_FLOORS-1; i++) {
		if (callBuffer[i] < head)
			down[down_count++] = callBuffer[i];
		else if (callBuffer[i] > head)
			up[up_count++] = callBuffer[i];
        else up[up_count++] = callBuffer[i];
	}

	// Sorting
    for (int i = 0; i < down_count - 1; i++) {
        for (int j = 0; j < down_count - i - 1; j++) {
            if (down[j] > down[j + 1]) {
                int temp = down[j];
                down[j] = down[j + 1];
                down[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < up_count - 1; i++) {
        for (int j = 0; j < up_count - i - 1; j++) {
            if (up[j] > up[j + 1]) {
                int temp = up[j];
                up[j] = up[j + 1];
                up[j + 1] = temp;
            }
        }
    }
	// run the while loop two times.
	// one by one scanning up
	// and down of the head
	int run = 2;
    int c = 0;
	while (run--) {
		if (direction == DOWN) {
			for (int i = down_count - 1; i >= 0; i--) {
                cur_track = down[i];
                
                // Appending current track to seek sequence
                if(cur_track != -1){
                    trackBuffer[c] = cur_track;
                    c++;
                }

                // Calculate absolute distance
                distance = abs(cur_track - head);

                // Increase the total count
                seek_count += distance;

                // Accessed track is now the new head
                head = cur_track;
            }
			direction = UP;
		}
		else if (direction == UP) {
			for (int i = 0; i < up_count; i++) {
                cur_track = up[i];
                
                // Appending current track to seek sequence
                if(cur_track != -1){
                    trackBuffer[c] = cur_track;
                    c++;
                }
                // Calculate absolute distance
                distance = abs(cur_track - head);

                // Increase the total count
                seek_count += distance;

                // Accessed track is now new head
                head = cur_track;
            }
			direction = DOWN;
		}
	}
}
