//
// ZigBeeTransmitStatusSerializer.h
//
// Package: Generated
// Module:  TypeSerializer
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2015-2022, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: GPL-3.0-only
//


#ifndef TypeSerializer_IoT_XBee_ZigBeeTransmitStatus_INCLUDED
#define TypeSerializer_IoT_XBee_ZigBeeTransmitStatus_INCLUDED


#include "IoT/XBee/XBeeNode.h"
#include "Poco/RemotingNG/TypeSerializer.h"


namespace Poco {
namespace RemotingNG {


template <>
class TypeSerializer<IoT::XBee::ZigBeeTransmitStatus>
{
public:
	static void serialize(const std::string& name, const IoT::XBee::ZigBeeTransmitStatus& value, Serializer& ser)
	{
		using namespace std::string_literals;
		
		ser.serializeStructBegin(name);
		serializeImpl(value, ser);
		ser.serializeStructEnd(name);
	}

	static void serializeImpl(const IoT::XBee::ZigBeeTransmitStatus& value, Serializer& ser)
	{
		using namespace std::string_literals;
		
		static const std::string REMOTING__NAMES[] = {"deliveryStatus"s,"discoveryStatus"s,"frameID"s,"networkAddress"s,""s};
		TypeSerializer<Poco::UInt8 >::serialize(REMOTING__NAMES[0], value.deliveryStatus, ser);
		TypeSerializer<Poco::UInt8 >::serialize(REMOTING__NAMES[1], value.discoveryStatus, ser);
		TypeSerializer<Poco::UInt8 >::serialize(REMOTING__NAMES[2], value.frameID, ser);
		TypeSerializer<std::string >::serialize(REMOTING__NAMES[3], value.networkAddress, ser);
	}

};


} // namespace RemotingNG
} // namespace Poco


#endif // TypeSerializer_IoT_XBee_ZigBeeTransmitStatus_INCLUDED

