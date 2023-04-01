#pragma once

#include <memory>
#include <vector>
#include <string>
#include "Poco/Activity.h"
#include "rknn_api.h"
#include "IFrameProcessor.h"
#include "Observable.h"
#include "IFrameCapturer.h"

class RknnList;

namespace xi {

	class RknnFrameProcessor : public IFrameProcessor, public ICapturerListener, public Observable, public std::enable_shared_from_this<RknnFrameProcessor> {
	public:
		RknnFrameProcessor(const std::string& modelFilePath, int32_t nmsThresh, int32_t boxThresh);

		~RknnFrameProcessor();

		void init() override;

		void destroy() override;

		void addListener(std::shared_ptr<IProcessorListener> listener) override;

		void removeListener(std::shared_ptr<IProcessorListener> listener) override;

		void start() override;

		void stop() override;

		bool initModel();

	protected:
		void runActivity();

		// ICapturerListener
		void onFrame(void* buffer, int32_t length) override;

	private:
		struct RknnData {
			rknn_input_output_num io_num;
			rknn_context ctx;
			int number;
		};

	private:
		std::vector<std::weak_ptr<IProcessorListener>> _observers;

		std::string _modelFilePath;

		int32_t _nmsThresh;

		int32_t _boxThresh;

		Poco::Activity<RknnFrameProcessor> _activity;

		RknnData _rknnData;

		int32_t _modelInputSize = 640;

		std::vector<float> _outScales;

		std::vector<uint32_t> _outZps;

		RknnList* _rknnResultList;
	};

}