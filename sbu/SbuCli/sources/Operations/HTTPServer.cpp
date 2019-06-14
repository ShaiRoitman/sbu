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

static auto logger = LoggerFactory::getLogger("Operations.HTTPServer");

class HttpUrlRouter
{
public:
	HttpUrlRouter()
	{
		this->count = 0;
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
		virtual void OnRequest(Verb verb, std::map<string, string> params, HTTPServerRequest& req, HTTPServerResponse& resp) = 0;
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
	MyRequestHandlerFactory()
	{
		router = std::make_shared<HttpUrlRouter>();
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
protected:
	int main(const vector<string> &vars)
	{
		MyRequestHandlerFactory* requestFactory = new MyRequestHandlerFactory();
		Poco::ThreadPool threadPool;
		HTTPServerParams* httpServerParams = new HTTPServerParams();
		ServerSocket serverSocket(9090);

		HTTPServer s(requestFactory, threadPool, serverSocket, httpServerParams);

		s.start();
		cout << endl << "Server started" << endl;

		waitForTerminationRequest();  // wait for CTRL-C or kill

		cout << endl << "Shutting down..." << endl;
		s.stop();

		return Application::EXIT_OK;
	}
};

class HTTPServerOperation : public Operation
{
public:
	HTTPServerOperation() {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		MyServerApp app;
		std::vector<std::string> args;
		args.push_back("HTTPServer");
		retValue =  app.run(args);

		logger->DebugFormat("Operation:[HTTPServer] retValue:[%d]", retValue);
		return retValue;

	}
};


std::shared_ptr<Operation> HTTPServerFactory()
{
	return std::make_shared<HTTPServerOperation>();
}
