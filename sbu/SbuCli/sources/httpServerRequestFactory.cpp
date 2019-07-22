#include "httpServerRequestFactory.h"
#include "sbu.h"
#include "factories.h"

#include "..\httpModels\HelpInformation.h"
#include "..\httpModels\ProgramInformation.h"
#include "..\httpModels\BackupDefs.h"
#include "..\httpModels\BackupInfo.h"
#include "..\httpModels\FullBackupDefInfo.h"
#include "..\httpModels\CreateBackupDef.h"
#include "..\httpModels\RestoreOptions.h"

#include "Operations.h"
#include "Operations/BackupOperation.h"

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
		io::swagger::server::model::HelpInformation outputBody;

		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();

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
		io::swagger::server::model::ProgramInformation outputBody;

		outputBody.setHostname(getHostName());
		outputBody.setVersion(g_Version);
		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();
	}
};

class ConfigurationBasedHandler : public HttpUrlRouter::HttpUrlRouterHandler
{
protected:
	void AugmentConfig(boost::program_options::variables_map& config, std::shared_ptr<io::swagger::server::model::Configuration> configParam)
	{
		auto params = configParam->toJson();
		auto val = params["SHia"].get<std::string>();
	}
};

static std::shared_ptr<io::swagger::server::model::BackupDef> CreateBackupDef(const IRepositoryDB::BackupDef& backupdef)
{
	auto retValue = std::make_shared<io::swagger::server::model::BackupDef>();
	retValue->setId(backupdef.id);
	retValue->setName(backupdef.name);
	retValue->setPath(backupdef.rootPath.generic_string());
	retValue->setAdded(get_string_from_time_point(backupdef.added));
	retValue->setHostName(backupdef.hostName);

	return retValue;
}

static std::shared_ptr<io::swagger::server::model::Backup> CreateBackup(const IRepositoryDB::BackupInfo& backup)
{
	auto retValue = std::make_shared<io::swagger::server::model::Backup>();

	retValue->setStatus(backup.status);
	retValue->setId(backup.id);
	retValue->setStarted(get_string_from_time_point(backup.started));
	retValue->setLastStatusUpdate(get_string_from_time_point(backup.lastUpdated));
	retValue->setBackupDefId(backup.backupDefId);

	return retValue;
}

class GetBackupDefHandler : public ConfigurationBasedHandler
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
		auto inputBody = std::make_shared<io::swagger::server::model::Configuration>();
		io::swagger::server::model::BackupDefs outputBody;
		this->AugmentConfig(config, inputBody);

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody->fromJson(inputBodyJson);

		auto backupdefs = outputBody.getBackupdefs();
		auto RepoDB = getRepository(config);
		RepoDB->ListBackupDefs(
			[&backupdefs](const IRepositoryDB::BackupDef& backupdef)
		{
			auto backupdefJson = CreateBackupDef(backupdef);
			backupdefs.push_back(backupdefJson);
		}
		);

		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();
	}
};

class PostBackupDefHandler : public ConfigurationBasedHandler
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
		io::swagger::server::model::CreateBackupDef inputBody;
		std::shared_ptr<io::swagger::server::model::BackupDef> outputBody;

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody.fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody.getConfig());

		auto RepoDB = getRepository(config);
		auto backupInfo = RepoDB->AddBackupDef(inputBody.getName(), inputBody.getPath());
		outputBody = CreateBackupDef(*backupInfo);

		ostream& out = resp.send();
		auto output = outputBody->toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefIDHandler : public ConfigurationBasedHandler
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
		auto inputBody = std::make_shared<io::swagger::server::model::Configuration>();
		io::swagger::server::model::FullBackupDefInfo outputBody;

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody->fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody);

		Integer id = getIntegerFromString(urlPathParams["Id"]);
		auto RepoDB = getRepository(config);
		auto def = RepoDB->GetBackupDef(id);
		auto backupDef = CreateBackupDef(*def);

		outputBody.setDef(backupDef);

		auto backups = outputBody.getBackups();
		RepoDB->ListBackups(def->id,
			[&backups](const IRepositoryDB::BackupInfo& backup)
		{
			auto backupModel = CreateBackup(backup);
			backups.push_back(backupModel);
		}
		);

		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();
	}
};

class PostBackupDefBackupHandler : public ConfigurationBasedHandler
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
		auto inputBody = std::make_shared<io::swagger::server::model::Configuration>();
		std::shared_ptr<io::swagger::server::model::Backup> outputBody;

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody->fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody);

		Integer id = getIntegerFromString(urlPathParams["Id"]);
		auto RepoDB = getRepository(config);
		auto def = RepoDB->GetBackupDef(id);

		config.insert(std::make_pair("action", boost::program_options::variable_value("Backup", false)));
		config.insert(std::make_pair("name", boost::program_options::variable_value(def->name, false)));

		std::shared_ptr<BackupOperation::Strategy> strategy = std::make_shared< BackupOperation::Strategy>();
		strategy->successFunc = [&outputBody](const IRepositoryDB::BackupInfo& backup)
		{
			outputBody = CreateBackup(backup);
		};

		auto operation = std::make_shared<BackupOperation>(strategy);

		operation->Operate(config);

		ostream& out = resp.send();
		auto output = outputBody->toJson().dump();
		out << output;
		out.flush();
	}
};

class GetBackupDefBackupIDHandler : public ConfigurationBasedHandler
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
		auto inputBody = std::make_shared<io::swagger::server::model::Configuration>();
		io::swagger::server::model::BackupInfo outputBody;

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody->fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody);



		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();
	}
};

class PostBackupDefRestoreHandler : public ConfigurationBasedHandler
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
		io::swagger::server::model::RestoreOptions inputBody;
		io::swagger::server::model::Backup outputBody;

		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody.fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody.getConfig());

		ostream& out = resp.send();
		auto output = outputBody.toJson().dump();
		out << output;
		out.flush();
	}
};

MyRequestHandlerFactory::MyRequestHandlerFactory(boost::program_options::variables_map& params)
{
	router = std::make_shared<HttpUrlRouter>(params);

	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/help", std::make_shared<HelpInfoHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/info", std::make_shared<GetInfoHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef", std::make_shared<GetBackupDefHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_POST, "/backupDef", std::make_shared<PostBackupDefHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef/{id}", std::make_shared<GetBackupDefIDHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_POST, "/backupDef/{id}/backup", std::make_shared<PostBackupDefBackupHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_GET, "/backupDef/{id}/backup/{bid}", std::make_shared<GetBackupDefBackupIDHandler>());
	router->AddRoute(HttpUrlRouter::Verb::HTTP_POST, "/backupDef/{id}/restore", std::make_shared<PostBackupDefRestoreHandler>());
}

HTTPRequestHandler* MyRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
	MyRequestHandler* retValue = new MyRequestHandler(router);
	return retValue;
}

