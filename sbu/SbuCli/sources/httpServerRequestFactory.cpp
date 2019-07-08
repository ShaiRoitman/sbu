#include "httpServerRequestFactory.h"
#include "sbu.h"
#include "factories.h"

#include "..\httpModels\HelpInformation.h"
#include "..\httpModels\ProgramInformation.h"
#include "..\httpModels\BackupDefs.h"
#include "..\httpModels\BackupInfo.h"
#include "..\httpModels\FullBackupDefInfo.h"
#include "..\httpModels\CreateBackupDef.h"

using namespace Poco::Net;
using namespace std;

MyRequestHandler::MyRequestHandler(std::shared_ptr<HttpUrlRouter> router)
{
	this->router = router;
}

void MyRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
	try {
		auto requestHandleStatus = this->router->HandleRequest(req, resp);
		if (requestHandleStatus == HttpUrlRouter::HandleStatus::Success)
		{
			resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
			resp.setContentType("application/json");
		}
		else
		{
			resp.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			resp.setContentType("text/html");
		}
	}
	catch (std::exception ex)
	{
		resp.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
		resp.setContentType("text/html");
		auto& outputResponse = resp.send();
		outputResponse << ex.what() << endl;
	}

	cout << endl
		<< "Response sent for count=" << this->router->GetCount()
		<< " and URI=" << req.getURI() << endl;
}

class HelpInfoHandler : public HttpUrlRouter::HttpUrlRouterHandler
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

class GetInfoHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::ProgramInformation bodyValue;
		bodyValue.setHostname(getHostName());
		bodyValue.setVersion(g_Version);
		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
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
		io::swagger::server::model::BackupDefs bodyValue;

		auto backupdefs = bodyValue.getBackupdefs();
		auto RepoDB = getRepository(config);
		RepoDB->ListBackupDefs(
			[&backupdefs](const IRepositoryDB::BackupDef& backupdef)
		{
			auto backupdefJson = std::make_shared<io::swagger::server::model::BackupDef>();
			backupdefJson->setId(backupdef.id);
			backupdefJson->setName(backupdef.name);
			backupdefJson->setPath(backupdef.rootPath.generic_string());
			backupdefJson->setAdded(get_string_from_time_point(backupdef.added));
			backupdefJson->setHostName(backupdef.hostName);
			backupdefs.push_back(backupdefJson);
		}
		);

		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
		out << output;
		out.flush();
	}
};

class PostBackupDefHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::CreateBackupDef body;
		auto RepoDB = getRepository(config);
		auto backupInfo = RepoDB->AddBackupDef(body.getName(), body.getPath());

		io::swagger::server::model::BackupDef retValue;
		retValue.setId(backupInfo->id);
		retValue.setName(backupInfo->name);
		retValue.setHostName(backupInfo->hostName);
		retValue.setAdded(get_string_from_time_point(backupInfo->added));
		retValue.setPath(backupInfo->rootPath.string());

		ostream& out = resp.send();
		auto output = retValue.toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefIDHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::BackupDef bodyValue;

		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefBackupHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::BackupInfo bodyValue;

		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefBackupIDHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::BackupInfo bodyValue;

		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefRestoreHandler : public HttpUrlRouter::HttpUrlRouterHandler
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
		io::swagger::server::model::Backup bodyValue;

		ostream& out = resp.send();
		auto output = bodyValue.toJson().dump();
		out << output;
		out.flush();
	}
};

MyRequestHandlerFactory::MyRequestHandlerFactory(boost::program_options::variables_map& params)
{
	router = std::make_shared<HttpUrlRouter>(params);

	// Returns httpServer::Models::HelpInformation
	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/help", std::make_shared<HelpInfoHandler>());

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

HTTPRequestHandler* MyRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
	MyRequestHandler* retValue = new MyRequestHandler(router);
	return retValue;
}

