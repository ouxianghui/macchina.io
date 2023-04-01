#include <cstring>
#include "DataPipeline.h"
#include "defines.h"
#include "rga.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"

namespace {
	bool initDec(int nDecChn)
	{
		int ret = 0;
		// 解码初始化
		// VDEC
		// 这里可以学习解码器数据结构配置
		VDEC_CHN_ATTR_S stVdecAttr;
		stVdecAttr.enCodecType = RK_CODEC_TYPE_H264; //解码格式
		//stVdecAttr.enImageType = IMAGE_TYPE_NV12;; //解码后输出格式
		stVdecAttr.enMode = VIDEO_MODE_STREAM;//流
		stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;

		ret = RK_MPI_VDEC_CreateChn(nDecChn, &stVdecAttr);//创建解码器通道 
		if (ret) {
			printf("Create Vdec[%d] failed! ret=%d\n", nDecChn, ret);
			return false;
		}

		return true;
	}

	bool initEnc(int nEncChn, int nWidth, int nHeight)
	{
		int ret = 0;
		// 这里可以学习 Venc 硬件编码器数据结构配置
		VENC_CHN_ATTR_S venc_chn_attr; 
		memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));

		venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
		venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = INPUT_VIDEO_FPS;
		venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = nWidth * nHeight;
		venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;//INPUT_VIDEO_FPS;
		venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;//INPUT_VIDEO_FPS;

		venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
		venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12; //输入图片的格式，和 VI 输出保持一致
		venc_chn_attr.stVencAttr.u32PicWidth = nWidth; //编码图像宽度，单位像素点
		venc_chn_attr.stVencAttr.u32PicHeight = nHeight;//编码图像高度，单位像素点
		venc_chn_attr.stVencAttr.u32VirWidth = nWidth;//stride 宽度，必须 16 对齐
		venc_chn_attr.stVencAttr.u32VirHeight = nHeight + 8;// stride 高度，必须 16 对齐
		venc_chn_attr.stVencAttr.u32Profile = 100; //编码等级 77,是中级 66,基础等级，100,高级

		ret = RK_MPI_VENC_CreateChn(nEncChn, &venc_chn_attr);//创建通道
		if (ret) {
			printf("ERROR: create VENC[%d] error! ret=%d\n", nEncChn, ret);
			return false;
		}
		return true;
	}

	bool initRGAYUV2RGB(int nRgaChn, int nWidth, int nHeight)
	{
		int ret = 0;
		RGA_ATTR_S stRgaAttr;
		stRgaAttr.bEnBufPool = RK_TRUE;
		stRgaAttr.u16BufPoolCnt = 2;
		stRgaAttr.u16Rotaion = 0;
		stRgaAttr.stImgIn.u32X = 0;
		stRgaAttr.stImgIn.u32Y = 0;
		stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
		stRgaAttr.stImgIn.u32Width = nWidth;
		stRgaAttr.stImgIn.u32Height = nHeight;
		stRgaAttr.stImgIn.u32HorStride = nWidth;
		stRgaAttr.stImgIn.u32VirStride = nHeight;
		stRgaAttr.stImgOut.u32X = 0;
		stRgaAttr.stImgOut.u32Y = 0;
		stRgaAttr.stImgOut.imgType = IMAGE_TYPE_RGB888;
		stRgaAttr.stImgOut.u32Width = nWidth;
		stRgaAttr.stImgOut.u32Height = nHeight;
		stRgaAttr.stImgOut.u32HorStride = nWidth;
		stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(nHeight,16);
		ret = RK_MPI_RGA_CreateChn(nRgaChn, &stRgaAttr);
		if (ret) {
			printf("ERROR: Create _rgaVencCHN[%d] falied! ret=%d\n", nRgaChn, ret);
		}
		return true;
	}

	bool initRGARGBResize(int nRgaChn, int nWidth, int nHeight, rk_IMAGE_TYPE_E imageType, int32_t resize)
	{
		int ret = 0;
		RGA_ATTR_S stRgaAttr;
		stRgaAttr.bEnBufPool = RK_TRUE;
		stRgaAttr.u16BufPoolCnt = 2;
		stRgaAttr.u16Rotaion = 0;
		stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
		stRgaAttr.stImgIn.u32X = 0;
		stRgaAttr.stImgIn.u32Y = 0;
		stRgaAttr.stImgIn.u32Width = nWidth;
		stRgaAttr.stImgIn.u32Height = nHeight;
		stRgaAttr.stImgIn.u32HorStride = nWidth;
		stRgaAttr.stImgIn.u32VirStride = nHeight;
		stRgaAttr.stImgOut.imgType = imageType;
		stRgaAttr.stImgOut.u32X = 0;
		stRgaAttr.stImgOut.u32Y = 0;
		stRgaAttr.stImgOut.u32Width = resize;
		stRgaAttr.stImgOut.u32Height = resize;
		stRgaAttr.stImgOut.u32HorStride = resize;
		stRgaAttr.stImgOut.u32VirStride = resize;

		stRgaAttr.enFlip = RGA_FLIP_NULL;
		ret = RK_MPI_RGA_CreateChn(nRgaChn, &stRgaAttr);
		if (ret) {
			printf("ERROR: Create _rgaVencCHN[%d] falied! ret=%d\n", nRgaChn, ret);
		}
		return true;
	}

	bool initRGADraw(int nRgaChn, int nWidth, int nHeight)
	{
		int ret = 0;
		RGA_ATTR_S stRgaAttr;
		stRgaAttr.bEnBufPool = RK_TRUE;
		stRgaAttr.u16BufPoolCnt = 2;
		stRgaAttr.u16Rotaion = 0;
		stRgaAttr.stImgIn.u32X = 0;
		stRgaAttr.stImgIn.u32Y = 0;
		stRgaAttr.stImgIn.imgType = IMAGE_TYPE_RGB888;
		stRgaAttr.stImgIn.u32Width = nWidth;
		stRgaAttr.stImgIn.u32Height = nHeight;
		stRgaAttr.stImgIn.u32HorStride = nWidth;
		stRgaAttr.stImgIn.u32VirStride = nHeight;
		stRgaAttr.stImgOut.u32X = 0;
		stRgaAttr.stImgOut.u32Y = 0;
		stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
		stRgaAttr.stImgOut.u32Width = nWidth;
		stRgaAttr.stImgOut.u32Height = nHeight;
		stRgaAttr.stImgOut.u32HorStride = nWidth;
		stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(nHeight,16);
		ret = RK_MPI_RGA_CreateChn(nRgaChn, &stRgaAttr);
		if (ret) {
			printf("ERROR: Create _rgaVencCHN[%d] falied! ret=%d\n", nRgaChn, ret);
		}
		return true;
	}
}

