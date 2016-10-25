#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <time.h>
#include "list.h"

#define GREENONBLACK 3
#define REDONBLACK 4

/* ADD MESSAGE TO LIST */
int add_message(struct messages *m, char *message, int b, time_t t) {
  struct messages *aux = m;
  struct messages *novo = malloc(sizeof(struct messages));
  strcpy(novo->message, message);
  novo->boolrecv = b;
  novo->time = t;
  novo->next = NULL;

  /* LOOP ALL MESSAGES */
  while(aux->next != NULL) {
    aux = aux->next;
  }
  /* ADD NEW MESSAGE IN THE LAST POSITION */
  aux->next = novo;
  novo->before = aux;
  return 0;
}

/* PRINT ALL MESSAGES */
int printmessages(struct messages *m) {
  struct messages *aux = m;

  if(aux->next == NULL)
    return 1;

  aux = aux->next; // SET LIST ON FIRST POSITION
  /* LOOP ALL MESSAGES */
  while(aux->next != NULL) {
    printf("MESSAGE: %s\n", aux->message);
    aux = aux->next;  // NEXT POSTION
  }
  printf("MESSAGE: %s\n", aux->message); // PRINT LAST POSITION
  return 0;
}

int windowprintmessages(WINDOW *win, struct messages *m) {
  struct messages *aux = m;

  if(aux->next == NULL)
    return 1;

  aux = aux->next; // SET LIST ON FIRST POSITION
  /* LOOP ALL MESSAGES */
  int count = 0;
  while(aux->next != NULL) {
    mvwaddstr(win, count, 1, aux->message);
    aux = aux->next;  // NEXT POSTION
    count++;
  }
  mvwaddstr(win, count, 1, aux->message); // PRINT LAST POSITION
  return 0;
}

int windowprintmessagesreverse(WINDOW *win, struct messages *m) {
  struct messages *aux = m;

  if(aux->next == NULL)
    return 1;

  init_pair(GREENONBLACK, COLOR_GREEN, COLOR_BLACK);
  init_pair(REDONBLACK, COLOR_RED, COLOR_BLACK);

  aux = aux->next; // SET LIST ON FIRST POSITION
  /* LOOP ALL MESSAGES */
  while(aux->next != NULL) {
    aux = aux->next;  // NEXT POSTION
  }

  int maxy, maxx;
  getmaxyx(win, maxy, maxx);
  int count = maxy - 1;
  while (aux->before != NULL) {
    if (count == 0)
      break;
    //char *pch = strtok (aux->message," ");
    if(aux->boolrecv == 0) {
      wattrset(win, COLOR_PAIR(REDONBLACK) | WA_BOLD);
      mvwprintw(win, count, 1, "[%2d:%2d] ", localtime(&aux->time)->tm_hour, localtime(&aux->time)->tm_min);
      mvwaddstr(win, count, 9, aux->message);
    } else {
      wattrset(win, COLOR_PAIR(GREENONBLACK) | WA_BOLD);
      mvwprintw(win, count, 1, "[%2d:%2d] ", localtime(&aux->time)->tm_hour, localtime(&aux->time)->tm_min);
      mvwaddstr(win, count, 9, aux->message);
    }
    aux = aux->before;
    count--;
  }

  return 0;
}
