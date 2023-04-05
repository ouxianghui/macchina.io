// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <set>
#include <vector>
#include "Postprocess.h"

static char* labels[OBJ_CLASS_NUM_MAX];

int g_nObjClassNum = 0;
int g_nPropBoxSize = 0;
char g_szLabelFilePath[128] = {0x0};
int  g_nModelSize = 640;
int g_nDetType = DET_MODEL_FOR_80CLASS;

void setModelSize(int nModelSize)
{
  g_nModelSize = nModelSize;
}

void setObjClassNum(int nObjClassNum)
{
  g_nObjClassNum = nObjClassNum;
  g_nPropBoxSize = 5+g_nObjClassNum;
  printf("model class num:%d\n", g_nObjClassNum);
}

void setDetType(int nDetType)
{
  g_nDetType = nDetType;
}

void setLabelTxtPath(char* szPath)
{
  sprintf(g_szLabelFilePath, "%s", szPath);
  printf("label file path:%s\n", g_szLabelFilePath);
}
#define SUB   0
#define PERSON  1

// for 80 classes
//const int anchor0[6] = {10, 13, 16, 30, 33, 23};
//const int anchor1[6] = {30, 61, 62, 45, 59, 119};
//const int anchor2[6] = {116, 90, 156, 198, 373, 326};

#if PERSON
// for 5 person classes
const int anchor0[6] = {4, 4, 8, 21, 18, 24};
const int anchor1[6] = {38, 42, 26, 71, 46, 120};
const int anchor2[6] = {110, 133, 114, 282, 301, 294};
//[[3.970703125, 4.43359375], [7.61328125, 20.90625], [17.984375, 23.515625],
//[38.25, 41.625], [26.3125, 70.9375], [45.5, 120.3125], 
//[109.5, 133.375], [113.8125, 281.5], [300.75, 293.75]]
#endif
// for 1280model 43 substation classes
const float anchor0_person_640[6] = {3.525390625, 3.953125, 8.65625, 20.890625, 26.265625, 31.28125};
const float anchor1_person_640[6] = {33.34375, 96.5625, 59.34375, 68.125, 68.5, 184.5};
const float anchor2_person_640[6] = {124.75, 161.875, 153.375, 384.5, 301.5, 235.25};
//const float anchor0_person_640[6] = {3.970703125, 4.43359375, 7.61328125, 20.90625, 17.984375, 23.515625};
//const float anchor1_person_640[6] = {38.25, 41.625, 26.3125, 70.9375, 45.5, 120.3125};
//const float anchor2_person_640[6] = {109.5, 133.375, 113.8125, 281.5, 300.75, 293.75};

const float anchor0_person_1280[6] = {6, 6, 15, 15, 31, 32};
const float anchor1_person_1280[6] = {52, 61, 141, 53, 90, 114};
const float anchor2_person_1280[6] = {175, 146, 181, 238, 403, 180};
const float anchor3_person_1280[6] = {345, 346, 616, 509, 1043, 650};



#if SUB
// for 43 substation classes
const int anchor0[6] = {4, 4, 12, 14, 22, 23};
const int anchor1[6] = {69, 26, 40, 49, 71, 77};
const int anchor2[6] = {117, 90, 157, 162, 267, 209};
//[[4.26953125, 4.22265625], [12.125, 13.7265625], [22.265625, 22.859375], 
//[69.125, 25.65625], [40.0625, 48.9375], [70.5625, 77.0],
//[116.9375, 89.5625], [157.375, 162.25], [267.0, 208.875]]
#endif
const float anchor0_sub_640[6] = {4.26953125, 4.22265625, 12.125, 13.7265625, 22.265625, 22.859375};
const float anchor1_sub_640[6] = {69.125, 25.65625, 40.0625, 48.9375, 70.5625, 77.0};
const float anchor2_sub_640[6] = {116.9375, 89.5625, 157.375, 162.25, 267.0, 208.875};
//[[4.26953125, 4.22265625], [12.125, 13.7265625], [22.265625, 22.859375], 
//[69.125, 25.65625], [40.0625, 48.9375], [70.5625, 77.0],
//[116.9375, 89.5625], [157.375, 162.25], [267.0, 208.875]]
// for 1280model 43 substation classes
const float anchor0_sub_1280[6] = {5.59765625, 5.6953125, 14.828125, 15.21875, 31.28125, 32.375};
const float anchor1_sub_1280[6] = {51.96875, 60.8125, 141.25, 52.875, 89.5625, 114.0};
const float anchor2_sub_1280[6] = {175.25, 146.375, 181.0, 237.625, 403.0, 180.375};
const float anchor3_sub_1280[6] = {344.5, 345.5, 616.0, 509.25, 1043.0, 649.5};
//[[5.59765625, 5.6953125], [14.828125, 15.21875], [31.28125, 32.375],
//[51.96875, 60.8125], [141.25, 52.875], [89.5625, 114.0],
//[175.25, 146.375], [181.0, 237.625], [403.0, 180.375],
//[344.5, 345.5], [616.0, 509.25], [1043.0, 649.5]]



