#
# Makefile
#
# Makefile for DetectionService
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects = BundleActivator DetectionService DataPipeline RtspFrameCapturer RknnFrameRenderer RknnFrameProcessor RtspStreamProvider Postprocess RknnResultList

target         = cn.xitech.iot.detectionservice
target_includes = $(PROJECT_BASE)/xitech/DetectionService/3rd/rkmedia/include $(PROJECT_BASE)/xitech/DetectionService/3rd/rga/include $(PROJECT_BASE)/xitech/DetectionService/3rd/ffrtsp $(PROJECT_BASE)/xitech/DetectionService/3rd/librtsp/include $(PROJECT_BASE)/xitech/DetectionService/3rd/rga/include $(PROJECT_BASE)/xitech/DetectionService/3rd/librknn_api/include
target_libs    = PocoOSP PocoUtil PocoXML PocoFoundation rtsp

postbuild      = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles DetectionService.bndlspec

include $(POCO_BASE)/build/rules/dylib
