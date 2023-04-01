#include "RknnFrameRenderer.h"
#include <memory>

namespace xi {

	RknnFrameRenderer::RknnFrameRenderer(const MPP_CHN_S& chn)
	: _rgaNNCHN(chn)
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

	}

}