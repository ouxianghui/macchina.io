#include "RtspStreamProvider.h"
#include "rkmedia_api.h"
#include "defines.h"
#include "rtsp_demo.h"
#include "IFrameRenderer.h"

namespace xi {

	RtspStreamProvider::RtspStreamProvider(const MPP_CHN_S& chn)
	: _vencCHN(chn)
	, _activity(this, &RtspStreamProvider::runActivity)
	{

	}

	RtspStreamProvider::~RtspStreamProvider()
	{

	}

	void RtspStreamProvider::init(std::shared_ptr<IFrameRenderer> renderer) 
	{
		_renderer = renderer;

		if (_renderer) {
			_renderer->init();
		}
	}

	void RtspStreamProvider::destroy() 
	{
		if (_renderer) {
			_renderer->destroy();
		}
	}

	void RtspStreamProvider::start() 
	{
		_activity.start();
	}

	void RtspStreamProvider::stop() 
	{
		_activity.stop();
		_activity.wait();
	}

	void RtspStreamProvider::runActivity()
	{
		// step 7
		// rtsp server初始化，取出编码模块的码流处理rtsp输出
		//printf("init rtsp\n");
		//rtsp_demo_handle rtsplive = create_rtsp_demo(8554);
		//void* rtspSession = rtsp_new_session(rtsplive, "/live/ai");
		//rtsp_set_video(rtspSession, RTSP_CODEC_ID_VIDEO_H264, NULL,0);
		//rtsp_sync_video_ts(rtspSession, rtsp_get_reltime(), rtsp_get_ntptime());
		//uint64_t enc_ts = 0;
		//while (!_activity.isStopped()) {
		//	MEDIA_BUFFER buffer;
		//	// send video buffer
		//	buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, _vencCHN.s32ChnId, -1);
		//	if (buffer) {
		//		if (_renderer) {
		//			int32_t len = RK_MPI_MB_GetSize(buffer);
		//			_renderer->renderFrame(buffer, len);
		//		}
		//		//printf("got encoded buffer to rtsp\n");
		//		rtsp_tx_video(rtspSession, 
		//					  (const uint8_t*)RK_MPI_MB_GetPtr(buffer),
		//					  RK_MPI_MB_GetSize(buffer),
		//					  rtsp_get_reltime());
		//		enc_ts += 3600;
		//		RK_MPI_MB_ReleaseBuffer(buffer);
		//	}
		//	//usleep(1000*35);
		//	Poco::Thread::sleep(35);
		//	rtsp_do_event(rtsplive);
		//}

	}

	void RtspStreamProvider::onFrame(void* buffer, int32_t length)
	{

	}
}