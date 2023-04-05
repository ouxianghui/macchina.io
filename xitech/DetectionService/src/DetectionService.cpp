#include "DetectionService.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
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

namespace {

	// TODO: move to utils
	int getLocalIP(char *strIP)  
	{
		int sock_fd, intrface;
		struct ifreq buf[INET_ADDRSTRLEN];
		struct ifconf ifc;
		char *local_ip = NULL;
		int status = -1;

		if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
			ifc.ifc_len = sizeof(buf);
			ifc.ifc_buf = (caddr_t)buf;
			if (!ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc)) {
				intrface = ifc.ifc_len/sizeof(struct ifreq);
				while (intrface-- > 0) {
					if (!(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[intrface]))) {
						local_ip = NULL;
						local_ip = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
						if(local_ip) {
							strcpy(strIP, local_ip);
							status = 0;
							if(strcmp("127.0.0.1", strIP)) {
								break;
							}
						}
					}
				}
			}
			close(sock_fd);
		}
		return status;
	}
}

namespace xi {

	DetectionService::DetectionService(BundleContext::Ptr pContext)
	: _pContext(pContext)
	{
		// TODO: set image type for cpaturer
		_dataPipeline = std::make_shared<DataPipeline>(IMAGE_TYPE_BGR888, 640);
		_dataPipeline->init();

		char szLocalIP[32] = {0};
		getLocalIP(szLocalIP);
		char szRtspUrl[128] = {0x0};
  		sprintf(szRtspUrl, "rtsp://%s:554/live/main_stream", szLocalIP);
		std::string rtspUrl(szRtspUrl);
		_frameCapturer = std::make_shared<RtspFrameCapturer>(_dataPipeline->rgaNNCHN(), rtspUrl);
		_frameCapturer->init();

		// // TODO:
		// std::string modelFilePath;
		// auto processor = std::make_shared<RknnFrameProcessor>(modelFilePath, 20, 40);
		// _frameProcessor->init();

		// _frameCapturer->addListener(processor);

		// _frameProcessor = processor;

		_frameRenderer = std::make_shared<RknnFrameRenderer>(_dataPipeline->rgaVencCHN(), _dataPipeline->rgaDrawCHN());
		_frameRenderer->init();

		_streamProvider = std::make_shared<RtspStreamProvider>(_dataPipeline->vencCHN());
		_streamProvider->init();
	}
	
	DetectionService::~DetectionService()
	{
		_frameCapturer->destroy();

		// _frameProcessor->destroy();

		_frameRenderer->destroy();

		_streamProvider->destroy();

		_dataPipeline->destroy();
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

	}

	void DetectionService::destroy() 
	{

	}

	void DetectionService::start()  
	{
		_frameCapturer->start();

		// _frameProcessor->start();

		_frameRenderer->start();

		_streamProvider->start();
	}

	void DetectionService::stop() 
	{
		_frameCapturer->stop();

		// _frameProcessor->stop();

		_frameRenderer->stop();

		_streamProvider->stop();
	}

}
