#include "FrameCapturer.h"
#include <iostream>
#include <cstring>
#include <atomic>
#include "defines.h"
#include "rga.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"


namespace xi {

	static std::atomic_bool _exit = { false };

	FrameCapturer::FrameCapturer(const MPP_CHN_S& chn, const std::string& rtspUrl)
	: _rgaNNCHN(chn)
	, _activityRtsp(this, &FrameCapturer::runRtsp)
	, _activityCapture(this, &FrameCapturer::runCapture)
	, _rtspUrl(rtspUrl)
	{
		_rtspClient.callback = FFRKMediaVdecSend;
		_rtspClient.count = 1;
		_rtspClient.ffrtsp_get_info[0].url = (char*)_rtspUrl.c_str();
	}

	FrameCapturer::~FrameCapturer()
	{

	}

	void FrameCapturer::init() 
	{

	}

	void FrameCapturer::destroy() 
	{

	}

	void FrameCapturer::addListener(std::shared_ptr<ICapturerListener> listener) 
	{
		addObserver<ICapturerListener>(_observers, listener);
	}

	void FrameCapturer::removeListener(std::shared_ptr<ICapturerListener> listener) 
	{
		removeObserver<ICapturerListener>(_observers, listener);
	}

	void FrameCapturer::start() 
	{
		_activityRtsp.start();
		_activityCapture.start();
	}

	void FrameCapturer::stop() 
	{
		_activityCapture.stop();
		_activityCapture.wait();

		_exit.store(true);

		_activityRtsp.stop();
		_activityRtsp.wait();
	}

	void FrameCapturer::runRtsp() 
	{
		ffrtspGet(_rtspClient);
	}

	void FrameCapturer::runCapture()
	{
		//rv1126_data * nndata = (struct rv1126_data *)data;
		MEDIA_BUFFER buffer = NULL;

		while (!_activityCapture.isStopped()) {
			try {
				//取 RK_NN_INDEX 通道的帧进行推理，因为
				//这个通道本身分辨率很低，所以进行缩放的时候会省去 rga 的资源
				buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, _rgaNNCHN.s32ChnId, -1);
				if (RK_MPI_MB_GetPtr(buffer) == NULL) {
					//usleep(1000*5);
					Poco::Thread::sleep(5);
					continue;
				}

				notifyObserver<ICapturerListener>(_observers, [&buffer](const auto& observer) {
					int32_t len = RK_MPI_MB_GetSize(buffer);
					observer->onFrame(buffer, len);
				} );

				if (debug) {
					printf("got decoded buffer size:%zu, timestamp:%lld\n", RK_MPI_MB_GetSize(buffer), RK_MPI_MB_GetTimestamp(buffer));
				}

				RK_MPI_MB_ReleaseBuffer(buffer);
				continue;

			} catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
			}
		}
	}

	int FrameCapturer::FFRKMediaVdecSend(u_int8_t* frameBuff, unsigned frameSize, bool* quit, int chn)
	{
		if (frameSize <= 0){
			printf("rtsp buf error size is :%d\n", frameSize);
			return 0;
		}
		if (frameBuff == NULL){
			printf("rtsp buf error is NULL\n");
			return 0;
		}
		//printf("rtsp buf frameSize: %ld\n", frameSize);
		MEDIA_BUFFER mb = RK_MPI_MB_CreateBuffer(frameSize, RK_FALSE, 0);
		//printf("#Send packet(%p, %zuBytes) to VDEC[0].\n", RK_MPI_MB_GetPtr(mb),RK_MPI_MB_GetSize(mb));//打印发送多大数据到 VDEC
		//RK_MPI_MB_SetSize(mb, frameSize);
		memcpy(RK_MPI_MB_GetPtr(mb), frameBuff, frameSize);
	#ifdef SAVE_FILE
		FILE *fp = fopen("test_rtsp", "a+b");
		fwrite(RK_MPI_MB_GetPtr(mb), frameSize, 1, fp);
		fclose(fp);
		fp = NULL;
	#else
		RK_MPI_MB_SetSize(mb, frameSize);
		RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, VDEC_CHN_ID, mb);
	#endif
		RK_MPI_MB_ReleaseBuffer(mb);
		if (_exit.load()) {
			*quit = true;
		}
		
		return 0;
	}
}