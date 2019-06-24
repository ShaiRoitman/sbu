#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "ExitCodes.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>

#include "httpServerRequestFactory.h"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

static auto myLogger = LoggerFactory::getLogger("Operations.HTTPServer");

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
