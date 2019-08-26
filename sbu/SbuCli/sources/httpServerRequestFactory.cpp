#include "httpServerRequestFactory.h"
#include "httpServerUtils.h"
#include "sbu.h"
#include "factories.h"

#include "Operations.h"
#include "Operations/BackupOperation.h"
#include "Operations/RestoreOperation.h"

using namespace Poco::Net;
using namespace std;

template<typename InputBodyType, typename OutputBodyType>
class ConfigurationBasedHandler : public HttpUrlRouter::HttpUrlRouterHandler
{
protected:
	void AugmentConfig(boost::program_options::variables_map& config, std::shared_ptr<io::swagger::server::model::Configuration> configParam)
	{
		auto params = configParam->toJson();
		auto val = params["Shai"].get<std::string>();
	}

	virtual void OnRequest(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp) override
	{
		std::shared_ptr<InputBodyType> inputBody = std::make_shared<InputBodyType>();
		nlohmann::json inputBodyJson;
		req.stream() >> inputBodyJson;
		inputBody->fromJson(inputBodyJson);
		this->AugmentConfig(config, inputBody->getConfig());

		std::shared_ptr<OutputBodyType> outputBody = std::make_shared<OutputBodyType>();
		this->OnRequestImpl(config, verb, urlPathParams, queryParams, req, resp, inputBody, outputBody);

		ostream& out = resp.send();
		auto output = outputBody->toJson().dump();
		out << output;
		out.flush();
	}

	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<InputBodyType> inputBody,
		std::shared_ptr<OutputBodyType> outputBody) = 0;
};

class HelpInfoHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody, 
	io::swagger::server::model::HelpInformation>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::HelpInformation> outputBody) override
	{
	}
};

class GetInfoHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody,
	io::swagger::server::model::ProgramInformation>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::ProgramInformation> outputBody) override
	{
		outputBody->setHostname(getHostName());
		outputBody->setVersion(g_Version);
	}
};

class GetBackupDefHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody,
	io::swagger::server::model::BackupDefs>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::BackupDefs> outputBody) override
	{
		auto backupdefs = outputBody->getBackupdefs();
		auto RepoDB = getRepository(config);
		RepoDB->ListBackupDefs(
			[&backupdefs](const IRepositoryDB::BackupDef& backupdef)
		{
			auto backupdefJson = CreateBackupDef(backupdef);
			backupdefs.push_back(backupdefJson);
		});
	}
};

class PostBackupDefHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::CreateBackupDef,
	io::swagger::server::model::BackupDef>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::CreateBackupDef> inputBody,
		std::shared_ptr < io::swagger::server::model::BackupDef> outputBody) override
	{
		auto RepoDB = getRepository(config);
		auto backupInfo = RepoDB->AddBackupDef(inputBody->getName(), inputBody->getPath());
		Convert(*backupInfo, outputBody);
	}
};

class GetBackupDefIDHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody,
	io::swagger::server::model::FullBackupDefInfo>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::FullBackupDefInfo> outputBody) override
	{
		Integer id = getIntegerFromString(urlPathParams["Id"]);
		auto RepoDB = getRepository(config);
		auto def = RepoDB->GetBackupDef(id);
		auto backupDef = CreateBackupDef(*def);

		outputBody->setDef(backupDef);

		auto backups = outputBody->getBackups();
		RepoDB->ListBackups(
			def->id,
			[&backups](const IRepositoryDB::BackupInfo& backup)
		{
			auto backupModel = CreateBackup(backup);
			backups.push_back(backupModel);
		}
		);
	}
};

class PostBackupDefBackupHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody,
	io::swagger::server::model::Backup>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::Backup> outputBody) override
	{
		Integer id = getIntegerFromString(urlPathParams["id"]);
		auto RepoDB = getRepository(config);
		auto def = RepoDB->GetBackupDef(id);
		auto backupDef = CreateBackupDef(*def);

		config.insert(std::make_pair("action", boost::program_options::variable_value("Backup", false)));
		config.insert(std::make_pair("name", boost::program_options::variable_value(def->name, false)));

		std::shared_ptr<BackupOperation::Strategy> strategy = std::make_shared< BackupOperation::Strategy>();
		strategy->successFunc = [&outputBody](const IRepositoryDB::BackupInfo& backup)
		{
			outputBody = CreateBackup(backup);
		};

		auto operation = std::make_shared<BackupOperation>(strategy);
		operation->Operate(config);
	}
};

class GetBackupDefBackupIDHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::ConfigurationBody, 
	io::swagger::server::model::BackupInfo>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::ConfigurationBody> inputBody,
		std::shared_ptr < io::swagger::server::model::BackupInfo> outputBody) override
	{
		Integer id = getIntegerFromString(urlPathParams["id"]);
		auto RepoDB = getRepository(config);
		auto def = RepoDB->GetBackupDef(id);
		Integer bid = getIntegerFromString(urlPathParams["bid"]);

		auto outputInfo = outputBody->getInfo();
		Convert(*def, outputInfo->getDef());
	}
};

class PostBackupDefRestoreHandler : public ConfigurationBasedHandler<
	io::swagger::server::model::RestoreOptions,
	io::swagger::server::model::Backup>
{
public:
	virtual void OnRequestImpl(
		boost::program_options::variables_map& config,
		HttpUrlRouter::Verb verb,
		std::map<string, string> urlPathParams,
		std::map<string, string> queryParams,
		HTTPServerRequest & req,
		HTTPServerResponse & resp,
		std::shared_ptr<io::swagger::server::model::RestoreOptions> inputBody,
		std::shared_ptr < io::swagger::server::model::Backup> outputBody) override
	{
		std::shared_ptr<RestoreOperation::Strategy> strategy = std::make_shared<RestoreOperation::Strategy>();
		strategy->altToCopy = [](boost::filesystem::path& destination)
		{
		};
		auto operation = std::make_shared<RestoreOperation>(strategy);
		operation->Operate(config);
	}
};

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

