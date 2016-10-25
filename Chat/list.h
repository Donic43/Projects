#ifndef _list_h
#define _list_h

#define MAX_DATA_RECV_SIZE 268

struct messages {
  char message[256];
  int boolrecv;
  time_t time;
  struct messages *next;
  struct messages *before;
}messages;

/* ADD MESSAGE TO LIST *//* ADD MESSAGE TO LIST */
int add_message(struct messages *m, char *message, int b, time_t t);

/* PRINT ALL MESSAGES */
int printmessages(struct messages *m);

/* PRINT ALL MESSAGES IN WINDOW */
int windowprintmessages(WINDOW *win, struct messages *m);

/* PRINT ALL MESSAGES IN REVERSE IN WINDOW */
int windowprintmessagesreverse(WINDOW *win, struct messages *m);

#endif
