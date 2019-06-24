#include "HttpUrlRouter.h"

#pragma once

#include "utils.h"
#include "boost/regex.hpp"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

HttpUrlRouter::Route::Route(HttpUrlRouter::Verb verb, const std::string path, std::shared_ptr<HttpUrlRouter::HttpUrlRouterHandler> handler) :
	path(path), handler(handler)
{
	switch (verb) {
	case HttpUrlRouter::Verb::HTTP_GET:
		this->verb = "GET";
		break;
	case HttpUrlRouter::Verb::HTTP_POST:
		this->verb = "POST";
		break;
	}
}

HttpUrlRouter::HandleStatus HttpUrlRouter::Route::HandleRequest(boost::program_options::variables_map& config, Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp)
{
	if (req.getMethod() != this->verb)
	{
		return HandleStatus::Missing;
	}

	auto reqURI = req.getURI();

	boost::regex expr(this->path);
	boost::smatch what;
	if (boost::regex_search(reqURI, what, expr))
	{
		boost::program_options::variables_map requestConfig(config);
		std::map<std::string, std::string> urlParams;
		std::map<std::string, std::string> queryParams;
		this->handler->OnRequest(requestConfig, Verb::HTTP_GET, urlParams, queryParams, req, resp);
	}

	return HandleStatus::Success;
}


HttpUrlRouter::HttpUrlRouter(boost::program_options::variables_map& params)
	: vm(params)
{
	this->count = 0;
}

HttpUrlRouter::HandleStatus HttpUrlRouter::HandleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
	HandleStatus retValue = HandleStatus::Missing;
	std::list<std::shared_ptr<Route>>::iterator iter;
	for (iter = routes.begin(); iter != routes.end() && retValue == HandleStatus::Missing; ++iter)
	{
		auto currentRoute = *iter;
		if (currentRoute->HandleRequest(this->vm, req, resp) != HandleStatus::Missing)
			break;
	}

	return retValue;
}

void HttpUrlRouter::AddRoute(HttpUrlRouter::Verb verb, const std::string &path, std::shared_ptr<HttpUrlRouter::HttpUrlRouterHandler> handler)
{
	std::shared_ptr<HttpUrlRouter::Route> newRoute = std::make_shared<HttpUrlRouter::Route>(verb, path, handler);
	routes.push_back(newRoute);
}

int HttpUrlRouter::TouchCount()
{
	count++;
	return count;
}

int HttpUrlRouter::GetCount()
{
	return count;
}