inline static int clamp(float val, int min, int max) { return val > min ? (val < max ? val : max) : min; }

char* readLine(FILE* fp, char* buffer, int* len)
{
  int    ch;
  int    i        = 0;
  size_t buff_len = 0;

  buffer = (char*)malloc(buff_len + 1);
  if (!buffer) {
    return NULL; // Out of memory
  }

  while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
    buff_len++;
    void* tmp = realloc(buffer, buff_len + 1);
    if (tmp == NULL) {
      free(buffer);
      return NULL; // Out of memory
    }
    buffer = (char*)tmp;

    buffer[i] = (char)ch;
    i++;
  }
  buffer[i] = '\0';

  *len = buff_len;

  // Detect end
  if (ch == EOF && (i == 0 || ferror(fp))) {
    free(buffer);
    return NULL;
  }
  return buffer;
}

int readLines(const char* fileName, char* lines[], int max_line)
{
  FILE* file = fopen(fileName, "r");
  char* s;
  int   i = 0;
  int   n = 0;
  while ((s = readLine(file, s, &n)) != NULL) {
    lines[i++] = s;
    if (i >= max_line) {
      break;
    }
  }
  return i;
}

int loadLabelName(const char* locationFilename, char* label[])
{
  printf("loadLabelName %s\n", locationFilename);
  readLines(locationFilename, label, g_nObjClassNum);
  return 0;
}

static float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0, float xmin1, float ymin1, float xmax1,
                              float ymax1)
{
  float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1) + 1.0);
  float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1) + 1.0);
  float i = w * h;
  float u = (xmax0 - xmin0 + 1.0) * (ymax0 - ymin0 + 1.0) + (xmax1 - xmin1 + 1.0) * (ymax1 - ymin1 + 1.0) - i;
  return u <= 0.f ? 0.f : (i / u);
}

static int nms(int validCount, std::vector<float>& outputLocations, std::vector<int> classIds, std::vector<int>& order,
               int filterId, float threshold)
{
  for (int i = 0; i < validCount; ++i) {
    if (order[i] == -1 || classIds[i] != filterId) {
      continue;
    }
    int n = order[i];
    for (int j = i + 1; j < validCount; ++j) {
      int m = order[j];
      if (m == -1 || classIds[i] != filterId) {
        continue;
      }
      float xmin0 = outputLocations[n * 4 + 0];
      float ymin0 = outputLocations[n * 4 + 1];
      float xmax0 = outputLocations[n * 4 + 0] + outputLocations[n * 4 + 2];
      float ymax0 = outputLocations[n * 4 + 1] + outputLocations[n * 4 + 3];

      float xmin1 = outputLocations[m * 4 + 0];
      float ymin1 = outputLocations[m * 4 + 1];
      float xmax1 = outputLocations[m * 4 + 0] + outputLocations[m * 4 + 2];
      float ymax1 = outputLocations[m * 4 + 1] + outputLocations[m * 4 + 3];

      float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

      if (iou > threshold) {
        order[j] = -1;
      }
    }
  }
  return 0;
}

static int quickSortIndiceInverse(std::vector<float>& input, int left, int right, std::vector<int>& indices)
{
  float key;
  int   key_index;
  int   low  = left;
  int   high = right;
  if (left < right) {
    key_index = indices[left];
    key       = input[left];
    while (low < high) {
      while (low < high && input[high] <= key) {
        high--;
      }
      input[low]   = input[high];
      indices[low] = indices[high];
      while (low < high && input[low] >= key) {
        low++;
      }
      input[high]   = input[low];
      indices[high] = indices[low];
    }
    input[low]   = key;
    indices[low] = key_index;
    quickSortIndiceInverse(input, left, low - 1, indices);
    quickSortIndiceInverse(input, low + 1, right, indices);
  }
  return low;
}

static float sigmoid(float x) { return 1.0 / (1.0 + expf(-x)); }

static float unsigmoid(float y) { return -1.0 * logf((1.0 / y) - 1.0); }

inline static int32_t __clip(float val, float min, float max)
{
  float f = val <= min ? min : (val >= max ? max : val);
  return f;
}

static uint8_t qntF32ToAffine(float f32, uint32_t zp, float scale)
{
  float   dst_val = (f32 / scale) + zp;
  uint8_t res     = (uint8_t)__clip(dst_val, 0, 255);
  return res;
}

static float deqntAffineToF32(uint8_t qnt, uint32_t zp, float scale) { return ((float)qnt - (float)zp) * scale; }

