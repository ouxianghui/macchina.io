#ifndef _RKNN_LIST_H_
#define _RKNN_LIST_H_

#include <stdint.h>
#include "Postprocess.h"

// rknn list to draw boxs asynchronously
typedef struct DataNode {
  long timeval;
  detect_result_group_t detect_result_group;
  struct DataNode *next;
} DataNode;

typedef struct RknnList {
  int size;
  DataNode *top;
} RknnList;


#ifdef __cplusplus
extern "C" {
#endif

void createRknnList(RknnList **s);

void destoryRknnList(RknnList **s);

void rknnListPush(RknnList *s, long timeval, detect_result_group_t detect_result_group);

void rknnListPop(RknnList *s, long *timeval, detect_result_group_t *detect_result_group);

void rknnListDrop(RknnList *s);

int rknnListSize(RknnList *s);

long getCurrentTimeMsec();

#ifdef __cplusplus
}
#endif

#endif //_RKNN_LIST_H_
