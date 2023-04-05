#include "RknnFrameRenderer.h"
#include <memory>

namespace xi {

	RknnFrameRenderer::RknnFrameRenderer(const MPP_CHN_S& vencCHN, const MPP_CHN_S& drawCHN)
	: _rgaVencCHN(vencCHN)
	, _rgaDrawCHN(drawCHN)
	, _activity(this, &RknnFrameRenderer::runActivity)
	{

	}

	RknnFrameRenderer::~RknnFrameRenderer()
	{

	}

	void RknnFrameRenderer::init() 
	{

	}

	void RknnFrameRenderer::destroy() 
	{

	}

	void RknnFrameRenderer::start() 
	{
		_activity.start();
	}

	void RknnFrameRenderer::stop() 
	{
		_activity.stop();
		_activity.wait();
	}

	void RknnFrameRenderer::runActivity()
	{
		MEDIA_BUFFER buffer;

		while (!_activity.isStopped()) {
			try {
				buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, _rgaVencCHN.s32ChnId, -1);
				if (!buffer) {
					continue;
					//usleep(5*1000);
					Poco::Thread::sleep(5);
				}
				
				// 将画框后的buffer发送到编码单元
				RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, _rgaDrawCHN.s32ChnId, buffer); //换框后发送本帧数据到 RgaDRAW，RGA转换格式，编码然后送往 RTSP 流。

				RK_MPI_MB_ReleaseBuffer(buffer);
			} catch (...) {
				continue;
			}
		}
	}

}