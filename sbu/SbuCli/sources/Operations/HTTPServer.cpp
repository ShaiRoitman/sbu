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
#include "httpServer.h"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

static auto myLogger = LoggerFactory::getLogger("Operations.HTTPServer");

class HttpUrlRouter
{
public:
	HttpUrlRouter(boost::program_options::variables_map& params) 
		: vm (params)
	{
		this->count = 0;

		// Returns httpServer::Models::ProgramInformation
		AddRoute(Verb::HTTP_GET, "/info", nullptr);

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

	void HandleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
	{
		auto verb = req.getMethod();
		auto uri = req.getURI();
		httpServer::Models::ProgramInformation info;
		info.hostName = getHostName();
		info.version = "0.9";
		auto d = info.getJson();

		int k = 0;
	}

	enum Verb {
		HTTP_GET,
		HTTP_POST,
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

	void AddRoute(Verb verb, const std::string &path, std::shared_ptr<HttpUrlRouterHandler> handler)
	{
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

		ostream& out = resp.send();
		out << "<h1>Hello world!</h1>"
			<< "<p>Count: " << this->router->TouchCount() << "</p>"
			<< "<p>Host: " << req.getHost() << "</p>"
			<< "<p>Method: " << req.getMethod() << "</p>"
			<< "<p>URI: " << req.getURI() << "</p>";

		this->router->HandleRequest(req, resp);

		out.flush();

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
