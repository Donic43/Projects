#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#define MAX_DATA_SIZE 256
#define MAX_THREADS 5

typedef struct _thread_data_t {
  int client_id;
  char nickname[9];
  struct _thread_data_t *next;
} thread_data_t;

void *thread_function(void *arg);

int addclient(struct _thread_data_t *c, int id) {
  struct _thread_data_t *aux = c;
  struct _thread_data_t *novo = (struct _thread_data_t*)malloc(sizeof(struct _thread_data_t));
  novo->client_id = id;
  snprintf (thread_data[count].nickname, sizeof(thread_data[count].nickname), "%d", id);
  novo->next = NULL;

}

int main(int argc, char *argv[]) {

	int socket_id = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5100);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(socket_id, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Error: Bind().\n");
		exit(1);
	}

	if (listen(socket_id, 5) < 0) {
		perror("Error: Listen().\n");
		exit(1);
	}


	pthread_t thread[MAX_THREADS];
	thread_data_t thread_data[MAX_THREADS];
	int count = 1;
	while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);

		//int client_id = accept(socket_id, (struct sockaddr *) &client_addr, &client_len);
		thread_data[count].client_id = accept(socket_id, (struct sockaddr *) &client_addr, &client_len);
		if (thread_data[count].client_id < 0) {
			perror("Error: Accept().\n");
			exit(1);
		}

		//thread_data_t thread_data;
		printf("Client: %d\n", thread_data[count].client_id);
		//thread_data[count].client_id = &client_id;
		thread_data[count].count = count;
		//itoa(count, thread_data[count].nickname, 10);
		snprintf (thread_data[count].nickname, sizeof(thread_data[count].nickname), "%d",count);

		//pthread_t thread;
		if (pthread_create(&thread[count], NULL, thread_function, &thread_data[count])) {
			perror("Error: Pthread().\n");
			exit(1);
		} else {
			count++;
		}

	}

	pthread_exit(NULL);
	int status = close(socket_id);
	return 0;
}

void *thread_function(void *arg) {
        //thread_data_t *data = (thread_data_t *)arg;
	pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t got_request = PTHREAD_COND_INITIALIZER;

	// Locking Mutex
	//int rc;
	int rc = pthread_mutex_lock(&a_mutex);
	if (rc) { // an error has occurred
		perror("pthread_mutex_lock");
		pthread_exit(NULL);
	}
        thread_data_t *data = (thread_data_t *)arg;

        char buffer[MAX_DATA_SIZE];
        int recv_size;
        rc = pthread_mutex_destroy(&a_mutex);
	//pthread_detach(pthread_self());

	memset(buffer, 0, MAX_DATA_SIZE);
        while ((recv_size = recv(data->client_id, buffer, 255, 0)) > 0) {
		if (buffer[strlen(buffer)-1] == '\n')
			buffer[strlen(buffer)-1] = '\0';
		if(strncmp(buffer, "/nick", 4) == 0) {
			printf("Name changed\n");
			strcpy(data->nickname, buffer + 6);
		}
                printf("MESSAGE %s: %s\n", data->nickname , buffer);
                memset(buffer, 0, MAX_DATA_SIZE);
        }
        printf("USER: %d disconected.\n", data->client_id); // *data->
	// Unlocking Mutex
        //rc = pthread_mutex_destroy(&a_mutex);
        pthread_exit(NULL);
	return 0;
}
