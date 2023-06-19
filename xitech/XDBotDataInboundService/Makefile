#
# Makefile
#
# Makefile for macchina.io Settings app
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects =  \
	EventRequestHandler \
	Utility \
	BundleActivator 

target      = io.macchina.xitech.xdbot.data.inbound.service
target_includes = $(PROJECT_BASE)/platform/Data/include $(PROJECT_BASE)/platform/Data/MySQL/include $(PROJECT_BASE)/xitech/common/include
target_libs = PocoOSPWeb PocoOSP PocoNet PocoZip PocoUtil PocoJSON PocoXML PocoFoundation PocoData PocoDataMySQL

postbuild = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -Npng,gif,jpg -o../bundles XDBotDataInboundService.bndlspec

include $(POCO_BASE)/build/rules/dylib
