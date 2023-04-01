#include "DetectionService.h"
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/ClassLibrary.h"
#include "DataPipeline.h"
#include "RtspFrameCapturer.h"
#include "RknnFrameProcessor.h"
#include "RknnFrameRenderer.h"
#include "RtspStreamProvider.h"
#include "rkmedia_common.h"

using Poco::OSP::BundleActivator;
using Poco::OSP::BundleContext;
using Poco::OSP::Bundle;
using Poco::OSP::Properties;
using Poco::OSP::ServiceRef;

namespace xi {

	DetectionService::DetectionService(BundleContext::Ptr pContext)
	: _pContext(pContext)
	{
		// TODO: set image type for cpaturer
		_dataPipeline = std::make_shared<DataPipeline>(IMAGE_TYPE_BGR888, 640);

		// TODO:
		std::string rtspUrl;
		_frameCapturer = std::make_shared<RtspFrameCapturer>(_dataPipeline->rgaNNCHN(), rtspUrl);

		// TODO:
		std::string modelFilePath;
		auto processor = std::make_shared<RknnFrameProcessor>(modelFilePath, 20, 40);
		
		_frameCapturer->addListener(processor);

		_frameProcessor = processor;

		_frameRenderer = std::make_shared<RknnFrameRenderer>(_dataPipeline->rgaVencCHN(), _dataPipeline->rgaDrawCHN());

		_streamProvider = std::make_shared<RtspStreamProvider>(_dataPipeline->vencCHN());
	}
	
	DetectionService::~DetectionService()
	{

	}
	
	const std::type_info& DetectionService::type() const
	{
		return typeid(DetectionService);
	}
	
	bool DetectionService::isA(const std::type_info& otherType) const
	{
		std::string name(typeid(DetectionService).name());
		return name == otherType.name() || Service::isA(otherType);
	}

	void DetectionService::init()  
	{
		_dataPipeline->init();

		_frameCapturer->init();

		_frameProcessor->init();

		_frameRenderer->init();

		_streamProvider->init();
	}

	void DetectionService::destroy() 
	{
		_frameCapturer->destroy();

		_frameProcessor->destroy();

		_frameRenderer->destroy();

		_streamProvider->destroy();

		_dataPipeline->destroy();
	}

}
