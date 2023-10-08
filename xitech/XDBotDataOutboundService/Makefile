#
# Makefile
#
# Makefile for macchina.io Settings app
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects =  \
	EventRequestHandler \
	GPSRequestHandler \
	TemperatureRequestHandler \
	WindSpeedRequestHandler \
	WaterImmersionRequestHandler \
	Utility \
	BundleActivator 

target      = cn.xitech.xdbot.data.outbound.service
target_includes = $(PROJECT_BASE)/devices/Devices/include $(PROJECT_BASE)/platform/Data/include $(PROJECT_BASE)/platform/Data/MySQL/include $(PROJECT_BASE)/xitech/common/include
target_libs = IoTDevices PocoOSPWeb PocoOSP PocoNet PocoZip PocoUtil PocoJSON PocoXML PocoFoundation PocoData PocoDataMySQL

postbuild = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -Npng,gif,jpg -o../bundles XDBotDataOutboundService.bndlspec

include $(POCO_BASE)/build/rules/dylib
