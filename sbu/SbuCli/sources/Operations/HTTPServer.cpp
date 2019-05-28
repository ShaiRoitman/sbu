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

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

static auto logger = LoggerFactory::getLogger("Operations.HTTPServer");

class MyRequestHandler : public HTTPRequestHandler
{
public:
	virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
	{
		resp.setStatus(HTTPResponse::HTTP_OK);
		resp.setContentType("text/html");

		ostream& out = resp.send();
		out << "<h1>Hello world!</h1>"
			<< "<p>Count: " << ++count << "</p>"
			<< "<p>Host: " << req.getHost() << "</p>"
			<< "<p>Method: " << req.getMethod() << "</p>"
			<< "<p>URI: " << req.getURI() << "</p>";
		out.flush();

		cout << endl
			<< "Response sent for count=" << count
			<< " and URI=" << req.getURI() << endl;
	}

private:
	static int count;
};

int MyRequestHandler::count = 0;

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &)
	{
		return new MyRequestHandler;
	}
};

class MyServerApp : public ServerApplication
{
protected:
	int main(const vector<string> &vars)
	{
		Poco::ThreadPool threadPool;
		HTTPServer s(new MyRequestHandlerFactory, threadPool, ServerSocket(9090), new HTTPServerParams);

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
