#pragma once

#include "utils.h"
#include "boost/regex.hpp"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>


class HttpUrlRouter
{
public:
	enum Verb {
		HTTP_GET,
		HTTP_POST,
	};

	enum HandleStatus {
		Missing,
		Success,
		Error,
		Busy,
	};

	class HttpUrlRouterHandler {
	public:
		virtual void OnRequest(
			boost::program_options::variables_map& config,
			Verb verb,
			std::map<std::string, std::string> urlPathParams,
			std::map<std::string, std::string> queryParams,
			Poco::Net::HTTPServerRequest& req,
			Poco::Net::HTTPServerResponse& resp) = 0;
	};

	class Route {
	public:
		Route(HttpUrlRouter::Verb verb, const std::string path, std::shared_ptr<HttpUrlRouter::HttpUrlRouterHandler> handler);
		HttpUrlRouter::HandleStatus HandleRequest(
			boost::program_options::variables_map& config,
			Poco::Net::HTTPServerRequest& req,
			Poco::Net::HTTPServerResponse& resp);

		std::string verb;
		const std::string path;
		std::shared_ptr<HttpUrlRouter::HttpUrlRouterHandler> handler;
	};

	HttpUrlRouter(boost::program_options::variables_map& params);
	HttpUrlRouter::HandleStatus HandleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
	void AddRoute(Verb verb, const std::string &path, std::shared_ptr<HttpUrlRouterHandler> handler);
	int TouchCount();
	int GetCount();

	int count;
	boost::program_options::variables_map& vm;
	std::list<std::shared_ptr<Route>> routes;
};