static int process(uint8_t* input, float* anchor, int grid_h, int grid_w, int height, int width, int stride,
                   std::vector<float>& boxes, std::vector<float>& objProbs, std::vector<int>& classId, float threshold,
                   uint32_t zp, float scale)
{
  int     validCount = 0;
  int     grid_len   = grid_h * grid_w;
  float   thres      = unsigmoid(threshold);
  uint8_t thres_u8   = qntF32ToAffine(thres, zp, scale);
  int anchor_num = 3;
  for (int a = 0; a < anchor_num; a++) {
    for (int i = 0; i < grid_h; i++) {
      for (int j = 0; j < grid_w; j++) {
        uint8_t box_confidence = input[(g_nPropBoxSize * a + 4) * grid_len + i * grid_w + j];
        //printf("process box_conf:%d, thres:%d\n", box_confidence, thres_u8);
        if (box_confidence >= thres_u8) {
          int      offset = (g_nPropBoxSize * a) * grid_len + i * grid_w + j;
          uint8_t* in_ptr = input + offset;
          float    box_x  = sigmoid(deqntAffineToF32(*in_ptr, zp, scale)) * 2.0 - 0.5;
          float    box_y  = sigmoid(deqntAffineToF32(in_ptr[grid_len], zp, scale)) * 2.0 - 0.5;
          float    box_w  = sigmoid(deqntAffineToF32(in_ptr[2 * grid_len], zp, scale)) * 2.0;
          float    box_h  = sigmoid(deqntAffineToF32(in_ptr[3 * grid_len], zp, scale)) * 2.0;
          box_x           = (box_x + j) * (float)stride;
          box_y           = (box_y + i) * (float)stride;
          box_w           = box_w * box_w * (float)anchor[a * 2];
          box_h           = box_h * box_h * (float)anchor[a * 2 + 1];
          box_x -= (box_w / 2.0);
          box_y -= (box_h / 2.0);
          boxes.push_back(box_x);
          boxes.push_back(box_y);
          boxes.push_back(box_w);
          boxes.push_back(box_h);

          uint8_t maxClassProbs = in_ptr[5 * grid_len];
          int     maxClassId    = 0;
          for (int k = 1; k < g_nObjClassNum; ++k) {
            uint8_t prob = in_ptr[(5 + k) * grid_len];
            if (prob > maxClassProbs) {
              maxClassId    = k;
              maxClassProbs = prob;
            }
          }
          objProbs.push_back(sigmoid(deqntAffineToF32(maxClassProbs, zp, scale)));
          classId.push_back(maxClassId);
          validCount++;
        }
      }
    }
  }
  return validCount;
}

