#include "RknnFrameProcessor.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include "defines.h"
#include "rknn_api.h"
#include "RknnResultList.h"


#define MAX_RKNN_LIST_NUM 10

namespace {
	inline const char* getTypeString(rknn_tensor_type type)
	{
		switch (type) {
		case RKNN_TENSOR_FLOAT32:
			return "FP32";
		case RKNN_TENSOR_FLOAT16:
			return "FP16";
		case RKNN_TENSOR_INT8:
			return "INT8";
		case RKNN_TENSOR_UINT8:
			return "UINT8";
		case RKNN_TENSOR_INT16:
			return "INT16";
		default:
			return "UNKNOW";
		}
	}

	inline const char* getQntTypeString(rknn_tensor_qnt_type type)
	{
		switch (type) {
		case RKNN_TENSOR_QNT_NONE:
			return "NONE";
		case RKNN_TENSOR_QNT_DFP:
			return "DFP";
		case RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC:
			return "AFFINE";
		default:
			return "UNKNOW";
		}
	}

	inline const char* getFormatString(rknn_tensor_format fmt)
	{
		switch (fmt) {
		case RKNN_TENSOR_NCHW:
			return "NCHW";
		case RKNN_TENSOR_NHWC:
			return "NHWC";
		default:
			return "UNKNOW";
		}
	}

	static void dumpTensorAttr(rknn_tensor_attr* attr)
	{
		printf("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
				"zp=%d, scale=%f\n",
				attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0],
				attr->n_elems, attr->size, getFormatString(attr->fmt), getTypeString(attr->type),
				getQntTypeString(attr->qnt_type), attr->zp, attr->scale);
	}

	static unsigned char* loadData(FILE* fp, size_t ofst, size_t sz)
	{
		unsigned char* data;
		int ret;

		data = NULL;

		if (NULL == fp) {
			return NULL;
		}

		ret = fseek(fp, ofst, SEEK_SET);
		if (ret != 0) {
			printf("blob seek failure.\n");
			return NULL;
		}

		data = (unsigned char*)malloc(sz);
		if (data == NULL) {
			printf("buffer malloc failure.\n");
			return NULL;
		}
		ret = fread(data, 1, sz, fp);
		return data;
	}

	static unsigned char* loadModel(const char* filename, int* model_size)
	{
		FILE* fp;
		unsigned char* data;

		fp = fopen(filename, "rb");
		if (NULL == fp) {
			printf("Open file %s failed.\n", filename);
			return NULL;
		}

		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);

		data = loadData(fp, 0, size);

		fclose(fp);

		*model_size = size;

		return data;
	}
}

namespace xi {

	RknnFrameProcessor::RknnFrameProcessor(const std::string& modelFilePath, int32_t nmsThresh, int32_t boxThresh)
	: _modelFilePath(modelFilePath)
	, _nmsThresh(nmsThresh)
	, _boxThresh(boxThresh)
	, _activity(this, &RknnFrameProcessor::runActivity)
	{

	}

	RknnFrameProcessor::~RknnFrameProcessor()
	{

	}

	void RknnFrameProcessor::init() 
	{
		initModel();
	}

	void RknnFrameProcessor::destroy() 
	{
		// TODO:
		destoryRknnList(&_rknnResultList);
	}

