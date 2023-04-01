#pragma once

#include <memory>
#include "rkmedia_common.h"
#include "rkmedia_api.h"

namespace xi {

	class DataPipeline : public std::enable_shared_from_this<DataPipeline> {
	public:
		DataPipeline(rk_IMAGE_TYPE_E imageType, int32_t resize);

		~DataPipeline();

		void init();

		void destroy();

		const MPP_CHN_S& vdecCHN() { return _vdecCHN; }

		const MPP_CHN_S& rgaVencCHN() { return _rgaVencCHN; }

		const MPP_CHN_S& rgaNNCHN() { return _rgaNNCHN; }

		const MPP_CHN_S& rgaDrawCHN() { return _rgaDrawCHN; }

		const MPP_CHN_S& vencCHN() { return _vencCHN; }

	private:
		rk_IMAGE_TYPE_E _imageType;

		int32_t _resize = 640;

		MPP_CHN_S _vdecCHN;
		
		MPP_CHN_S _rgaVencCHN;

		MPP_CHN_S _rgaNNCHN;

		MPP_CHN_S _rgaDrawCHN;

		MPP_CHN_S _vencCHN;
	};
}