int postprocess(uint8_t* input0, uint8_t* input1, uint8_t* input2, uint8_t *input3, int model_in_h, int model_in_w,
                 float conf_threshold, float nms_threshold, float scale_w, float scale_h,
                 std::vector<uint32_t>& qnt_zps, std::vector<float>& qnt_scales, detect_result_group_t* group)
{
  static int init = -1;
  if (init == -1) {
    int ret = 0;
    ret     = loadLabelName(g_szLabelFilePath, labels);
    if (ret < 0) {
      return -1;
    }

    init = 0;
  }
  memset(group, 0, sizeof(detect_result_group_t));

  std::vector<float> filterBoxes;
  std::vector<float> objProbs;
  std::vector<int>   classId;

  // stride 8
  int stride0     = 8;
  int grid_h0     = model_in_h / stride0;
  int grid_w0     = model_in_w / stride0;
  int validCount0 = 0;

  // stride 16
  int stride1     = 16;
  int grid_h1     = model_in_h / stride1;
  int grid_w1     = model_in_w / stride1;
  int validCount1 = 0;

  // stride 32
  int stride2     = 32;
  int grid_h2     = model_in_h / stride2;
  int grid_w2     = model_in_w / stride2;
  int validCount2 = 0;
  
  // stride 64
  int stride3     = 64;
  int grid_h3     = model_in_h / stride3;
  int grid_w3     = model_in_w / stride3;
  int validCount3 = 0;

  if (g_nDetType == DET_MODEL_FOR_PERSON && g_nModelSize == 640){
    validCount0 = process(input0, (float*)anchor0_person_640, grid_h0, grid_w0, model_in_h, model_in_w, stride0, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[0], qnt_scales[0]);
    validCount1 = process(input1, (float*)anchor1_person_640, grid_h1, grid_w1, model_in_h, model_in_w, stride1, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[1], qnt_scales[1]);
    validCount2 = process(input2, (float*)anchor2_person_640, grid_h2, grid_w2, model_in_h, model_in_w, stride2, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[2], qnt_scales[2]);
    //printf("det for person 640, %d:%d:%d:%d\n", validCount0, validCount1, validCount2, validCount3);
  }else if (g_nDetType == DET_MODEL_FOR_PERSON && g_nModelSize == 1280){
    validCount0 = process(input0, (float*)anchor0_person_1280, grid_h0, grid_w0, model_in_h, model_in_w, stride0, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[0], qnt_scales[0]);
    validCount1 = process(input1, (float*)anchor1_person_1280, grid_h1, grid_w1, model_in_h, model_in_w, stride1, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[1], qnt_scales[1]);
    validCount2 = process(input2, (float*)anchor2_person_1280, grid_h2, grid_w2, model_in_h, model_in_w, stride2, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[2], qnt_scales[2]);
    validCount3 = process(input3, (float*)anchor3_person_1280, grid_h3, grid_w3, model_in_h, model_in_w, stride3, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[3], qnt_scales[3]); 
    printf("det for person 1280, %d:%d:%d:%d\n", validCount0, validCount1, validCount2, validCount3);
  }else if (g_nDetType == DET_MODEL_FOR_SUBSTATION && g_nModelSize == 640){
    validCount0 = process(input0, (float*)anchor0_sub_640, grid_h0, grid_w0, model_in_h, model_in_w, stride0, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[0], qnt_scales[0]);
    validCount1 = process(input1, (float*)anchor1_sub_640, grid_h1, grid_w1, model_in_h, model_in_w, stride1, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[1], qnt_scales[1]);
    validCount2 = process(input2, (float*)anchor2_sub_640, grid_h2, grid_w2, model_in_h, model_in_w, stride2, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[2], qnt_scales[2]);
    printf("det for sub 640, %d:%d:%d:%d\n", validCount0, validCount1, validCount2, validCount3);
  }else if (g_nDetType == DET_MODEL_FOR_SUBSTATION && g_nModelSize == 1280){
    validCount0 = process(input0, (float*)anchor0_sub_1280, grid_h0, grid_w0, model_in_h, model_in_w, stride0, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[0], qnt_scales[0]);
    validCount1 = process(input1, (float*)anchor1_sub_1280, grid_h1, grid_w1, model_in_h, model_in_w, stride1, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[1], qnt_scales[1]);
    validCount2 = process(input2, (float*)anchor2_sub_1280, grid_h2, grid_w2, model_in_h, model_in_w, stride2, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[2], qnt_scales[2]);
    validCount3 = process(input3, (float*)anchor3_sub_1280, grid_h3, grid_w3, model_in_h, model_in_w, stride3, filterBoxes, objProbs,
                          classId, conf_threshold, qnt_zps[3], qnt_scales[3]); 
    printf("det for sub 1280, %d:%d:%d:%d\n", validCount0, validCount1, validCount2, validCount3);
  }

  int validCount = validCount0 + validCount1 + validCount2 + validCount3;
  // no object detect
  if (validCount <= 0) {
    return 0;
  }

  std::vector<int> indexArray;
  for (int i = 0; i < validCount; ++i) {
    indexArray.push_back(i);
  }

  quickSortIndiceInverse(objProbs, 0, validCount - 1, indexArray);

  std::set<int> class_set(std::begin(classId), std::end(classId));

  for (auto c : class_set) {
    nms(validCount, filterBoxes, classId, indexArray, c, nms_threshold);
  }

  int last_count = 0;
  group->count   = 0;
  /* box valid detect target */
  for (int i = 0; i < validCount; ++i) {
    if (indexArray[i] == -1 || last_count >= OBJ_NUMB_MAX_SIZE) {
      continue;
    }
    int n = indexArray[i];

    float x1       = filterBoxes[n * 4 + 0];
    float y1       = filterBoxes[n * 4 + 1];
    float x2       = x1 + filterBoxes[n * 4 + 2];
    float y2       = y1 + filterBoxes[n * 4 + 3];
    int   id       = classId[n];
    float obj_conf = objProbs[i];

    group->results[last_count].box.left   = (int)(clamp(x1, 0, model_in_w) / scale_w);
    group->results[last_count].box.top    = (int)(clamp(y1, 0, model_in_h) / scale_h);
    group->results[last_count].box.right  = (int)(clamp(x2, 0, model_in_w) / scale_w);
    group->results[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / scale_h);
    group->results[last_count].prop       = obj_conf;
    char* label                           = labels[id];
    strncpy(group->results[last_count].name, label, OBJ_NAME_MAX_SIZE);

    // printf("result %2d: (%4d, %4d, %4d, %4d), %s\n", i, group->results[last_count].box.left,
    // group->results[last_count].box.top,
    //        group->results[last_count].box.right, group->results[last_count].box.bottom, label);
    last_count++;
  }
  group->count = last_count;

  return 0;
}
