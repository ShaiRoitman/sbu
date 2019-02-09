#pragma once

#include <string>
#include "boost/filesystem.hpp"

typedef std::string Name;
typedef std::string DateTime;
typedef boost::filesystem::path Path;

namespace Sbu
{
	namespace Commands
	{
		class CreateBackupDefCommand
		{
		public:
			Name name;
			Path rootPath;
		};

		class ListBackupDefCommand
		{
		};

		class BackupCommand
		{
		public:
			Name backupDefName;
		};

		class ListBackupsCommand
		{
			Name backupDefName;
		};

		class RestoreCommand
		{
		public:
			enum class RestoreOptions
			{
				OVERWRITE = 1 << 0,
				COPY = 1 << 1,
				NO_OP = 1 << 2,
			};

			Name backupDefName;
			Path destinationPath;
			RestoreOptions restoreOptions;
			DateTime snapshotTimePoint;
		};
	}
}