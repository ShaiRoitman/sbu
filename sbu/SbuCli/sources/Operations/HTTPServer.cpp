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
#include "Operations.h"

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
		Busy,
	};

	class HttpUrlRouterHandler {
	public:
		virtual void OnRequest(
			boost::program_options::variables_map& config,
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

		HandleStatus HandleRequest(boost::program_options::variables_map& config, HTTPServerRequest& req, HTTPServerResponse& resp)
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
				std::map<string, string> urlParams;
				std::map<string, string> queryParams;
				this->handler->OnRequest(requestConfig, Verb::HTTP_GET, urlParams, queryParams, req, resp);
			}

			return HandleStatus::Success;
		}

		std::string verb;
		const std::string path;
		std::shared_ptr<HttpUrlRouterHandler> handler;
	};

	HttpUrlRouter(boost::program_options::variables_map& params)
		: vm (params)
	{
		this->count = 0;
	}

	HandleStatus HandleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
	{
		HandleStatus retValue = HandleStatus::Missing;
		list<std::shared_ptr<Route>>::iterator iter;
		for (iter = routes.begin(); iter != routes.end() && retValue == HandleStatus::Missing; ++iter)
		{
			auto currentRoute = *iter;
			if (currentRoute->HandleRequest(this->vm, req, resp) != HandleStatus::Missing)
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
		try {
			auto requestHandleStatus = this->router->HandleRequest(req, resp);
			if (requestHandleStatus == HttpUrlRouter::HandleStatus::Success)
			{
				resp.setStatus(HTTPResponse::HTTP_OK);
				resp.setContentType("application/json");
			}
			else
			{
				resp.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				resp.setContentType("text/html");
			}
		}
		catch (std::exception ex)
		{
			resp.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			resp.setContentType("text/html");
			auto& outputResponse = resp.send();
			outputResponse << ex.what() << endl;

		}

		cout	<< endl
				<< "Response sent for count=" << this->router->GetCount()
				<< " and URI=" << req.getURI() << endl;
	}

private:
	std::shared_ptr<HttpUrlRouter> router;
};

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
	class GetInfoHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
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

	class GetBackupDefHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(
			boost::program_options::variables_map& config,
			HttpUrlRouter::Verb verb,
			std::map<string, string> urlPathParams,
			std::map<string, string> queryParams,
			HTTPServerRequest & req,
			HTTPServerResponse & resp) override
		{

		}
	};

	class PostBackupDefHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
		}
	};

	class GetBackupDefIDHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
		}
	};

	class GetBackupDefBackupHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
		}
	};

	class GetBackupDefBackupIDHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
		}
	};

	class GetBackupDefRestoreHandler : public HttpUrlRouter::HttpUrlRouterHandler
	{
	public:
		virtual void OnRequest(boost::program_options::variables_map& config, HttpUrlRouter::Verb verb, std::map<string, string> urlPathParams, std::map<string, string> queryParams, HTTPServerRequest & req, HTTPServerResponse & resp) override
		{
		}
	};

public:
	MyRequestHandlerFactory(boost::program_options::variables_map& params) 
	{
		router = std::make_shared<HttpUrlRouter>(params);

		// Returns httpServer::Models::ProgramInformation
		router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/info", std::make_shared<GetInfoHandler>());

		// Returns Array of httpServer::Models::BackupDef 
		router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef", std::make_shared<GetBackupDefHandler>());

		// Input httpServer::Models::CreateBackupDef
		// Returns httpServer::Models::BackupDef;
		router->AddRoute(HttpUrlRouter::Verb::HTTP_POST, "/backupDef", std::make_shared<PostBackupDefHandler>());

		// Returns httpServer::Models::BackupDef
		router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef/{id}", std::make_shared<GetBackupDefIDHandler>());

		// Returns httpServer::Models::Backup
		router->AddRoute(HttpUrlRouter::Verb::HTTP_POST, "/backupDef/{id}/backup", std::make_shared<GetBackupDefBackupHandler>());

		// Returns httpServer::Models::FullBackupDefInfo
		router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef/{id}/backup/{bid}", std::make_shared<GetBackupDefBackupIDHandler>());

		// Input httpServer::Models::RestoreOptions
		// Returns httpServer::Models::Backup
		router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef/{id}/restore", std::make_shared<GetBackupDefRestoreHandler>());
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
