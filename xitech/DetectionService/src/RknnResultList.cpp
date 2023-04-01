
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <set>
#include <vector>

#include "RknnResultList.h"


void createRknnList(RknnList **s) {
  if (*s != NULL) {
    return;
  }
  *s = (RknnList *)malloc(sizeof(RknnList));
  (*s)->top = NULL;
  (*s)->size = 0;
  printf("create RknnList success\n");
}

void destoryRknnList(RknnList **s) {
  DataNode *t = NULL;
  if (*s == NULL) {
    return;
  }
  while ((*s)->top) {
    t = (*s)->top;
    (*s)->top = t->next;
    free(t);
  }
  free(*s);
  *s = NULL;
}

void rknnListPush(RknnList *s, long timeval,
                    detect_result_group_t detect_result_group) {
  DataNode *t = NULL;
  t = (DataNode *)malloc(sizeof(DataNode));
  t->timeval = timeval;
  t->detect_result_group = detect_result_group;
  if (s->top == NULL) {
    s->top = t;
    t->next = NULL;
  } else {
    t->next = s->top;
    s->top = t;
  }
  s->size++;
}

void rknnListPop(RknnList *s, long *timeval,
                   detect_result_group_t *detect_result_group) {
  DataNode *t = NULL;
  if (s == NULL || s->top == NULL) {
    return;
  }
  t = s->top;
  *timeval = t->timeval;
  *detect_result_group = t->detect_result_group;
  s->top = t->next;
  free(t);
  s->size--;
}

void rknnListDrop(RknnList *s) {
  DataNode *t = NULL;
  if (s == NULL || s->top == NULL) {
    return;
  }
  t = s->top;
  s->top = t->next;
  free(t);
  s->size--;
}

int rknnListSize(RknnList *s) {
  if (s == NULL) {
    return -1;
  }
  return s->size;
}

long getCurrentTimeMsec() {
  long msec = 0;
  char str[20] = {0};
  struct timeval stuCurrentTime;

  gettimeofday(&stuCurrentTime, NULL);
  sprintf(str, "%ld%03ld", stuCurrentTime.tv_sec, (stuCurrentTime.tv_usec) / 1000);
  for (size_t i = 0; i < strlen(str); i++) {
    msec = msec * 10 + (str[i] - '0');
  }

  return msec;
}