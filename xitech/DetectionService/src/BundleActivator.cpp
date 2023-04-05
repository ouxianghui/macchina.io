#include "Poco/OSP/BundleActivator.h" 
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/ClassLibrary.h"
#include "Poco/SharedLibrary.h"
#include "DetectionService.h"

#include <iostream>

using Poco::OSP::BundleActivator;
using Poco::OSP::BundleContext;
using Poco::OSP::Bundle;
using Poco::OSP::Properties;
using Poco::OSP::ServiceRef;
using namespace xi;

class DetectionServiceBundleActivator: public BundleActivator
	/// The BundleActivator for the DetectionService.
	/// Registers the DetectionService with the Service Registry.
{
public:
	DetectionServiceBundleActivator()
	{
		std::cerr << " ---------------------------------> load dylibs" << std::endl;
		auto dylib = std::make_shared<Poco::SharedLibrary>("libffrtsp.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("librknn_api.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libeasymedia.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libthird_media.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libBasicUsageEnvironment.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libliveMedia.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("librockx.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("librockchip_mpp.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("librockface.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("librga.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libdrm.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libgroupsock.so");
		_dylibs.emplace_back(dylib);

		dylib = std::make_shared<Poco::SharedLibrary>("libmd_share.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libod_share.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("librkaiq.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libRKAP_3A.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libRKAP_ANR.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libRKAP_Common.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libsqlite3.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libUsageEnvironment.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libv4l2.so");
		_dylibs.emplace_back(dylib);

				dylib = std::make_shared<Poco::SharedLibrary>("libOpenVX.so");
		_dylibs.emplace_back(dylib);
		

				dylib = std::make_shared<Poco::SharedLibrary>("libArchModelSw.so");
		_dylibs.emplace_back(dylib);


				dylib = std::make_shared<Poco::SharedLibrary>("libv4lconvert.so");
		_dylibs.emplace_back(dylib);


				dylib = std::make_shared<Poco::SharedLibrary>("libVSC.so");
		_dylibs.emplace_back(dylib);


				dylib = std::make_shared<Poco::SharedLibrary>("libz.so");
		_dylibs.emplace_back(dylib);


		//dylib = std::make_shared<Poco::SharedLibrary>("libc.so");
		//_dylibs.emplace_back(dylib);


				dylib = std::make_shared<Poco::SharedLibrary>("libuuid.so");
		_dylibs.emplace_back(dylib);


				dylib = std::make_shared<Poco::SharedLibrary>("libcrypto.so");
		_dylibs.emplace_back(dylib);

						dylib = std::make_shared<Poco::SharedLibrary>("libssl.so");
		_dylibs.emplace_back(dylib);

		// libthird_media.so
        // libeasymedia.so
        // librockx.so
        // librockchip_mpp.so
        // librockface.so
        // libasound.so
        // libBasicUsageEnvironment.so
        // libdrm.so
        // libgroupsock.so
        // libliveMedia.so
        // libmd_share.so
        // libod_share.so
        // librga.so
        // librkaiq.so
        // libRKAP_3A.so
        // libRKAP_ANR.so
        // libRKAP_Common.so
        // #librknn_runtime.so #!!!!!!!!!!!!if include librknn_api.so cannot include this
        // libsqlite3.so
        // libUsageEnvironment.so
        // libv4l2.so
        // libOpenVX.so
        // libArchModelSw.so
        // libGAL.so
        // libNNArchPerf.so
        // libv4lconvert.so
        // libVSC.so
		
		// libz.so
        // libopencv_core.so
        // libfreetype.a
        // libharfbuzz.a
        // libharfbuzz-subset.a
        // libjsoncpp.a
        // Threads::Threads
        // libc.so
        // libmosquitto.so
        // libuuid.so
        // libcrypto.so
        // libssl.so
	}
	
	~DetectionServiceBundleActivator()
	{
		for (const auto& lib : _dylibs) {
			lib->unload();
		}
	}
	
	void start(BundleContext::Ptr pContext)
	{
		// Create an instance of the DetectionService
		_detectionService = new DetectionService(pContext);

		_detectionService->init();
		
		_detectionService->start();

		// Register the DetectionService with the ServiceRegistry.
		_pService = pContext->registry().registerService("cn.xitech.iot.DetectionService", _detectionService, Properties());
	}
		
	void stop(BundleContext::Ptr pContext)
	{
		_detectionService->stop();

		_detectionService->destroy();

		// Unregister the DetectionService
		pContext->registry().unregisterService(_pService);
	}
	
private:
	std::vector<std::shared_ptr<Poco::SharedLibrary>> _dylibs;

	ServiceRef::Ptr _pService;

	IDetectionService::Ptr _detectionService;
};


POCO_BEGIN_MANIFEST(BundleActivator)
	POCO_EXPORT_CLASS(DetectionServiceBundleActivator)
POCO_END_MANIFEST
