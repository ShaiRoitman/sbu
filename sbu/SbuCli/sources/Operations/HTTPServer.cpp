#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include "boost/regex.hpp"
#include "httpServer.h"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

static auto myLogger = LoggerFactory::getLogger("Operations.HTTPServer");

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
	};

	class HttpUrlRouterHandler {
	public:
		virtual void OnRequest(
			Verb verb,
			std::map<string, string> urlPathParams,
			std::map<string, string> queryParams,
			HTTPServerRequest& req,
			HTTPServerResponse& resp) = 0;
	};

	class Route {
	public:
		Route(Verb verb, const std::string path, std::shared_ptr<HttpUrlRouterHandler> handler) :
			path(path), handler(handler)
		{
			switch (verb) {
			case Verb::HTTP_GET:
				this->verb = "GET";
				break;
			case Verb::HTTP_POST:
				this->verb = "POST";
				break;
			}
		}

		HandleStatus HandleRequest(HTTPServerRequest& req, HTTPServerResponse& resp)
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
				std::map<string, string> urlParams;
				std::map<string, string> queryParams;
				this->handler->OnRequest(Verb::HTTP_GET, urlParams, queryParams, req, resp);
			}



			return HandleStatus::Success;
		}

		std::string verb;
		const std::string path;
		std::shared_ptr<HttpUrlRouterHandler> handler;
	};

	class InfoHandler : public HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
			httpServer::Models::ProgramInformation info;
			info.hostName = getHostName();
			info.version = "0.9";
			ostream& out = resp.send();
			auto output = info.getJson();
			out << output;
			out.flush();

		}
	};

	HttpUrlRouter(boost::program_options::variables_map& params)
		: vm (params)
	{
		this->count = 0;

		// Returns httpServer::Models::ProgramInformation
		AddRoute(Verb::HTTP_GET, "/info", std::make_shared<InfoHandler>());

		// Returns Array of httpServer::Models::BackupDef 
		AddRoute(Verb::HTTP_GET, "/backupDef", nullptr);

		// Input httpServer::Models::CreateBackupDef
		// Returns httpServer::Models::BackupDef;
		AddRoute(Verb::HTTP_POST, "/backupDef", nullptr);

		// Returns httpServer::Models::BackupDef
		AddRoute(Verb::HTTP_GET, "/backupDef/{id}", nullptr);

		// Returns httpServer::Models::Backup
		AddRoute(Verb::HTTP_POST, "/backupDef/{id}/backup", nullptr);

		// Returns httpServer::Models::FullBackupDefInfo
		AddRoute(Verb::HTTP_GET, "/backupDef/{id}/backup/{bid}", nullptr);

		// Input httpServer::Models::RestoreOptions
		// Returns httpServer::Models::Backup
		AddRoute(Verb::HTTP_GET, "/backupDef/{id}/restore", nullptr);

	}

	HandleStatus HandleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
	{
		HandleStatus retValue = HandleStatus::Missing;
		list<std::shared_ptr<Route>>::iterator iter;
		for (iter = routes.begin(); iter != routes.end() && retValue == HandleStatus::Missing; ++iter)
		{
			auto currentRoute = *iter;
			if (currentRoute->HandleRequest(req, resp) != HandleStatus::Missing)
				break;
		}

		return retValue;
	}

	void AddRoute(Verb verb, const std::string &path, std::shared_ptr<HttpUrlRouterHandler> handler)
	{
		std::shared_ptr<Route> newRoute = std::make_shared<Route>(verb, path, handler);
		routes.push_back(newRoute);
	}

	int TouchCount() 
	{
		count++;
		return count;
	}

	int GetCount()
	{
		return count;
	}

	int count;
	boost::program_options::variables_map& vm;
	list<std::shared_ptr<Route>> routes;
};

class MyRequestHandler : public HTTPRequestHandler
{
public:
	MyRequestHandler(std::shared_ptr<HttpUrlRouter> router)
	{
		this->router = router;
	}

	virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
	{
		resp.setStatus(HTTPResponse::HTTP_OK);
		resp.setContentType("text/html");

		this->router->HandleRequest(req, resp);

		cout << endl
			<< "Response sent for count=" << this->router->GetCount()
			<< " and URI=" << req.getURI() << endl;
	}

private:
	std::shared_ptr<HttpUrlRouter> router;
};

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	MyRequestHandlerFactory(boost::program_options::variables_map& params) 
	{
		router = std::make_shared<HttpUrlRouter>(params);
	}

	std::shared_ptr<HttpUrlRouter> router;
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &)
	{
		MyRequestHandler* retValue = new MyRequestHandler(router);
		return retValue;
	}

};

class MyServerApp : public ServerApplication
{
public:
	MyServerApp(boost::program_options::variables_map& params) : 
		vm(params)
	{
	}

protected:
	int main(const vector<string> &vars) 
	{
		MyRequestHandlerFactory* requestFactory = new MyRequestHandlerFactory(this->vm);
		Poco::ThreadPool threadPool;
		HTTPServerParams* httpServerParams = new HTTPServerParams();
		int serverPort = vm["HTTPServer.Port"].as<int>();

		ServerSocket serverSocket(serverPort);
		HTTPServer s(requestFactory, threadPool, serverSocket, httpServerParams);

		s.start();
		myLogger->InfoFormat("Server Started Port [%d]", serverPort);
		cout << endl << "Server started [" << return_current_time_and_date() << "] " << endl;

		waitForTerminationRequest();  // wait for CTRL-C or kill

		cout << endl << "Shutting down... [" << return_current_time_and_date() << "] " << endl; 
		myLogger->Info("Server Shutting down");
		s.stop();
		myLogger->Info("Server Down");

		return Application::EXIT_OK;
	}

	boost::program_options::variables_map& vm;
};

class HTTPServerOperation : public Operation
{
public:
	HTTPServerOperation() {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		MyServerApp app(vm);
		std::vector<std::string> args;
		args.push_back("HTTPServer");
		retValue =  app.run(args);

		myLogger->InfoFormat("Operation:[HTTPServer] retValue:[%d]", retValue);
		return retValue;

	}
};

std::shared_ptr<Operation> HTTPServerFactory()
{
	return std::make_shared<HTTPServerOperation>();
}
