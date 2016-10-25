#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>
#define MAX_DATA_SIZE 256

#include <curses.h>
#define WHITEONRED 1
#define WHITEONBLUE 2

#include "list.h"

typedef struct _thread_data_recv_ {
	int socket;
	struct messages *message;
	WINDOW *win;
} thread_data_recv;

void printwintitle(WINDOW *win, const char *title);

void paintscreen(WINDOW * win);

void *thread_recv(void *arg) {
	pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;
	thread_data_recv *data = (thread_data_recv *)arg;

	// Locking Mutex
	int rc = pthread_mutex_lock(&a_mutex);
	if (rc) { // an error has occurred
		perror("pthread_mutex_lock");
		pthread_exit(NULL);
	}
	char buffer[MAX_DATA_RECV_SIZE];
	int recv_size;
	rc = pthread_mutex_destroy(&a_mutex);
	while ((recv_size = recv(data->socket, buffer, MAX_DATA_RECV_SIZE-1, 0)) > 0) { // VER TAMANHO
		//buffer[strlen(buffer)-1] = '\0';
		time_t t;
		time(&t);
		add_message(data->message, buffer, 0, t);
		paintscreen(data->win);
		windowprintmessagesreverse(data->win, data->message);
		wrefresh(data->win);
		memset(buffer, 0, MAX_DATA_RECV_SIZE);
	}
}

void printwintitle(WINDOW *win, const char *title) {
	int x, maxx, maxy;
	getmaxyx(win, maxy, maxx);
	x = (maxx - strlen(title))/2;
	mvwaddstr(win, 0, x, title);
}

void paintscreen(WINDOW * win) {
	int y, x, maxy, maxx;
	getmaxyx(win, maxy, maxx);
	for(y=0; y < maxy; y++)
		for(x=0; x < maxx; x++)
			mvwaddch(win, y, x, ' ');
	}

int main(int argc, char *argv[]) {

	int socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id < 0) {
		perror("Error: Socket().\n");
		exit(1);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5100);

	//int server_len = sizeof(server_addr);
	if (connect(socket_id, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("Error: Connect().\n");
		exit(1);
	}

	// MESSAGES LIST
	struct messages *m = (struct messages*)malloc(sizeof(struct messages));
	m->next = NULL;
	m->before = NULL;

	/*FILE *fp = fopen("/tmp/test", "a+");

	fprintf(fp, "OLA OLA");
	fclose(fp); */

	// NCURSES
	int maxx, maxy;
	int mainwin;
	/*if ((mainwin = initscr()) == NULL) {
		perror("Error: Initializing ncurses.\n");
		exit(1);
	}*/
	initscr();
	getmaxyx(stdscr,maxy,maxx);

	/* START COLORS AND INIT PAIRS */
	start_color();
	init_pair(WHITEONRED, COLOR_WHITE, COLOR_RED);
	init_pair(WHITEONBLUE, COLOR_WHITE, COLOR_BLUE);

  /* DISPLAY SCREEN */
	touchwin(stdscr);
	wrefresh(stdscr);
	/* CREATE THE BASE WINDOW */
	WINDOW *base_win = newwin(maxy-4, (maxx*70)/100, 0, 0);
	/* CREATE THE BASE WINDOW TEXT*/
	WINDOW *base_win_text = newwin(maxy-6, (maxx*70)/100-2, 1, 1);
	/* CREATE THE USERS WINDOW */
	WINDOW *users_win = newwin(maxy-4, (maxx*30)/100, 0, maxx-((maxx*30)/100));
	/* CREATE THE MESSAGE WINDOW */
	WINDOW *message_win = newwin(4, maxx, maxy-4, 0);
	/* CREATE THE TEXT WINDOW */
	WINDOW *text_win = newwin(2, maxx-2, maxy-3, 1);
	// Linhas
	//mvvline(1, maxx-20, ACS_VLINE, maxy-4);
	//mvhline(maxy-4, 1, ACS_HLINE, maxx-2);

	/* THREAD RECV DATA */
	thread_data_recv data;
	data.socket = socket_id;
	data.message = m;
	data.win = base_win_text;

	pthread_t thread;
	/* CREATE CLIENT THREAD */
	if (pthread_create(&thread, NULL, thread_recv, &data)) {
		perror("Error: Pthread().\n");
		exit(1);
	}

	char message[256];
	while (1) {
		getmaxyx(stdscr, maxy, maxx);

		/* DISPLAY SCREEN */
		wrefresh(stdscr);

		/* REFRESH BASE WINDOW */
		base_win = newwin(maxy-4, (maxx*70)/100, 0, 0);
		box(base_win, ACS_VLINE, ACS_HLINE);
		printwintitle(base_win, " CHAT ");
		wrefresh(base_win);

		/* REFRESH BASE WINDOW TEXT*/
		base_win_text = newwin(maxy-6, ((maxx*70)/100)-2, 1, 1);
		wrefresh(base_win_text);
		//scrollok(base_win_text, true);

		/* REFRESH THE USERS WINDOW */
		//users_win = newwin(maxy-4, 30, 0, maxx-30);
		WINDOW *users_win = newwin(maxy-4, (maxx*30)/100, 0, maxx-((maxx*30)/100));
		wattrset(users_win, COLOR_PAIR(WHITEONBLUE) | WA_BOLD);
		paintscreen(users_win);
		box(users_win, 0, 0);
		printwintitle(users_win, " USERS ");
		wrefresh(users_win);

		/* REFRESH THE MESSAGE WINDOW */
		message_win = newwin(4, maxx, maxy-4, 0);
		wattrset(message_win, COLOR_PAIR(WHITEONRED) | WA_BOLD);
		paintscreen(message_win);
		box(message_win, 0, 0);
		printwintitle(message_win, " MESSAGE ");
		mvwaddch(message_win, 1, 1, ' ');
		wrefresh(message_win);

		/* REFRESH THE TEXT WINDOW */
		text_win = newwin(2, maxx-2, maxy-3, 1);
		wattrset(text_win, COLOR_PAIR(WHITEONRED) | WA_BOLD);
		paintscreen(text_win);
		wrefresh(text_win);

		//mvwaddstr(base_win, 1, 1, message);
		//fprintf(fp, "%s", message);
		//fwrite(message, sizeof(message[0]), sizeof(message)/sizeof(message[0]), fp);
		windowprintmessagesreverse(base_win_text, m);
		wrefresh(base_win_text);
		//mvwscanw(text_win, 0, 0, "%s", message);
		mvwgetnstr(text_win, 0, 0, message, 256);
		/*endwin();
		int count = strlen(message);
		printf("LEN: %d", count);
		return 0;*/
		if(strlen(message) != 0) {
			if (send(socket_id, message, strlen(message), 0) != strlen(message)) {
				perror("Error: Send().\n");
				exit(1);
			}
			char *tmp = strdup(message);
			strcpy(message, "<EU> ");
			strcat(message, tmp);
			time_t t;
			time(&t);
			add_message(m, message, 1, t);
		}
		//paintscreen(text_win);
	}

	//fclose(fp);
	endwin();
	close(socket_id);
	exit(0);
}
