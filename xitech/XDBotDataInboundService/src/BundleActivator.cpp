//
// BundleActivator.cpp
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-only
//


#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/BundleLoader.h"
#include "Poco/OSP/OSPSubsystem.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/Util/Application.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "Poco/ClassLibrary.h"
#include "Poco/SharedPtr.h"
#include "EventRequestHandler.h"


namespace xi {
namespace XDBotDataInboundService {


class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator()
	{
	}
	
	~BundleActivator()
	{
	}
	
	void start(Poco::OSP::BundleContext::Ptr pContext)
	{		
	}
		
	void stop(Poco::OSP::BundleContext::Ptr pContext)
	{
	}
};


} } // namespace xi::XDBotDataInboundService


POCO_BEGIN_NAMED_MANIFEST(WebServer, Poco::OSP::Web::WebRequestHandlerFactory)
	POCO_EXPORT_CLASS(xi::XDBotDataInboundService::EventRequestHandlerFactory)
POCO_END_MANIFEST


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(xi::XDBotDataInboundService::BundleActivator)
POCO_END_MANIFEST