namespace xi {

	DataPipeline::DataPipeline(rk_IMAGE_TYPE_E imageType, int32_t resize)
	: _imageType(imageType)
	, _resize(resize)
	{
		_vdecCHN.enModId = RK_ID_VDEC;
		_vdecCHN.s32ChnId = VDEC_CHN_ID;
		
		_rgaVencCHN.enModId = RK_ID_RGA;
		_rgaVencCHN.s32ChnId = RGA_VENC_CHN_ID;

		_rgaNNCHN.enModId = RK_ID_RGA;
		_rgaNNCHN.s32ChnId = RGA_NN_CHN_ID;

		_rgaDrawCHN.enModId = RK_ID_RGA;
		_rgaDrawCHN.s32ChnId = RGA_DRAW_CHN_ID;

		_vencCHN.enModId = RK_ID_VENC;
		_vencCHN.s32ChnId = VENC_CHN_ID;
	}

	DataPipeline::~DataPipeline()
	{

	}

	void DataPipeline::init()
	{
		bool bRet = false;
		int nRet = 0;
		int nInputImgWidth = INPUT_VIDEO_IMG_WIDTH;
		int nInputImgHeight = INPUT_VIDEO_IMG_HEIGHT;
		
		// step 1
		// 初始化 MPI 系统
		RK_MPI_SYS_Init();

		// step 2
		// 初始化解码通道
		bRet = initDec(_vdecCHN.s32ChnId);
		if (bRet == false) {
			return;
		}

		printf("init dec ok\n");

		// 初始化rga YUV2RGB通道
		bRet = initRGAYUV2RGB(_rgaVencCHN.s32ChnId, nInputImgWidth, nInputImgHeight);
		if (bRet == false) {
			return;
		}
		printf("init yuv to rgb ok\n");

		// 初始化rga RGB Resize通道
		bRet = initRGARGBResize(_rgaNNCHN.s32ChnId, nInputImgWidth, nInputImgHeight, _imageType, _resize);
		if (bRet == false) {
			return;
		}
		printf("init rgb resize ok\n");

		// 初始化rga draw通道
		bRet = initRGADraw(_rgaDrawCHN.s32ChnId, nInputImgWidth, nInputImgHeight);
		if (bRet == false) {
			return;
		}
		printf("init rgb draw ok\n");

		// 初始化编码通道
		bRet = initEnc(_vencCHN.s32ChnId, nInputImgWidth, nInputImgHeight);
		if (bRet == false) {
			return;
		}
		printf("init enc ok\n");

		// step 3
		// 绑定 解码通道 - rga的ReSize输出
		nRet = RK_MPI_SYS_Bind(&_vdecCHN, &_rgaNNCHN);
		if (nRet) {
			printf("ERROR: Bind Vdec[%d] and _rgaNNCHN[%d] failed! ret=%d\n", _vdecCHN.s32ChnId, _rgaNNCHN.s32ChnId, nRet);
			return;
		}

		// 绑定 解码通道 - rga的编码
		nRet = RK_MPI_SYS_Bind(&_vdecCHN, &_rgaVencCHN);
		if (nRet) {
			printf("ERROR: Bind Vdec[%d] and _rgaVencCHN[%d] failed! ret=%d\n", _vdecCHN.s32ChnId, _rgaVencCHN.s32ChnId, nRet);
			return;
		}

		// 绑定 rga画框 - 编码通道， 用于编码后RTSP输出
		nRet = RK_MPI_SYS_Bind(&_rgaDrawCHN, &_vencCHN);
		if (nRet) {
			printf("ERROR: Bind RgaDRAW[%d] and stVenChn[%d] failed! ret=%d\n", _rgaDrawCHN.s32ChnId, _vencCHN.s32ChnId, nRet);
			return;
		}

		RK_MPI_SYS_SetMediaBufferDepth(RK_ID_VDEC, VDEC_CHN_ID, 50);
		RK_MPI_SYS_SetMediaBufferDepth(RK_ID_RGA, RGA_DRAW_CHN_ID, 50);
	}

	void DataPipeline::destroy()
	{
		RK_MPI_VDEC_DestroyChn(_vdecCHN.s32ChnId);

		RK_MPI_VDEC_DestroyChn(_rgaVencCHN.s32ChnId);

		RK_MPI_VDEC_DestroyChn(_rgaNNCHN.s32ChnId);

		RK_MPI_VDEC_DestroyChn(_vencCHN.s32ChnId);

		// TODO: should close the channel, will confirm with cai
		//RK_MPI_VDEC_DestroyChn(_rgaDrawCHN.s32ChnId);
	}
}