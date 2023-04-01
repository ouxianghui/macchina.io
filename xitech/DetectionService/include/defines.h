#ifndef _MAIN_DEFINE_H_
#define _MAIN_DEFINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <mutex>
#include <string>

#define debug 0

// 原始取到的RTSP视频流的画面大小、帧率
#define INPUT_VIDEO_IMG_WIDTH	1920
#define INPUT_VIDEO_IMG_HEIGHT	1080
#define INPUT_VIDEO_FPS			25

// 显示时保留多久最后的显示框，单位为毫秒
#define SHOW_SAVE_LAST_RESULT_TIME	2500

#define VDEC_CHN_ID		4
#define RGA_VENC_CHN_ID	1
#define RGA_NN_CHN_ID	2
#define RGA_DRAW_CHN_ID	3
#define VENC_CHN_ID		2

#define USE_OPENCV_DRAW 1
#define USE_OPENCV_SAVE_DET_JPG 0

#define CELING_2_POWER(x,a)  (((x) + ((a)-1)) & (~((a) - 1))) 

// #include <opencv2/core/types_c.h>
// #include <opencv2/core/core_c.h>
// #include <opencv2/core/core.hpp>
// #include <opencv2/opencv.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/freetype.hpp>


#endif //_MAIN_DEFINE_H_
