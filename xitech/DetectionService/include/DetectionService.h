#include <memory>
#include "IDetectionService.h"
#include "Poco/OSP/BundleContext.h"

using Poco::OSP::BundleContext;

namespace xi {

	class DataPipeline;
	class IFrameCapturer;
	class IFrameProcessor;
	class IStreamProvider;

	class DetectionService: public IDetectionService
	{
	public:
		DetectionService(BundleContext::Ptr pContext);
		
		~DetectionService();
		
		const std::type_info& type() const override;
		
		bool isA(const std::type_info& otherType) const override;

		void init() override ;

		void destroy() override;

		void start() override ;

		void stop() override;

	protected:
		void copyModel(BundleContext::Ptr pContext, const std::string& path, const std::string& name);

	private:
		BundleContext::Ptr _pContext;

		std::shared_ptr<DataPipeline> _dataPipeline;

		std::shared_ptr<IFrameCapturer> _frameCapturer;

		std::shared_ptr<IFrameProcessor> _frameProcessor;

		std::shared_ptr<IStreamProvider> _streamProvider;
	};

}
