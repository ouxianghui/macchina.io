#
# Makefile
#
# Makefile for macchina.io XDBotAlarmer applications
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects = BundleActivator AlarmHandler

target          = cn.xitech.xdbot.alarmer
target_includes = $(PROJECT_BASE)/devices/Devices/include $(PROJECT_BASE)/platform/Data/include $(PROJECT_BASE)/platform/Data/MySQL/include $(PROJECT_BASE)/xitech/common/include
target_libs     = IoTDevices PocoOSP PocoUtil PocoJSON PocoXML PocoFoundation

postbuild = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles XDBotAlarmer.bndlspec

include $(POCO_BASE)/build/rules/dylib
