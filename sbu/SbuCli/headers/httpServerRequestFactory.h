#pragma once

#include "HttpUrlRouter.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>

class MyRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
	MyRequestHandler(std::shared_ptr<HttpUrlRouter> router);
	virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

protected:
	std::shared_ptr<HttpUrlRouter> router;
};

class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	MyRequestHandlerFactory(boost::program_options::variables_map& params);

	virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request);

protected:
	std::shared_ptr<HttpUrlRouter> router;
};