	// 初始化模型
	bool RknnFrameProcessor::initModel()
	{
		if (_modelFilePath.empty()) {
			std::cout << "model file path is empty" << std::endl;
			return false;
		}
		int ret = 0;
		unsigned char* model;
		// 加载模型文件
		printf("Loading model ...\n");
		int model_len = 0;
		model = loadModel(_modelFilePath.c_str(), &model_len);
		//model = loadModel("./model.rknn", &model_len);
		printf("model file loaded ok, model file size:%d, model file:%s\n\n", model_len, _modelFilePath.c_str());
		// 模型初始化
		printf("rknn_init ...\n");
		ret = rknn_init(&_rknnData.ctx, model, model_len, 0);
		if (ret < 0) {
			printf("rknn_init fail! ret=%d\n", ret);
			return false;
		}
		printf("rknn_init ok\n\n");
		// 查询sdk和驱动版本信息
		printf("query sdk/driver version...");
		rknn_sdk_version version;
		ret = rknn_query(_rknnData.ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
		if (ret < 0) {
			printf("rknn_query error ret=%d\n", ret);
			return false;
		}
		printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);
		// 获取模型输入输出信息
		printf("query  Model Input Output Info...");
		ret = rknn_query(_rknnData.ctx, RKNN_QUERY_IN_OUT_NUM, &_rknnData.io_num, sizeof(_rknnData.io_num));
		if (ret != RKNN_SUCC) {
			printf("rknn_query fail! ret=%d\n", ret);
			return false;
		}
		printf("model input num: %d, output num: %d\n", _rknnData.io_num.n_input, _rknnData.io_num.n_output);

		printf("input tensors:\n");
		rknn_tensor_attr input_attrs[_rknnData.io_num.n_input];
		memset(input_attrs, 0, sizeof(input_attrs));

		for (unsigned int i = 0; i < _rknnData.io_num.n_input; i++) {
			input_attrs[i].index = i;
			ret = rknn_query(_rknnData.ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
			if (ret != RKNN_SUCC) {
			printf("rknn_query fail! ret=%d\n", ret);
			return false;
			}
			dumpTensorAttr(&(input_attrs[i]));
		}

		printf("output tensors:\n");
		rknn_tensor_attr output_attrs[_rknnData.io_num.n_output];
		memset(output_attrs, 0, sizeof(output_attrs));
		for (unsigned int i = 0; i < _rknnData.io_num.n_output; i++) {
			output_attrs[i].index = i;
			ret = rknn_query(_rknnData.ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
			if (ret != RKNN_SUCC) {
			printf("rknn_query fail! ret=%d\n", ret);
			return false;
			}
			dumpTensorAttr(&(output_attrs[i]));
		}

		// 模型输入数据格式
		int channel = 3;
		int width   = 0;
		int height  = 0;
		if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
			printf("model is NCHW input fmt\n");
			width  = input_attrs[0].dims[0];
			height = input_attrs[0].dims[1];
		} else {
			printf("model is NHWC input fmt\n");
			width  = input_attrs[0].dims[1];
			height = input_attrs[0].dims[2];
		}
		
		printf("model input height=%d, width=%d, channel=%d\n", height, width, channel);

		for (int i = 0; i < _rknnData.io_num.n_output; ++i) {
			_outScales.push_back(output_attrs[i].scale);
			printf("output scale%d:%.2f\n", i, output_attrs[i].scale);
			_outZps.push_back(output_attrs[i].zp);
			printf("output zp%d:%d\n", i, output_attrs[i].zp);
		}

		// 创建存储结果表
		createRknnList(&_rknnResultList);

		return true;
	}

	void RknnFrameProcessor::addListener(std::shared_ptr<IProcessorListener> listener) 
	{
		addObserver<IProcessorListener>(_observers, listener);
	}

	void RknnFrameProcessor::removeListener(std::shared_ptr<IProcessorListener> listener) 
	{
		removeObserver<IProcessorListener>(_observers, listener);
	}

	void RknnFrameProcessor::start() 
	{
		_activity.start();
	}

	void RknnFrameProcessor::stop() 
	{
		_activity.stop();
		_activity.wait();
	}

	// TODO:
	unsigned char g_InferBuffer[1024*1024*2] = {0x0};
	int g_nInferBufferLen = 0;
	std::mutex g_mtInferBuf;
	std::mutex g_mtxRknnList;

	void RknnFrameProcessor::runActivity()
	{
		int ret = 0;
		rknn_input inputs[1];
		while (!_activity.isStopped()) {
			try {
				unsigned char tmpInferBuffer[1024*1024*2] = {0x0};
				g_mtInferBuf.lock();
				memcpy(tmpInferBuffer, g_InferBuffer, g_nInferBufferLen);
				if (g_nInferBufferLen <= 0){
					//usleep(100*1000);
					Poco::Thread::sleep(100);
					g_mtInferBuf.unlock();
					continue;
				}
				g_nInferBufferLen = 0;
				g_mtInferBuf.unlock();

				//usleep(100*1000);
				Poco::Thread::sleep(100);

				// Set Input Data
				memset(inputs, 0, sizeof(inputs));
				inputs[0].index = 0;
				inputs[0].type = RKNN_TENSOR_UINT8;
				inputs[0].size = _modelInputSize * _modelInputSize * 3;
				inputs[0].fmt = RKNN_TENSOR_NHWC;
				inputs[0].buf = tmpInferBuffer;

				ret = rknn_inputs_set(_rknnData.ctx, _rknnData.io_num.n_input, inputs);
				if (ret < 0) {
					printf("rknn_input_set fail! ret=%d\n", ret);
					continue;
				}

				rknn_output outputs[_rknnData.io_num.n_output];
				memset(outputs, 0, sizeof(outputs));
				for (int i = 0; i < _rknnData.io_num.n_output; i++) {
					outputs[i].want_float = 0;
				}

				// Run
				printf("rknn_run detection start===========================================================\n");
				if (debug)
					printf("rknn_run\n");
				ret = rknn_run(_rknnData.ctx, NULL);
				if (ret < 0) {
					printf("rknn_run fail! ret=%d\n", ret);
					continue;
				}
				printf("rknn_run detection stop===========================================================\n");

				// Get Output
				ret = rknn_outputs_get(_rknnData.ctx, _rknnData.io_num.n_output, outputs, NULL);
				if (ret < 0) {
					printf("rknn_outputs_get fail! ret=%d\n", ret);
					continue;
				}
				//printf("output size0:%d,size1:%d,size2:%d,size3:%d\n", outputs[0].size, outputs[1].size, outputs[2].size, outputs[3].size);
				//printf("output index0:%d,index1:%d,index2:%d,index3:%d\n", outputs[0].index, outputs[1].index, outputs[2].index, outputs[3].index);

				float scale_w = (float)_modelInputSize / INPUT_VIDEO_IMG_WIDTH;
				float scale_h = (float)_modelInputSize / INPUT_VIDEO_IMG_HEIGHT;

				// Post Process
				const float nms_threshold = ((float)_nmsThresh)/100.0;
				const float box_conf_threshold = ((float)_boxThresh)/100.0;
				detect_result_group_t detect_result_group;
				if (_modelInputSize == 1280) {
					postprocess((uint8_t*)outputs[0].buf, (uint8_t*)outputs[1].buf,
								(uint8_t*)outputs[2].buf, (uint8_t*)outputs[3].buf,
								_modelInputSize, _modelInputSize,
								box_conf_threshold, nms_threshold, scale_w, scale_h, _outZps, _outScales, &detect_result_group);
				} else if (_modelInputSize == 640) {
					postprocess((uint8_t*)outputs[0].buf, (uint8_t*)outputs[1].buf,
								(uint8_t*)outputs[2].buf, NULL,
								_modelInputSize, _modelInputSize,
								box_conf_threshold, nms_threshold, scale_w, scale_h, _outZps, _outScales, &detect_result_group);
				}
				// Release rknn_outputs
				rknn_outputs_release(_rknnData.ctx, 2, outputs);

				// Dump Objects
				// for (int i = 0; i < detect_result_group.count; i++) {
				//   detect_result_t *det_result = &(detect_result_group.results[i]);
				//   printf("%s @ (%d %d %d %d) %f\n", det_result->name,
				//   det_result->box.left,
				//          det_result->box.top, det_result->box.right,
				//          det_result->box.bottom,
				//          det_result->prop);
				// }
				if (detect_result_group.count > 0) {
					if (debug) {
						printf("detect result count:%d,%d:%d %d %d %d\n", 
								detect_result_group.count,
								0,
								detect_result_group.results[0].box.left,
								detect_result_group.results[0].box.top,
								detect_result_group.results[0].box.right,
								detect_result_group.results[0].box.bottom);
					}
					g_mtxRknnList.lock();
					rknnListPush(_rknnResultList, getCurrentTimeMsec(), detect_result_group);//把信息存放到链表中去
					int size = rknnListSize(_rknnResultList);
					if (size >= MAX_RKNN_LIST_NUM) {
						rknnListDrop(_rknnResultList);
					}
					g_mtxRknnList.unlock();

					if (debug)
					printf("result list size is %d\n", size);
				}

				// if (g_nDetType == DET_MODEL_FOR_PERSON) {
				// 	_detect_result_group_add_time_t tDetTimeRes;
				// 	tDetTimeRes.dettime = getCurrentTimeMsec();
				// 	memcpy(&(tDetTimeRes.tDetRes), &detect_result_group, sizeof(detect_result_group_t));

				// 	_t_person_alarm_process_result_ tAlarmRes;
				// 	memset(&tAlarmRes, 0, sizeof(_t_person_alarm_process_result_));

				// 	processPersonAlarm(tDetTimeRes, &tAlarmRes);

				// 	g_mtPsDetAlarmRes.lock();
				// 	memcpy(&g_tCurrentPsAlarmInfo, &tAlarmRes, sizeof(_t_person_alarm_process_result_));
				// 	g_mtPsDetAlarmRes.unlock();
				// }
			} catch(const std::exception& e) {
				std::cerr << e.what() << '\n';
			}
		}
	}

	void RknnFrameProcessor::onFrame(void* buffer, int32_t length)
	{

	}
}