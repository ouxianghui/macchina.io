#
# Makefile
#
# Makefile for Streamer
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects = BundleActivator StreamerManager MediasoupStreamer 

target = cn.xitech.xdbot.streamer

target_includes = $(PROJECT_BASE)/xitech/Streamer/3rd/include $(PROJECT_BASE)/xitech/Streamer/3rd/include/include $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps/webrtc/include $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps/webrtc/include/third_party/abseil-cpp $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps/libmediasoupclient/include $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps/libmediasoupclient/deps/libsdptransform/include $(PROJECT_BASE)/xitech/Streamer/3rd/include/deps/spdlog/include

target_libs = PocoOSP PocoUtil PocoXML PocoFoundation

postbuild = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles Streamer.bndlspec

include $(POCO_BASE)/build/rules/dylib
