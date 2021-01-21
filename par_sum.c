/*
 * Author - Sowmya Kudva
 * Makefile used:
 * all: opsdemo.c
 *		gcc -g -O0 --std=c99 -Wall -o par_sum par_sum.c
 *
 * clean:
 *		rm -f par_sum
 */

#include<limits.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

//aggregate variables
long  sum = 0;
long  odd = 0;
long  min = INT_MAX;
long  max = INT_MIN;
bool volatile okTogo = false;


//mutex and condition variables
pthread_mutex_t  mutex 		= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  q_mutex 	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   q_cond 	= PTHREAD_COND_INITIALIZER;


typedef struct Node {
	int data;
	struct Node* next;
} Node_t;

Node_t * volatile rear = NULL;
Node_t * volatile front = NULL;


struct Node* newNode(int item){
	struct Node* node = (struct Node*)malloc(sizeof(struct Node));

	if(node != NULL){
		node->data = item;
		node->next = NULL;
		return node;
	} else {
		printf("\n Heap overflow");
		exit(EXIT_FAILURE);
	}
}

int dequeue(){
	if (front == NULL){
		return 0;
	}

	struct Node *temp = front;
	front = front->next;

	if(front == NULL){
		rear = NULL;
	}

	int item = temp->data;
	free(temp);
	return item;
}

void enqueque(int item){
	struct Node* node = newNode(item);

	if(front == NULL){
		front = node;
		rear = node;
	} else {
		rear->next = node;
		rear = node;
	}
}

int isEmpty(){
	bool test = (rear == NULL && front == NULL);
	return test;
}

void* squaringFunction(void *arg){
	int number = *(int *)arg;
	long the_sq = number * number;
	sleep(number);

	//enter critical section
	pthread_mutex_lock(&mutex);
		sum +=the_sq;
		if(number % 2 == 1){
			odd++;
		}
		if(number < min){
			min = number;
		}
		if( number > max){
			max = number;
		}
	pthread_mutex_unlock(&mutex);
return NULL;
}

void* threadFunction(void *arg){
	while(true) {
		pthread_mutex_lock(&q_mutex);
		int num;
		while(isEmpty()){
			if(okTogo){
				pthread_mutex_unlock(&q_mutex);
				pthread_exit(0);
			}
			pthread_cond_wait(&q_cond, &q_mutex);
		}
		num = dequeue();
		pthread_mutex_unlock(&q_mutex);
		if(num !=0){
			squaringFunction(&num);
		}
	}
	pthread_exit(0);
}


int main(int argc, char **argv){

	if(argc < 2) {
		printf("Usage: %s <filename> <numOfThreads>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int num_of_threads = atoi(argv[2]);

	pthread_t tid[num_of_threads];
	pthread_attr_t attr;


	pthread_attr_init(&attr);
	for(int i=0; i<num_of_threads; i++){
		pthread_create(&tid[i], &attr, threadFunction, NULL);
	}

	char *fn= argv[1];
		FILE* fin = fopen(fn, "r");
		if( fin == NULL){
			printf("ERROR : file empty\n");
			exit(EXIT_FAILURE);
		}
		char action;
		long num;

		while(fscanf(fin, "%c %ld\n", &action, &num) ==2){
			if(action == 'p'){
				pthread_mutex_lock(&q_mutex);
				enqueque(num);
				pthread_cond_signal(&q_cond);
				pthread_mutex_unlock(&q_mutex);
			} else if(action == 'w'){
				sleep(num);
			} else {
				printf("Error unrecognized action : '%c'\n", action);
				exit(EXIT_FAILURE);
			}
		}
		fclose(fin);

	pthread_mutex_lock(&q_mutex);
	okTogo = true;
	pthread_cond_broadcast(&q_cond);
	pthread_mutex_unlock(&q_mutex);


	//wait and join the threads after exit
	for(int i=0; i<num_of_threads; i++){
		pthread_join(tid[i], NULL);
	}

	//print results
	printf("%ld %ld %ld %ld\n", sum, odd, min, max);
	//clean up and return
	return (EXIT_SUCCESS);
}

