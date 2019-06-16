#pragma once

#include <iostream>
#include <vector>
#include <Poco/JSON/Object.h>

namespace httpServer
{
	namespace Models 
	{
		typedef long Id;
		typedef long Size;
		typedef std::string Path;
		typedef std::string DateTime;
		typedef std::string Hostname;

		class ProgramInformation 
		{
		public:
			std::string version;
			Hostname hostName;

			std::string getJson()
			{
				Poco::JSON::Object value;
				value.set("version", this->version);
				value.set("hostname", this->hostName);
				
				std::ostringstream stream;
				value.stringify(stream);
				auto str = stream.str();
				return str;
			}
		};

		class CreateBackupDef 
		{
		public:
			std::string name;
			Path path;
		};

		class BackupDef 
		{
		public:
			Id id;
			std::string name;
			Path path;
			DateTime added;
			Hostname hostname;
		};

		class BackupDefs
		{
		public:
			std::vector<BackupDef> backupdefs;
		};

		class Backup
		{
			Id id;
			Id backupDefId;
			std::string status;
			DateTime started;
			DateTime lastStatusUpdate;
		};

		class FullBackupDefInfo
		{
		public:
			BackupDef def;
			std::vector<Backup> backups;
		};


		class File
		{
		public:
			Id id;
			std::string status;
			std::string type;
			Path path;
			Size size;
			DateTime created;
			DateTime modified;
			DateTime accessed;
			std::string digestType;
			std::string digestValue;
			std::string fileHandle;
		};

		class BackupInfo
		{
		public:
			FullBackupDefInfo info;
			std::vector<File> files;
		};


		class RestoreOptions 
		{
		public:
			Id id;
			DateTime date;
		};
	}
}