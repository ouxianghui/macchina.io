// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _FFRTSP_HH
#define _FFRTSP_HH

enum RTSP_CODEC_TYPE {
  RTSP_CODEC_H264 = 1,  
  RTSP_CODEC_H265           
};

struct RTSP_PUSH_INFO {
	FILE *fp;
	int idex;
	int port;
	int type;
};

#define MAXFFRTSPChn 16

struct FFRTSPGet_URL_INFO {
   char * url;
};

struct FFRTSPGet {
   int count;
   struct  FFRTSPGet_URL_INFO ffrtsp_get_info[MAXFFRTSPChn];
   int (*callback)(u_int8_t*,unsigned,bool*,int);
};

int ffrtspGet(struct FFRTSPGet info);
int ffrtsph264Push(struct RTSP_PUSH_INFO * info);
int ffrtsph265Push(struct RTSP_PUSH_INFO * info);
int ffrtspPush(struct RTSP_PUSH_INFO *info);
#endif
