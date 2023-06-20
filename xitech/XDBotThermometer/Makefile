#
# Makefile
#
# Makefile for macchina.io thermometer applications
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/OSP/BundleCreator/BundleCreator.make

objects = BundleActivator ActivityThermometer SensorDataHandler

target          = cn.xitech.xdbot.thermometer
target_includes = $(PROJECT_BASE)/protocols/Modbus/include $(PROJECT_BASE)/platform/Data/include $(PROJECT_BASE)/platform/Data/MySQL/include $(PROJECT_BASE)/xitech/XDBotThermometer/include $(PROJECT_BASE)/xitech/common/include
target_libs     = IoTModbus PocoOSP PocoUtil PocoJSON PocoXML PocoFoundation PocoData PocoDataMySQL

postbuild = $(SET_LD_LIBRARY_PATH) $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles XDBotThermometer.bndlspec

include $(POCO_BASE)/build/rules/dylib