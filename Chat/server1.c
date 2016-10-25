#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#define MAX_DATA_SIZE 256
#define MAX_DATA_SEND_SIZE 268
#define MAX_THREADS 5

typedef struct _thread_data_t {
  int client_id;
  char nickname[9];
  struct _thread_data_t *next;
  struct _thread_data_t *before;
} thread_data_t;

typedef struct _thread_data_send {
  int sender;
  char message[MAX_DATA_SIZE];
  thread_data_t *clients;
} thread_data_send;

void *thread_client(void *arg);

int addclient(struct _thread_data_t *c, int id) {
  if(id < 0)
    return id;

  struct _thread_data_t *aux = c;
  struct _thread_data_t *novo = (struct _thread_data_t*)malloc(sizeof(struct _thread_data_t));
  novo->client_id = id;
  snprintf (novo->nickname, sizeof(novo->nickname), "%d", id);
  novo->next = NULL;

  while(aux->next != NULL) {
    aux = aux->next;
  }
  aux->next = novo;
  novo->before = aux;
  return 0;
}

int removeclient(struct _thread_data_t *c, int id) {
  if(id < 0)
    return id;

  struct _thread_data_t *aux = c;
  struct _thread_data_t *bef = c;
  aux = aux->next;

  while(aux->next != NULL) {
    if(aux->client_id == id) {
      bef->next = aux->next;
      return 0;
    }
    bef = aux;
    aux = aux->next;
  }
  if(aux->client_id == id) {
    bef->next = NULL;
    return 0;
  }

  return 1;
}

char *getclientnick(struct _thread_data_t *c, int id) {
  if(id < 0)
    return NULL;

  struct _thread_data_t *aux = c;
  aux = aux->next;

  while(aux->next != NULL) {
    if(aux->client_id == id) {
      return aux->nickname;
    }
    aux = aux->next;
  }
  if(aux->client_id == id) {
    return aux->nickname;
  }

  return NULL;
}

struct _thread_data_t * getlastclient(struct _thread_data_t *c) {
  struct _thread_data_t *aux = c;

  while(aux->next != NULL) {
    aux = aux->next;
  }
  return aux;
}

struct _thread_data_t * getfirstclient(struct _thread_data_t *c) {
  struct _thread_data_t *aux = c;

  while(aux->before != NULL) {
    aux = aux->before;
  }
  return aux;
}

int main(int argc, char *argv[]) {

  /* CREATE SOCKET */
	int socket_id = socket(AF_INET, SOCK_STREAM, 0);

  /* SERVER SOCKET INFO */
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5100);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* CREATE SOCKET LISTENER */
	if (bind(socket_id, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Error: Bind().\n");
		exit(1);
	}

  /* LISTEN */
	if (listen(socket_id, 5) < 0) {
		perror("Error: Listen().\n");
		exit(1);
	}

	pthread_t thread[MAX_THREADS];
	thread_data_t *thread_data = (thread_data_t*)malloc(sizeof(thread_data_t));
  thread_data->next = NULL;
  thread_data->before = NULL;
	int count = 1;
	while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);

    /* ACCEPT AND ADD CLIENT TO LIST */
    int client_id = addclient(thread_data, accept(socket_id, (struct sockaddr *) &client_addr, &client_len));
    if (client_id < 0) {
			perror("Error: Accept().\n");
			exit(1);
		}
		printf("Client: %d\n", client_id);//thread_data[count].client_id);

    /* CREATE CLIENT THREAD */
		if (pthread_create(&thread[count], NULL, thread_client, getlastclient(thread_data))) {
			perror("Error: Pthread().\n");
			exit(1);
		} else {
			count++;
		}
	}  // END LOOP

	pthread_exit(NULL);
	int status = close(socket_id);
	return 0;
}

void *thread_send(void *arg) {
  pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;

  // Locking Mutex
	int rc = pthread_mutex_lock(&a_mutex);
	if (rc) { // an error has occurred
		perror("pthread_mutex_lock");
		pthread_exit(NULL);
	}
        thread_data_send *data = (thread_data_send *)arg;

        char buffer[MAX_DATA_SEND_SIZE];

        thread_data_t *aux = data->clients;
        while(aux->before != NULL) {
          aux = aux->before;
        }

        /* GET SENDER NICKNAME */
        char *sendernick = getclientnick(aux, data->sender);
        char tmp[MAX_DATA_SEND_SIZE];
        strcpy(tmp, "<");
        strcat(tmp, sendernick);
        strcat(tmp, "> ");

        aux = aux->next;
        while(aux->next != NULL) {
          if(aux->client_id != data->sender) {
            strcpy(buffer, tmp);
      			strcat(buffer, data->message);
            //send(aux->client_id, data->message, strlen(data->message), 0);
            send(aux->client_id, buffer, strlen(buffer), 0);
            //printf("SEND to %d: %s\n", aux->client_id, data->message);
            memset(buffer, 0, MAX_DATA_SEND_SIZE);
          }
          aux = aux->next;
        }
        if(aux->client_id != data->sender) {
          strcpy(buffer, tmp);
          strcat(buffer, data->message);
          send(aux->client_id, buffer, strlen(buffer), 0);
        }

        rc = pthread_mutex_destroy(&a_mutex);
}

void *thread_client(void *arg) {
        //thread_data_t *data = (thread_data_t *)arg;
	pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t got_request = PTHREAD_COND_INITIALIZER;

	// Locking Mutex
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
      /* GET NICKNAME */
      char *pch = strtok (buffer," ");
      pch = strtok (NULL, " ");
			strcpy(data->nickname, pch); //buffer + 6);
		}
                printf("MESSAGE %s: %s\n", data->nickname , buffer);

                pthread_t thread;
                thread_data_send send;
                send.sender = data->client_id;
                strcpy(send.message, buffer);
                send.clients = data;
                pthread_create(&thread, NULL, thread_send, &send);
                //send(data->client_id, buffer, 255, 0);

                memset(buffer, 0, MAX_DATA_SIZE);
        }
        printf("USER: %d disconected.\n", data->client_id); // *data->
	// Unlocking Mutex
        //rc = pthread_mutex_destroy(&a_mutex);
        removeclient(getfirstclient(data), data->client_id);
        close(data->client_id);
        pthread_exit(NULL);
	return 0;
}
