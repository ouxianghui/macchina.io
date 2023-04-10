#include "DetectionService.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/ClassLibrary.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "DataPipeline.h"
#include "FrameCapturer.h"
#include "RknnFrameProcessor.h"
#include "FrameRenderer.h"
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
		// // TODO: set image type for cpaturer
		// _dataPipeline = std::make_shared<DataPipeline>(IMAGE_TYPE_BGR888, 640);
		// _dataPipeline->init();

		// char szLocalIP[32] = {0};
		// getLocalIP(szLocalIP);
		// char szRtspUrl[128] = {0x0};
  		// sprintf(szRtspUrl, "rtsp://%s:554/live/main_stream", szLocalIP);
		// std::string rtspUrl(szRtspUrl);
		// _frameCapturer = std::make_shared<FrameCapturer>(_dataPipeline->rgaNNCHN(), rtspUrl);
		// _frameCapturer->init();

		std::string modelFileName("person-5m-6.2.rknn");
		std::string modelFilePath = "/userdata/model";

		copyModel(pContext, modelFilePath, modelFileName);

		std::string modelFullName = modelFilePath + "/" + modelFileName;

		auto processor = std::make_shared<RknnFrameProcessor>(modelFullName, 20, 40);
		processor->init();
		//_frameCapturer->addListener(processor);
		_frameProcessor = processor;

		// _frameRenderer = std::make_shared<FrameRenderer>(_dataPipeline->rgaVencCHN(), _dataPipeline->rgaDrawCHN());
		// _frameRenderer->init();

		// auto provider = std::make_shared<RtspStreamProvider>(_dataPipeline->vencCHN());
		// provider->init(std::make_shared<FrameRenderer>());
		// _frameCapturer->addListener(provider);
		// _streamProvider = provider;
	}
	
	DetectionService::~DetectionService()
	{
		//_frameCapturer->destroy();

		_frameProcessor->destroy();

		//_streamProvider->destroy();

		//_dataPipeline->destroy();
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
		//_frameCapturer->start();

		_frameProcessor->start();

		//_streamProvider->start();
	}

	void DetectionService::stop() 
	{
		//_frameCapturer->stop();

		_frameProcessor->stop();

		//_streamProvider->stop();
	}

	void DetectionService::copyModel(BundleContext::Ptr pContext, const std::string& path, const std::string& name)
	{
		std::unique_ptr<std::istream> stream(pContext->thisBundle()->getResource(name));
		if (!stream->good()) {
			std::cerr << "read model resource faild: " << name << std::endl;
			return;
		}
		Poco::Path d(path);
		Poco::File f(path);
		if (!f.exists()) {
			d.makeDirectory();
			if (!f.exists()) {
				std::cerr << "create directory failed: " << path << std::endl;
				return;
			}
		}

		std::string modelFullName = path + "/" + name;

		{
			std::ofstream output;
			output.open(modelFullName, std::ios::binary | std::ios::out | std::ios::trunc);

			char buf[1024] = {0};
			std::streamsize bytesRead; 
			do 
			{ 
				stream->read(buf, 1024);
				bytesRead = stream->gcount();
				//std::cout << " -------------> bytesRead: " << bytesRead << std::endl;
				output.write(buf, bytesRead);
				// do stuff with the bytes you read, if any 
			} while (!stream->eof()); 

			output.close();
		}
	}
}
