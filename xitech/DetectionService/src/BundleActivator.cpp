#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/ClassLibrary.h"
#include "DetectionService.h"

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

	}
	
	~DetectionServiceBundleActivator()
	{
		
	}
	
	void start(BundleContext::Ptr pContext)
	{
		// Create an instance of the DetectionService
		_detectionService = new DetectionService(pContext);
		_detectionService->init();
		// Register the DetectionService with the ServiceRegistry.
		_pService = pContext->registry().registerService("cn.xitech.iot.DetectionService", _detectionService, Properties());
	}
		
	void stop(BundleContext::Ptr pContext)
	{
		_detectionService->destroy();
		// Unregister the DetectionService
		pContext->registry().unregisterService(_pService);
	}
	
private:
	ServiceRef::Ptr _pService;
	IDetectionService::Ptr _detectionService;
};


POCO_BEGIN_MANIFEST(BundleActivator)
	POCO_EXPORT_CLASS(DetectionServiceBundleActivator)
POCO_END_MANIFEST
